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
#include "userGroupComponent.h"

// ECS
#include "ecs.h"
#include "eventManager.h"
#include "entityCounter.h"

class MiaBot
{
public:
    MiaBot(SharedQueue<Event*>& eventQueue);
    ~MiaBot();
    void Loop();
    void HandleCommand(std::string command, std::string content, std::string channelId, std::string guildId);
private:
    // Shared resources
    SharedQueue<Event*>* m_eventQueue;

    // Components
    UserGroups m_userGroups;

	// Misc
	EntityCounter m_entityCounter;
	EventManager m_eventManager;
	ECS m_ecs;
};
