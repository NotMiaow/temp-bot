#ifndef ECS_H__
#define ECS_H__

#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include <atomic>

#include "event.h"
#include "eventManager.h"

#include "entityCounter.h"
#include "componentArray.h"
//Components
#include "lobbyComponent.h"
#include "preparationComponent.h"
#include "groupComponent.h"
#include "queueComponent.h"


// Systems
#include "matchmakingSystem.h"

// Misc
#include "definitions.h"

class ECS
{
public:
	ECS() {}
	~ECS();
	void Init(SharedQueue<Event*>& robotQueue, EntityCounter& entityCounter, Lobbies& lobbies, Preparations& preparations, Groups& groups, Queues& queues);
	void Loop(const float& deltaTime);
private:
	void WaitForTerminate();
private:
	//Components
	Lobbies* m_lobbies;
	Preparations* m_preparations;
	Groups* m_groups;
	Queues* m_queues;
	
	//Shared resources
	EntityCounter* m_entityCounter;
	SharedQueue<Event*>* m_robotQueue;

	//Systems
	MatchmakingSystem m_matchmakingSystem;
};


#endif
