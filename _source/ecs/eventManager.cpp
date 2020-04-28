#include "eventManager.h"

EventManager::~EventManager()
{

}

void EventManager::Init(SharedQueue<Event*>& eventQueue, EntityCounter& entityCounter)
{
    m_eventQueue = &eventQueue;
    m_entityCounter = &entityCounter;
}

void EventManager::Loop()
{
    while (m_eventQueue->size())
    {
        m_event = m_eventQueue->front();
        if (m_event != 0) SwitchEvent();
        delete m_event;
        m_eventQueue->pop_front();
    }
}

void EventManager::SwitchEvent()
{
    switch (m_event->GetType())
    {
    case EError:
        break;
    case ECreateVoiceChannel:
//        CreateVoiceChannel();
        break;
}