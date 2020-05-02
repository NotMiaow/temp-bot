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
	EUpdateCategory,
	EMoveCategory
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
	{ "move-category", EMoveCategory }
};

#endif