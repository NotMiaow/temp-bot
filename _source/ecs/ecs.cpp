#include "ecs.h"

ECS::~ECS()
{

}

void ECS::Init(SharedQueue<Event*>& robotQueue, EntityCounter& entityCounter, Lobbies& lobbies, Preparations& preparations, Groups& groups, Queues& queues)
{
    //Components
    m_lobbies = &lobbies;
    m_preparations = &preparations;
    m_groups = &groups;
    m_queues = &queues;

    m_robotQueue = &robotQueue;
    m_entityCounter = &entityCounter;

    //Systems
    m_matchmakingSystem = MatchmakingSystem(*m_robotQueue, *m_queues);
}

void ECS::Loop(const float& deltaTime)
{
    m_matchmakingSystem.Loop(deltaTime);
}

