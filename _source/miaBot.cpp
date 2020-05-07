#include "miaBot.h"

MiaBot::MiaBot(SharedQueue<Event*>& eventQueue, SharedQueue<Event*>& robotQueue, SharedQueue<Event*>& humanQueue)
{
    m_lobbies.Initialise({20});
    m_preparations.Initialise({20});
    m_groups.Initialise({20,20,40});

    m_eventQueue = &eventQueue;
    m_robotQueue = &robotQueue;
    m_humanQueue = &humanQueue;
    m_eventManager.Init(*m_robotQueue, m_entityCounter, m_lobbies, m_preparations, m_groups);
    m_ecs.Init(*m_robotQueue, m_entityCounter, m_lobbies, m_preparations, m_groups);
}

MiaBot::~MiaBot()
{

}

void MiaBot::Loop(const float& deltaTime)
{
    m_ecs.Loop(deltaTime);
}

void MiaBot::QueueCommand(bool fromAPI, std::string command, std::string content, std::string channelId, std::string guildId)
{
    Event* event = CreateEvent(fromAPI, command, content, channelId, guildId);
    if(fromAPI)
        m_eventQueue->push_back(event);
    else
        m_humanQueue->push_back(event);
}

bool MiaBot::HandleEvent(Event* event)
{
	std::cout << "handle : " << event->ToDebuggable() << std::endl;
    return m_eventManager.HandleEvent(event);
}