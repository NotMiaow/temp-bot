#ifndef DEFINITIONS_H__
#define DEFINITIONS_H__

#include "groupComponent.h"

#include "componentArray.h"

//typedef ComponentArray<OffenseComponent, 476, 8> Offenses;
typedef ComponentArray<GroupComponent, 50, 3> Groups;

typedef Groups::Iterator GroupsIterator;

enum GroupCheckpoint { GROUP_CATEGORIES = 0, GROUP_TEXT_CHANNELS, GROUP_VOICE_CHANNELS, GROUP_CHECKPOINTS_SIZE };

#endif