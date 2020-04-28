#ifndef TIME_SYSTEM_H__
#define	TIME_SYSTEM_H__

#include <iostream>
#include <thread>
#include <chrono>

class TimeSystem
{
public:
	TimeSystem();
	~TimeSystem();
	void Loop();
	const double GetTime();
	const float DeltaTime();
private:
	double m_currentTime;
	float m_deltaTIme;

	std::chrono::time_point<std::chrono::high_resolution_clock> m_curTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_prevTime;
};

#endif