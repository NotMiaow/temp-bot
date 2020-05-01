#ifndef EVENT_HANDLER_H__
#define EVENT_HANDLER_H__

#include <iostream>
#include <math.h>
#include <deque>
#include <vector>
#include <string>

// Events
#include "eventLanguage.h"
#include "event.h"
#include "shared_queue.h"
#include "eventTranslator.h"

#include "entityCounter.h"
#include "componentArray.h"

// Components
#include "userGroupComponent.h"

// Misc
#include "definitions.h"

class EventManager
{
public:
    EventManager() { }
    ~EventManager();
    void Init(SharedQueue<Event*>& robotQueue, EntityCounter& entityCounter, UserGroups& userGroups);
    bool HandleEvent(Event* event);
    bool NewChannel(Event* event);
    bool CreateChannel(Event* event);
    bool UpdateChannel(Event* event);
    bool DeleteChannel(Event* event);
    bool MoveChannel(Event* event);
    bool CreateCategory(Event* event);
    void AdjustChannelpositions(std::map<int,int>& userGroupOrder, std::string parentId, std::string channelId, std::string guildId);
    bool GetChannelById(UserGroups::Entry*& userGroup, std::string id);
private:
    // Components
    UserGroups* m_userGroups;

    EntityCounter* m_entityCounter;
    SharedQueue<Event*>* m_robotQueue;
};

#endif
