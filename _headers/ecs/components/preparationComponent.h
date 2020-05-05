#ifndef PREPARATION_COMPONENT_H__
#define PREPARATION_COMPONENT_H__

#include <vector>
#include <string>

struct PreparationComponent
{
    PreparationComponent() { }
    PreparationComponent(int userCount)
    {
        for(int i = 0; i < userCount; i++)
        {
            presences.push_back(false);
            readyUps.push_back(false);
        }
    }

    std::vector<bool> presences;
    std::vector<bool> readyUps;
    std::vector<std::string> userIds;
};

#endif