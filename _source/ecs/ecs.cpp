#include "ecs.h"

ECS::~ECS()
{

}

void ECS::Init(SharedQueue<Event*>& eventQueue, EntityCounter& entityCounter, UserGroups& userGroups)
{
    //Components
    m_userGroups = &userGroups;

    m_eventQueue = &eventQueue;
    m_entityCounter = &entityCounter;

    //Systems
    m_timeSystem = TimeSystem();
}

void ECS::Loop()
{
    m_timeSystem.Loop();
}

