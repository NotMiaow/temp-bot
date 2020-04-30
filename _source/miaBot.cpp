#include "miaBot.h"

MiaBot::MiaBot(SharedQueue<Event*>& eventQueue, SharedQueue<Event*>& robotQueue, SharedQueue<Event*>& humanQueue)
{
    m_userGroups.Initialise({20});

    m_eventQueue = &eventQueue;
    m_robotQueue = &robotQueue;
    m_humanQueue = &humanQueue;
    m_eventManager.Init(*m_robotQueue, m_entityCounter, m_userGroups);
    m_ecs.Init(*m_robotQueue, m_entityCounter, m_userGroups);
}

MiaBot::~MiaBot()
{

}

void MiaBot::Loop()
{
    m_ecs.Loop();
}

void MiaBot::QueueCommand(bool fromAPI, std::string command, std::string content, std::string channelId, std::string guildId)
{
    Event* event = CreateEvent(fromAPI, command, content, channelId, guildId);
    if(fromAPI)
        m_eventQueue->push_back(event);
    else
    {
        m_humanQueue->push_back(event);
    }
}

bool MiaBot::HandleEvent(Event* event)
{
	std::cout << "handle : " << event->ToDebuggable() << std::endl;
    return m_eventManager.HandleEvent(event);
}