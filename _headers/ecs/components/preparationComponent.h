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
            active.push_back(false);
            readies.push_back(false);
        }
    }
    ~PreparationComponent() { }

    std::vector<bool> active;
    std::vector<bool> readies;
};

#endif