#include "eventManager.h"

EventManager::~EventManager()
{

}

void EventManager::Init(SharedQueue<Event*>& robotQueue, EntityCounter& entityCounter, UserGroups& userGroups)
{
    m_userGroups = &userGroups;

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
    case ENewChannelEvent:
        return NewChannel(event);
    case ECreateChannel:
        return CreateChannel(event);
    case EUpdateChannel:
        return UpdateChannel(event);
    case EDeleteChannel:
        return DeleteChannel(event);
    case EMoveChannel:
        return MoveChannel(event);
    default:
        return false;
    }
}

bool EventManager::NewChannel(Event* event)
{
    NewChannelEvent* newChannelEvent = dynamic_cast<NewChannelEvent*>(event);

    if(!newChannelEvent->fromAPI)
    {
        m_robotQueue->push_back(CreateErrorEvent(
            "Use of command : \"new-channel\" is forbidden",
            newChannelEvent->channelId,
            EUser,
            ECreateChannel,
            EForbidden
        ));
        return false;
    }

    for(UserGroupsIterator userGroupIt = m_userGroups->GetIterator();
        !userGroupIt.End(); userGroupIt++)
    {
        UserGroups::Entry* channel = userGroupIt.GetEntry();
        if( channel->data.parentId == newChannelEvent->userGroup.parentId &&
            channel->data.name == newChannelEvent->userGroup.name &&
            channel->data.type == newChannelEvent->userGroup.type)
        {
            channel->data.id = newChannelEvent->userGroup.id;

            std::map<int, int> userGroupOrder;
            for(UserGroupsIterator userGroupIt = m_userGroups->GetIterator();
                !userGroupIt.End(); userGroupIt++)
            {
                UserGroups::Entry* userGroup = userGroupIt.GetEntry();
                if(userGroup->data.parentId == newChannelEvent->userGroup.parentId && userGroup->data.id != channel->data.id)
                {
                    userGroupOrder.insert(
                        std::make_pair(
                            userGroup->data.position >= newChannelEvent->userGroup.position ? userGroup->data.position + 1 : userGroup->data.position,
                            userGroup->entityId
                        )
                    );
                }
            }

            channel->data.position = newChannelEvent->userGroup.position;
            userGroupOrder.insert(std::make_pair(channel->data.position, channel->entityId));
            AdjustChannelpositions(userGroupOrder, newChannelEvent->userGroup.parentId, newChannelEvent->channelId, newChannelEvent->guildId);

            return true;
        }
    }

    std::vector<std::string> parameters;
    parameters.push_back(newChannelEvent->userGroup.id);
    m_robotQueue->push_back(CreateDeleteChannelEvent(false, "delete-channel", parameters, newChannelEvent->channelId, newChannelEvent->guildId));

    m_robotQueue->push_back(
            CreateErrorEvent(
            "Use command the command : \"create-voice-channel (channel_name) (user_limit) (parent_id) (position)\".",
            "705503952585883810",
            EUser,
            EDeleteChannel,
            EForbidden
        )
    );

    return false;
}

bool EventManager::CreateChannel(Event* event)
{
    CreateChannelEvent* createChannelEvent = dynamic_cast<CreateChannelEvent*>(event);

    std::map<int, int> userGroupOrder;
    for(UserGroupsIterator userGroupIt = m_userGroups->GetIterator();
        !userGroupIt.End(); userGroupIt++)
    {
        UserGroups::Entry* userGroup = userGroupIt.GetEntry();
        if(userGroup->data.parentId == createChannelEvent->userGroup.parentId)
        {
            // Reorder channels underneath requested category
            userGroupOrder.insert(std::make_pair(userGroup->data.position, userGroup->entityId));

            // Allow name once per channel types (text and)
            if(userGroup->data.type == createChannelEvent->userGroup.type && userGroup->data.name == createChannelEvent->userGroup.name)
            {
                m_robotQueue->push_back(CreateErrorEvent(
                    "There is already a channel named : \"" + createChannelEvent->userGroup.name + "\"",
                    createChannelEvent->channelId,
                    EUser,
                    ECreateChannel,
                    EForbidden
                ));
                return false;
            }
        }
    }

    m_userGroups->Add(
        createChannelEvent->userGroup,
        m_entityCounter->GetId(),
        createChannelEvent->userGroup.type == 0 ? USERGROUP_TEXT_CHANNELS : USER_GROUP_VOICE_CHANNELS
    );

    return true;
}

bool EventManager::UpdateChannel(Event* event)
{
    UpdateVoiceChannelEvent* updateVoiceChannelEvent = dynamic_cast<UpdateVoiceChannelEvent*>(event);
    
    // TODO : FORBID USERS FROM CALLING THIS COMMAND

    for(UserGroupsIterator userGroupIt = m_userGroups->GetIterator();
        !userGroupIt.End(); userGroupIt++)
    {
        UserGroups::Entry* userGroup = userGroupIt.GetEntry();
        if(userGroup->data.id == updateVoiceChannelEvent->userGroup.id)
        {
            userGroup->data.position = updateVoiceChannelEvent->userGroup.position;
            return true;
        }
    }

    std::cout << "SYSTEM ERROR : Channel " << updateVoiceChannelEvent->userGroup.name << " does not exist." << std::endl;
    return false;
}

bool EventManager::DeleteChannel(Event* event)
{
    DeleteChannelEvent* deleteChannelEvent = dynamic_cast<DeleteChannelEvent*>(event);

    for(UserGroupsIterator userGroupIt = m_userGroups->GetIterator();
        !userGroupIt.End(); userGroupIt++)
    {
        UserGroups::Entry* channel = userGroupIt.GetEntry();
        if(channel->data.id == deleteChannelEvent->userGroup.id)
        {
            std::map<int, int> userGroupOrder;
            for(UserGroupsIterator userGroupIt = m_userGroups->GetIterator();
                !userGroupIt.End(); userGroupIt++)
            {
                UserGroups::Entry* userGroup = userGroupIt.GetEntry();
                if(userGroup->data.parentId == channel->data.parentId && userGroup->data.position > channel->data.position)
                    userGroupOrder.insert(std::make_pair(userGroup->data.position - 1, userGroup->entityId));
                
            }
            m_userGroups->Remove(channel->entityId);
            AdjustChannelpositions(userGroupOrder,deleteChannelEvent->userGroup.parentId, deleteChannelEvent->channelId, deleteChannelEvent->guildId);
            return true;
        }   
    }

    m_robotQueue->push_back(CreateErrorEvent(
        "Channel with id " + deleteChannelEvent->userGroup.id + " does not exist.",
        deleteChannelEvent->channelId,
        EUser,
        ECreateChannel,
        EForbidden
    ));

    return false;
}

bool EventManager::MoveChannel(Event* event)
{
    MoveChannelEvent* moveChannelEvent = dynamic_cast<MoveChannelEvent*>(event);

    UserGroups::Entry* channel;
    if(!GetChannelById(channel, moveChannelEvent->userGroup.id))
    {
        m_robotQueue->push_back(CreateErrorEvent(
            "Channel with id " + moveChannelEvent->userGroup.id + " does not exist.",
            moveChannelEvent->channelId,
            EUser,
            ECreateChannel,
            EForbidden
        ));
        return false;
    }

    std::map<int, int> userGroupOrder;
    for(UserGroupsIterator userGroupIt = m_userGroups->GetIterator();
        !userGroupIt.End(); userGroupIt++)
    {
        UserGroups::Entry* userGroup = userGroupIt.GetEntry();
        if(userGroup->data.parentId == channel->data.parentId && userGroup->data.id != channel->data.id)
        {
            if(userGroup->data.position > channel->data.position && userGroup->data.position <= moveChannelEvent->userGroup.position)
                userGroupOrder.insert(std::make_pair(userGroup->data.position - 1, userGroup->entityId));
            else if (userGroup->data.position < channel->data.position && userGroup->data.position >= moveChannelEvent->userGroup.position)
                userGroupOrder.insert(std::make_pair(userGroup->data.position + 1, userGroup->entityId));
        }
    }

    userGroupOrder.insert(std::make_pair(moveChannelEvent->userGroup.position, channel->entityId));
    AdjustChannelpositions(userGroupOrder,moveChannelEvent->userGroup.parentId, moveChannelEvent->channelId, moveChannelEvent->guildId);

    return true;
}

bool EventManager::CreateCategory(Event* event)
{
    CreateCategoryEvent* createCategoryEvent = dynamic_cast<CreateCategoryEvent*>(event);

    return true;
}

void EventManager::AdjustChannelpositions(std::map<int,int>& userGroupOrder, std::string parentId, std::string channelId, std::string guildId)
{
    int next = 1;
    for (std::map<int, int>::iterator it = userGroupOrder.begin(); it != userGroupOrder.end(); it++, next++)
    {
        UserGroups::Row userGroup = m_userGroups->GetById(it->second);
        if(userGroup.data->position != next)
        {
            std::vector<std::string> parameters;
            parameters.push_back(userGroup.data->id);
            parameters.push_back(std::to_string(next));
            m_robotQueue->push_back(CreateUpdateChannelEvent(false, "update-voice-channel", parameters, channelId, guildId));
            m_userGroups->GetById(it->second).data->position = next;
        }
    }
}

bool EventManager::GetChannelById(UserGroups::Entry*& userGroup, std::string id)
{
    for(UserGroupsIterator userGroupIt = m_userGroups->GetIterator();
        !userGroupIt.End(); userGroupIt++)
    {
        userGroup = userGroupIt.GetEntry();
        if(userGroup->data.id == id)
            return true;
    }

    userGroup = NULL;
    return false;
}
