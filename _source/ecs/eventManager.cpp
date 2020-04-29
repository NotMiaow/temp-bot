#include "eventManager.h"

EventManager::~EventManager()
{

}

void EventManager::Init(SharedQueue<Event*>& eventQueue, EntityCounter& entityCounter, UserGroups& userGroups)
{
    m_userGroups = &userGroups;

    m_entityCounter = &entityCounter;
    m_eventQueue = &eventQueue;
}

bool EventManager::HandleEvent(Event* event)
{
    switch (event->GetType())
    {
    case EError:
        break;
    case EShutdown:
        return true;
    case ESetChannelId:
        return SetChannelId(event);
    case ECreateVoiceChannel:
        return CreateVoiceChannel(event);
    default:
        return false;
        break;
    }
}

bool EventManager::SetChannelId(Event* event)
{
    SetChannelIdEvent* setChannelIdEvent = dynamic_cast<SetChannelIdEvent*>(event);

    for(UserGroupsIterator userGroupIt = m_userGroups->GetIterator(GetCheckpoint(TUserGroup, USER_GROUP_VOICE_CHANNELS));
        !userGroupIt.End(); userGroupIt++)
    {
        UserGroups::Entry* userGroup = userGroupIt.GetEntry();
        if( userGroup->data.parentId == setChannelIdEvent->userGroup.parentId &&
            userGroup->data.name == setChannelIdEvent->userGroup.name)
        {
            userGroup->data.id = setChannelIdEvent->userGroup.id;
            std::cout << "set id to : " << userGroup->data.id << " on channel with name : " << userGroup->data.name << std::endl;
            return true;
        }
    }

    std::cout << "SYSTEM ERROR : Channel does not exist." << std::endl;
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
                m_eventQueue->push_back(CreateErrorEvent(
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
    for (std::map<int, int>::iterator it = userGroupOrder.begin(); it != userGroupOrder.end(); it++)
    {
        if(next == createVoiceChannelEvent->userGroup.position)
        {
            std::cout << "inserting at : " << next << std::endl;
            createVoiceChannelEvent->userGroup.position = next++;
            m_userGroups->Add(createVoiceChannelEvent->userGroup, m_entityCounter->GetId(), GetCheckpoint(TUserGroup, USER_GROUP_VOICE_CHANNELS));
            inserted = true;
        }
        std::cout << "before : " << m_userGroups->GetById(it->second).data->position << ";";
        if(m_userGroups->GetById(it->second).data->position != next)
        {
            //UPDATE CHANNEL POSITION
        }
        m_userGroups->GetById(it->second).data->position = next++;
        std::cout << "after : " << m_userGroups->GetById(it->second).data->position << std::endl;
    }

    if(!inserted)
    {
        std::cout << "inserting at : " << next << std::endl;
        createVoiceChannelEvent->userGroup.position = next;
        m_userGroups->Add(createVoiceChannelEvent->userGroup, m_entityCounter->GetId(), GetCheckpoint(TUserGroup, USER_GROUP_VOICE_CHANNELS));
    }

    return true;
}