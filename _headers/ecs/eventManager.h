#ifndef EVENT_HANDLER_H__
#define EVENT_HANDLER_H__

#include <iostream>
#include <tuple>
#include <math.h>
#include <deque>

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
    void Init(SharedQueue<Event*>& eventQueue, EntityCounter& entityCounter, UserGroups& userGroups);
    bool HandleEvent(Event* event);
    bool SetChannelId(Event* event);
    bool CreateVoiceChannel(Event* event);
private:
    // Components
    UserGroups* m_userGroups;

    EntityCounter* m_entityCounter;
    SharedQueue<Event*>* m_eventQueue;
};

#endif
