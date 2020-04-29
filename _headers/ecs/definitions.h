#ifndef DEFINITIONS_H__
#define DEFINITIONS_H__

#include "userGroupComponent.h"

#include "componentArray.h"

//typedef ComponentArray<OffenseComponent, 476, 8> Offenses;
typedef ComponentArray<UserGroupComponent, 20, 1> UserGroups;

typedef UserGroups::Iterator UserGroupsIterator;

enum UserGroupsCheckpointIndexes { USER_GROUP_VOICE_CHANNELS = 0, USER_GROUPS_CHECKPOINTS_SIZE };

enum ComponentTypes { TUserGroup = 0 };
static int GetCheckpoint(const ComponentTypes& componentType, const int& checkpointIndex)
{
    switch (componentType)
    {
    case TUserGroup:    return checkpointIndex;
    default:        return -1;
    }
}

#endif