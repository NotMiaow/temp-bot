#ifndef MATCHMAKING_SYSTEM_H__
#define MATCHMAKING_SYSTEM_H__

// Components
#include "queueComponent.h"

// Robot Queue
#include "shared_queue.h"
#include "event.h"

// Misc
#include "eventTranslator.h"
#include "definitions.h"

class MatchmakingSystem
{
public:
    MatchmakingSystem() { }
    MatchmakingSystem(SharedQueue<Event*>& robotQueue, Queues& queues);
    ~MatchmakingSystem() { }
    void Loop(const float& deltaTime);
private:
    void SwitchQueuePosition(const float& deltaTime, QueueComponent& queue);
    void SRankQueue(const float& deltaTime, QueueComponent& queue);
    void SramQueue(const float& deltaTime, QueueComponent& queue);
private:
    Queues* m_queues;

   	SharedQueue<Event*>* m_robotQueue;

    float waitTimer;
};

#endif