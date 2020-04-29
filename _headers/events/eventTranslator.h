#ifndef	EVENT_TRANSLATOR_H__
#define EVENT_TRANSLATOR_H__

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "basicLib.h"

#include "event.h"
#include "eventLanguage.h"

static Event* CreateErrorEvent(EEventType aType, EErrorType eType)
{
	Event* e = new ErrorEvent(aType, eType);
	return e;
}

static Event* CreateShutdownEvent(std::string guildId)
{
	Event* e = new ShutdownEvent(guildId);
	return e;
}

static Event* CreateCreateChannelEvent(std::string message, std::string guildId)
{
    std::string method = "POST";
    std::string type = "/guilds/" + guildId + "/channels";
    nlohmann::json content = {
        { "name", message },
        { "type", 2 }
    };

	Event *e = new CreateChannelEvent(method, type, guildId, content);
	return e;
}

static Event* CreateEvent(std::string command, std::string content, std::string guildId)
{
	auto it = optionEventTypes.find(command);
	if(it != optionEventTypes.end())
	{
		switch (it->second)
		{
		case EShutdown:
			return CreateShutdownEvent(guildId);
		case ECreateVoiceChannel:
			return CreateCreateChannelEvent(content, guildId);
		default:
			return CreateErrorEvent(ECreateEvent, EWrongEventType);
		}
	}
	return CreateErrorEvent(ECreateEvent, EWrongEventType);
}

#endif