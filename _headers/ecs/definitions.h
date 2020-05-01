#ifndef DEFINITIONS_H__
#define DEFINITIONS_H__

#include "userGroupComponent.h"

#include "componentArray.h"

//typedef ComponentArray<OffenseComponent, 476, 8> Offenses;
typedef ComponentArray<UserGroupComponent, 40, 2> UserGroups;

typedef UserGroups::Iterator UserGroupsIterator;

enum UserGroupsCheckpointIndexes { USERGROUP_TEXT_CHANNELS = 0, USER_GROUP_VOICE_CHANNELS, USER_GROUPS_CHECKPOINTS_SIZE };

#endif