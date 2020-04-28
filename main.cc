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

void Loop(std::shared_ptr<DppBot> bot, SharedQueue<Event*>& eventQueue);
SharedQueue<Event*> eventQueue;
MiaBot* mia = nullptr;

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
	bot->debugUnhandled = false;									// Don't complain about unhandled events

	json self;
	bot->prefix = "~";
	bot->handlers.insert(
		{"MESSAGE_CREATE",
		 [&bot, &self](json msg) {
			if(msg["content"].get<std::string>().substr(0,1) == bot->prefix)
			{
				std::string content = msg["content"].get<std::string>().substr(1);
				mia->NewMessage(content, msg["guild_id"].get<std::string>());
			}
		 }});

	auto aioc = std::make_shared<asio::io_context>();

	// Set the bot up
	bot->initBot(6, token, aioc);

	mia = new MiaBot(eventQueue);
	std::thread miaLoop = std::thread(&Loop, bot, std::ref(eventQueue));

	// Run the bot!
	bot->run();

	miaLoop.join();
	delete mia;

	return 0;
}

std::istream &safeGetline(std::istream &is, std::string &t)
{
	t.clear();

	// The characters in the stream are read one-by-one using a std::streambuf.
	// That is faster than reading them one-by-one using the std::istream.
	// Code that uses streambuf this way must be guarded by a sentry object.
	// The sentry object performs various tasks,
	// such as thread synchronization and updating the stream state.

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
	while(true)
	{
		mia->Loop();
		while (eventQueue.size())
		{
			Event* message = eventQueue.front();
//			std::cout << message->ToDebuggable() << std::endl;
			if(message != 0)
			{
//				std::cout << message->method << message->type << message->content << std::endl;
				bot->call(message->method, message->type, message->content);
			}
			delete message;
			eventQueue.pop_front();
		}
	}
}
