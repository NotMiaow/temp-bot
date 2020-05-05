#ifndef GROUP_COMPONENT_H__
#define GROUP_COMPONENT_H__

#include <string>
#include <vector>

struct GroupComponent
{
    GroupComponent() { }
    GroupComponent(const uint maxUsers)
    { 
        this->userLimit = maxUsers;
    }
    ~GroupComponent() { }
    std::string id;
    std::string parentId;
    std::string name;
    int type;
    int userLimit;
    int position;
    std::vector<std::string> userIds;
};

#endif