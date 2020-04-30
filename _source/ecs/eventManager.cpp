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
    case ECreateVoiceChannel:
        return CreateVoiceChannel(event);
    case EUpdateVoiceChannel:
        return UpdateVoiceChannel(event);
    case EDeleteChannel:
        return DeleteChannel(event);
    default:
        return false;
    }
}

bool EventManager::NewChannel(Event* event)
{
    NewChannelEvent* newChannelEvent = dynamic_cast<NewChannelEvent*>(event);

    for(UserGroupsIterator userGroupIt = m_userGroups->GetIterator(GetCheckpoint(TUserGroup, USER_GROUP_VOICE_CHANNELS));
        !userGroupIt.End(); userGroupIt++)
    {
        UserGroups::Entry* userGroup = userGroupIt.GetEntry();
        if( userGroup->data.parentId == newChannelEvent->userGroup.parentId &&
            userGroup->data.name == newChannelEvent->userGroup.name)
        {
            userGroup->data.id = newChannelEvent->userGroup.id;
            return true;
        }
    }

    std::vector<std::string> parameters;
    parameters.push_back(newChannelEvent->userGroup.id);
    m_robotQueue->push_back(CreateDeleteChannelEvent(false, "remove-channel", parameters, newChannelEvent->channelId, newChannelEvent->guildId));

    std::cout << "SYSTEM ERROR : Channel " << newChannelEvent->userGroup.name << " does not exist." << std::endl;
    return false;
}

bool EventManager::CreateVoiceChannel(Event* event)
{
    CreateVoiceChannelEvent* createVoiceChannelEvent = dynamic_cast<CreateVoiceChannelEvent*>(event);

    std::map<int, int> userGroupOrder;
    for(UserGroupsIterator userGroupIt = m_userGroups->GetIterator(GetCheckpoint(TUserGroup, USER_GROUP_VOICE_CHANNELS));
        !userGroupIt.End(); userGroupIt++)
    {
        UserGroups::Entry* userGroup = userGroupIt.GetEntry();
        if(userGroup->data.parentId == createVoiceChannelEvent->userGroup.parentId)
        {
            userGroupOrder.insert(std::make_pair(userGroup->data.position, userGroup->entityId));
            if(userGroup->data.name == createVoiceChannelEvent->userGroup.name)
            {
                m_robotQueue->push_back(CreateErrorEvent(
                    "There is already a channel named : \"" + createVoiceChannelEvent->userGroup.name + "\"",
                    createVoiceChannelEvent->channelId,
                    EUser,
                    ECreateVoiceChannel,
                    EForbidden
                ));
                return false;
            }
        }
    }

    int next = 1;
    bool inserted = false;
    for (std::map<int, int>::iterator it = userGroupOrder.begin(); it != userGroupOrder.end(); it++, next++)
    {
        // If inserting channel before at least one other channel
        if(next == createVoiceChannelEvent->userGroup.position)
        {
            createVoiceChannelEvent->userGroup.position = next++;
            m_userGroups->Add(createVoiceChannelEvent->userGroup, m_entityCounter->GetId(), GetCheckpoint(TUserGroup, USER_GROUP_VOICE_CHANNELS));
            inserted = true;
        }
        // If current channel's position needs to be updated
        if(inserted && m_userGroups->GetById(it->second).data->position != next)
        {
            std::vector<std::string> parameters;
            parameters.push_back(m_userGroups->GetById(it->second).data->id);
            parameters.push_back(std::to_string(next));
            m_robotQueue->push_back(CreateUpdateChannelEvent(false, "update-voice-channel", parameters, createVoiceChannelEvent->channelId, createVoiceChannelEvent->guildId));
        }
        m_userGroups->GetById(it->second).data->position = next;
    }

    if(!inserted)
    {
        createVoiceChannelEvent->userGroup.position = userGroupOrder.size() + 1;
        m_userGroups->Add(createVoiceChannelEvent->userGroup, m_entityCounter->GetId(), GetCheckpoint(TUserGroup, USER_GROUP_VOICE_CHANNELS));
    }

    return true;
}

bool EventManager::UpdateVoiceChannel(Event* event)
{
    UpdateVoiceChannelEvent* updateVoiceChannelEvent = dynamic_cast<UpdateVoiceChannelEvent*>(event);
    
    std::cout << updateVoiceChannelEvent->userGroup.name << ';' << updateVoiceChannelEvent->userGroup.position << std::endl;

    for(UserGroupsIterator userGroupIt = m_userGroups->GetIterator(GetCheckpoint(TUserGroup, USER_GROUP_VOICE_CHANNELS));
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

    for(UserGroupsIterator userGroupIt = m_userGroups->GetIterator(GetCheckpoint(TUserGroup, USER_GROUP_VOICE_CHANNELS));
        !userGroupIt.End(); userGroupIt++)
    {
        UserGroups::Entry* userGroup = userGroupIt.GetEntry();
        if(userGroup->data.id == deleteChannelEvent->userGroup.id)
            return true;
    }



    m_robotQueue->push_back(
            CreateErrorEvent(
            "Use command the command : \"create-voice-channel (channel_name) (user_limit) (parent_id) (position)\".",
            "705503952585883810",
            EUser,
            EDeleteChannel,
            EForbidden
        )
    );

    return true;
}