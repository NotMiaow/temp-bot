#pragma once

#include <iostream>
#include <fstream>
#include <string>

#include <nlohmann/json.hpp>

#include "event.h"
#include "shared_queue.h"
#include "eventLanguage.h"
#include "eventTranslator.h"

// Components
#include "lobbyComponent.h"
#include "preparationComponent.h"
#include "groupComponent.h"
#include "queueComponent.h"

// ECS
#include "ecs.h"
#include "eventManager.h"
#include "entityCounter.h"

class MiaBot
{
public:
    MiaBot(SharedQueue<Event*>& eventQueue, SharedQueue<Event*>& robotQueue, SharedQueue<Event*>& humanQueue);
    ~MiaBot();
    void Loop(const float& deltaTime);
    void QueueCommand(bool fromAPI, std::string command, std::string content, std::string channelId, std::string guildId);
    bool HandleEvent(Event* event);
private:
    // Shared resources
    SharedQueue<Event*>* m_eventQueue;
    SharedQueue<Event*>* m_robotQueue;
    SharedQueue<Event*>* m_humanQueue;

    // Components
    Lobbies m_lobbies;
    Preparations m_preparations;
    Groups m_groups;
    Queues m_queues;

	// Misc
	EntityCounter m_entityCounter;
	EventManager m_eventManager;
	ECS m_ecs;
};
