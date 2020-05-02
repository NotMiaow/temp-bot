#include "ecs.h"

ECS::~ECS()
{

}

void ECS::Init(SharedQueue<Event*>& robotQueue, EntityCounter& entityCounter, Groups& groups)
{
    //Components
    m_groups = &groups;

    m_robotQueue = &robotQueue;
    m_entityCounter = &entityCounter;

    //Systems
    m_timeSystem = TimeSystem();
}

void ECS::Loop()
{
    m_timeSystem.Loop();
}

