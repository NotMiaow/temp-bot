#ifndef EVENT_H__
#define	EVENT_H__

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <nlohmann/json.hpp>

// Components
#include "preparationComponent.h"
#include "lobbyComponent.h"
#include "groupComponent.h"
#include "roleComponent.h"

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
		os << '{' << "Error" << ';' << message << ';' << eOffender  << ';' << eType << ';' << geType << '}';
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
		os << '{' << "Empty" << '}';
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
		os << '{' << "shutdown" << '}';
		return os.str();
	}
};

struct SendMessageEvent : public Event
{
	SendMessageEvent(std::string message, std::string channelId)
	{
		this->waitForResponse = false;
		fromAPI = false;

		this->message = message;
		this->channelId = channelId;
	}
	bool ReadOnly() const { return false; }
	EEventType GetType() const { return ESendMessage; }
	void CreateJson() { }
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << "send-message" << ';' << channelId << ';' << message << '}';
		return os.str();
	}

	std::string message;
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
		os << '{' << "new-group" << ';' << group.name << ';';
		if(group.type != 4)
			os << group.parentId << ';';
		os << group.id << ';' << group.position  << '}';
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
		os << '{' << "update-group" << ';' << group.id << ';' << group.type << ';' << group.position << '}';
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
		os << '{' << "create-channel" << ';' << channel.name << ';' << channel.type << ';' << channel.parentId <<  ';' << channel.position;
		if(channel.type == 2)
			os << ';' << channel.userLimit;
		os << '}';
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
		os << '{' << "delete-channel" << ';' << channel.id << '}';
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
		os << '{' << "move-channel" << ';' << channel.id << ';' << channel.position << '}';
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
		os << '{' << "create-category" << ';' << category.name << ';' << category.position << '}';
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
		os << '{' << "delete-category" << ';' << category.id << '}';
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
		os << '{' << "move-category" << ';' << category.id << ';' << category.position << '}';
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
		os << '{' << "move-user" << ';' << voiceChannel.id << ';' << voiceChannel.userIds[0]  << '}';
		return os.str();
	}

	GroupComponent voiceChannel;
};

struct CreateMatchEvent : public Event
{
	CreateMatchEvent(std::string channelId, std::string guildId,
		int creationStep, int entityId, std::string matchId, std::string matchName, int userCount, std::vector<std::string> userIds)
	{
		this->waitForResponse = false;
		this->fromAPI = false;
		this->method = "";
		this->type = "";

		this->channelId = channelId;
		this->guildId = guildId;
		this->creationStep = creationStep;
		this->entityId = entityId;
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
		os << '{' << "create-match" << ';' << creationStep << ';' << entityId << ';'<< matchId << ';'<< matchName << ';' << userCount << ';';
		std::vector<std::string>::const_iterator userIt = userIds.begin();
		for(int i = 0; i < userCount; userIt++, i++)
			os << *userIt  << (i + 1 < userCount ? ";" : "");
		os << '}';
		return os.str();
	}

	int creationStep;
	int entityId;
	std::string matchId;
	std::string matchName;
	int userCount;
	std::vector<std::string> userIds;
};

struct ChangeGroupPermissionsEvent : public Event
{
	ChangeGroupPermissionsEvent(bool fromAPI, std::string method, std::string type, std::string channelId, std::string guildId,
		bool give, GroupComponent group, int permissions, int userCount, std::vector<std::string> userIds)
	{
		this->waitForResponse = true;
		this->fromAPI = fromAPI;
		this->method = method;
		this->type = type;
		
		this->channelId = channelId;
		this->guildId = guildId;

		this->give = give;
		this->group = group;
		this->userCount = userCount;
		this->userIds = userIds;
		this->permissions = permissions;
		CreateJson();
	}
	bool ReadOnly() const { return false; }
	EEventType GetType() const { return EChangeGroupPermissions; }
	void CreateJson()
	{
		nlohmann::json overwrites;

		std::vector<std::string>::const_iterator userIt = userIds.begin();
		for(int i = 0; i < userCount; userIt++, i++)
		{
			nlohmann::json overwrite = {
				{ "id", userIds[i] },
				{ "type", "member" },
				{ "allow", permissions },
				{ "deny", 0 }
			};
			overwrites.push_back(overwrite);
		}

		this->content = {
			{ "name", group.name },
			{ "type", group.type },
			{ "permission_overwrites", overwrites }
	    };
	}
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << "change-group-permissions" << ';' << give << ';' << group.id << ';' << group.name << ';' << group.type << ';'
		<< permissions << ';' << userIds.size() << ';';
		std::vector<std::string>::const_iterator userIt = userIds.begin();
		for(int i = 0; i < userCount; userIt++, i++)
			os << *userIt  << (i + 1 < userCount ? ";" : "");
		os << '}';
		return os.str();
	}

	bool give;
	GroupComponent group;
	int permissions;
	int userCount;
	std::vector<std::string> userIds;
};

struct SetMatchVoicePermissionsEvent : public Event
{
	SetMatchVoicePermissionsEvent(bool fromAPI, std::string method, std::string type, std::string channelId, std::string guildId,
		bool give, GroupComponent group, int permissions1, int permissions2, int userCount, std::vector<std::string> userIds)
	{
		this->waitForResponse = true;
		this->fromAPI = fromAPI;
		this->method = method;
		this->type = type;
		
		this->channelId = channelId;
		this->guildId = guildId;

		this->give = give;
		this->group = group;
		this->userCount = userCount;
		this->userIds = userIds;
		this->permissions1 = permissions1;
		this->permissions2 = permissions2;
		CreateJson();
	}
	bool ReadOnly() const { return false; }
	EEventType GetType() const { return ESetMatchVoicePermissions; }
	void CreateJson()
	{
		nlohmann::json overwrites;

		std::vector<std::string>::const_iterator userIt = userIds.begin();
		for(int i = 0; i < userCount; userIt++, i++)
		{
			nlohmann::json overwrite;
			if(i < userCount / 2)
			{
				overwrite = {
					{ "id", userIds[i] },
					{ "type", "member" },
					{ "allow", permissions1 },
					{ "deny", 0 }
				};
			}
			else
			{
				overwrite = {
					{ "id", userIds[i] },
					{ "type", "member" },
					{ "allow", permissions2 },
					{ "deny", 0 }
				};
			}
			overwrites.push_back(overwrite);
		}

		this->content = {
			{ "name", group.name },
			{ "type", group.type },
			{ "permission_overwrites", overwrites }
	    };
		std::cout << std::endl << std::endl << this->content << std::endl << std::endl;
	}
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << "set-match-voice-permissions" << ';' << give << ';' << group.id << ';' << group.name << ';' << group.type << ';'
		<< permissions1 << ';' << permissions2 << ';' << userIds.size() << ';';
		std::vector<std::string>::const_iterator userIt = userIds.begin();
		for(int i = 0; i < userCount; userIt++, i++)
			os << *userIt  << (i + 1 < userCount ? ";" : "");
		os << '}';
		return os.str();
	}

	bool give;
	GroupComponent group;
	int permissions1;
	int permissions2;
	int userCount;
	std::vector<std::string> userIds;
};

struct MessageMatchEvent : public Event
{
	MessageMatchEvent(int entityId, std::string message)
	{
		this->waitForResponse = false;
		fromAPI = false;

		this->message = message;
		this->entityId = entityId;
	}
	bool ReadOnly() const { return false; }
	EEventType GetType() const { return ESendMessage; }
	void CreateJson() { }
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << "send-message" << ';' << entityId << ';' << message << '}';
		return os.str();
	}

	int entityId;
	std::string message;
};


struct JoinQueueEvent : public Event
{
	JoinQueueEvent(std::string channelId, std::string guildId, std::string queueName, std::string userId)
	{
		this->waitForResponse = false;
		this->fromAPI = false;
		this->method = "";
		this->type = "";

		this->channelId = channelId;
		this->guildId = guildId;
		this->queueName = queueName;
		this->userId = userId;
		CreateJson();
	}
	bool ReadOnly() const { return true; }
	EEventType GetType() const { return EJoinQueue; }
	void CreateJson() { }
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << "join-queue" << ';' << channelId << ';' << queueName << ';' << userId  << '}';
		return os.str();
	}

	std::string queueName;
	std::string userId;
};


#endif
