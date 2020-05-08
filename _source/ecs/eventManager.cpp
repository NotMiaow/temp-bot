#include "eventManager.h"

EventManager::~EventManager()
{

}

void EventManager::Init(SharedQueue<Event*>& robotQueue, EntityCounter& entityCounter, Lobbies& lobbies, Preparations& preparations, Groups& groups, Queues& queues)
{
    m_lobbies = &lobbies;
    m_preparations = &preparations;
    m_groups = &groups;
    m_queues = &queues;

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
    case ESendMessage:
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
    case EJoinQueue:
        return JoinQueue(event);
    default:
        return false;
    }
}

bool EventManager::NewGroup(Event* event)
{
    NewGroupEvent* newGroupEvent = dynamic_cast<NewGroupEvent*>(event);

    if(!newGroupEvent->info.fromAPI)
    {
        std::string message = "Use of command : \"new-group\" is forbidden";
        m_robotQueue->push_back(new ErrorEvent(newGroupEvent->info, message, EUser, ECreateChannel, EForbidden));
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
    std::map<int, Groups::Entry*> modifiedGroupPositions;
    for(GroupsIterator i = m_groups->GetIterator(groupCheckpoint); !i.End(); i++)
    {
        Groups::Entry* channel;
        channel = i.GetEntry();
        // If sibbling of added channel
        if(channel->data.parentId == newGroupEvent->group.parentId)
        {
            // If channel found, set it's id
            if(channel->data.name == newGroupEvent->group.name)
            {
                channel->data.id = newGroupEvent->group.id;
                exists = true;
            }
            // If channel is being inserted, push others
            else if(channel->data.position >= newGroupEvent->group.position)
                modifiedGroupPositions.insert(std::make_pair(++channel->data.position, channel));
        }
    }

    if(!exists)
    {
        std::string message = "Channel creation from the left hand side panel is forbidden.\nUse the command : \"create-channel (name) (type(0=text)(2=voice) (parent_id) (position) (user_limit?)\"";
        m_robotQueue->push_back(new DeleteChannelEvent(newGroupEvent->info, newGroupEvent->group.id));
        m_robotQueue->push_back(new ErrorEvent(newGroupEvent->info, message, EUser, ENewGroup, EForbidden));
        return false;
    }

    SetGroupPositions(modifiedGroupPositions, newGroupEvent->info);    
    return true;
}

bool EventManager::UpdateGroup(Event* event)
{
    UpdateGroupEvent* updateGroupEvent = dynamic_cast<UpdateGroupEvent*>(event);
    
    int groupCheckpoint;
    switch (updateGroupEvent->group.type)
    {
        case 0: groupCheckpoint = GROUP_TEXT_CHANNELS; break;
        case 2: groupCheckpoint = GROUP_VOICE_CHANNELS; break;
        case 4: groupCheckpoint = GROUP_MATCH_CATEGORIES; break;
        default: return false;
    }

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

    Groups::Entry* parentCategory;
    if(!GetGroupById(parentCategory, createChannelEvent->channel.parentId))
    {
        std::string message = "The category \"" + createChannelEvent->channel.parentId + "\"does not exist.";
        m_robotQueue->push_back(new ErrorEvent(createChannelEvent->info, message, EUser, ECreateChannel, EForbidden));
        return false;
    }

    int sibblingsCount = 0;
    for(GroupsIterator i = m_groups->GetIterator(createChannelEvent->channel.type == 0 ? GROUP_TEXT_CHANNELS : GROUP_VOICE_CHANNELS); !i.End(); i++)
    {
        Groups::Entry* channel = i.GetEntry();
        // Allow name once per channel types (text and voice) and parentId
        if(channel->data.parentId == parentCategory->data.id)
        {
            sibblingsCount++;
            if( channel->data.type == createChannelEvent->channel.type &&
                channel->data.name == createChannelEvent->channel.name)
            {
                std::string message = "There is already a channel named : \"" + createChannelEvent->channel.name + "\"";
                m_robotQueue->push_back(new ErrorEvent(createChannelEvent->info, message, EUser, ECreateChannel, EForbidden));
                return false;
            }
        }
    }

    // If the channel's position is the greatest of it's sibblings
    // Eliminate the gap between it and the second greatest
    if(createChannelEvent->channel.position > sibblingsCount)
        createChannelEvent->channel.position = sibblingsCount + 1;
    createChannelEvent->CreateJson();

    // Convert capital letters to lower case, else channel wont be found while comparing with API data
    for(int i = 0; i < createChannelEvent->channel.name.length(); i++)
        createChannelEvent->channel.name[i] = putchar(tolower(createChannelEvent->channel.name[i]));

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
    if(!GetGroupById(deleted, deleteChannelEvent->channelId))
    {
        std::string message = "Channel with id " + deleteChannelEvent->channelId + " does not exist.";
        m_robotQueue->push_back(new ErrorEvent(deleteChannelEvent->info, message, EUser, ECreateChannel, EForbidden));
        return false;
    }

    if(deleted->data.type != 0 && deleted->data.type != 2)
    {
        std::string message = "Command \"delete-channel\" can only be used to delete text and voice channels.";
        m_robotQueue->push_back(new ErrorEvent(deleteChannelEvent->info, message, EUser, EDeleteChannel, EForbidden));
        return false;
    }

    std::map<int, Groups::Entry*> channelOrder;
    for(GroupsIterator i = m_groups->GetIterator(deleted->data.type == 0 ? GROUP_TEXT_CHANNELS : GROUP_VOICE_CHANNELS); !i.End(); i++)
    {
        Groups::Entry* channel = i.GetEntry();
        if(channel->data.parentId == deleted->data.parentId && channel->data.position > deleted->data.position)
            channelOrder.insert(std::make_pair(channel->data.position--, channel));
    }
    SetGroupPositions(channelOrder, deleteChannelEvent->info);
    m_groups->Remove(deleted->entityId);

    return true;
}

bool EventManager::MoveChannel(Event* event)
{
    MoveChannelEvent* moveChannelEvent = dynamic_cast<MoveChannelEvent*>(event);

    Groups::Entry* moving;
    if(!GetGroupById(moving, moveChannelEvent->channel.id))
    {
        std::string message = "Channel with id " + moveChannelEvent->channel.id + " does not exist.";
        m_robotQueue->push_back(new ErrorEvent(moveChannelEvent->info, message, EUser, EMoveChannel, EForbidden));
        return false;
    }

    int childrenCount = 0;
    std::map<int, Groups::Entry*> channelOrder;
    for(GroupsIterator i = m_groups->GetIterator(moving->data.type == 0 ? GROUP_TEXT_CHANNELS : GROUP_VOICE_CHANNELS); !i.End(); i++)
    {
        Groups::Entry* channel = i.GetEntry();
        if(channel->data.parentId == moving->data.parentId)
        {
            childrenCount++;
            if(channel->data.id != moving->data.id)
            {
                if(channel->data.position > moving->data.position && channel->data.position <= moveChannelEvent->channel.position)
                    channelOrder.insert(std::make_pair(channel->data.position--, channel));
                else if (channel->data.position < moving->data.position && channel->data.position >= moveChannelEvent->channel.position)
                    channelOrder.insert(std::make_pair(channel->data.position++, channel));
            }
        }
    }

    if(moveChannelEvent->channel.position > childrenCount)
        moveChannelEvent->channel.position = childrenCount;
    moveChannelEvent->CreateJson();

    channelOrder.insert(std::make_pair(moveChannelEvent->channel.position, moving));
    SetGroupPositions(channelOrder, moveChannelEvent->info);
    return true;
}

bool EventManager::CreateCategory(Event* event)
{
    CreateCategoryEvent* createCategoryEvent = dynamic_cast<CreateCategoryEvent*>(event);

    int sibblingsCount = 0;
    for(GroupsIterator i = m_groups->GetIterator(GROUP_MATCH_CATEGORIES); !i.End(); i++)
    {
        sibblingsCount++;
        Groups::Entry* category = i.GetEntry();
        // Counte category's children
        if(category->data.name == createCategoryEvent->category.name)
        {
            std::string message = "There is already a category named : \"" + createCategoryEvent->category.name + "\"";
            m_robotQueue->push_back(new ErrorEvent(createCategoryEvent->info, message, EUser, EDeleteCategory, EForbidden));
            return false;
        }
    }

    // If the category's position is the greatest of it's sibblings
    // Eliminate the gap between it and the second greatest
    if(createCategoryEvent->category.position > sibblingsCount)
        createCategoryEvent->category.position = sibblingsCount + 1;
    createCategoryEvent->CreateJson();

    // Convert capital letters to lower case
    for(int i = 0; i < createCategoryEvent->category.name.length(); i++)
        createCategoryEvent->category.name[i] = putchar(tolower(createCategoryEvent->category.name[i]));

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
    if(!GetGroupById(deleted, deleteCategoryEvent->categoryId))
    {
        std::string message = "Category with id " + deleteCategoryEvent->categoryId + " does not exist.";
        m_robotQueue->push_back(new ErrorEvent(deleteCategoryEvent->info, message, EUser, EDeleteCategory, EForbidden));
        return false;
    }

    if(deleted->data.type != 4)
    {
        std::string message = "Command \"delete-category\" can only be used to delete categories.";
        m_robotQueue->push_back(new ErrorEvent(deleteCategoryEvent->info, message, EUser, EDeleteCategory, EForbidden));
        return false;
    }

    if(deleteCategoryEvent->deletionQueued)
    {
        std::cout << "sup" << std::endl;
        std::map<int, int> categoryOrder;
        for(GroupsIterator i = m_groups->GetIterator(deleted->data.type == 0 ? GROUP_TEXT_CHANNELS : GROUP_VOICE_CHANNELS); !i.End(); i++)
        {
            Groups::Entry* channel = i.GetEntry();
            if(channel->data.parentId == deleted->data.parentId && channel->data.position > deleted->data.position)
                categoryOrder.insert(std::make_pair(channel->data.position--, channel->entityId));
        }

        m_groups->Remove(deleted->entityId);
//        SetGroupPositions(categoryOrder, deleteCategoryEvent->category.parentId, deleteCategoryEvent->channelId, deleteCategoryEvent->guildId);
        return true;
    }
    else
    {
        std::cout << "hi" << std::endl;
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
            m_robotQueue->push_back(new DeleteChannelEvent(deleteCategoryEvent->info, i->second));
            std::cout << "textChannel: " << i->second << std::endl;
        }

        for (std::map<int, std::string>::reverse_iterator i = voiceChannelsDeletionOrder.rbegin(); i != voiceChannelsDeletionOrder.rend(); i++)
        {
            m_robotQueue->push_back(new DeleteChannelEvent(deleteCategoryEvent->info, i->second));
            std::cout << "voiceChannel: " << i->second << std::endl;
        }
        deleteCategoryEvent->deletionQueued = true;
        std::cout << deleteCategoryEvent->info.ToDebuggable() << std::endl;
        m_robotQueue->push_back(new DeleteCategoryEvent(*deleteCategoryEvent));
        return false;
    }
}

bool EventManager::MoveCategory(Event* event)
{
    MoveCategoryEvent* moveCategoryEvent = dynamic_cast<MoveCategoryEvent*>(event);

    Groups::Entry* moving;
    if(!GetGroupById(moving, moveCategoryEvent->category.id))
    {
        std::string message = "Channel with id " + moveCategoryEvent->category.id + " does not exist.";
        m_robotQueue->push_back(new ErrorEvent(moveCategoryEvent->info, message, EUser, EDeleteCategory, EForbidden));
        return false;
    }

    int childrenCount = 0;
    std::map<int, Groups::Entry*> categoryOrder;
    for(GroupsIterator i = m_groups->GetIterator(GROUP_MATCH_CATEGORIES); !i.End(); i++)
    {
        Groups::Entry* category = i.GetEntry();
        if(category->data.parentId == moving->data.parentId)
        {
            childrenCount++;
            if(category->data.id != moving->data.id)
            {
                if(category->data.position > moving->data.position && category->data.position <= moveCategoryEvent->category.position)
                    categoryOrder.insert(std::make_pair(category->data.position--, category));
                else if (category->data.position < moving->data.position && category->data.position >= moveCategoryEvent->category.position)
                    categoryOrder.insert(std::make_pair(category->data.position++, category));
            }
        }
    }

    if(moveCategoryEvent->category.position > childrenCount)
        moveCategoryEvent->category.position = childrenCount;
    moveCategoryEvent->CreateJson();


    categoryOrder.insert(std::make_pair(moveCategoryEvent->category.position, moving));
    SetGroupPositions(categoryOrder, moveCategoryEvent->info);

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
        createMatchEvent->matchName = "lobby-" + std::to_string(lobbyPosition);

        //Create match's category
        GroupComponent category;
        category.name = createMatchEvent->matchName;
        category.position = lobbyPosition;
        m_robotQueue->push_back(new CreateCategoryEvent(createMatchEvent->info, category));

        //Launch CreateMatchEvent Step 1
        createMatchEvent->creationStep++;
        m_robotQueue->push_back(new CreateMatchEvent(*createMatchEvent));
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
        for(int i = 0; i < 2; i++)
        {
            GroupComponent voiceChannel;
            voiceChannel.name = "team-" + std::to_string((i + 1));
            voiceChannel.type = 2;
            voiceChannel.position = i;
            voiceChannel.userLimit = createMatchEvent->userCount / 2;
            voiceChannel.parentId = createMatchEvent->matchId;
            voiceChannel.userIds = i == 0 ? createMatchEvent->userIds1 : createMatchEvent->userIds2;
            m_robotQueue->push_back(new CreateChannelEvent(createMatchEvent->info, voiceChannel));
        }

        //Create All Chat
        GroupComponent textChannel;
        textChannel.name = "all-chat";
        textChannel.type = 0;
        textChannel.position = 1;
        textChannel.userLimit = createMatchEvent->userCount / 2;
        textChannel.parentId = createMatchEvent->matchId;
        m_robotQueue->push_back(new CreateChannelEvent(createMatchEvent->info, textChannel));

        //Launch CreateMatchEvent Step 2
        createMatchEvent->creationStep++;
        m_robotQueue->push_back(new CreateMatchEvent(*createMatchEvent));
        return true;
    }
    //Handle CreateMatchEvent Step 2
    else if(createMatchEvent->creationStep == 2)
    {
        // ECS components to add to match entity
        int entityId;
        PreparationComponent preparation;
        LobbyComponent lobby;

        std::vector<std::string> parameters;
        // Text channel permissions
        for(Groups::Iterator i = m_groups->GetIterator(GROUP_TEXT_CHANNELS); !i.End(); i++)
        {
            Groups::Entry* channel = i.GetEntry();
            if(channel->data.parentId == createMatchEvent->matchId)
            {
                entityId = channel->entityId;
                std::vector<std::string> userIds;
                for(std::vector<std::string>::iterator j = createMatchEvent->userIds1.begin(); j < createMatchEvent->userIds1.end(); j++)
                    userIds.push_back(*j);
                for(std::vector<std::string>::iterator j = createMatchEvent->userIds2.begin(); j < createMatchEvent->userIds2.end(); j++)
                    userIds.push_back(*j);

                // Give both team VIEW_CHANNEL, SEND_MESSAGES
                m_robotQueue->push_back(new ChangeGroupPermissionsEvent(createMatchEvent->info, true, channel->data, 68608, userIds));

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
                m_robotQueue->push_back(new SetMatchVoicePermissionsEvent(
                    createMatchEvent->info, true, channel->data, 36701184, 1024, createMatchEvent->userIds1, createMatchEvent->userIds2));

                // ECS add voice channels as children of lobby
                lobby.groupIds.push_back(channel->data.id);
            }
        }
        
        // ECS create match entity
        m_preparations->Add(preparation, entityId, PREPARATION_MATCHES);
        m_lobbies->Add(lobby, entityId, LOBBY_MATCHES);

        for(QueueIterator i = m_queues->GetIterator(QUEUE_LEAGUE_OF_LEGENDS); !i.End(); i++)
        {
            QueueComponent* queue = i.GetData();
            if(queue->type == createMatchEvent->queueType)
            {
                queue->pending.push(lobby.groupIds[0]);
                std::cout << "pushing pending" << std::endl;
            }
        }
        return true;
    }
    std::cout << "failed at step : " << createMatchEvent->creationStep << std::endl;
    return false;
}

bool EventManager::JoinQueue(Event* event)
{
    JoinQueueEvent* joinQueueEvent = dynamic_cast<JoinQueueEvent*>(event);

	auto queueType = leagueQueueTypes.find(joinQueueEvent->queueName);
	if(queueType != leagueQueueTypes.end())
    {
        for(QueueIterator i = m_queues->GetIterator(); !i.End(); i++)
        {
            QueueComponent* queue = i.GetData();
            if(queue->type == (int)queueType->second)
                queue->spot.push(std::make_pair(joinQueueEvent->info.userId, joinQueueEvent->info.channelId));
        }

        m_robotQueue->push_back(
            new SendMessageEvent(
                joinQueueEvent->info,
                "You have successfully queued to \"" + joinQueueEvent->queueName + "\".\n" +
                "You will be sent an invite link to your team's channel when your queue pops!" +
                " ***(Not implemented yet)***"
            )
        );
        return true;
    }

    std::string queueNames = "";
    for(QueueIterator i = m_queues->GetIterator(); !i.End(); i++)
    {
        QueueComponent* queue = i.GetData();
        if(queue->up)
            queueNames += "\"" + queue->name + "\" ";
    }

    std::string message = "\"" + joinQueueEvent->queueName + "\" is not a valid queue name.\nChoices are : " + queueNames;
    m_robotQueue->push_back(new ErrorEvent(joinQueueEvent->info, message, EUser, EJoinQueue, EForbidden));
    return false;
}

void EventManager::SetGroupPositions(std::map<int, Groups::Entry*>& channelOrder, EventInfo info)
{
    for (std::map<int, Groups::Entry*>::reverse_iterator i = channelOrder.rbegin(); i != channelOrder.rend(); i++)
    {
        info.fromAPI = false;
        m_robotQueue->push_back(new UpdateGroupEvent(info, i->second->data));
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
