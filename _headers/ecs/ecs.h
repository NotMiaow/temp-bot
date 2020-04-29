#ifndef ECS_H__
#define ECS_H__

#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include <atomic>

#include "event.h"
#include "eventManager.h"

#include "entityCounter.h"
#include "componentArray.h"
//Components

//Systems
#include "timeSystem.h"

class ECS
{
public:
	ECS() {}
	~ECS();
	void Init(SharedQueue<Event*>& eventQueue, EntityCounter& entityCounter, UserGroups& userGroups);
	void Loop();
private:
	void WaitForTerminate();
private:
	//Components
	UserGroups* m_userGroups;
	
	//Shared resources
	EntityCounter* m_entityCounter;
	SharedQueue<Event*>* m_eventQueue;

	//Systems
	TimeSystem m_timeSystem;
};


#endif
