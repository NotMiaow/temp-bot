#ifndef	EVENT_TRANSLATOR_H__
#define EVENT_TRANSLATOR_H__

#include <boost/algorithm/string.hpp>
#include <string>
#include <vector>
#include <ctype.h>
#include <sstream>

#include <nlohmann/json.hpp>

#include "basicLib.h"

#include "lobbyComponent.h"
#include "preparationComponent.h"
#include "groupComponent.h"
#include "roleComponent.h"

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

static Event* CreateNewGroupEvent(bool fromAPI, std::string command, std::vector<std::string> parameters, std::string channelId, std::string guildId)
{
	std::string seeSignature = "\nSee signature : \"" + command + " (name) (parent_id) (id) (position) (type)\"";

	if(parameters.size() != 5)
		return CreateErrorEvent(
			"Wrong parameter amount." + seeSignature,
			channelId, EUser, ECreateChannel, EWrongParemeterAmount
		);

	GroupComponent group;
	group.name = parameters[0];
	group.parentId = parameters[1];
	group.id = parameters[2];

	if(!ToInt(parameters[3], group.position))
		return CreateErrorEvent("Parameter 4 (position) of " + command + " must be an integral number.", channelId, EUser, ENewGroup, EWrongParameterType);

	if(!ToInt(parameters[4], group.type))
		return CreateErrorEvent("Parameter 5 (type) of " + command + " must be an integral number.", channelId, EUser, ENewGroup, EWrongParameterType);

	Event* e = new NewGroupEvent(fromAPI, channelId, guildId, group);
	return e;
}

static Event* CreateUpdateGroupEvent(bool fromAPI, std::string command, std::vector<std::string> parameters, std::string channelId, std::string guildId)
{
	std::string seeSignature = "\nSee signature : \"" + command + " (id) (type(0=text)(2=voice) (position)\"";

	if(parameters.size() != 3)
		return CreateErrorEvent(
			"Wrong parameter amount." + seeSignature,
			channelId, EUser, ECreateChannel, EWrongParemeterAmount
		);

    std::string method = "PATCH";
    std::string type = "/channels/";
	GroupComponent group;
	group.id = parameters[0];
	type += parameters[0];

	if(!ToInt(parameters[1], group.type))
		return CreateErrorEvent("Parameter 2 (type) of " + command + " must be an integral number." + seeSignature, channelId, EUser, EUpdateGroup, EWrongParameterType);
	if(!ToInt(parameters[2], group.position))
		return CreateErrorEvent("Parameter 2 (new channel position) of " + command + " must be an integral number.", channelId, EUser, EUpdateGroup, EWrongParameterType);

	Event* e = new UpdateGroupEvent(fromAPI, method, type, channelId, guildId, group);
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
	GroupComponent channel;

	channel.name = parameters[0];
	for(int i = 0; i < channel.name.length(); i++)
	{
		if(!isalnum(channel.name[i]) && channel.name[i] != '-')
		{
			std::cout << "FORBIDDEN_CHARACTER: " << channel.name[i] << std::endl;
			return CreateErrorEvent(
				"Parameter 1 (name) of " + command + " must be composed of only alphanumeric caracters and '-'." + seeSignature,
				channelId, EUser, ECreateChannel, EWrongParameterType
			);
		}
	}

	if(!ToInt(parameters[1], channel.type))
		return CreateErrorEvent(
			"Parameter 2 (type) of " + command + " must be an integral number." + seeSignature,
			channelId, EUser, ECreateChannel, EWrongParameterType
		);

	if(channel.type == 0)
	{
		if(parameters.size() != 4)
			return CreateErrorEvent(
				"Wrong parameter amount. Command : \"" + command + "\" with parameter \"type=0\" or \"type=4\" can not have a \"user_limit\" parameter." + seeSignature,
				channelId, EUser, ECreateChannel, EWrongParemeterAmount
			);
	}
	else if(channel.type == 2)
	{
		if(parameters.size() != 5)
			return CreateErrorEvent(
				"Wrong parameter amount. Command : \"" + command + "\" with parameter \"type=2\" must have a \"user_limit\" parameter." + seeSignature,
				channelId, EUser, ECreateChannel, EWrongParemeterAmount
			);
		if(!ToInt(parameters[4], channel.userLimit))
			return CreateErrorEvent("Parameter 5 (user limit) of " + command + " must be an integral number.", channelId, EUser, ECreateChannel, EWrongParameterType);
	}
	else
		return CreateErrorEvent("Parameter 2 (type) of " + command + " must be either 0 or 2.", channelId, EUser, ECreateChannel, EForbidden);
	

	if(!ToInt(parameters[3], channel.position))
		return CreateErrorEvent("Parameter 4 (channel position) of " + command + " must be an integral number.", channelId, EUser, ECreateChannel, EWrongParameterType);

	channel.parentId = parameters[2];

	Event *e = new CreateChannelEvent(fromAPI, method, type, channelId, guildId, channel);
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
	GroupComponent channel;
	channel.id = parameters[0];
	type += parameters[0];

	Event* e = new DeleteChannelEvent(fromAPI, method, type, channelId, guildId, channel);
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

	GroupComponent channel;
	channel.id = parameters[0];

	if(parameters.size() == 3)
		channel.parentId = parameters[2];

	if(!ToInt(parameters[1], channel.position))
		return CreateErrorEvent("Parameter 2 (new channel position) of " + command + " must be an integral number.", channelId, EUser, EMoveChannel, EWrongParameterType);

	Event* e = new MoveChannelEvent(fromAPI, channelId, guildId, channel);
	return e;
}

static Event* CreateCreateCategoryEvent(bool fromAPI, std::string command, std::vector<std::string> parameters, std::string channelId, std::string guildId)
{
	std::string seeSignature = "\nSee signature : \"" + command + " (name) (position) \"";

	if(parameters.size() != 2)
		return CreateErrorEvent(
			"Wrong parameter amount." + seeSignature,
			channelId, EUser, ECreateChannel, EWrongParemeterAmount
		);

    std::string method = "POST";
    std::string type = "/guilds/" + guildId + "/channels";
	GroupComponent category;
	category.type = 4;

	category.name = parameters[0];
	for(int i = 0; i < category.name.length(); i++)
	{
		if(!isalnum(category.name[i]) && category.name[i] != '-')
			return CreateErrorEvent(
				"Parameter 1 (name) of " + command + " must be composed of only alphanumeric caracters and '-'." + seeSignature,
				channelId, EUser, ECreateChannel, EWrongParameterType
			);
	}

	if(!ToInt(parameters[1], category.position))
		return CreateErrorEvent("Parameter 5 (channel position) of " + command + " must be an integral number.", channelId, EUser, ECreateCategory, EWrongParameterType);

	Event *e = new CreateCategoryEvent(fromAPI, method, type, channelId, guildId, category);
	return e;
}

static Event* CreateDeleteCategoryEvent(bool fromAPI, std::string command, std::vector<std::string> parameters, std::string channelId, std::string guildId)
{
	std::string seeSignature = "\nSee signature : \"" + command + " (id) (deletion_queued)\"";

	if(parameters.size() != 2)
		return CreateErrorEvent(
			"Wrong parameter amount." + seeSignature,
			channelId, EUser, ECreateChannel, EWrongParemeterAmount
		);

    std::string method = "DELETE";
    std::string type = "/channels/";
	GroupComponent category;
	category.id = parameters[0];
	type += parameters[0];

	bool deletionQueued;
	if(parameters.size() == 2)
		std::istringstream(parameters[1]) >> deletionQueued;

	Event* e = new DeleteCategoryEvent(deletionQueued, method, type, channelId, guildId, category);
	return e;
}

static Event* CreateMoveCategoryEvent(bool fromAPI, std::string command, std::vector<std::string> parameters, std::string channelId, std::string guildId)
{
	std::string seeSignature = "\nSee signature : \"" + command + " (id) (position)\"";

	if(parameters.size() != 2)
		return CreateErrorEvent(
			"Wrong parameter amount." + seeSignature,
			channelId, EUser, ECreateChannel, EWrongParemeterAmount
		);

	GroupComponent category;
	category.id = parameters[0];

	if(!ToInt(parameters[1], category.position))
		return CreateErrorEvent("Parameter 2 (new channel position) of " + command + " must be an integral number.", channelId, EUser, EMoveCategory, EWrongParameterType);

	Event* e = new MoveCategoryEvent(fromAPI, channelId, guildId, category);
	return e;
}

static Event* CreateMoveUserEvent(bool fromAPI, std::string command, std::vector<std::string> parameters, std::string channelId, std::string guildId)
{
	std::string seeSignature = "\nSee signature : \"" + command + " (user_id) (group_id)\"";

	if(parameters.size() != 2)
		return CreateErrorEvent(
			"Wrong parameter amount." + seeSignature,
			channelId, EUser, EMoveUser, EWrongParemeterAmount
		);

    std::string method = "PATCH";
    std::string type = "/guilds/" + guildId + "/members/" + parameters[0];
	GroupComponent voiceChannel;
	voiceChannel.id = parameters[1];
	voiceChannel.userIds.push_back(parameters[0]);

	Event* e = new MoveUserEvent(fromAPI, method, type, channelId, guildId, voiceChannel);
	return e;
}

static Event* CreateCreateMatchEvent(bool fromAPI, std::string command, std::vector<std::string> parameters, std::string channelId, std::string guildId)
{
	std::string seeSignature = "\nSee signature : \"" + command + " (creation_step) (game_id) (game_name) (user_count) (user_id) (...) (user_id)\"";

	if(parameters.size() < 4)
		return CreateErrorEvent(
			"Wrong parameter amount." + seeSignature,
			channelId, EUser, ECreateMatch, EWrongParemeterAmount
		);

	int creationStep;
	std::string gameId = parameters[1];
	std::string gameName = parameters[2];
	int userCount;
	std::vector<std::string> userIds;

	if(!ToInt(parameters[0], creationStep))
		return CreateErrorEvent("Parameter 1 (creation_step) of " + command + " must be an integral number.", channelId, EUser, ECreateMatch, EWrongParameterType);
	
	if(!ToInt(parameters[3], userCount))
		return CreateErrorEvent("Parameter 4 (user_count) of " + command + " must be an integral number.", channelId, EUser, ECreateMatch, EWrongParameterType);

	if(parameters.size() != 4 + userCount)
		return CreateErrorEvent(
			"Wrong parameter amount." + seeSignature,
			channelId, EUser, ECreateMatch, EWrongParemeterAmount
		);
	
	for(int i = 4; i < 4 + userCount; i++)
		userIds.push_back(parameters[i]);

	Event* e = new CreateMatchEvent(channelId, guildId, creationStep, gameId, gameName, userCount, userIds);
	return e;
}

static Event* CreateChangeGroupPermissionsEvent(bool fromAPI, std::string command, std::vector<std::string> parameters, std::string channelId, std::string guildId)
{
	std::string seeSignature = "\nSee signature : \"" + command + " (give) (group_id) (name) (type) (permissions) (user_count) (user_id) ... (user_id)\"";

		if(parameters.size() < 6)
		return CreateErrorEvent(
			"Wrong parameter amount." + seeSignature,
			channelId, EUser, EChangeGroupPermissions, EWrongParemeterAmount
		);

    std::string method = "PATCH";
    std::string type = "/channels/" + parameters[1];

	bool give;
	GroupComponent group;
	int userCount;
	int permissions;
	std::vector<std::string> userIds;

	std::istringstream(parameters[0]) >> give;


	group.id = parameters[1];
	group.name = parameters[2];
	if(!ToInt(parameters[3], group.type))
		return CreateErrorEvent("Parameter 4 (type) of " + command + " must be an integral number.", channelId, EUser, EChangeGroupPermissions, EWrongParameterType);

	if(!ToInt(parameters[4], permissions))
		return CreateErrorEvent("Parameter 5 (permissions) of " + command + " must be an integral number.", channelId, EUser, EChangeGroupPermissions, EWrongParameterType);

	if(!ToInt(parameters[5], userCount))
		return CreateErrorEvent("Parameter 6 (user_count) of " + command + " must be an integral number.", channelId, EUser, EChangeGroupPermissions, EWrongParameterType);

	for(int i = 6; i < 6 + userCount; i++)
		userIds.push_back(parameters[i]);

	Event* e = new ChangeGroupPermissionsEvent(fromAPI, method, type, channelId, guildId, give, group, permissions, userCount, userIds);
	return e;
}

static Event* CreateSetMatchVoicePermissionsEvent(bool fromAPI, std::string command, std::vector<std::string> parameters, std::string channelId, std::string guildId)
{
	std::string seeSignature = "\nSee signature : \"" + command + " (give) (group_id) (name) (type) (permmissions1) (permissions2) (user_count) (user_id) ... (user_id)\"";

	if(parameters.size() < 7)
		return CreateErrorEvent(
			"Wrong parameter amount." + seeSignature,
			channelId, EUser, EChangeGroupPermissions, EWrongParemeterAmount
		);

    std::string method = "PATCH";
    std::string type = "/channels/" + parameters[1];

	bool give;
	GroupComponent group;
	int userCount;
	int permissions1;
	int permissions2;
	std::vector<std::string> userIds;

	std::istringstream(parameters[0]) >> give;


	group.id = parameters[1];
	group.name = parameters[2];
	if(!ToInt(parameters[3], group.type))
		return CreateErrorEvent("Parameter 4 (type) of " + command + " must be an integral number.", channelId, EUser, EChangeGroupPermissions, EWrongParameterType);

	if(!ToInt(parameters[4], permissions1))
		return CreateErrorEvent("Parameter 5 (permissions1) of " + command + " must be an integral number.", channelId, EUser, EChangeGroupPermissions, EWrongParameterType);

	if(!ToInt(parameters[5], permissions2))
		return CreateErrorEvent("Parameter 6 (permissions2) of " + command + " must be an integral number.", channelId, EUser, EChangeGroupPermissions, EWrongParameterType);

	if(!ToInt(parameters[6], userCount))
		return CreateErrorEvent("Parameter 7 (user_count) of " + command + " must be an integral number.", channelId, EUser, EChangeGroupPermissions, EWrongParameterType);

	for(int i = 7; i < 7 + userCount; i++)
		userIds.push_back(parameters[i]);

	Event* e = new SetMatchVoicePermissionsEvent(fromAPI, method, type, channelId, guildId, give, group, permissions1, permissions2, userCount, userIds);
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
		case ENewGroup:
			return CreateNewGroupEvent(fromAPI, command, parameters, channelId, guildId);
		case EUpdateGroup:
			return CreateUpdateGroupEvent(fromAPI, command, parameters, channelId, guildId);
		case ECreateChannel:
			return CreateCreateChannelEvent(fromAPI, command, parameters, channelId, guildId);
		case EDeleteChannel:
			return CreateDeleteChannelEvent(fromAPI, command, parameters, channelId, guildId);
		case EMoveChannel:
			return CreateMoveChannelEvent(fromAPI, command, parameters, channelId, guildId);
		case ECreateCategory:
			return CreateCreateCategoryEvent(fromAPI, command, parameters, channelId, guildId);
		case EDeleteCategory:
			return CreateDeleteCategoryEvent(fromAPI, command, parameters, channelId, guildId);
		case EMoveCategory:
			return CreateMoveCategoryEvent(fromAPI, command, parameters, channelId, guildId);
		case EMoveUser:
			return CreateMoveUserEvent(fromAPI, command, parameters, channelId, guildId);
		case ECreateMatch:
			return CreateCreateMatchEvent(fromAPI, command, parameters, channelId, guildId);
		case EChangeGroupPermissions:
			return CreateChangeGroupPermissionsEvent(fromAPI, command, parameters, channelId, guildId);
		case ESetMatchVoicePermissions:
			return CreateSetMatchVoicePermissionsEvent(fromAPI, command, parameters, channelId, guildId);
		default:
			return CreateErrorEvent("This command does not exist.", channelId, EUser, ECreateEvent, EWrongEventType);
		}
	}
	return CreateErrorEvent("This command does not exist.", channelId, EUser, ECreateEvent, EWrongEventType);
}

#endif