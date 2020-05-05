#ifndef DEFINITIONS_H__
#define DEFINITIONS_H__

#include "lobbyComponent.h"
#include "preparationComponent.h"
#include "groupComponent.h"

#include "componentArray.h"

typedef ComponentArray<LobbyComponent, 20, 1> Lobbies;
typedef ComponentArray<PreparationComponent, 20, 1> Preparations;
typedef ComponentArray<GroupComponent, 80, 3> Groups;

typedef Lobbies::Iterator LobbiesIterator;
typedef Preparations::Iterator PreparationsIterator;
typedef Groups::Iterator GroupsIterator;

enum LobbyCheckpoint { LOBBY_MATCHES = 0, LOBBY_CHECKPOINTS_SIZE };
enum PreparationCheckpoint { PREPARATION_MATCHES = 0, PREPARATION_CHECKPOINTS_SIZE };
enum GroupCheckpoint { GROUP_MATCH_CATEGORIES = 0, GROUP_TEXT_CHANNELS, GROUP_VOICE_CHANNELS, GROUP_CHECKPOINTS_SIZE };

#endif