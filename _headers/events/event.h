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
	virtual EEventType GetType() const = 0;
	virtual std::string ToDebuggable() const = 0;

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
		this->eOffender = eOffender;
		this->eType = eType;
		this->geType = geType;
		this->message = message;
		this->channelId = channelId;
	}
	bool ReadOnly() const { return false; }
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

struct ShutdownEvent : public Event
{
	ShutdownEvent()
	{
		
	}
	bool ReadOnly() const { return false; }
	EEventType GetType() const { return EShutdown; }
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << EShutdown << '}';
		return os.str();
	}
};

struct SetChannelIdEvent : public Event
{
	SetChannelIdEvent(UserGroupComponent userGroup)
	{
		this->userGroup = userGroup;
		this->content = { };
	}
	bool ReadOnly() const { return true; }
	EEventType GetType() const { return ESetChannelId; }
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << ESetChannelId << ';' << userGroup.name << ';' << userGroup.parentId << ';' << userGroup.id  << '}';
		return os.str();
	}
	UserGroupComponent userGroup;
};

struct CreateVoiceChannelEvent : public Event
{
	CreateVoiceChannelEvent(std::string method, std::string type, std::string channelId, std::string guildId, UserGroupComponent userGroup)
	{
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
	bool ReadOnly() const { return false; }
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
	UpdateVoiceChannelEvent(std::string method, std::string type, std::string channelId, std::string guildId, UserGroupComponent userGroup)
	{
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
	EEventType GetType() const { return EUpdateVoiceChannel; }
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << EUpdateVoiceChannel << ';' << channelId  << ';' << guildId << ';' << userGroup.position << '}';
		return os.str();
	}

	UserGroupComponent userGroup;
};

#endif
