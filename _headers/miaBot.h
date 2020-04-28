#pragma once

#include <iostream>
#include <fstream>
#include <string>

#include <nlohmann/json.hpp>

#include "event.h"
#include "shared_queue.h"
#include "eventLanguage.h"
#include "eventTranslator.h"

#include "ecs.h"

class MiaBot
{
public:
    MiaBot(SharedQueue<Event*>& eventQueue);
    ~MiaBot();
    void Loop();
    void HandleCommand(std::string command, std::string content, std::string guildId);
private:
    SharedQueue<Event*>* m_eventQueue;
};
