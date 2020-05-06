#ifndef ROLE_COMPONENT_H__
#define ROLE_COMPONENT_H__

#include <string>

struct RoleComponent
{
    RoleComponent() { }

    std::string id;
    std::string name;
    int permissions;
    int color;
    bool hoist;
    bool mentionable;
};

#endif