#include "ecs.h"

ECS::~ECS()
{

}

void ECS::Init(SharedQueue<Event*>& eventQueue, EntityCounter& entityCounter, Players& players, Banks& banks, Offenses& offenses, Motors& motors, Transforms& transforms)
{
    //Components
    m_entityCounter = &entityCounter;
    m_players = &players;
    m_banks = &banks;
    m_offenses = &offenses;
    m_motors = &motors;
    m_transforms = &transforms;

    m_eventQueue = &eventQueue;

    //Systems
    m_timeSystem = TimeSystem();
    m_movementSystem = MovementSystem(*m_motors, *m_transforms);
    m_rageSystem = RageSystem(*m_eventQueue, *m_motors, *m_transforms);
    m_pathfindingSystem = PathfindingSystem(*m_eventQueue, *m_motors, *m_transforms);
}

bool ECS::Loop()
{
    m_timeSystem.Loop();
    m_pathfindingSystem.Loop();
//    m_rageSystem.Loop();
    m_movementSystem.Loop(m_timeSystem.DeltaTime());
    
    //Kill server from within ecs
//    return false;
    return true;
}

