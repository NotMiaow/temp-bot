#ifndef QUEUE_COMPONENT_H__
#define QUEUE_COMPONENT_H__

#include <queue>
#include <deque>
#include <string>

struct QueueComponent{
    QueueComponent() { }
    ~QueueComponent() { }

    int type;
    bool up;
    int fairnessLevel;
    int startTreshold;
    std::string name;
    std::deque<std::pair<std::string, std::string>> spot;
    std::queue<std::string> pending;
};

#endif