#ifndef EVENT_H__
#define	EVENT_H__

#include <iostream>
#include <string>
#include <sstream>

#include <nlohmann/json.hpp>

#include "eventLanguage.h"

struct Event
{
	virtual EEventType GetType() const = 0;
	virtual std::string ToDebuggable() const = 0;

    std::string method;
    std::string type;
    nlohmann::json content;
};

struct ErrorEvent : public Event
{
	ErrorEvent(EEventType eType, EErrorType geType)
	{
		this->eType = eType;
		this->geType = geType;
	}
	EEventType GetType() const { return EError; }
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << EError << ';' << eType << ';' << geType << '}';
		return os.str();
	}

	EEventType eType;
	EErrorType geType;
};

struct CreateChannelEvent : public Event
{
	CreateChannelEvent(std::string method, std::string type, std::string guildId, nlohmann::json content)
	{
		this->method = method;
		this->type = type;
		this->content = content;
		this->guildId = guildId;
	}
	EEventType GetType() const { return ECreateVoiceChannel; }
	std::string ToDebuggable() const
	{
		std::ostringstream os;
		os << '{' << ECreateVoiceChannel << ';' << guildId << ';' << content["name"].get<std::string>() << '}';
		return os.str();
	}

	std::string guildId;
};

#endif
