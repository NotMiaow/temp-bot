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
    m_eventQueue->push_back(CreateEvent(command, content, guildId));
}