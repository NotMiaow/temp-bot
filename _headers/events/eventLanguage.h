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
	ENewGroup,
	EUpdateGroup,
	ECreateChannel,
	EDeleteChannel,
	EMoveChannel,
	ECreateCategory,
	EDeleteCategory,
	EMoveCategory,
	EMoveUser,
	ECreateMatch
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
	{ "new-group", ENewGroup },
	{ "update-group", EUpdateGroup },
	{ "create-channel", ECreateChannel },
	{ "delete-channel", EDeleteChannel },
	{ "move-channel", EMoveChannel },
	{ "create-category", ECreateCategory },
	{ "delete-category", EDeleteCategory },
	{ "move-category", EMoveCategory },
	{ "move-user", EMoveUser},
	{ "create-match", ECreateMatch }
};

#endif