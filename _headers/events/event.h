#ifndef EVENT_H__
#define	EVENT_H__

#include <iostream>
#include <string>
#include <sstream>

#include <nlohmann/json.hpp>

// Components
#include "userGroupComponent.h"

#include "eventLanguage.h"

struct Event
{
	virtual bool ReadOnly() const = 0;
	virtual bool WaitForResponse() const = 0;
	virtual EEventType GetType() const = 0;
	virtual std::string ToDebuggable() const = 0;

	bool fromAPI;
    std::string method;
    std::string type;
    nlohmann::json content;
	std::string guildId;
	std::string channelId;
};

struct ErrorEvent : public Event
{
	ErrorEvent(std::string message, std::string channelId, EOffender eOffender, EEventType eType, EErrorType geType)
	{
		fromAPI = false;
		this->eOffender = eOffender;
		this->eType = eType;
		this->geType = geType;
		this->message = message;
		this->channelId = channelId;
	}
	bool ReadOnly() const { return false; }
	bool WaitForResponse() const { return false; }
	EEventType GetType() const { return EError; }
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << EError << ';' << message << ';' << channelId << ';' << eOffender  << ';' << eType << ';' << geType << '}';
		return os.str();
	}

	EOffender eOffender;
	EEventType eType;
	EErrorType geType;
	std::string message;
};

struct EmptyEvent : public Event
{
	EmptyEvent()
	{
		fromAPI = true;
	}
	bool ReadOnly() const { return true; }
	bool WaitForResponse() const { return false; }
	EEventType GetType() const { return EEmpty; }
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << " " << '}';
		return os.str();
	}
};

struct ShutdownEvent : public Event
{
	ShutdownEvent()
	{
		fromAPI = false;
	}
	bool ReadOnly() const { return false; }
	bool WaitForResponse() const { return false; }
	EEventType GetType() const { return EShutdown; }
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << EShutdown << '}';
		return os.str();
	}
};

struct NewChannelEvent : public Event
{
	NewChannelEvent(UserGroupComponent userGroup)
	{
		fromAPI = true;
		this->userGroup = userGroup;
		this->content = { };
	}
	bool ReadOnly() const { return true; }
	bool WaitForResponse() const { return false; }
	EEventType GetType() const { return ENewChannelEvent; }
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << ENewChannelEvent << ';' << userGroup.name << ';' << userGroup.parentId << ';' << userGroup.id << ';' << userGroup.position  << '}';
		return os.str();
	}
	UserGroupComponent userGroup;
};

struct CreateChannelEvent : public Event
{
	CreateChannelEvent(bool fromAPI, std::string method, std::string type, std::string channelId, std::string guildId, UserGroupComponent userGroup)
	{
		this->fromAPI = fromAPI;
		this->method = method;
		this->type = type;
		this->channelId = channelId;
		this->guildId = guildId;
		this->userGroup = userGroup;
		this->content = {
    	    { "name", userGroup.name },
    	    { "type", userGroup.type },
			{ "parent_id", userGroup.parentId },
			{ "position", userGroup.position },
			{ "user_limit", (userGroup.type == 0 ? 1 : userGroup.userLimit) }
	    };
	}
	bool ReadOnly() const { return false; }
	bool WaitForResponse() const { return true; }
	EEventType GetType() const { return ECreateChannel; }
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << ECreateChannel << ';' << channelId  << ';' << guildId << ';' << userGroup.name << ';' << userGroup.type << ';' << 
		userGroup.parentId <<  ';' << userGroup.position << std::string(userGroup.type == 0 ? "" : ";" + std::to_string(userGroup.userLimit)) << '}';
		return os.str();
	}

	UserGroupComponent userGroup;
};

struct UpdateVoiceChannelEvent : public Event
{
	UpdateVoiceChannelEvent(bool fromAPI, std::string method, std::string type, std::string channelId, std::string guildId, UserGroupComponent userGroup)
	{
		this->fromAPI = fromAPI;
		this->method = method;
		this->type = type;
		this->channelId = channelId;
		this->guildId = guildId;
		this->userGroup = userGroup;
		this->content = {
			{ "position", userGroup.position }
	    };
	}
	bool ReadOnly() const { return false; }
	bool WaitForResponse() const { return true; }
	EEventType GetType() const { return EUpdateChannel; }
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << EUpdateChannel << ';' << channelId  << ';' << guildId << ';' << userGroup.id << ';' << userGroup.position << '}';
		return os.str();
	}

	UserGroupComponent userGroup;
};

struct DeleteChannelEvent : public Event
{
	DeleteChannelEvent(bool fromAPI, std::string method, std::string type, std::string channelId, std::string guildId, UserGroupComponent userGroup)
	{
		this->fromAPI = fromAPI;
		this->method = method;
		this->type = type;
		this->channelId = channelId;
		this->guildId = guildId;
		this->userGroup = userGroup;
		this->content = { };
	}
	bool ReadOnly() const { return false; }
	bool WaitForResponse() const { return true; }
	EEventType GetType() const { return EDeleteChannel; }
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << EDeleteChannel << ';' << userGroup.id << '}';
		return os.str();
	}

	UserGroupComponent userGroup;
};

struct MoveChannelEvent : public Event
{
	MoveChannelEvent(bool fromAPI, std::string channelId, std::string guildId, UserGroupComponent userGroup)
	{
		this->fromAPI = fromAPI;
		this->method = method;
		this->type = type;
		this->channelId = channelId;
		this->guildId = guildId;
		this->userGroup = userGroup;
		this->content = { };
	}
	bool ReadOnly() const { return true; }
	bool WaitForResponse() const { return true; }
	EEventType GetType() const { return EMoveChannel; }
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << EMoveChannel << ';' << channelId  << ';' << guildId << ';' << userGroup.id << ';' << userGroup.position << '}';
		return os.str();
	}

	UserGroupComponent userGroup;
};

struct CreateCategoryEvent : public Event
{
	CreateCategoryEvent(bool fromAPI, std::string method, std::string type, std::string channelId, std::string guildId, UserGroupComponent userGroup)
	{
		this->fromAPI = fromAPI;
		this->method = method;
		this->type = type;
		this->channelId = channelId;
		this->guildId = guildId;
		this->userGroup = userGroup;
		this->content = {
    	    { "name", userGroup.name },
    	    { "type", userGroup.type },
			{ "position", userGroup.position }
	    };
	}
	bool ReadOnly() const { return false; }
	bool WaitForResponse() const { return true; }
	EEventType GetType() const { return ECreateCategory; }
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << ECreateCategory << ';' << channelId  << ';' << guildId << ';' << userGroup.name << ';' << userGroup.type << ';' << 
		userGroup.position << '}';
		return os.str();
	}

	UserGroupComponent userGroup;
};

#endif
