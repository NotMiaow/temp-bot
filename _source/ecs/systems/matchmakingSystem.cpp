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

}

void MatchmakingSystem::SramQueue(const float& deltaTime, QueueComponent& queue)
{
    if(queue.pending.size() > 0)
    {
        srand(time(0));
        std::random_shuffle(leagueChampions.begin(), leagueChampions.end());
        std::string team1Champions = "";
        std::string team2Champions = "";
        int teamPoolSize = queue.startTreshold / 2 * 3;
        std::cout << "teamPoolSize: " << teamPoolSize << std::endl;
        for(int i = 0; i < teamPoolSize; i++)
        {
            team1Champions += "\"" + leagueChampions[i] + "\", ";
            team2Champions += "\"" + leagueChampions[i + 1] + "\", ";
        }
        team1Champions = team1Champions.substr(0, team1Champions.length() - 2);
        team2Champions = team2Champions.substr(0, team2Champions.length() - 2);

        m_robotQueue->push_back(
            CreateSendMessageEvent(
                "Team 1 champion pool is " + team1Champions + "\n" +
                "Team 2 champion pool is " + team2Champions,
                queue.pending.front()
            )
        );

        queue.pending.pop();
    }

    if(queue.spot.size() >= queue.startTreshold)
    {
        std::vector<std::string> userIds;
        for(int i = 0; i < queue.startTreshold; i++)
        {
            userIds.push_back(queue.spot.front().first);
            queue.spot.pop();
        }

        std::random_shuffle(userIds.begin(), userIds.end());
        std::vector<std::string> parameters;
        parameters.push_back("0");
        parameters.push_back(std::to_string(LEAGUE_SRAM));
        parameters.push_back("0");
        parameters.push_back("0");
        parameters.push_back(std::to_string(queue.startTreshold));
        for(std::vector<std::string>::iterator i = userIds.begin(); i < userIds.end(); i++)
            parameters.push_back(*i);

        m_robotQueue->push_back(
            CreateCreateMatchEvent(
                false, "create-match", parameters, "asdf", "640549931190321152"
            )
        );
    }
}