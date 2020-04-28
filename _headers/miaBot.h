#pragma once

#include <iostream>
#include <fstream>
#include <string>

#include <nlohmann/json.hpp>

#include "shared_queue.h"
#include "event.h"
#include "eventLanguage.h"
#include "eventTranslator.h"

class MiaBot
{
public:
    MiaBot(SharedQueue<Event*>& eventQueue);
    ~MiaBot();
    void Loop();
    void NewMessage(std::string content, std::string guildId);
    void SwitchMessage();
private:
    SharedQueue<Event*>* m_eventQueue;
};
