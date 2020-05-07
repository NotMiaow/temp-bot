#include "matchmakingSystem.h"

MatchmakingSystem::MatchmakingSystem(SharedQueue<Event*>& robotQueue, EntityCounter& entityCounter, Queues& queues)
{
    m_queues = &queues;

    m_entityCounter = &entityCounter;

    m_robotQueue = &robotQueue;

    float waitTimer = 0.0f;
}

void MatchmakingSystem::Loop(const float& deltaTime)
{
    waitTimer += deltaTime;
    if(waitTimer > 0.5f)
    {
        waitTimer = 0.0f;
        for(QueueIterator matchmakingIt = m_queues->GetIterator(QUEUE_LEAGUE_OF_LEGENDS); !matchmakingIt.End(); matchmakingIt++)
        {
            QueueComponent* queue = matchmakingIt.GetData();
            SwitchQueuePosition(deltaTime, *queue);
        }
    }
}

void MatchmakingSystem::SwitchQueuePosition(const float& deltaTime, QueueComponent& queue)
{
    switch (queue.type)
    {
    case LEAGUE_S_Rank:
        SRankQueue(deltaTime, queue);
        break;
    case LEAGUE_SRAM:
        SramQueue(deltaTime, queue);
        break;
    default:
        break;
    }
}

void MatchmakingSystem::SRankQueue(const float& deltaTime, QueueComponent& queue)
{
//    std::cout << "srank empty" << std::endl;
}

void MatchmakingSystem::SramQueue(const float& deltaTime, QueueComponent& queue)
{
    while(queue.pending.size() > 0)
    {
//        if(queue.pending.front().first-- <= 0)
//        {
//            std::random_shuffle(leagueChampions.begin(), leagueChampions.end());
//            std::string team1Champions;
//            std::string team2Champions;
//            int teamPoolSize = queue.startTreshold / 2 * 3;
//            for(int i = 0; i < teamPoolSize; i++)
//            {
//                team1Champions += leagueChampions[i++] + ", ";
//                team1Champions += leagueChampions[i] + ", ";
//            }
//            team1Champions.substr(0, team1Champions.size() - 2);
//            team2Champions.substr(0, team1Champions.size() - 2);
//
//            std::vector<std::string> parameters;
//            parameters.push_back(std::to_string(queue.pending.front().second));
//            parameters.push_back(
//                "Team 1's champion pool is : " + team1Champions + "\n" +
//                "Team 2's champion pool is : " + team2Champions
//            );
//            m_robotQueue->push_back(
//                CreateMessageMatchEvent(false, "message-match", parameters, "", "")
//            );
//            queue.pending.pop();
//        }
    }

    if(queue.spot.size() >= queue.startTreshold)
    {
        std::vector<std::string> userIds;
        for(int i = 0; i < queue.startTreshold; i++)
        {
            userIds.push_back(queue.spot.front().first);
            queue.spot.pop();
        }

        int entityId = m_entityCounter->GetId();
        std::random_shuffle(userIds.begin(), userIds.end());
        std::vector<std::string> parameters;
        parameters.push_back("0");
        parameters.push_back(std::to_string(entityId));
        parameters.push_back("0");
        parameters.push_back("0");
        parameters.push_back(std::to_string(queue.startTreshold));
        for(std::vector<std::string>::iterator i = userIds.begin(); i < userIds.end(); i++)
            parameters.push_back(*i);

        queue.pending.push(std::make_pair(2, entityId));

        m_robotQueue->push_back(
            CreateCreateMatchEvent(
                false, "create-match", parameters, "asdf", "640549931190321152"
            )
        );
    }
}