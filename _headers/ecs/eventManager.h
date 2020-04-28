#ifndef EVENT_HANDLER_H__
#define EVENT_HANDLER_H__

#include <iostream>
#include <tuple>
#include <math.h>
#include <deque>

//Events
#include "eventLanguage.h"
#include "event.h"
#include "shared_queue.h"

#include "entityCounter.h"
#include "componentArray.h"
//Components

//Networking

//Misc
#include "definitions.h"

class EventManager
{
public:
    EventManager() { }
    ~EventManager();
    void Init(SharedQueue<Event*>& eventQueue, EntityCounter& entityCounter);
    void Loop();
private:
    void SwitchEvent();
private:
    EntityCounter* m_entityCounter;
    Event* m_event;
    SharedQueue<Event*>* m_eventQueue;
};

#endif
