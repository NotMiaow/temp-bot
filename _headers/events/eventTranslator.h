#ifndef	EVENT_TRANSLATOR_H__
#define EVENT_TRANSLATOR_H__

#include <boost/algorithm/string.hpp>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "basicLib.h"

#include "event.h"
#include "eventLanguage.h"

static Event* CreateErrorEvent(std::string message, std::string channelId, EOffender offender, EEventType eventType, EErrorType errorType)
{
	Event* e = new ErrorEvent(message, channelId, offender, eventType, errorType);
	return e;
}

static Event* CreateEmptyEvent()
{
	Event* e = new EmptyEvent();
	return e;
}

static Event* CreateShutdownEvent()
{
	Event* e = new ShutdownEvent();
	return e;
}

static Event* CreateNewChannelEvent(bool fromAPI, std::string command, std::vector<std::string> parameters, std::string channelId, std::string guildId)
{
	UserGroupComponent userGroup;
	userGroup.name = parameters[0];
	userGroup.parentId = parameters[1];
	userGroup.id = parameters[2];

	if(!ToInt(parameters[3], userGroup.position))
		return CreateErrorEvent("Parameter 4 (channel position) of " + command + " must be a integral number.", channelId, EUser, ENewChannelEvent, EWrongParameterType);

	Event* e = new NewChannelEvent(userGroup);
	return e;
}

static Event* CreateCreateChannelEvent(bool fromAPI, std::string command, std::vector<std::string> parameters, std::string channelId, std::string guildId)
{
    std::string method = "POST";
    std::string type = "/guilds/" + guildId + "/channels";
	UserGroupComponent userGroup;
	userGroup.name = parameters[0];
	userGroup.parentId = parameters[2];

	if(!ToInt(parameters[1], userGroup.userLimit))
		return CreateErrorEvent("Parameter 2 (user limit) of " + command + " must be a integral number.", channelId, EUser, ECreateVoiceChannel, EWrongParameterType);
	if(!ToInt(parameters[3], userGroup.position))
		return CreateErrorEvent("Parameter 4 (channel position) of " + command + " must be a integral number.", channelId, EUser, ECreateVoiceChannel, EWrongParameterType);

	Event *e = new CreateVoiceChannelEvent(fromAPI, method, type, channelId, guildId, userGroup);
	return e;
}

static Event* CreateUpdateChannelEvent(bool fromAPI, std::string command, std::vector<std::string> parameters, std::string channelId, std::string guildId)
{
    std::string method = "PATCH";
    std::string type = "/channels/";
	UserGroupComponent userGroup;
	userGroup.id = parameters[0];
	type += parameters[0];

	if(!ToInt(parameters[1], userGroup.position))
		return CreateErrorEvent("Parameter 2 (new channel position) of " + command + " must be a integral number.", channelId, EUser, ECreateVoiceChannel, EWrongParameterType);

	Event* e = new UpdateVoiceChannelEvent(fromAPI, method, type, channelId, guildId, userGroup);
	return e;
}

static Event* CreateDeleteChannelEvent(bool fromAPI, std::string command, std::vector<std::string> parameters, std::string channelId, std::string guildId)
{
    std::string method = "DELETE";
    std::string type = "/channels/";
	UserGroupComponent userGroup;
	userGroup.id = parameters[0];
	type += parameters[0];

	Event* e = new DeleteChannelEvent(fromAPI, method, type, channelId, guildId, userGroup);
	return e;
}

static Event* CreateEvent(bool fromAPI, std::string command, std::string content, std::string channelId, std::string guildId)
{
	std::vector<std::string> parameters = Split(content, ' ');

	auto it = optionEventTypes.find(command);
	if(it != optionEventTypes.end())
	{
		switch (it->second)
		{
		case EEmpty:
			return CreateEmptyEvent();
		case EShutdown:
			return CreateShutdownEvent();
		case ENewChannelEvent:
			return CreateNewChannelEvent(fromAPI, command, parameters, channelId, guildId);
		case ECreateVoiceChannel:
			return CreateCreateChannelEvent(fromAPI, command, parameters, channelId, guildId);
		case EUpdateVoiceChannel:
			return CreateUpdateChannelEvent(fromAPI, command, parameters, channelId, guildId);
		default:
			return CreateErrorEvent("This command does not exist.", channelId, EUser, ECreateEvent, EWrongEventType);
		}
	}
	return CreateErrorEvent("This command does not exist.", channelId, EUser, ECreateEvent, EWrongEventType);
}

#endif