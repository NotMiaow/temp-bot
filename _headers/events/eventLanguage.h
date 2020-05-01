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
	ECreateChannel,
	EUpdateChannel,
	EDeleteChannel,
	EMoveChannel,
	ECreateCategory
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
	{ "shutdown", EShutdown },
	{ "new-channel", ENewChannelEvent },
	{ "create-channel", ECreateChannel },
	{ "update-channel", EUpdateChannel },
	{ "delete-channel", EDeleteChannel },
	{ "move-channel", EMoveChannel },
	{ "create-category", ECreateCategory }
};

#endif