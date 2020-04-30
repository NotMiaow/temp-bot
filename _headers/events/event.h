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

struct CreateVoiceChannelEvent : public Event
{
	CreateVoiceChannelEvent(bool fromAPI, std::string method, std::string type, std::string channelId, std::string guildId, UserGroupComponent userGroup)
	{
		this->fromAPI = fromAPI;
		this->method = method;
		this->type = type;
		this->channelId = channelId;
		this->guildId = guildId;
		this->userGroup = userGroup;
		this->content = {
    	    { "name", userGroup.name },
    	    { "type", 2 },
			{ "user_limit",  userGroup.userLimit },
			{ "parent_id", userGroup.parentId },
			{ "position", userGroup.position }
	    };
	}
	bool WaitForResponse() const { return true; }
	EEventType GetType() const { return ECreateVoiceChannel; }
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << ECreateVoiceChannel << ';' << channelId  << ';' << guildId << ';' << userGroup.name <<  ';' << 
			userGroup.userLimit <<  ';' << userGroup.parentId <<  ';' << userGroup.position << '}';
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
	bool WaitForResponse() const { return true; }
	EEventType GetType() const { return EUpdateVoiceChannel; }
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << EUpdateVoiceChannel << ';' << channelId  << ';' << guildId << ';' << userGroup.id << ';' << userGroup.position << '}';
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

#endif
