#include <iostream>
#include <fstream>
#include <thread>

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

void Loop(std::shared_ptr<DppBot> bot, SharedQueue<Event*>& eventQueue);
void Stop();
void WaitForTerminate();
SharedQueue<Event*> eventQueue;
MiaBot* mia = nullptr;
std::thread miaLoop;

std::atomic<bool> alive;
std::promise<void> exitSignal;
std::shared_future<void> futureObj;
std::thread terminateThread;

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
		{"MESSAGE_CREATE",
		 [&bot, &self](json msg) {
			if(msg["content"].get<std::string>().substr(0,1) == bot->prefix)
			{
				std::string content = msg["content"].get<std::string>().substr(1);
				int pos = content.find(' ');
				std::string command = content.substr(0,pos++);
				mia->HandleCommand(command, pos < content.length() - 1 ? content.substr(pos) : "", msg["guild_id"].get<std::string>());
			}
		 }});

	bot->initBot(6, token, aioc);

	// Allow clean termination
	alive = true;
	futureObj = exitSignal.get_future();
	terminateThread = std::thread(&WaitForTerminate);

	// Launch MiaBot
	mia = new MiaBot(eventQueue);
	miaLoop = std::thread(&Loop, bot, std::ref(eventQueue));

	bot->run();

	// Wait for clean termination
	miaLoop.join();
	terminateThread.join();
	delete mia;

	return 0;
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

void Loop(std::shared_ptr<DppBot> bot, SharedQueue<Event*>& eventQueue)
{
	while(alive)
	{
		mia->Loop();
		while (eventQueue.size())
		{
			Event* event = eventQueue.front();
			std::cout << event->ToDebuggable() << std::endl;
			if(event != 0)
			{
				switch(event->GetType())
				{
					case EShutdown:
						Stop();
						break;
					case EError:
						std::cout << "wutface" << std::endl;
//						LogError();
						break;
					default :
						bot->call(event->method, event->type, event->content);
						break;
				}
			}
			delete event;
			eventQueue.pop_front();
		}
	}
}

void Stop()
{
	if(futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout)
		exitSignal.set_value();
    alive = false;

	aioc->stop();
}

void WaitForTerminate()
{
    while(alive && futureObj.wait_for(std::chrono::milliseconds(1)) == std::future_status::timeout);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
}
