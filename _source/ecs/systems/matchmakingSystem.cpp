#include "matchmakingSystem.h"

MatchmakingSystem::MatchmakingSystem(SharedQueue<Event*>& robotQueue, Queues& queues)
{
    m_queues = &queues;

    m_robotQueue = &robotQueue;

    float waitTimer = 0.0f;
}

void MatchmakingSystem::Loop(const float& deltaTime)
{
    waitTimer += deltaTime;
    if(waitTimer > 0.5f)
    {
        waitTimer = 0.0f;
        for(QueueIterator matchmakingIt = m_queues->GetIterator(QUEUE_LEAGUE_OF_LEGENDS); !matchmakingIt.End(); matchmakingIt++)
        {
            QueueComponent* queue = matchmakingIt.GetData();
            SwitchQueuePosition(deltaTime, *queue);
        }
    }
}

void MatchmakingSystem::SwitchQueuePosition(const float& deltaTime, QueueComponent& queue)
{
    switch (queue.type)
    {
    case LEAGUE_S_Rank:
        SRankQueue(deltaTime, queue);
        break;
    case LEAGUE_SRAM:
        SramQueue(deltaTime, queue);
        break;
    default:
        break;
    }
}

void MatchmakingSystem::SRankQueue(const float& deltaTime, QueueComponent& queue)
{
    std::cout << "srank empty" << std::endl;
}

void MatchmakingSystem::SramQueue(const float& deltaTime, QueueComponent& queue)
{
    std::cout << "sram empty" << std::endl;
}