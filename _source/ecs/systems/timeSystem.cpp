#include "timeSystem.h"

TimeSystem::TimeSystem()
{
	m_currentTime = 0;
	m_curTime = std::chrono::high_resolution_clock::now();
}

TimeSystem::~TimeSystem()
{
}

void TimeSystem::Loop()
{
	m_prevTime = m_curTime;
	m_curTime = std::chrono::high_resolution_clock::now();
	m_deltaTIme = (float)((std::chrono::duration<double>)(m_curTime - m_prevTime)).count();
	m_currentTime += m_deltaTIme;
}

const double TimeSystem::GetTime()
{
	return m_currentTime;
}

const float TimeSystem::DeltaTime()
{
	return m_deltaTIme;
}

