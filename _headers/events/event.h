#ifndef EVENT_H__
#define	EVENT_H__

#include <iostream>
#include <string>
#include <sstream>

#include <nlohmann/json.hpp>

// Components
#include "preparationComponent.h"
#include "lobbyComponent.h"
#include "groupComponent.h"

#include "eventLanguage.h"

struct Event
{
	virtual bool ReadOnly() const = 0;
	virtual EEventType GetType() const = 0;
	virtual void CreateJson() = 0;
	virtual std::string ToDebuggable() const = 0;

	bool waitForResponse;
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
		this->waitForResponse = false;
		fromAPI = false;

		this->eOffender = eOffender;
		this->eType = eType;
		this->geType = geType;
		this->message = message;
		this->channelId = channelId;
	}
	bool ReadOnly() const { return false; }
	EEventType GetType() const { return EError; }
	void CreateJson() { }
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
		this->waitForResponse = false;
		fromAPI = true;
	}
	bool ReadOnly() const { return true; }
	EEventType GetType() const { return EEmpty; }
	void CreateJson() {	}
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
		this->waitForResponse = false;
		fromAPI = false;
	}
	bool ReadOnly() const { return false; }
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
		this->waitForResponse = false;
		this->fromAPI = fromAPI;

		this->channelId = channelId;
		this->guildId = guildId;
		this->group = group;
		CreateJson();
	}
	bool ReadOnly() const { return true; }
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
		this->waitForResponse = true;
		this->fromAPI = fromAPI;
		this->method = method;
		this->type = type;

		this->channelId = channelId;
		this->guildId = guildId;
		this->group = group;
		CreateJson();
	}
	bool ReadOnly() const { return false; }
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
		this->waitForResponse = true;
		this->fromAPI = fromAPI;
		this->method = method;
		this->type = type;
		
		this->channelId = channelId;
		this->guildId = guildId;
		this->channel = channel;
		CreateJson();
	}
	bool ReadOnly() const { return false; }
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
		this->waitForResponse = true;
		this->fromAPI = fromAPI;
		this->method = method;
		this->type = type;

		this->channelId = channelId;
		this->guildId = guildId;
		this->channel = channel;
		CreateJson();
	}
	bool ReadOnly() const { return false; }
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
		this->waitForResponse = false;
		this->fromAPI = fromAPI;
		this->method = method;
		this->type = type;

		this->channelId = channelId;
		this->guildId = guildId;
		this->channel = channel;
		CreateJson();
	}
	bool ReadOnly() const { return true; }
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
		this->waitForResponse = true;
		this->fromAPI = fromAPI;
		this->method = method;
		this->type = type;

		this->channelId = channelId;
		this->guildId = guildId;
		this->category = category;
		CreateJson();
	}
	bool ReadOnly() const { return false; }
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

struct DeleteCategoryEvent : public Event
{
	DeleteCategoryEvent(bool deletionQueued, std::string method, std::string type, std::string channelId, std::string guildId, GroupComponent channel)
	{
		this->waitForResponse = deletionQueued;
		this->fromAPI = false;
		this->method = method;
		this->type = type;

		this->deletionQueued = deletionQueued;
		this->channelId = channelId;
		this->guildId = guildId;
		this->category = channel;
		CreateJson();
	}
	bool ReadOnly() const { return false; }
	EEventType GetType() const { return EDeleteCategory; }
	void CreateJson()
	{
		this->content = { };
	}
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << EDeleteCategory << ';' << category.id << '}';
		return os.str();
	}

	GroupComponent category;
	bool deletionQueued;
};

struct MoveCategoryEvent : public Event
{
	MoveCategoryEvent(bool fromAPI, std::string channelId, std::string guildId, GroupComponent category)
	{
		this->waitForResponse = false;
		this->fromAPI = fromAPI;
		this->method = "";
		this->type = "";

		this->channelId = channelId;
		this->guildId = guildId;
		this->category = category;
		CreateJson();
	}
	bool ReadOnly() const { return true; }
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

struct MoveUserEvent : public Event
{
	MoveUserEvent(bool fromAPI, std::string method, std::string type, std::string channelId, std::string guildId, GroupComponent voiceChannel)
	{
		this->waitForResponse = false;
		this->fromAPI = fromAPI;
		this->method = method;
		this->type = type;

		this->channelId = channelId;
		this->guildId = guildId;
		this->voiceChannel = voiceChannel;
		CreateJson();
	}
	bool ReadOnly() const { return false; }
	EEventType GetType() const { return EMoveUser; }
	void CreateJson()
	{
		this->content = { 
			{ "channel_id", voiceChannel.id }
		};
	}
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << EMoveUser << ';' << channelId  << ';' << guildId << ';' << voiceChannel.id << ';' << voiceChannel.userIds[0]  << '}';
		return os.str();
	}

	GroupComponent voiceChannel;
};

struct CreateMatchEvent : public Event
{
	CreateMatchEvent(std::string channelId, std::string guildId,
		int creationStep, std::string matchId, std::string matchName, int userCount, std::vector<std::string> userIds)
	{
		this->waitForResponse = false;
		this->fromAPI = false;
		this->method = "";
		this->type = "";

		this->channelId = channelId;
		this->guildId = guildId;
		this->creationStep = creationStep;
		this->matchId = matchId;
		this->matchName = matchName;
		this->userCount = userCount;
		this->userIds = userIds;
		CreateJson();
	}

	bool ReadOnly() const { return true; }
	EEventType GetType() const { return ECreateMatch; }
	void CreateJson()
	{
		this->content = { };
	}
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << ECreateMatch << ';' << channelId  << ';' << guildId << ';' << creationStep << ';'<< matchId << ';'<< matchName << ';' << userCount << '}';
		return os.str();
	}

	int creationStep;
	std::string matchId;
	std::string matchName;
	int userCount;
	std::vector<std::string> userIds;
};

#endif
