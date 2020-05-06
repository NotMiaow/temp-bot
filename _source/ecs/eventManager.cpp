#include "eventManager.h"

EventManager::~EventManager()
{

}

void EventManager::Init(SharedQueue<Event*>& robotQueue, EntityCounter& entityCounter, Lobbies& lobbies, Preparations& preparations, Groups& groups)
{
    m_lobbies = &lobbies;
    m_preparations = &preparations;
    m_groups = &groups;

    m_entityCounter = &entityCounter;
    m_robotQueue = &robotQueue;
}

bool EventManager::HandleEvent(Event* event)
{
    switch (event->GetType())
    {
    case EError:
        return true;
    case EEmpty:
        return true;
    case EShutdown:
        return true;
    case ENewGroup:
        return NewGroup(event);
    case EUpdateGroup:
        return UpdateGroup(event);
    case ECreateChannel:
        return CreateChannel(event);
    case EDeleteChannel:
        return DeleteChannel(event);
    case EMoveChannel:
        return MoveChannel(event);
    case ECreateCategory:
        return CreateCategory(event);
    case EDeleteCategory:
        return DeleteCategory(event);
    case EMoveCategory:
        return MoveCategory(event);
    case EMoveUser:
        return MoveUser(event);
    case ECreateMatch:
        return CreateMatch(event);
    case EChangeGroupPermissions:
        return true;
    case ESetMatchVoicePermissions:
        return true;
    default:
        return false;
    }
}

bool EventManager::NewGroup(Event* event)
{
    NewGroupEvent* newGroupEvent = dynamic_cast<NewGroupEvent*>(event);

    if(!newGroupEvent->fromAPI)
    {
        m_robotQueue->push_back(CreateErrorEvent(
            "Use of command : \"new-group\" is forbidden",
            newGroupEvent->channelId,
            EUser,
            ECreateChannel,
            EForbidden
        ));
        return false;
    }

    int groupCheckpoint;
    switch (newGroupEvent->group.type)
    {
        case 0: groupCheckpoint = GROUP_TEXT_CHANNELS; break;
        case 2: groupCheckpoint = GROUP_VOICE_CHANNELS; break;
        case 4: groupCheckpoint = GROUP_MATCH_CATEGORIES; break;
        default: return false;
    }

    bool exists = false;
    Groups::Entry* newChannel;
    for(GroupsIterator i = m_groups->GetIterator(groupCheckpoint); !i.End(); i++)
    {
        newChannel = i.GetEntry();
        if( newChannel->data.parentId == newGroupEvent->group.parentId &&
            newChannel->data.type == newGroupEvent->group.type &&
            newChannel->data.name == newGroupEvent->group.name)
        {
                newChannel->data.id = newGroupEvent->group.id;
                exists = true;
        }
    }

    if(!exists)
    {
        std::vector<std::string> parameters;
        parameters.push_back(newGroupEvent->group.id);
        m_robotQueue->push_back(CreateDeleteChannelEvent(false, "delete-channel", parameters, newGroupEvent->channelId, newGroupEvent->guildId));
        m_robotQueue->push_back(CreateErrorEvent(
            "Channel creation from the left hand side panel is forbidden.\nUse the command : \"create-channel (name) (type(0=text)(2=voice) (parent_id) (position) (user_limit?)\"",
            // HARDCODED BECAUSE SET BOT CHANNEL IS NOT IMPLEMENTED YET
            "705503952585883810",
            EUser,
            ECreateChannel,
            EForbidden
        ));
        return false;
    }

//    std::map<int, int> channelOrder;
//    for(GroupsIterator i = m_groups->GetIterator(groupCheckpoint); !i.End(); i++)
//    {
//        Groups::Entry* channel = i.GetEntry();
//        if(channel->data.parentId == newGroupEvent->group.parentId && channel->data.id != newChannel->data.id)
//        {
//            channelOrder.insert(
//                std::make_pair(
//                    channel->data.position >= newGroupEvent->group.position ? channel->data.position + 1 : channel->data.position,
//                    channel->entityId
//                )
//            );
//        }
//    }
//
//    SetGroupPositions(channelOrder, newGroupEvent->group.parentId, newGroupEvent->channelId, newGroupEvent->guildId);
    return true;
}

bool EventManager::UpdateGroup(Event* event)
{
    UpdateGroupEvent* updateGroupEvent = dynamic_cast<UpdateGroupEvent*>(event);
    
//    if(!updateGroupEvent->fromAPI)
//    {
//        m_robotQueue->push_back(CreateErrorEvent(
//            "Use of command : \"update-channel\" is forbidden",
//            updateGroupEvent->channelId,
//            EUser,
//            ECreateChannel,
//            EForbidden
//        ));
//        return false;
//    }

    int groupCheckpoint;
    switch (updateGroupEvent->group.type)
    {
        case 0: groupCheckpoint = GROUP_TEXT_CHANNELS; break;
        case 2: groupCheckpoint = GROUP_VOICE_CHANNELS; break;
        case 4: groupCheckpoint = GROUP_MATCH_CATEGORIES; break;
        default: return false;
    }

    // TODO : FORBID USERS FROM CALLING THIS COMMAND

    for(GroupsIterator i = m_groups->GetIterator(groupCheckpoint); !i.End(); i++)
    {
        Groups::Entry* group = i.GetEntry();
        if(group->data.id == updateGroupEvent->group.id)
        {
            group->data.position = updateGroupEvent->group.position;
            return true;
        }
    }

    std::cout << "SYSTEM ERROR : Channel " << updateGroupEvent->group.name << " does not exist." << std::endl;
    return false;
}

bool EventManager::CreateChannel(Event* event)
{
    CreateChannelEvent* createChannelEvent = dynamic_cast<CreateChannelEvent*>(event);

    int childrenCount = 0;
    for(GroupsIterator i = m_groups->GetIterator(createChannelEvent->channel.type == 0 ? GROUP_TEXT_CHANNELS : GROUP_VOICE_CHANNELS); !i.End(); i++)
    {
        Groups::Entry* channel = i.GetEntry();
        // Allow name once per channel types (text and voice) and parentId
        if(channel->data.parentId == createChannelEvent->channel.parentId)
        {
            childrenCount++;
            if( channel->data.type == createChannelEvent->channel.type &&
                channel->data.name == createChannelEvent->channel.name)
            {
                m_robotQueue->push_back(CreateErrorEvent(
                    "There is already a channel named : \"" + createChannelEvent->channel.name + "\"",
                    createChannelEvent->channelId,
                    EUser,
                    ECreateChannel,
                    EForbidden
                ));
                return false;
            }
        }
    }

    if(createChannelEvent->channel.position > childrenCount)
        createChannelEvent->channel.position = childrenCount + 1;
    createChannelEvent->CreateJson();

    m_groups->Add(
        createChannelEvent->channel,
        m_entityCounter->GetId(),
        createChannelEvent->channel.type == 0 ? GROUP_TEXT_CHANNELS : GROUP_VOICE_CHANNELS
    );
    return true;
}

bool EventManager::DeleteChannel(Event* event)
{
    DeleteChannelEvent* deleteChannelEvent = dynamic_cast<DeleteChannelEvent*>(event);

    Groups::Entry* deleted;
    if(!GetGroupById(deleted, deleteChannelEvent->channel.id))
    {
        m_robotQueue->push_back(CreateErrorEvent(
            "Channel with id " + deleteChannelEvent->channel.id + " does not exist.",
            deleteChannelEvent->channelId,
            EUser,
            ECreateChannel,
            EForbidden
        ));
        return false;
    }

    if(deleted->data.type != 0 && deleted->data.type != 2)
    {
        m_robotQueue->push_back(CreateErrorEvent(
            "Command \"delete-channel\" can only be used to delete text and voice channels.",
            deleteChannelEvent->channelId,
            EUser,
            EDeleteChannel,
            EForbidden
        ));
        return false;
    }

    std::map<int, int> channelOrder;
    for(GroupsIterator i = m_groups->GetIterator(deleted->data.type == 0 ? GROUP_TEXT_CHANNELS : GROUP_VOICE_CHANNELS); !i.End(); i++)
    {
        Groups::Entry* channel = i.GetEntry();
        if(channel->data.parentId == deleted->data.parentId && channel->data.position > deleted->data.position)
            channelOrder.insert(std::make_pair(channel->data.position - 1, channel->entityId));
    }
    m_groups->Remove(deleted->entityId);
    SetGroupPositions(channelOrder, deleteChannelEvent->channel.parentId, deleteChannelEvent->channelId, deleteChannelEvent->guildId);

    return true;
}

bool EventManager::MoveChannel(Event* event)
{
    MoveChannelEvent* moveChannelEvent = dynamic_cast<MoveChannelEvent*>(event);

    Groups::Entry* moving;
    if(!GetGroupById(moving, moveChannelEvent->channel.id))
    {
        m_robotQueue->push_back(CreateErrorEvent(
            "Channel with id " + moveChannelEvent->channel.id + " does not exist.",
            moveChannelEvent->channelId,
            EUser,
            ECreateChannel,
            EForbidden
        ));
        return false;
    }

    int childrenCount = 0;
    std::map<int, int> channelOrder;
    for(GroupsIterator i = m_groups->GetIterator(moving->data.type == 0 ? GROUP_TEXT_CHANNELS : GROUP_VOICE_CHANNELS); !i.End(); i++)
    {
        Groups::Entry* channel = i.GetEntry();
        if(channel->data.parentId == moving->data.parentId)
        {
            childrenCount++;
            if(channel->data.id != moving->data.id)
            {
                if(channel->data.position > moving->data.position && channel->data.position <= moveChannelEvent->channel.position)
                    channelOrder.insert(std::make_pair(channel->data.position - 1, channel->entityId));
                else if (channel->data.position < moving->data.position && channel->data.position >= moveChannelEvent->channel.position)
                    channelOrder.insert(std::make_pair(channel->data.position + 1, channel->entityId));
            }
        }
    }

    if(moveChannelEvent->channel.position > childrenCount)
        moveChannelEvent->channel.position = childrenCount;
    moveChannelEvent->CreateJson();

    channelOrder.insert(std::make_pair(moveChannelEvent->channel.position, moving->entityId));
    SetGroupPositions(channelOrder, moveChannelEvent->channel.parentId, moveChannelEvent->channelId, moveChannelEvent->guildId);

    return true;
}

bool EventManager::CreateCategory(Event* event)
{
    CreateCategoryEvent* createCategoryEvent = dynamic_cast<CreateCategoryEvent*>(event);

    int childrenCount = 0;
    for(GroupsIterator i = m_groups->GetIterator(GROUP_MATCH_CATEGORIES); !i.End(); i++)
    {
        Groups::Entry* channel = i.GetEntry();
        // Allow name once per channel types (text and voice) and parentId
        if(channel->data.parentId == createCategoryEvent->category.parentId)
        {
            childrenCount++;
            if( channel->data.type == createCategoryEvent->category.type &&
                channel->data.name == createCategoryEvent->category.name)
            {
                m_robotQueue->push_back(CreateErrorEvent(
                    "There is already a channel named : \"" + createCategoryEvent->category.name + "\"",
                    createCategoryEvent->channelId,
                    EUser,
                    ECreateChannel,
                    EForbidden
                ));
                return false;
            }
        }
    }

    if(createCategoryEvent->category.position > childrenCount)
        createCategoryEvent->category.position = childrenCount + 1;
    createCategoryEvent->CreateJson();

    m_groups->Add(
        createCategoryEvent->category,
        m_entityCounter->GetId(),
        GROUP_MATCH_CATEGORIES
    );
    return true;
}

bool EventManager::DeleteCategory(Event* event)
{
    DeleteCategoryEvent* deleteCategoryEvent = dynamic_cast<DeleteCategoryEvent*>(event);

    Groups::Entry* deleted;
    if(!GetGroupById(deleted, deleteCategoryEvent->category.id))
    {
        m_robotQueue->push_back(CreateErrorEvent(
            "Category with id " + deleteCategoryEvent->category.id + " does not exist.",
            deleteCategoryEvent->channelId,
            EUser,
            ECreateChannel,
            EForbidden
        ));
        return false;
    }

    if(deleted->data.type != 4)
    {
        m_robotQueue->push_back(CreateErrorEvent(
            "Command \"delete-category\" can only be used to delete categories.",
            deleteCategoryEvent->channelId,
            EUser,
            EDeleteChannel,
            EForbidden
        ));
        return false;
    }

    if(deleteCategoryEvent->deletionQueued)
    {
        std::map<int, int> categoryOrder;
        for(GroupsIterator i = m_groups->GetIterator(deleted->data.type == 0 ? GROUP_TEXT_CHANNELS : GROUP_VOICE_CHANNELS); !i.End(); i++)
        {
            Groups::Entry* channel = i.GetEntry();
            if(channel->data.parentId == deleted->data.parentId && channel->data.position > deleted->data.position)
                categoryOrder.insert(std::make_pair(channel->data.position - 1, channel->entityId));
        }

        m_groups->Remove(deleted->entityId);
//        SetGroupPositions(categoryOrder, deleteCategoryEvent->category.parentId, deleteCategoryEvent->channelId, deleteCategoryEvent->guildId);
        return true;
    }
    else
    {
        // Get text channels to be deleted
        std::map<int, std::string> textChannelsDeletionOrder;
        for(GroupsIterator i = m_groups->GetIterator(GROUP_TEXT_CHANNELS); !i.End(); i++)
        {
            Groups::Entry* channel = i.GetEntry();
            if(channel->data.parentId == deleted->data.id)
                textChannelsDeletionOrder.insert(std::make_pair(channel->data.position, channel->data.id));
        }

        // Get voice channels to be deleted
        std::map<int, std::string> voiceChannelsDeletionOrder;
        for(GroupsIterator i = m_groups->GetIterator(GROUP_VOICE_CHANNELS); !i.End(); i++)
        {
            Groups::Entry* channel = i.GetEntry();
            if(channel->data.parentId == deleted->data.id)
                voiceChannelsDeletionOrder.insert(std::make_pair(channel->data.position, channel->data.id));
        }

        // Queue deletion of the category's channels by reverse order of position
        // Reverse order saves up alot on command load.
        for (std::map<int, std::string>::reverse_iterator i = textChannelsDeletionOrder.rbegin(); i != textChannelsDeletionOrder.rend(); i++)
        {
            std::vector<std::string> parameters;
            parameters.push_back(i->second);
            m_robotQueue->push_back(CreateDeleteChannelEvent(false, "delete-channel", parameters, deleteCategoryEvent->channelId, deleteCategoryEvent->guildId));
        }

        for (std::map<int, std::string>::reverse_iterator i = voiceChannelsDeletionOrder.rbegin(); i != voiceChannelsDeletionOrder.rend(); i++)
        {
            std::vector<std::string> parameters;
            parameters.push_back(i->second);
            m_robotQueue->push_back(CreateDeleteChannelEvent(false, "delete-channel", parameters, deleteCategoryEvent->channelId, deleteCategoryEvent->guildId));
        }

        std::vector<std::string> parameters;
        parameters.push_back(deleteCategoryEvent->category.id);
        parameters.push_back("1");
        m_robotQueue->push_back(CreateDeleteCategoryEvent(false, "delete-category", parameters, deleteCategoryEvent->channelId, deleteCategoryEvent->guildId));
        return false;
    }
}

bool EventManager::MoveCategory(Event* event)
{
    MoveCategoryEvent* moveCategoryEvent = dynamic_cast<MoveCategoryEvent*>(event);

    Groups::Entry* moving;
    if(!GetGroupById(moving, moveCategoryEvent->category.id))
    {
        m_robotQueue->push_back(CreateErrorEvent(
            "Channel with id " + moveCategoryEvent->category.id + " does not exist.",
            moveCategoryEvent->channelId,
            EUser,
            ECreateChannel,
            EForbidden
        ));
        return false;
    }

    int childrenCount = 0;
    std::map<int, int> categoryOrder;
    for(GroupsIterator i = m_groups->GetIterator(GROUP_MATCH_CATEGORIES); !i.End(); i++)
    {
        Groups::Entry* category = i.GetEntry();
        if(category->data.parentId == moving->data.parentId)
        {
            childrenCount++;
            if(category->data.id != moving->data.id)
            {
                if(category->data.position > moving->data.position && category->data.position <= moveCategoryEvent->category.position)
                    categoryOrder.insert(std::make_pair(category->data.position - 1, category->entityId));
                else if (category->data.position < moving->data.position && category->data.position >= moveCategoryEvent->category.position)
                    categoryOrder.insert(std::make_pair(category->data.position + 1, category->entityId));
            }
        }
    }

    if(moveCategoryEvent->category.position > childrenCount)
        moveCategoryEvent->category.position = childrenCount;
    moveCategoryEvent->CreateJson();


    categoryOrder.insert(std::make_pair(moveCategoryEvent->category.position, moving->entityId));
    SetGroupPositions(categoryOrder, moveCategoryEvent->category.parentId, moveCategoryEvent->channelId, moveCategoryEvent->guildId);

    return true;
}

bool EventManager::MoveUser(Event* event)
{
    MoveUserEvent* moveUserEvent = dynamic_cast<MoveUserEvent*>(event);
    
    

    return true;
}

bool EventManager::CreateMatch(Event* event)
{
    CreateMatchEvent* createMatchEvent = dynamic_cast<CreateMatchEvent*>(event);

    //Handle CreateMatchEvent Step 0
    if(createMatchEvent->creationStep == 0)
    {
        std::map<int, int> matchOrder;
        for(Groups::Iterator i = m_groups->GetIterator(GROUP_MATCH_CATEGORIES); !i.End(); i++)
        {
            Groups::Entry* match = i.GetEntry();
            matchOrder.insert(std::make_pair(match->data.position, match->entityId));
        }
        int lobbyPosition = 1;
        for (std::map<int, int>::iterator i = matchOrder.begin(); i != matchOrder.end(); i++)
            if(lobbyPosition == i->first)
                lobbyPosition++;

        //Set match name
        std::string matchName = "lobby-";
        matchName += std::to_string(lobbyPosition);
        createMatchEvent->matchName = matchName;

        //Create Match Holder
        std::vector<std::string> parameters;
        parameters.push_back(createMatchEvent->matchName);
        parameters.push_back(std::to_string(lobbyPosition));
        m_robotQueue->push_back(
            CreateCreateCategoryEvent(false, "create-category", parameters, createMatchEvent->channelId, createMatchEvent->guildId)
        );

        //Launch CreateMatchEvent Step 1
        parameters.clear();
        parameters.push_back("1");
        parameters.push_back("temp");
        parameters.push_back(createMatchEvent->matchName);
        parameters.push_back(std::to_string(createMatchEvent->userCount));
        for(std::vector<std::string>::iterator i = createMatchEvent->userIds.begin(); i < createMatchEvent->userIds.end(); i++)
            parameters.push_back(*i);

        m_robotQueue->push_back(CreateCreateMatchEvent(false, "create-match", parameters, createMatchEvent->channelId, createMatchEvent->guildId));

        return true;
    }
    //Handle CreateMatchEvent Step 1
    else if(createMatchEvent->creationStep == 1)
    {
        for(Groups::Iterator i = m_groups->GetIterator(GROUP_MATCH_CATEGORIES); !i.End(); i++)
        {
            Groups::Entry* category = i.GetEntry();
            if(category->data.name == createMatchEvent->matchName)
                createMatchEvent->matchId = category->data.id;
        }

        //Create Team Channels
        std::vector<std::string> parameters;
        for(int i = 0; i < 2; i++)
        {
            std::string voiceChannelName = "team-";
            voiceChannelName += std::to_string((i + 1));
            parameters.clear();
            parameters.push_back(voiceChannelName);
            parameters.push_back("2");
            parameters.push_back(createMatchEvent->matchId);
            parameters.push_back(std::to_string(i + 1));
            parameters.push_back(std::to_string(createMatchEvent->userCount / 2));
            m_robotQueue->push_back(
                CreateCreateChannelEvent(false, "create-channel", parameters, createMatchEvent->channelId, createMatchEvent->guildId)
            );
        }

        //Create All Chat
        parameters.clear();
        parameters.push_back("all-chat");
        parameters.push_back("0");
        parameters.push_back(createMatchEvent->matchId);
        parameters.push_back("1");
        m_robotQueue->push_back(
            CreateCreateChannelEvent(false, "create-channel", parameters, createMatchEvent->channelId, createMatchEvent->guildId)
        );

        //Launch CreateMatchEvent Step 2
        parameters.clear();
        parameters.push_back("2");
        parameters.push_back(createMatchEvent->matchId);
        parameters.push_back(createMatchEvent->matchName);
        parameters.push_back(std::to_string(createMatchEvent->userCount));
        for(std::vector<std::string>::iterator i = createMatchEvent->userIds.begin(); i < createMatchEvent->userIds.end(); i++)
            parameters.push_back(*i);
        m_robotQueue->push_back(CreateCreateMatchEvent(false, "create-match", parameters, createMatchEvent->channelId, createMatchEvent->guildId));

        return true;
    }
    //Handle CreateMatchEvent Step 2
    else if(createMatchEvent->creationStep == 2)
    {
        // ECS new match entity
        PreparationComponent preparation;
        LobbyComponent lobby;

        std::vector<std::string> parameters;
        // Text channel permissions
        for(Groups::Iterator i = m_groups->GetIterator(GROUP_TEXT_CHANNELS); !i.End(); i++)
        {
            Groups::Entry* channel = i.GetEntry();
            if(channel->data.parentId == createMatchEvent->matchId)
            {
                // Give both team VIEW_CHANNEL, SEND_MESSAGES
                parameters.clear();
                parameters.push_back("1");
                parameters.push_back(channel->data.id);
                parameters.push_back(channel->data.name);
                parameters.push_back(std::to_string(channel->data.type));
                parameters.push_back("3072");
                parameters.push_back(std::to_string(createMatchEvent->userIds.size()));
                for(std::vector<std::string>::const_iterator j = createMatchEvent->userIds.begin(); j < createMatchEvent->userIds.end(); j++)
                    parameters.push_back(*j);
                m_robotQueue->push_back(
                    CreateChangeGroupPermissionsEvent(false, "give-group-permissions", parameters, createMatchEvent->channelId, createMatchEvent->guildId)
                );

                //ECS add text channel as child of lobby
                lobby.groupIds.push_back(channel->data.id);
            }
        }

        // Voice channel permissions
        for(Groups::Iterator i = m_groups->GetIterator(GROUP_VOICE_CHANNELS); !i.End(); i++)
        {
            Groups::Entry* channel = i.GetEntry();
            if(channel->data.parentId == createMatchEvent->matchId)
            {
                // Give each team VIEW_CHANNEL, CONNECT, SPEAK, USE_VAD on their own voice channel
                // Give each team VIEW_CHANNEL on their opponent's voice channel
                parameters.clear();
                parameters.push_back("1");
                parameters.push_back(channel->data.id);
                parameters.push_back(channel->data.name);
                parameters.push_back("2");
                parameters.push_back("36701184");
                parameters.push_back("1024");
                parameters.push_back(std::to_string(createMatchEvent->userIds.size()));
                if(channel->data.name == "team-1")
                {
                    int userCount = 0;
                    for(std::vector<std::string>::const_iterator j = createMatchEvent->userIds.begin(); j < createMatchEvent->userIds.end();j++)
                    {
                        parameters.push_back(*j);
                        if(userCount < createMatchEvent->userIds.size() / 2)
                            channel->data.userIds.push_back(*j);
                    }
                }
                else
                {
                    int userCount = 0;
                    for(std::vector<std::string>::const_reverse_iterator j = createMatchEvent->userIds.rbegin(); j < createMatchEvent->userIds.rend();j++)
                    {
                        parameters.push_back(*j);
                        if(userCount < createMatchEvent->userIds.size() / 2)
                            channel->data.userIds.push_back(*j);
                    }
                }
                
                m_robotQueue->push_back(
                    CreateSetMatchVoicePermissionsEvent(false, "set-match-voice-permissions", parameters, createMatchEvent->channelId, createMatchEvent->guildId)
                );

                //ECS add voice channels as children of lobby
                lobby.groupIds.push_back(channel->data.id);
            }
        }
        
        int entityId = m_entityCounter->GetId();
        m_preparations->Add(preparation, entityId, PREPARATION_MATCHES);
        m_lobbies->Add(lobby, entityId, LOBBY_MATCHES);

        return true;
    }
    std::cout << "failed at step : " << createMatchEvent->creationStep << std::endl;
    return false;
}

void EventManager::SetGroupPositions(std::map<int,int>& channelOrder, std::string parentId, std::string channelId, std::string guildId)
{
    int next = 1;
    for (std::map<int, int>::reverse_iterator i = channelOrder.rbegin(); i != channelOrder.rend(); i++, next++)
    {
        Groups::Row group = m_groups->GetById(i->second);
        if(group.data->position != i->first)
        {
            std::vector<std::string> parameters;
            parameters.push_back(group.data->id);
            parameters.push_back(std::to_string(group.data->type));
            parameters.push_back(std::to_string(i->first));
            m_robotQueue->push_back(CreateUpdateGroupEvent(false, "update-group", parameters, channelId, guildId));
            m_groups->GetById(i->second).data->position = i->first;
        }
    }
}

bool EventManager::GetGroupById(Groups::Entry*& group, std::string id)
{
    for(GroupsIterator userGroupIt = m_groups->GetIterator();
        !userGroupIt.End(); userGroupIt++)
    {
        group = userGroupIt.GetEntry();
        if(group->data.id == id)
            return true;
    }

    group = NULL;
    return false;
}
