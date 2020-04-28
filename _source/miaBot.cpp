#include "miaBot.h"

MiaBot::MiaBot(SharedQueue<Event*>& eventQueue)
{
    m_eventQueue = &eventQueue;
}

MiaBot::~MiaBot()
{

}

void MiaBot::Loop()
{

}

void MiaBot::HandleCommand(std::string command, std::string content, std::string guildId)
{
    Event* event = CreateEvent(command, content, guildId);
    if(event->GetType() != EError)
        m_eventQueue->push_back(event);
}