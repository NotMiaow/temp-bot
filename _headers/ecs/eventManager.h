#ifndef EVENT_HANDLER_H__
#define EVENT_HANDLER_H__

#include <iostream>
#include <math.h>
#include <deque>
#include <queue>
#include <vector>
#include <string>

#include <nlohmann/json.hpp>

// Events
#include "eventLanguage.h"
#include "event.h"
#include "shared_queue.h"
#include "eventTranslator.h"

#include "entityCounter.h"
#include "componentArray.h"

// Components
#include "lobbyComponent.h"
#include "groupComponent.h"
#include "preparationComponent.h"
#include "queueComponent.h"

// Misc
#include "definitions.h"

class EventManager
{
public:
    EventManager() { }
    ~EventManager();
    void Init(SharedQueue<Event*>& robotQueue, EntityCounter& entityCounter, Lobbies& lobbies, Preparations& preparations, Groups& groups, Queues& queues);
    bool HandleEvent(Event* event);
    bool NewGroup(Event* event);
    bool UpdateGroup(Event* event);
    bool CreateChannel(Event* event);
    bool DeleteChannel(Event* event);
    bool MoveChannel(Event* event);
    bool CreateCategory(Event* event);
    bool DeleteCategory(Event* event);
    bool MoveCategory(Event* event);
    bool MoveUser(Event* event);
    bool CreateMatch(Event* event);
    bool MessageMatch(Event* event);
    bool JoinQueue(Event* event);
    bool LeaveQueue(Event* event);
    bool SetQueueThreshold(Event* event);
    void SetGroupPositions(std::map<int, Groups::Entry*>& channelOrder, EventInfo info);
    bool GetGroupById(Groups::Entry*& group, std::string id);
private:
    // Components
    Lobbies* m_lobbies;
    Preparations* m_preparations;
    Groups* m_groups;
    Queues* m_queues;
    
    EntityCounter* m_entityCounter;
    SharedQueue<Event*>* m_robotQueue;
};

#endif
