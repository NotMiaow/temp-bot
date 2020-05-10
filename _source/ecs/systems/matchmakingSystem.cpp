#include "matchmakingSystem.h"

MatchmakingSystem::MatchmakingSystem(SharedQueue<Event*>& robotQueue, Queues& queues)
{
    m_queues = &queues;

    m_robotQueue = &robotQueue;

    float waitTimer = 0.0f;
}

void MatchmakingSystem::Loop(const float& deltaTime)
{
    waitTimer += deltaTime;
    if(waitTimer > 2.0f)
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

}

void MatchmakingSystem::SramQueue(const float& deltaTime, QueueComponent& queue)
{
    std::cout << "queue : " << queue.name << " starts at : " << queue.startTreshold << " current : " << queue.spot.size() << std::endl;

    if(queue.pending.size() > 0)
    {
        srand(time(0));
        std::random_shuffle(leagueChampions.begin(), leagueChampions.end());
        std::string team1Champions = "";
        std::string team2Champions = "";
        int championPool = queue.startTreshold * 3;
        for(int i = 0; i < championPool; i++)
        {
            team1Champions += "\"" + leagueChampions[i] + "\", ";
            team2Champions += "\"" + leagueChampions[++i] + "\", ";
        }
        team1Champions = team1Champions.substr(0, team1Champions.length() - 2);
        team2Champions = team2Champions.substr(0, team2Champions.length() - 2);

        json content = { };
        EventInfo info(false, "", "", content, "", queue.pending.front(), "");
        m_robotQueue->push_back(new SendMessageEvent(info, "Team 1 champion pool is " + team1Champions + "\n" + "Team 2 champion pool is " + team2Champions));
        queue.pending.pop();
    }

    if(queue.spot.size() >= queue.startTreshold)
    {
        std::vector<std::string> userIds;
        std::vector<std::string> userIds1;
        std::vector<std::string> userIds2;
        for(int i = 0; i < queue.startTreshold; i++)
        {
            userIds.push_back(queue.spot.front().first);
            queue.spot.pop_front();
        }
        srand(time(0));
        std::random_shuffle(userIds.begin(), userIds.end());

        int count = 0;
        for(std::vector<std::string>::iterator i = userIds.begin(); i < userIds.end(); i++)
        {
            if(count++ < queue.startTreshold / 2)
                userIds1.push_back(*i);
            else
                userIds2.push_back(*i);
        }

        json content = { };
        EventInfo info(false, "", "", content, "99681408724766720", "705503952585883810", "640549931190321152");
        m_robotQueue->push_back(new CreateMatchEvent(info, 0, LEAGUE_SRAM, "", "", userIds1, userIds2));
    }
}