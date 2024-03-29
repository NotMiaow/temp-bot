#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <string>
#include <mutex>
#include <condition_variable>

#include <boost/asio.hpp>
#include <discordpp/bot.hh>
#include <discordpp/plugin-overload.hh>
#include <discordpp/rest-beast.hh>
#include <discordpp/websocket-beast.hh>
#include <discordpp/plugin-responder.hh>

#include "shared_queue.h"
#include "event.h"
#include "miaBot.h"

namespace asio = boost::asio;
using json = nlohmann::json;
namespace dpp = discordpp;
using DppBot = dpp::PluginResponder<dpp::PluginOverload<dpp::WebsocketBeast<dpp::RestBeast<dpp::Bot>>>>;

std::istream &safeGetline(std::istream &is, std::string &t);
void filter(std::string &target, const std::string &pattern);

auto aioc = std::make_shared<asio::io_context>();

// Mia related
void Loop(std::shared_ptr<DppBot>& bot);
double currentTime;
float deltaTime;
std::chrono::time_point<std::chrono::high_resolution_clock> curTime;
std::chrono::time_point<std::chrono::high_resolution_clock> prevTime;
std::thread miaLoop;
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
std::atomic<bool> waitForBot;
std::promise<void> exitSignal;
std::shared_future<void> futureObj;

int main()
{
	alive = false;
	do
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
		bot->prefix = "/";
		bot->handlers.insert({
				"MESSAGE_CREATE",
				[&bot, &self](json msg) {
					if(msg["content"].get<std::string>().substr(0,1) == bot->prefix)
					{
						std::string content = msg["content"].get<std::string>().substr(1);
						int pos = content.find(' ');
						std::string command = content.substr(0,pos++);
						content = pos < content.length() - 1 ? content.substr(pos) : "";
						if(msg["author"]["id"].get<std::string>() == "99681408724766720" || command == "join-queue")
						{
							EventInfo info(false, "", "", { }, msg["author"]["id"],	msg["channel_id"], msg["guild_id"].is_null() ? "" : msg["guild_id"]);
							mia->QueueCommand(info,	command, content);
						}
					}
				}});
		bot->handlers.insert({
				"CHANNEL_CREATE",
				[&bot, &self](json msg) {
					std::string command = "new-group";
					if(!msg["guild_id"].is_null())
					{
						json empty = { };
						EventInfo info(true, "", "", empty, "", "", "");
						std::string content = msg["name"].get<std::string>() + " " +
							(msg["parent_id"].is_null() ? "" : msg["parent_id"].get<std::string>()) + " " +
							msg["id"].get<std::string>() + " " +
							std::to_string(msg["position"].get<int>()) + " " +
							std::to_string(msg["type"].get<int>());
						mia->QueueCommand(info, command, content);
					}
				}});
		bot->handlers.insert({
				"CHANNEL_DELETE",
				[&bot, &self](json msg) {
					json empty = { };
					EventInfo info(true, "", "", empty, "", "", "");
					mia->QueueCommand(info, "empty", "");
				}});
		bot->handlers.insert({
				"CHANNEL_UPDATE",
				[&bot, &self](json msg) {
					json empty = { };
					EventInfo info(true, "", "", empty, "", "", "");
					mia->QueueCommand(info, "empty", "");
				}});
		bot->handlers.insert({
				"GUILD_MEMBERS",
				[&bot, &self](json msg) {
					json empty = { };
					EventInfo info(true, "", "", empty, "", "", "");
					mia->QueueCommand(info, "empty", "");
				}});
		bot->handlers.insert({
				"GUILD_MEMBER_UPDATE",
				[&bot, &self](json msg) {
					json empty = { };
					EventInfo info(true, "", "", empty, "", "", "");
					mia->QueueCommand(info, "empty", "");
				}});

		if(alive == false)
		{
			// Launch MiaBot
			curTime = std::chrono::high_resolution_clock::now();
			mia = new MiaBot(eventQueue, robotQueue, humanQueue);
			// Prepare threads
			alive = true;
			waitForBot = false;
			futureObj = exitSignal.get_future();
			// Fire threads
			terminateThread = std::thread(&WaitForTerminate);
			miaLoop = std::thread(&Loop, std::ref(bot));
		}

		try
		{
			bot->initBot(6, token, aioc);
			bot->run();
			waitForBot = true;
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
			waitForBot = true;
			std::ofstream errorLogFile;
			errorLogFile.open ("example.txt", std::ios::app);
			errorLogFile << "\n" << e.what() << "\n";
			errorLogFile.close();
			std::this_thread::sleep_for(std::chrono::milliseconds(6000));
		}
	} while(alive);

	// Wait for clean termination
	miaLoop.join();
	terminateThread.join();
	delete mia;
	return 0;
}

void Loop(std::shared_ptr<DppBot>& bot) {
	bool delayNextAPIRequest = false;
	float waitForBotTimer = 0.0f;
	float delayTimer = 0.0f;
	float waitTimer = 0.0f;
	float waitForResponse = 0.0f;

	while(alive == true || waitForResponse > 0.0f)
	{
		prevTime = curTime;
		curTime = std::chrono::high_resolution_clock::now();
		deltaTime = (float)((std::chrono::duration<double>)(curTime - prevTime)).count();
		currentTime += deltaTime;

		// If bot crashed or stopped for whatever reason,
		// Give it time to restart and then resume.
		if(waitForBot)
		{
			waitForBotTimer += deltaTime;
			if(waitForBotTimer > 10.0f)
			{
				waitForBotTimer = 0.0f;
				waitForBot = false;
			}
		}
		else
		{
			// ECS loop
			mia->Loop(deltaTime);

			// *****Discord API has a limit of 2 requests per second*****
			// Delay request in order not to spam the API
			// Set to 550 milliseconds to be safe
			if(delayNextAPIRequest)
			{
				delayTimer += deltaTime;
				if(delayTimer > 0.550f)
				{
					delayTimer = 0.0f;
					delayNextAPIRequest = false;
				}
			}
			// Wait for API response to a previous request
			else if(eventQueue.size() < 1 && waitForResponse > 0.0f)
			{
				waitForResponse += -deltaTime;
				waitTimer += deltaTime;
				if(waitTimer > 0.5f)
				{
					waitTimer = 0.0f;
					std::cout << "Waiting for API ..." << std::endl;
				}
				if(waitForResponse <= 0.0f)
					std::cout << "Did not catch API response, if any." << std::endl;
			}
			// Handle new event
			else
			{
				waitForResponse = 0;
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
					if(event != 0 && mia->HandleEvent(event) && !event->info.fromAPI && !event->ReadOnly())
					{
						std::cout << "send : " << event->ToDebuggable() << std::endl;
						delayNextAPIRequest = true;
						switch(event->GetType())
						{
							case EShutdown:
								Stop();
								break;
							case EError:
								bot->call("POST", "/channels/" + event->info.channelId + "/messages", json({{"content", ((ErrorEvent*)event)->message }}));
								break;
							case ESendMessage:
								bot->call("POST", "/channels/" + event->info.channelId + "/messages", json({{"content", ((SendMessageEvent*)event)->message }}));
								break;
							default :
//								std::cout << event->info.method << event->info.type << event->info.content << std::endl;
								bot->call(event->info.method, event->info.type, event->info.content);
								break;
						}
						if(event->waitForResponse)
							waitForResponse = 2.0f;
					}

					delete event;
					eventQueue.pop_front();
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(15));
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