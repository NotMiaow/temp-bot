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

static Event* CreateErrorEvent(EventInfo info, std::string message, EOffender offender, EEventType eventType, EErrorType errorType)
{
	Event* e = new ErrorEvent(info, message, offender, eventType, errorType);
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

static Event* CreateSendMessageEvent(EventInfo info, std::string message)
{
	Event* e = new SendMessageEvent(info, message);
	return e;
}

static Event* CreateNewGroupEvent(EventInfo info, std::string command, std::vector<std::string> parameters)
{
	std::string seeSignature = "\nSee signature : \"" + command + " (name) (parent_id) (id) (position) (type)\"";

	if(parameters.size() != 5)
		return CreateErrorEvent(
			info,
			"Wrong parameter amount." + seeSignature,
			EUser, ECreateChannel, EWrongParemeterAmount
		);

	GroupComponent group;
	group.name = parameters[0];
	group.parentId = parameters[1];
	group.id = parameters[2];

	if(!ToInt(parameters[3], group.position))
		return CreateErrorEvent(info, "Parameter 4 (position) of " + command + " must be an integral number.", EUser, ENewGroup, EWrongParameterType);

	if(!ToInt(parameters[4], group.type))
		return CreateErrorEvent(info, "Parameter 5 (type) of " + command + " must be an integral number.", EUser, ENewGroup, EWrongParameterType);

	Event* e = new NewGroupEvent(info, group);
	return e;
}

static Event* CreateUpdateGroupEvent(EventInfo info, std::string command, std::vector<std::string> parameters)
{
	std::string seeSignature = "\nSee signature : \"" + command + " (id) (type(0=text)(2=voice) (position)\"";

	if(parameters.size() != 3)
		return CreateErrorEvent(
			info, 
			"Wrong parameter amount." + seeSignature,
			EUser, ECreateChannel, EWrongParemeterAmount
		);

	GroupComponent group;
	group.id = parameters[0];
	if(!ToInt(parameters[1], group.type))
		return CreateErrorEvent(info, "Parameter 2 (type) of " + command + " must be an integral number." + seeSignature, EUser, EUpdateGroup, EWrongParameterType);
	if(!ToInt(parameters[2], group.position))
		return CreateErrorEvent(info, "Parameter 2 (new channel position) of " + command + " must be an integral number.", EUser, EUpdateGroup, EWrongParameterType);

	Event* e = new UpdateGroupEvent(info, group);
	return e;
}

static Event* CreateCreateChannelEvent(EventInfo info, std::string command, std::vector<std::string> parameters)
{
	std::string seeSignature = "\nSee signature : \"" + command + " (name) (type(0=text)(2=voice) (parent_id) (position) (user_limit?)\"";

	if(parameters.size() < 4 || parameters.size() > 5)
		return CreateErrorEvent(
			info,
			"Wrong parameter amount." + seeSignature,
			EUser, ECreateChannel, EWrongParemeterAmount
		);

	GroupComponent channel;
	channel.name = parameters[0];
	for(int i = 0; i < channel.name.length(); i++)
	{
		if(!isalnum(channel.name[i]) && channel.name[i] != '-')
		{
			std::cout << "FORBIDDEN_CHARACTER: " << channel.name[i] << std::endl;
			return CreateErrorEvent(
				info,
				"Parameter 1 (name) of " + command + " must be composed of only alphanumeric caracters and '-'." + seeSignature,
				EUser, ECreateChannel, EWrongParameterType
			);
		}
	}

	if(!ToInt(parameters[1], channel.type))
		return CreateErrorEvent(
			info,
			"Parameter 2 (type) of " + command + " must be an integral number." + seeSignature,
			EUser, ECreateChannel, EWrongParameterType
		);

	if(channel.type == 0)
	{
		if(parameters.size() != 4)
			return CreateErrorEvent(
				info, 
				"Wrong parameter amount. Command : \"" + command + "\" with parameter \"type=0\" or \"type=4\" can not have a \"user_limit\" parameter." + seeSignature,
				EUser, ECreateChannel, EWrongParemeterAmount
			);
	}
	else if(channel.type == 2)
	{
		if(parameters.size() != 5)
			return CreateErrorEvent(
				info, 
				"Wrong parameter amount. Command : \"" + command + "\" with parameter \"type=2\" must have a \"user_limit\" parameter." + seeSignature,
				EUser, ECreateChannel, EWrongParemeterAmount
			);
		if(!ToInt(parameters[4], channel.userLimit))
			return CreateErrorEvent(info, "Parameter 5 (user limit) of " + command + " must be an integral number.", EUser, ECreateChannel, EWrongParameterType);
	}
	else
		return CreateErrorEvent(info, "Parameter 2 (type) of " + command + " must be either 0 or 2.", EUser, ECreateChannel, EForbidden);
	

	if(!ToInt(parameters[3], channel.position))
		return CreateErrorEvent(info, "Parameter 4 (channel position) of " + command + " must be an integral number.", EUser, ECreateChannel, EWrongParameterType);

	channel.parentId = parameters[2];

	Event *e = new CreateChannelEvent(info, channel);
	return e;
}

static Event* CreateDeleteChannelEvent(EventInfo info, std::string command, std::vector<std::string> parameters)
{
	std::string seeSignature = "\nSee signature : \"" + command + " (id)\"";

	if(parameters.size() != 1)
		return CreateErrorEvent(
			info, 
			"Wrong parameter amount." + seeSignature,
			EUser, ECreateChannel, EWrongParemeterAmount
		);

	Event* e = new DeleteChannelEvent(info, parameters[0]);
	return e;
}

static Event* CreateMoveChannelEvent(EventInfo info, std::string command, std::vector<std::string> parameters)
{
	std::string seeSignature = "\nSee signature : \"" + command + " (id) (position) (parent_id?)\"";

	if(parameters.size() < 2 || parameters.size() > 3)
		return CreateErrorEvent(
			info, 
			"Wrong parameter amount." + seeSignature,
			EUser, ECreateChannel, EWrongParemeterAmount
		);

	GroupComponent channel;
	channel.id = parameters[0];

	if(parameters.size() == 3)
		channel.parentId = parameters[2];

	if(!ToInt(parameters[1], channel.position))
		return CreateErrorEvent(info, "Parameter 2 (new channel position) of " + command + " must be an integral number.", EUser, EMoveChannel, EWrongParameterType);

	Event* e = new MoveChannelEvent(info, channel);
	return e;
}

static Event* CreateCreateCategoryEvent(EventInfo info, std::string command, std::vector<std::string> parameters)
{
	std::string seeSignature = "\nSee signature : \"" + command + " (name) (position) \"";

	if(parameters.size() != 2)
		return CreateErrorEvent(
			info,
			"Wrong parameter amount." + seeSignature,
			EUser, ECreateChannel, EWrongParemeterAmount
		);

	GroupComponent category;
	category.type = 4;
	category.name = parameters[0];
	for(int i = 0; i < category.name.length(); i++)
	{
		if(!isalnum(category.name[i]) && category.name[i] != '-')
			return CreateErrorEvent(
				info, 
				"Parameter 1 (name) of " + command + " must be composed of only alphanumeric caracters and '-'." + seeSignature,
				EUser, ECreateChannel, EWrongParameterType
			);
	}

	if(!ToInt(parameters[1], category.position))
		return CreateErrorEvent(info, "Parameter 5 (channel position) of " + command + " must be an integral number.", EUser, ECreateCategory, EWrongParameterType);

	Event *e = new CreateCategoryEvent(info, category);
	return e;
}

static Event* CreateDeleteCategoryEvent(EventInfo info, std::string command, std::vector<std::string> parameters)
{
	std::string seeSignature = "\nSee signature : \"" + command + " (id)\"";

	if(parameters.size() != 1)
		return CreateErrorEvent(
			info,
			"Wrong parameter amount." + seeSignature,
			EUser, ECreateChannel, EWrongParemeterAmount
		);

	Event* e = new DeleteCategoryEvent(info, false, parameters[0]);
	return e;
}

static Event* CreateMoveCategoryEvent(EventInfo info, std::string command, std::vector<std::string> parameters)
{
	std::string seeSignature = "\nSee signature : \"" + command + " (id) (position)\"";

	if(parameters.size() != 2)
		return CreateErrorEvent(
			info,
			"Wrong parameter amount." + seeSignature,
			EUser, ECreateChannel, EWrongParemeterAmount
		);

	GroupComponent category;
	category.id = parameters[0];

	if(!ToInt(parameters[1], category.position))
		return CreateErrorEvent(info, "Parameter 2 (new channel position) of " + command + " must be an integral number.", EUser, EMoveCategory, EWrongParameterType);

	Event* e = new MoveCategoryEvent(info, category);
	return e;
}

static Event* CreateMoveUserEvent(EventInfo info, std::string command, std::vector<std::string> parameters)
{
	std::string seeSignature = "\nSee signature : \"" + command + " (user_id) (group_id)\"";

	if(parameters.size() != 2)
		return CreateErrorEvent(
			info,
			"Wrong parameter amount." + seeSignature,
			EUser, EMoveUser, EWrongParemeterAmount
		);

	Event* e = new MoveUserEvent(info, parameters[0], parameters[1]);
	return e;
}

static Event* CreateJoinQueueEvent(EventInfo info, std::string command, std::vector<std::string> parameters)
{
	std::string seeSignature = "\nSee signature : \"" + command + " (queueName)\"";

	if(parameters.size() != 1)
		return CreateErrorEvent(
			info,
			"Wrong parameter amount." + seeSignature,
			EUser, EJoinQueue, EWrongParemeterAmount
		);

	Event* e = new JoinQueueEvent(info, parameters[0]);
	return e;
}

static Event* CreateEvent(EventInfo info, std::string command, std::string content)
{
	std::vector<std::string> parameters = Split(content, ' ');

	auto it = optionEventTypes.find(command);
	if(it != optionEventTypes.end())
	{
		switch (it->second)
		{
		case EEmpty:					return CreateEmptyEvent();
		case EShutdown:					return CreateShutdownEvent();
		case ENewGroup:					return CreateNewGroupEvent(info, command, parameters);
		case EUpdateGroup:				return CreateUpdateGroupEvent(info, command, parameters);
		case ECreateChannel:			return CreateCreateChannelEvent(info, command, parameters);
		case EDeleteChannel:			return CreateDeleteChannelEvent(info, command, parameters);
		case EMoveChannel:				return CreateMoveChannelEvent(info, command, parameters);
		case ECreateCategory:			return CreateCreateCategoryEvent(info, command, parameters);
		case EDeleteCategory:			return CreateDeleteCategoryEvent(info, command, parameters);
		case EMoveCategory:				return CreateMoveCategoryEvent(info, command, parameters);
		case EMoveUser:					return CreateMoveUserEvent(info, command, parameters);
		case ECreateMatch:				return CreateErrorEvent(info, "This command can not be used by a human.", EUser, ECreateEvent, EWrongEventType);
		case EChangeGroupPermissions:	return CreateErrorEvent(info, "This command can not be used by a human.", EUser, ECreateEvent, EWrongEventType);
		case ESetMatchVoicePermissions:	return CreateErrorEvent(info, "This command can not be used by a human.", EUser, ECreateEvent, EWrongEventType);
		case EJoinQueue:				return CreateJoinQueueEvent(info, command, parameters);
		default:						return CreateErrorEvent(info, "This command does not exist.", EUser, ECreateEvent, EWrongEventType);
		}
	}
	return CreateErrorEvent(info, "This command does not exist.", EUser, ECreateEvent, EWrongEventType);
}

#endif