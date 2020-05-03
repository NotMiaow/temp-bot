#ifndef	EVENT_TRANSLATOR_H__
#define EVENT_TRANSLATOR_H__

#include <boost/algorithm/string.hpp>
#include <string>
#include <vector>
#include <ctype.h>
#include <sstream>

#include <nlohmann/json.hpp>

#include "basicLib.h"

#include "groupComponent.h"

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
			return CreateErrorEvent(
				"Parameter 1 (name) of " + command + " must be composed of only alphanumeric caracters and '-'." + seeSignature,
				channelId, EUser, ECreateChannel, EWrongParameterType
			);
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

	if(parameters.size() < 2)
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
	std::string seeSignature = "\nSee signature : \"" + command + " (id)\"";

	if(parameters.size() < 1 || parameters.size() > 2)
		return CreateErrorEvent(
			"Wrong parameter amount." + seeSignature,
			channelId, EUser, ECreateChannel, EWrongParemeterAmount
		);

    std::string method = "DELETE";
    std::string type = "/channels/";
	GroupComponent category;
	category.id = parameters[0];
	type += parameters[0];

	bool deletionQueued = false;
	if(parameters.size() == 2)
		std::istringstream(parameters[1]) >> deletionQueued;
	
	if(deletionQueued)
		std::cout << "success" << std::endl;

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
			return CreateMoveCategoryEvent(fromAPI,command, parameters, channelId, guildId);
		default:
			return CreateErrorEvent("This command does not exist.", channelId, EUser, ECreateEvent, EWrongEventType);
		}
	}
	return CreateErrorEvent("This command does not exist.", channelId, EUser, ECreateEvent, EWrongEventType);
}

#endif