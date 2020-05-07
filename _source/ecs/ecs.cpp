#include "ecs.h"

ECS::~ECS()
{

}

void ECS::Init(SharedQueue<Event*>& robotQueue, EntityCounter& entityCounter, Lobbies& lobbies, Preparations& preparations, Groups& groups)
{
    //Components
    m_lobbies = &lobbies;
    m_preparations = &preparations;
    m_groups = &groups;

    m_robotQueue = &robotQueue;
    m_entityCounter = &entityCounter;

    //Systems
}

void ECS::Loop(const float& deltaTime)
{
    
}

