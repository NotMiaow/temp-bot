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

using json = nlohmann::json;

struct EventInfo
{
	EventInfo() { }
	EventInfo(bool fromAPI, std::string method, std::string type,json content, std::string userId,
		std::string channelId, std::string guildId) :	
		fromAPI(fromAPI), method(method), type(type), content(content), userId(userId),
		channelId(channelId), guildId(guildId) { }
	~EventInfo() { }
	std::string ToDebuggable()
	{
		std::ostringstream os;
		os << '{' << "EventInfo" << ';' << fromAPI << ';' << method  << ';' << type << ';' <<
			content << ';' << userId << ';' << channelId << ';' << guildId << '}';
		return os.str();
	}
	
	bool fromAPI;
    std::string method;
    std::string type;
    json content;
	std::string userId;
	std::string channelId;
	std::string guildId;
};

struct Event
{
	virtual bool ReadOnly() const = 0;
	virtual EEventType GetType() const = 0;
	virtual void CreateJson() = 0;
	virtual std::string ToDebuggable() const = 0;

	EventInfo info;
	bool waitForResponse;
};

struct ErrorEvent : public Event
{
	ErrorEvent(EventInfo info, std::string message, EOffender eOffender, EEventType eType, EErrorType geType)
	{
		this->info = info;
		this->waitForResponse = false;

		this->message = message;
		this->eOffender = eOffender;
		this->eType = eType;
		this->geType = geType;
	}
	bool ReadOnly() const { return false; }
	EEventType GetType() const { return EError; }
	void CreateJson() { }
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << "error" << ';' << message << ';' << eOffender  << ';' << eType << ';' << geType << '}';
		return os.str();
	}

	std::string message;
	EOffender eOffender;
	EEventType eType;
	EErrorType geType;
};

struct EmptyEvent : public Event
{
	EmptyEvent()
	{
		json content = { };
		this->info = EventInfo(true, "", "", content, "", "", "");
		this->waitForResponse = false;
	}
	~EmptyEvent() { }
	bool ReadOnly() const { return true; }
	EEventType GetType() const { return EEmpty; }
	void CreateJson() {	}
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << "empty" << '}';
		return os.str();
	}
};

struct ShutdownEvent : public Event
{
	ShutdownEvent()
	{
		json content = { };
		this->info = EventInfo(false, "", "", content, "", "", "");
		this->waitForResponse = false;
	}
	bool ReadOnly() const { return false; }
	EEventType GetType() const { return EShutdown; }
	void CreateJson() {	}
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << "shutdown" << '}';
		return os.str();
	}
};

struct SendMessageEvent : public Event
{
	SendMessageEvent(EventInfo info, std::string message)
	{
		this->info = info;
		this->waitForResponse = false;

		this->message = message;
	}
	bool ReadOnly() const { return false; }
	EEventType GetType() const { return ESendMessage; }
	void CreateJson() { }
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << "send-message" << ';' << info.channelId << ';' << "[message]" << '}';
		return os.str();
	}

	std::string message;
};

struct NewGroupEvent : public Event
{
	NewGroupEvent(EventInfo info, GroupComponent group)
	{
		this->info = info;
		this->waitForResponse = false;

		this->group = group;
		CreateJson();
	}
	bool ReadOnly() const { return true; }
	EEventType GetType() const { return ENewGroup; }
	void CreateJson() {	this->info.content = { }; }
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << "new-group" << ';' << group.id << ';';
		if(group.type != 4)
			os << group.parentId << ';';
		os << group.name << ';' << group.type  << ';' << group.position << '}';
		return os.str();
	}
	GroupComponent group;
};

struct UpdateGroupEvent : public Event
{
	UpdateGroupEvent(EventInfo info, GroupComponent group)
	{
		this->info = info;
		this->waitForResponse = true;

		this->group = group;

		this->info.method = "PATCH";
		this->info.type = "/channels/" + group.id;
		CreateJson();
	}
	bool ReadOnly() const { return false; }
	EEventType GetType() const { return EUpdateGroup; }
	void CreateJson() {	this->info.content = { { "position", group.position } }; }
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
	CreateChannelEvent(EventInfo info, GroupComponent channel)
	{
		this->info = info;
		this->waitForResponse = true;

		this->channel = channel;
		this->info.method = "POST";
		this->info.type = "/guilds/" + info.guildId + "/channels";
		CreateJson();
	}
	bool ReadOnly() const { return false; }
	EEventType GetType() const { return ECreateChannel; }
	void CreateJson()
	{
		this->info.content = {
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
	DeleteChannelEvent(EventInfo info, std::string channelId)
	{
		this->info = info;
		this->waitForResponse = true;

		this->channelId = channelId;
		this->info.method = "DELETE";
		this->info.type = "/channels/" + this->channelId;
		CreateJson();
	}
	bool ReadOnly() const { return false; }
	EEventType GetType() const { return EDeleteChannel; }
	void CreateJson()
	{
		this->info.content = { };
	}
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << "delete-channel" << ';' << channelId << '}';
		return os.str();
	}

	std::string channelId;
};

struct MoveChannelEvent : public Event
{
	MoveChannelEvent(EventInfo info, GroupComponent channel)
	{
		this->info = info;
		this->waitForResponse = false;

		this->channel = channel;
		CreateJson();
	}
	bool ReadOnly() const { return true; }
	EEventType GetType() const { return EMoveChannel; }
	void CreateJson() { this->info.content = { }; }
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
	CreateCategoryEvent(EventInfo info, GroupComponent category)
	{
		this->info = info;
		this->waitForResponse = true;

		this->category = category;
		this->info.method = "POST";
		this->info.type = "/guilds/" + info.guildId + "/channels";
		CreateJson();
	}
	bool ReadOnly() const { return false; }
	EEventType GetType() const { return ECreateCategory; }
	void CreateJson()
	{
		this->info.content = {
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
	DeleteCategoryEvent(EventInfo info, bool deletionQueued, std::string categoryId)
	{
		this->info = info;
		this->waitForResponse = deletionQueued;
		this->deletionQueued = deletionQueued;

		this->categoryId = categoryId;
		this->info.method = "DELETE";
		this->info.type = "/channels/" + categoryId;
		CreateJson();
	}
	DeleteCategoryEvent(const DeleteCategoryEvent& other)
	{
		this->info = other.info;
		this->waitForResponse = other.deletionQueued;
		this->deletionQueued = other.deletionQueued;

		this->categoryId = other.categoryId;
		this->info.method = other.info.method;
		this->info.type = other.info.type;
		CreateJson();
	}
	bool ReadOnly() const { return false; }
	EEventType GetType() const { return EDeleteCategory; }
	void CreateJson() { this->info.content = { }; }
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << "delete-category" << ';' << categoryId << '}';
		return os.str();
	}

	bool deletionQueued;
	std::string categoryId;
};

struct MoveCategoryEvent : public Event
{
	MoveCategoryEvent(EventInfo info, GroupComponent category)
	{
		this->info = info;
		this->waitForResponse = false;

		this->category = category;
		CreateJson();
	}
	bool ReadOnly() const { return true; }
	EEventType GetType() const { return EMoveCategory; }
	void CreateJson() { this->info.content = { }; }
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
	MoveUserEvent(EventInfo info, std::string userId, std::string voiceChannelId)
	{
		this->info = info;
		this->waitForResponse = false;

		this->voiceChannelId = voiceChannelId;
		this->info.method = "PATCH";
		this->info.type = "/guilds/" + info.guildId + "/members/" + userId;
		CreateJson();
	}
	bool ReadOnly() const { return false; }
	EEventType GetType() const { return EMoveUser; }
	void CreateJson()
	{
		this->info.content = { 
			{ "channel_id", voiceChannelId }
		};
	}
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << "move-user" << ';' << userId << ';' << voiceChannelId << '}';
		return os.str();
	}

	std::string userId;
	std::string voiceChannelId;
};

struct CreateMatchEvent : public Event
{
	CreateMatchEvent(EventInfo info, int creationStep, int queueType, std::string matchId,
		std::string matchName, std::vector<std::string> userIds1, std::vector<std::string> userIds2)
	{
		this->info = info;
		this->waitForResponse = false;

		this->creationStep = creationStep;
		this->queueType = queueType;
		this->matchId = matchId;
		this->matchName = matchName;
		this->userCount = userCount;
		this->userIds1 = userIds1;
		this->userIds2 = userIds2;
		CreateJson();
	}
	CreateMatchEvent(const CreateMatchEvent& other)
	{
		this->info = other.info;
		this->waitForResponse = false;

		this->creationStep = other.creationStep;
		this->queueType = other.queueType;
		this->matchId = other.matchId;
		this->matchName = other.matchName;
		this->userCount = other.userCount;
		this->userIds1 = userIds1;
		this->userIds2 = userIds2;
	}
	bool ReadOnly() const { return true; }
	EEventType GetType() const { return ECreateMatch; }
	void CreateJson() { this->info.content = { }; }
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << "create-match" << ';' << creationStep << ';' << queueType << ';' << matchId << ';'<< matchName << ';' << userCount << '}';
		return os.str();
	}

	int creationStep;
	int queueType;
	std::string matchId;
	std::string matchName;
	int userCount;
	std::vector<std::string> userIds1;
	std::vector<std::string> userIds2;
};

struct ChangeGroupPermissionsEvent : public Event
{
	ChangeGroupPermissionsEvent(EventInfo info,	bool give, GroupComponent group, int permissions,
		std::vector<std::string> userIds)
	{
		this->info = info;
		this->waitForResponse = true;

		this->give = give;
		this->group = group;
		this->userIds = userIds;
		this->permissions = permissions;
		CreateJson();
	}
	bool ReadOnly() const { return false; }
	EEventType GetType() const { return EChangeGroupPermissions; }
	void CreateJson()
	{
		json overwrites;

		std::vector<std::string>::const_iterator userIt = userIds.begin();
		for(int i = 0; i < userIds.size(); userIt++, i++)
		{
			json overwrite = {
				{ "id", userIds[i] },
				{ "type", "member" },
				{ "allow", permissions },
				{ "deny", 0 }
			};
			overwrites.push_back(overwrite);
		}

		this->info.content = {
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
		for(int i = 0; i < userIds.size(); userIt++, i++)
			os << *userIt  << (i + 1 < userIds.size() ? ";" : "");
		os << '}';
		return os.str();
	}

	bool give;
	GroupComponent group;
	int permissions;
	std::vector<std::string> userIds;
};

struct SetMatchVoicePermissionsEvent : public Event
{
	SetMatchVoicePermissionsEvent(EventInfo info, bool give, GroupComponent group, int permissions1,
		int permissions2, std::vector<std::string> userIds1, std::vector<std::string> userIds2)
	{
		this->info = info;
		this->waitForResponse = true;

		this->give = give;
		this->group = group;
		this->permissions1 = permissions1;
		this->permissions2 = permissions2;
		this->userIds1 = userIds1;
		this->userIds2 = userIds2;
		CreateJson();
	}
	bool ReadOnly() const { return false; }
	EEventType GetType() const { return ESetMatchVoicePermissions; }
	void CreateJson()
	{
		json overwrites;
		for(std::vector<std::string>::const_iterator i = userIds1.begin(); i < userIds1.end(); i++, i++)
		{
			json overwrite =
			{
					{ "id", *i },
					{ "type", "member" },
					{ "allow", permissions1 },
					{ "deny", 0 }
			};
			overwrites.push_back(overwrite);
		}
		for(std::vector<std::string>::const_iterator i = userIds2.begin(); i < userIds2.end(); i++, i++)
		{
			json overwrite =
			{
					{ "id", *i },
					{ "type", "member" },
					{ "allow", permissions2 },
					{ "deny", 0 }
			};
			overwrites.push_back(overwrite);
		}

		this->info.content = {
			{ "name", group.name },
			{ "type", group.type },
			{ "permission_overwrites", overwrites }
	    };
		std::cout << std::endl << std::endl << this->info.content << std::endl << std::endl;
	}
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << "set-match-voice-permissions" << ';' << give << ';' << group.id << ';' << group.name << ';' << group.type << ';'
		<< permissions1 << ';' << permissions2 << ';' << "[userIds1]" << "[userIds2]" << '}';
		return os.str();
	}

	bool give;
	GroupComponent group;
	int permissions1;
	int permissions2;
	int userCount;
	std::vector<std::string> userIds1;
	std::vector<std::string> userIds2;
};

struct JoinQueueEvent : public Event
{
	JoinQueueEvent(EventInfo info, std::string queueName)
	{
		this->info = info;
		this->waitForResponse = false;

		this->queueName = queueName;
		CreateJson();
	}
	bool ReadOnly() const { return true; }
	EEventType GetType() const { return EJoinQueue; }
	void CreateJson() { }
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << "join-queue" << ';' << info.channelId << ';' << info.userId << ';' << queueName << '}';
		return os.str();
	}

	std::string queueName;
};

#endif
