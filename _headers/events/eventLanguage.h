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
	EShutdown,
	ECreateEvent,
	ESetChannelId,
	ECreateVoiceChannel,
	EUpdateVoiceChannel
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
	{ "CreateEvent", ECreateEvent },
	{ "SetChannelId", ESetChannelId },
	{ "shutdown", EShutdown },
	{ "create-voice-channel", ECreateVoiceChannel }
};

#endif