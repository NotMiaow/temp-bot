#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <string>
#include <mutex>
#include <condition_variable>

#include "shared_queue.h"
#include "event.h"
#include "miaBot.h"

#include <boost/asio.hpp>
#include <discordpp/bot.hh>
#include <discordpp/plugin-overload.hh>
#include <discordpp/rest-beast.hh>
#include <discordpp/websocket-beast.hh>
#include <discordpp/plugin-responder.hh>

namespace asio = boost::asio;
using json = nlohmann::json;
namespace dpp = discordpp;
using DppBot = dpp::PluginResponder<dpp::PluginOverload<dpp::WebsocketBeast<dpp::RestBeast<dpp::Bot>>>>;

std::istream &safeGetline(std::istream &is, std::string &t);
void filter(std::string &target, const std::string &pattern);

auto aioc = std::make_shared<asio::io_context>();

// Mia related
void Loop();
void SendQueuedRequests(std::shared_ptr<DppBot> bot);
std::thread miaLoop;
std::thread miaRequests;
MiaBot* mia = nullptr;
SharedQueue<Event*> eventQueue;
SharedQueue<Event*> robotQueue;
SharedQueue<Event*> humanQueue;
std::mutex mutex;
std::condition_variable cond;

// Shutdown related
void Stop();
void WaitForTerminate();
std::thread terminateThread;
std::atomic<bool> alive;
std::promise<void> exitSignal;
std::shared_future<void> futureObj;

void SendError();

int main()
{
	std::cout << "Starting Mia...\n\n";

	std::string token;
	{
		std::ifstream tokenFile("token.dat");
		if (!tokenFile)
		{
			std::cerr << "CRITICAL: There is no valid way for MiaBot to obtain a token! " << std::endl;
			exit(1);
		}
		safeGetline(tokenFile, token);
		tokenFile.close();
	}

	std::shared_ptr<DppBot> bot = std::make_shared<DppBot>();
	bot->debugUnhandled = false;

	json self;
	bot->prefix = "~";
	bot->handlers.insert(
		{
			"MESSAGE_CREATE",
			[&bot, &self](json msg) {
				if(msg["content"].get<std::string>().substr(0,1) == bot->prefix)
				{
					std::string content = msg["content"].get<std::string>().substr(1);
					int pos = content.find(' ');
					std::string command = content.substr(0,pos++);
					mia->QueueCommand(
						false,
						command,
						pos < content.length() - 1 ? content.substr(pos) : "",
						msg["channel_id"],
						msg["guild_id"]
					);
				}
			}}
	);
	bot->handlers.insert( 
		{
			"CHANNEL_CREATE",
			[&bot, &self](json msg) {
				mia->QueueCommand(
					true,
					"new-channel",
					msg["name"].get<std::string>() + " " +
					(msg["parent_id"].is_null() ? "" : msg["parent_id"].get<std::string>()) + " " +
					msg["id"].get<std::string>() + " " +
					std::to_string(msg["position"].get<int>()) + " " +
					std::to_string(msg["type"].get<int>()),
					"",
					""
				);
			}}
	);
	bot->handlers.insert(
		{
			"CHANNEL_UPDATE",
			[&bot, &self](json msg) {
				mia->QueueCommand(
					true,
					"EmptyEvent",
					"",
					"",
					""
				);
			}
		}
	);

	bot->initBot(6, token, aioc);

	// Allow clean termination
	alive = true;
	futureObj = exitSignal.get_future();
	terminateThread = std::thread(&WaitForTerminate);

	// Launch MiaBot
	mia = new MiaBot(eventQueue, robotQueue, humanQueue);
	miaLoop = std::thread(&Loop);
	miaRequests = std::thread(&SendQueuedRequests, bot);

	// Run bot
	bot->run();

	// Wait for clean termination
	miaLoop.join();
	miaRequests.join();
	terminateThread.join();
	delete mia;

	return 0;
}

void Loop() 
{
	while(alive)
	{
		mia->Loop();
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}

void SendQueuedRequests(std::shared_ptr<DppBot> bot) {
	while(alive)
	{
		if(!eventQueue.size())
		{
			if(robotQueue.size())
			{
				eventQueue.push_back(robotQueue.front());
				robotQueue.pop_front();
			}
			else if(humanQueue.size())
			{
				eventQueue.push_back(humanQueue.front());
				humanQueue.pop_front();
			}
			else
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}

		if (eventQueue.size())
		{
			Event* event = eventQueue.front();
			if(event != 0 && mia->HandleEvent(event) && !event->fromAPI && !event->ReadOnly())
			{
				std::cout << "send : " << event->ToDebuggable() << std::endl;
				switch(event->GetType())
				{
					case EShutdown:
						Stop();
						break;
					case EError:
						bot->call("POST", "/channels/" + event->channelId + "/messages", json({{"content", ((ErrorEvent*)event)->message }}));
						break;
					default :
						bot->call(event->method, event->type, event->content);
						break;
				}
			}

			if(!event->fromAPI && event->WaitForResponse())
			{
				while(eventQueue.size() > 1)
				{
					std::cout << "Waiting for API ..." << std::endl;
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
			}			

			delete event;
			eventQueue.pop_front();

			std::this_thread::sleep_for(std::chrono::milliseconds(1150));
		}
	}
}

void Stop()
{
	if(futureObj.wait_for(std::chrono::milliseconds(1000)) == std::future_status::timeout)
		exitSignal.set_value();
    alive = false;

	aioc->stop();
}

void WaitForTerminate()
{
    while(alive && futureObj.wait_for(std::chrono::milliseconds(1000)) == std::future_status::timeout);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

std::istream &safeGetline(std::istream &is, std::string &t)
{
	t.clear();
	std::istream::sentry se(is, true);
	std::streambuf *sb = is.rdbuf();

	for (;;)
	{
		int c = sb->sbumpc();
		switch (c)
		{
		case '\n':
			return is;
		case '\r':
			if (sb->sgetc() == '\n')
			{
				sb->sbumpc();
			}
			return is;
		case std::streambuf::traits_type::eof():
			// Also handle the case when the last line has no line ending
			if (t.empty())
			{
				is.setstate(std::ios::eofbit);
			}
			return is;
		default:
			t += (char)c;
		}
	}
}

void filter(std::string &target, const std::string &pattern)
{
	while (target.find(pattern) != std::string::npos)
	{
		target = target.substr(0, target.find(pattern)) +
				 target.substr(target.find(pattern) + (pattern).size());
	}
}
