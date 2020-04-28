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

void MiaBot::NewMessage(std::string content, std::string guildId)
{
    Event* event = CreateEvent(content, guildId);
    if(event->GetType() != EError)
        m_eventQueue->push_back(event);
}

void MiaBot::SwitchMessage() {

}