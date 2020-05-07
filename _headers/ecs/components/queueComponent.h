#ifndef QUEUE_COMPONENT_H__
#define QUEUE_COMPONENT_H__

#include <vector>
#include <string>

struct QueueComponent{
    QueueComponent() { }
    ~QueueComponent() { }

    int type;
    int fairnessLevel;
    int startTreshold;
    std::string userIds;
};

#endif