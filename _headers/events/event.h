#ifndef EVENT_H__
#define	EVENT_H__

#include <iostream>
#include <string>
#include <sstream>

#include <nlohmann/json.hpp>

// Components
#include "groupComponent.h"

#include "eventLanguage.h"

struct Event
{
	virtual bool ReadOnly() const = 0;
	virtual bool WaitForResponse() const = 0;
	virtual EEventType GetType() const = 0;
	virtual void CreateJson() = 0;
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
	void CreateJson()
	{

	}
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
	void CreateJson()
	{

	}
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
	void CreateJson()
	{

	}
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << EShutdown << '}';
		return os.str();
	}
};

struct NewGroupEvent : public Event
{
	NewGroupEvent(bool fromAPI, std::string channelId, std::string guildId, GroupComponent group)
	{
		this->channelId = channelId;
		this->guildId = guildId;
		this->fromAPI = fromAPI;
		this->group = group;
		CreateJson();
	}
	bool ReadOnly() const { return true; }
	bool WaitForResponse() const { return false; }
	EEventType GetType() const { return ENewGroup; }
	void CreateJson()
	{
		this->content = { };
	}
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << ENewGroup << ';' << channelId  << ';' << guildId << ';' << group.name << ';' << group.parentId << ';' <<
		group.id << ';' << group.position  << '}';
		return os.str();
	}
	GroupComponent group;
};

struct UpdateGroupEvent : public Event
{
	UpdateGroupEvent(bool fromAPI, std::string method, std::string type, std::string channelId, std::string guildId, GroupComponent group)
	{
		this->fromAPI = fromAPI;
		this->method = method;
		this->type = type;
		this->channelId = channelId;
		this->guildId = guildId;
		this->group = group;
		CreateJson();
	}
	bool ReadOnly() const { return false; }
	bool WaitForResponse() const { return true; }
	EEventType GetType() const { return EUpdateGroup; }
	void CreateJson()
	{
		this->content = {
			{ "position", group.position }
	    };
	}
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << EUpdateGroup << ';' << channelId  << ';' << guildId << ';' << group.id << ';' << group.type << ';' << group.position << '}';
		return os.str();
	}

	GroupComponent group;
};

struct CreateChannelEvent : public Event
{
	CreateChannelEvent(bool fromAPI, std::string method, std::string type, std::string channelId, std::string guildId, GroupComponent channel)
	{
		this->fromAPI = fromAPI;
		this->method = method;
		this->type = type;
		this->channelId = channelId;
		this->guildId = guildId;
		this->channel = channel;
		CreateJson();
	}
	bool ReadOnly() const { return false; }
	bool WaitForResponse() const { return true; }
	EEventType GetType() const { return ECreateChannel; }
	void CreateJson()
	{
		this->content = {
    	    { "name", channel.name },
    	    { "type", channel.type },
			{ "parent_id", channel.parentId },
			{ "position", channel.position },
			{ "user_limit", (channel.type == 0 ? 1 : channel.userLimit) }
	    };
	}
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << ECreateChannel << ';' << channelId  << ';' << guildId << ';' << channel.name << ';' << channel.type << ';' << 
		channel.parentId <<  ';' << channel.position << std::string(channel.type == 0 ? "" : ";" + std::to_string(channel.userLimit)) << '}';
		return os.str();
	}

	GroupComponent channel;
};

struct DeleteChannelEvent : public Event
{
	DeleteChannelEvent(bool fromAPI, std::string method, std::string type, std::string channelId, std::string guildId, GroupComponent channel)
	{
		this->fromAPI = fromAPI;
		this->method = method;
		this->type = type;
		this->channelId = channelId;
		this->guildId = guildId;
		this->channel = channel;
		CreateJson();
	}
	bool ReadOnly() const { return false; }
	bool WaitForResponse() const { return true; }
	EEventType GetType() const { return EDeleteChannel; }
	void CreateJson()
	{
		this->content = { };
	}
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << EDeleteChannel << ';' << channel.id << '}';
		return os.str();
	}

	GroupComponent channel;
};

struct MoveChannelEvent : public Event
{
	MoveChannelEvent(bool fromAPI, std::string channelId, std::string guildId, GroupComponent channel)
	{
		this->fromAPI = fromAPI;
		this->method = method;
		this->type = type;
		this->channelId = channelId;
		this->guildId = guildId;
		this->channel = channel;
		CreateJson();
	}
	bool ReadOnly() const { return true; }
	bool WaitForResponse() const { return true; }
	EEventType GetType() const { return EMoveChannel; }
	void CreateJson()
	{
		this->content = { };
	}
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << EMoveChannel << ';' << channelId  << ';' << guildId << ';' << channel.id << ';' << channel.position << '}';
		return os.str();
	}

	GroupComponent channel;
};

struct CreateCategoryEvent : public Event
{
	CreateCategoryEvent(bool fromAPI, std::string method, std::string type, std::string channelId, std::string guildId, GroupComponent category)
	{
		this->fromAPI = fromAPI;
		this->method = method;
		this->type = type;
		this->channelId = channelId;
		this->guildId = guildId;
		this->category = category;
		CreateJson();
	}
	bool ReadOnly() const { return false; }
	bool WaitForResponse() const { return true; }
	EEventType GetType() const { return ECreateCategory; }
	void CreateJson()
	{
		this->content = {
    	    { "name", category.name },
    	    { "type", category.type },
			{ "position", category.position }
	    };
	}
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << ECreateCategory << ';' << channelId  << ';' << guildId << ';' << category.name << ';' << category.type << ';' << 
		category.position << '}';
		return os.str();
	}

	GroupComponent category;
};

struct MoveCategoryEvent : public Event
{
	MoveCategoryEvent(bool fromAPI, std::string channelId, std::string guildId, GroupComponent category)
	{
		this->fromAPI = fromAPI;
		this->method = method;
		this->type = type;
		this->channelId = channelId;
		this->guildId = guildId;
		this->category = category;
		CreateJson();
	}
	bool ReadOnly() const { return true; }
	bool WaitForResponse() const { return true; }
	EEventType GetType() const { return EMoveCategory; }
	void CreateJson()
	{
		this->content = { };
	}
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << EMoveCategory << ';' << channelId  << ';' << guildId << ';' << category.id << ';' << category.position << '}';
		return os.str();
	}

	GroupComponent category;
};

#endif
