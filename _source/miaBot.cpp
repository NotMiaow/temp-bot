#include "miaBot.h"

MiaBot::MiaBot(SharedQueue<Event*>& eventQueue)
{
    m_userGroups.Initialise({20});

    m_eventQueue = &eventQueue;
    m_eventManager.Init(eventQueue, m_entityCounter, m_userGroups);
    m_ecs.Init(eventQueue, m_entityCounter, m_userGroups);
}

MiaBot::~MiaBot()
{

}

void MiaBot::Loop()
{
    m_ecs.Loop();
}

void MiaBot::HandleCommand(std::string command, std::string content, std::string channelId, std::string guildId)
{
    Event* event = CreateEvent(command, content, channelId, guildId);
    if(m_eventManager.HandleEvent(event))
        m_eventQueue->push_back(event);
    else delete event;
}