#include "eventManager.h"

EventManager::~EventManager()
{

}

void EventManager::Init(SharedQueue<Event*>& robotQueue, EntityCounter& entityCounter, Groups& userGroups)
{
    m_groups = &userGroups;

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
        case 4: groupCheckpoint = GROUP_CATEGORIES; break;
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

    std::map<int, int> channelOrder;
    for(GroupsIterator i = m_groups->GetIterator(groupCheckpoint); !i.End(); i++)
    {
        Groups::Entry* channel = i.GetEntry();
        if(channel->data.parentId == newGroupEvent->group.parentId && channel->data.id != newChannel->data.id)
        {
            channelOrder.insert(
                std::make_pair(
                    channel->data.position >= newGroupEvent->group.position ? channel->data.position + 1 : channel->data.position,
                    channel->entityId
                )
            );
        }
    }

    SetGroupPositions(channelOrder, newGroupEvent->group.parentId, newGroupEvent->channelId, newGroupEvent->guildId);
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
        case 4: groupCheckpoint = GROUP_CATEGORIES; break;
        default: std::cout << "wut: " << updateGroupEvent->group.type << std::endl; return false;
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
    for(GroupsIterator i = m_groups->GetIterator(GROUP_CATEGORIES); !i.End(); i++)
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
        GROUP_CATEGORIES
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
        SetGroupPositions(categoryOrder, deleteCategoryEvent->category.parentId, deleteCategoryEvent->channelId, deleteCategoryEvent->guildId);
        return true;
    }

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
    m_robotQueue->push_back(CreateDeleteCategoryEvent(false, "delete-channel", parameters, deleteCategoryEvent->channelId, deleteCategoryEvent->guildId));
    return false;
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
    for(GroupsIterator i = m_groups->GetIterator(GROUP_CATEGORIES); !i.End(); i++)
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

void EventManager::SetGroupPositions(std::map<int,int>& channelOrder, std::string parentId, std::string channelId, std::string guildId)
{
    int next = 1;
    std::cout << channelOrder.size() << std::endl;
    for (std::map<int, int>::reverse_iterator i = channelOrder.rbegin(); i != channelOrder.rend(); i++, next++)
    {

        Groups::Row group = m_groups->GetById(i->second);
        if(group.data->position != i->first)
        {
            std::cout << group.data->name << "'s new position is " << i->first << std::endl;
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
