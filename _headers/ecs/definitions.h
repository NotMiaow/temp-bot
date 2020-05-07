#ifndef DEFINITIONS_H__
#define DEFINITIONS_H__

#include <vector>

#include "lobbyComponent.h"
#include "preparationComponent.h"
#include "groupComponent.h"
#include "roleComponent.h"
#include "queueComponent.h"

#include "componentArray.h"

typedef ComponentArray<LobbyComponent, 20, 1> Lobbies;
typedef ComponentArray<PreparationComponent, 20, 1> Preparations;
typedef ComponentArray<GroupComponent, 80, 3> Groups;
typedef ComponentArray<RoleComponent, 60, 1> Roles;
typedef ComponentArray<QueueComponent, 2, 1> Queues;

typedef Lobbies::Iterator LobbiesIterator;
typedef Preparations::Iterator PreparationsIterator;
typedef Groups::Iterator GroupsIterator;
typedef Roles::Iterator RoleIterator;
typedef Queues::Iterator QueueIterator;

enum LobbyCheckpoint { LOBBY_MATCHES = 0, LOBBY_CHECKPOINTS_SIZE };
enum PreparationCheckpoint { PREPARATION_MATCHES = 0, PREPARATION_CHECKPOINTS_SIZE };
enum GroupCheckpoint { GROUP_MATCH_CATEGORIES = 0, GROUP_TEXT_CHANNELS, GROUP_VOICE_CHANNELS, GROUP_CHECKPOINTS_SIZE };
enum QueueCheckpoint { QUEUE_LEAGUE_OF_LEGENDS = 0, QUEUE_CHECKPOINTS_SIZE };

//LOL queues
enum LeagueQueueType { LEAGUE_S_Rank, LEAGUE_SRAM };

static const std::map<std::string, LeagueQueueType> leagueQueueTypes {
//	{ "srank", LEAGUE_S_Rank },
	{ "sram", LEAGUE_SRAM }
};

#endif