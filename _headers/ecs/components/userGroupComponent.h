#ifndef USER_GROUP_H__
#define USER_GROUP_H__

#include <string>
#include <vector>

struct UserGroupComponent {
public:
    UserGroupComponent() { }
    UserGroupComponent(const uint maxUsers)
    { 
        this->userLimit = maxUsers;
    }
    ~UserGroupComponent() { }
    std::string id;
    int type;
    std::string name;
    int userLimit;
    std::string parentId;
    int position;
    std::vector<std::string> userIds;

private:
};

#endif