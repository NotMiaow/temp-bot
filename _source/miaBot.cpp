#include "miaBot.h"

MiaBot::MiaBot(SharedQueue<Event*>& eventQueue, SharedQueue<Event*>& robotQueue, SharedQueue<Event*>& humanQueue)
{
    m_lobbies.Initialise({20});
    m_preparations.Initialise({20});
    m_groups.Initialise({20,20,40});
    m_queues.Initialise({2});

    QueueComponent leaguePremadeS;
    leaguePremadeS.name = "srank";
    leaguePremadeS.type = 0;
    leaguePremadeS.fairnessLevel = 2;
    leaguePremadeS.startTreshold = 10;
    leaguePremadeS.up = false;
    QueueComponent leagueSRAM;
    leagueSRAM.name = "sram";
    leagueSRAM.type = 1;
    leagueSRAM.fairnessLevel = 1;
    leagueSRAM.startTreshold = 10;
    leagueSRAM.up = true;
    m_queues.Add(leaguePremadeS, m_entityCounter.GetId(), QUEUE_LEAGUE_OF_LEGENDS);
    m_queues.Add(leagueSRAM, m_entityCounter.GetId(), QUEUE_LEAGUE_OF_LEGENDS);

    m_eventQueue = &eventQueue;
    m_robotQueue = &robotQueue;
    m_humanQueue = &humanQueue;
    m_eventManager.Init(*m_robotQueue, m_entityCounter, m_lobbies, m_preparations, m_groups, m_queues);
    m_ecs.Init(*m_robotQueue, m_entityCounter, m_lobbies, m_preparations, m_groups, m_queues);
}

MiaBot::~MiaBot()
{

}

void MiaBot::Loop(const float& deltaTime)
{
    m_ecs.Loop(deltaTime);
}

void MiaBot::QueueCommand(EventInfo info, std::string command, std::string content)
{
    Event* event = CreateEvent(info, command, content);
    if(info.fromAPI)
        m_eventQueue->push_back(event);
    else
        m_humanQueue->push_back(event);
}

bool MiaBot::HandleEvent(Event* event)
{
	std::cout << "handle : " << event->ToDebuggable() << std::endl;
    return m_eventManager.HandleEvent(event);
}