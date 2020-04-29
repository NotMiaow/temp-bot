#ifndef EVENT_LANGUAGE_H__
#define	EVENT_LANGUAGE_H__

#include <map>

enum EEventType
{
	EError,
	EShutdown,
	ECreateEvent,
	ECreateVoiceChannel
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
	{ "shutdown", EShutdown },
	{ "create-voice-channel", ECreateVoiceChannel }
};

#endif