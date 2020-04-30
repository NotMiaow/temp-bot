#ifndef EVENT_LANGUAGE_H__
#define	EVENT_LANGUAGE_H__

#include <map>

enum EOffender
{
	ESystem,
	EUser
};

enum EEventType
{
	EError,
	EEmpty,
	EShutdown,
	ECreateEvent,
	ENewChannelEvent,
	ECreateVoiceChannel,
	EUpdateVoiceChannel,
	EDeleteChannel
};

enum EErrorType
{
	EWrongEventType,
	EForbidden,
	EWrongParemeterAmount,
	EWrongParameterType
};

static const std::map<std::string, EEventType> optionEventTypes {
	{ "ErrorEvent", EError },
	{ "EmptyEvent", EEmpty },
	{ "CreateEvent", ECreateEvent },
	{ "SetChannelId", ENewChannelEvent },
	{ "shutdown", EShutdown },
	{ "create-voice-channel", ECreateVoiceChannel },
	{ "update-voice-channel", EUpdateVoiceChannel },
	{ "remove-channel", EDeleteChannel }
};

#endif