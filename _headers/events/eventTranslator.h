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
	std::string seeSignature = "\nSee signature : \"" + command + " (name) (parent_id) (id) (position)\"";

	if(parameters.size() != 5)
		return CreateErrorEvent(
			"Wrong parameter amount." + seeSignature,
			channelId, EUser, ECreateChannel, EWrongParemeterAmount
		);

	UserGroupComponent userGroup;
	userGroup.name = parameters[0];
	userGroup.parentId = parameters[1];
	userGroup.id = parameters[2];

	if(!ToInt(parameters[3], userGroup.position))
		return CreateErrorEvent("Parameter 4 (position) of " + command + " must be an integral number.", channelId, EUser, ENewChannelEvent, EWrongParameterType);

	if(!ToInt(parameters[4], userGroup.type))
		return CreateErrorEvent("Parameter 5 (type) of " + command + " must be an integral number.", channelId, EUser, ENewChannelEvent, EWrongParameterType);

	Event* e = new NewChannelEvent(userGroup);
	return e;
}

static Event* CreateCreateChannelEvent(bool fromAPI, std::string command, std::vector<std::string> parameters, std::string channelId, std::string guildId)
{
	std::string seeSignature = "\nSee signature : \"" + command + " (name) (type(0=text)(2=voice) (parent_id) (position) (user_limit?)\"";

	if(parameters.size() < 4 || parameters.size() > 5)
		return CreateErrorEvent(
			"Wrong parameter amount." + seeSignature,
			channelId, EUser, ECreateChannel, EWrongParemeterAmount
		);

    std::string method = "POST";
    std::string type = "/guilds/" + guildId + "/channels";
	UserGroupComponent userGroup;

	if(!ToInt(parameters[1], userGroup.type))
		return CreateErrorEvent(
			"Parameter 2 (type) of " + command + " must be an integral number." + seeSignature,
			channelId, EUser, ECreateChannel, EWrongParameterType
		);

	if(userGroup.type == 0)
	{
		if(parameters.size() != 4)
			return CreateErrorEvent(
				"Wrong parameter amount. Command : \"" + command + "\" with parameter \"type=0\" or \"type=4\" can not have a \"user_limit\" parameter." + seeSignature,
				channelId, EUser, ECreateChannel, EWrongParemeterAmount
			);
	}
	else if(userGroup.type == 2)
	{
		if(parameters.size() != 5)
			return CreateErrorEvent(
				"Wrong parameter amount. Command : \"" + command + "\" with parameter \"type=2\" must have a \"user_limit\" parameter." + seeSignature,
				channelId, EUser, ECreateChannel, EWrongParemeterAmount
			);
		if(!ToInt(parameters[4], userGroup.userLimit))
			return CreateErrorEvent("Parameter 3 (user limit) of " + command + " must be an integral number.", channelId, EUser, ECreateChannel, EWrongParameterType);
	}

	if(!ToInt(parameters[3], userGroup.position))
		return CreateErrorEvent("Parameter 5 (channel position) of " + command + " must be an integral number.", channelId, EUser, ECreateChannel, EWrongParameterType);

	userGroup.name = parameters[0];
	userGroup.parentId = parameters[2];

	Event *e = new CreateChannelEvent(fromAPI, method, type, channelId, guildId, userGroup);
	return e;
}

static Event* CreateUpdateChannelEvent(bool fromAPI, std::string command, std::vector<std::string> parameters, std::string channelId, std::string guildId)
{
	std::string seeSignature = "\nSee signature : \"" + command + " (id) (position)\"";

	if(parameters.size() != 2)
		return CreateErrorEvent(
			"Wrong parameter amount." + seeSignature,
			channelId, EUser, ECreateChannel, EWrongParemeterAmount
		);

    std::string method = "PATCH";
    std::string type = "/channels/";
	UserGroupComponent userGroup;
	userGroup.id = parameters[0];
	type += parameters[0];

	if(!ToInt(parameters[1], userGroup.position))
		return CreateErrorEvent("Parameter 2 (new channel position) of " + command + " must be an integral number.", channelId, EUser, EUpdateChannel, EWrongParameterType);

	Event* e = new UpdateVoiceChannelEvent(fromAPI, method, type, channelId, guildId, userGroup);
	return e;
}

static Event* CreateDeleteChannelEvent(bool fromAPI, std::string command, std::vector<std::string> parameters, std::string channelId, std::string guildId)
{
	std::string seeSignature = "\nSee signature : \"" + command + " (id)\"";

	if(parameters.size() != 1)
		return CreateErrorEvent(
			"Wrong parameter amount." + seeSignature,
			channelId, EUser, ECreateChannel, EWrongParemeterAmount
		);

    std::string method = "DELETE";
    std::string type = "/channels/";
	UserGroupComponent userGroup;
	userGroup.id = parameters[0];
	type += parameters[0];

	Event* e = new DeleteChannelEvent(fromAPI, method, type, channelId, guildId, userGroup);
	return e;
}

static Event* CreateMoveChannelEvent(bool fromAPI, std::string command, std::vector<std::string> parameters, std::string channelId, std::string guildId)
{
	std::string seeSignature = "\nSee signature : \"" + command + " (id) (position) (parent_id?)\"";

	if(parameters.size() < 2 || parameters.size() > 3)
		return CreateErrorEvent(
			"Wrong parameter amount." + seeSignature,
			channelId, EUser, ECreateChannel, EWrongParemeterAmount
		);

	UserGroupComponent userGroup;
	userGroup.id = parameters[0];

	if(parameters.size() == 3)
		userGroup.parentId = parameters[2];

	if(!ToInt(parameters[1], userGroup.position))
		return CreateErrorEvent("Parameter 2 (new channel position) of " + command + " must be an integral number.", channelId, EUser, EUpdateChannel, EWrongParameterType);

	Event* e = new MoveChannelEvent(fromAPI, channelId, guildId, userGroup);
	return e;
}

static Event* CreateCreateCategoryEvent(bool fromAPI, std::string command, std::vector<std::string> parameters, std::string channelId, std::string guildId)
{
	std::string seeSignature = "\nSee signature : \"" + command + " (name) (position) \"";

	if(parameters.size() < 2)
		return CreateErrorEvent(
			"Wrong parameter amount." + seeSignature,
			channelId, EUser, ECreateChannel, EWrongParemeterAmount
		);

    std::string method = "POST";
    std::string type = "/guilds/" + guildId + "/channels";
	UserGroupComponent userGroup;
	userGroup.type = 4;

	if(!ToInt(parameters[1], userGroup.position))
		return CreateErrorEvent("Parameter 5 (channel position) of " + command + " must be an integral number.", channelId, EUser, ECreateChannel, EWrongParameterType);

	userGroup.name = parameters[0];

	Event *e = new CreateCategoryEvent(fromAPI, method, type, channelId, guildId, userGroup);
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
		case ECreateChannel:
			return CreateCreateChannelEvent(fromAPI, command, parameters, channelId, guildId);
		case EUpdateChannel:
			return CreateUpdateChannelEvent(fromAPI, command, parameters, channelId, guildId);
		case EDeleteChannel:
			return CreateDeleteChannelEvent(fromAPI, command, parameters, channelId, guildId);
		case EMoveChannel:
			return CreateMoveChannelEvent(fromAPI, command, parameters, channelId, guildId);
		case ECreateCategory:
			return CreateCreateCategoryEvent(fromAPI, command, parameters, channelId, guildId);
		default:
			return CreateErrorEvent("This command does not exist.", channelId, EUser, ECreateEvent, EWrongEventType);
		}
	}
	return CreateErrorEvent("This command does not exist.", channelId, EUser, ECreateEvent, EWrongEventType);
}

#endif