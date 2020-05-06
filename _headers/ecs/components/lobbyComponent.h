#ifndef LOBBY_COMPONENT_H__
#define LOBBY_COMPONENT_H__

#include <string>
#include <vector>

struct LobbyComponent
{
    LobbyComponent() { }

    std::vector<std::string> groupIds;
};

#endif