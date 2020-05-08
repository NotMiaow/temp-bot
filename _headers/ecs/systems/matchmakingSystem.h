#ifndef MATCHMAKING_SYSTEM_H__
#define MATCHMAKING_SYSTEM_H__

#include <nlohmann/json.hpp>
#include <algorithm>
#include <queue>
#include <vector>
#include <string>
#include <time.h>
#include <stdlib.h>

// Components
#include "queueComponent.h"

// Robot Queue
#include "shared_queue.h"
#include "event.h"

// Misc
#include "eventTranslator.h"
#include "definitions.h"

using json = nlohmann::json;

class MatchmakingSystem
{
public:
    MatchmakingSystem() { }
    MatchmakingSystem(SharedQueue<Event*>& robotQueue, Queues& queues);
    ~MatchmakingSystem() { }
    void Loop(const float& deltaTime);
private:
    void SwitchQueuePosition(const float& deltaTime, QueueComponent& queue);
    void SRankQueue(const float& deltaTime, QueueComponent& queue);
    void SramQueue(const float& deltaTime, QueueComponent& queue);
private:
    Queues* m_queues;

   	SharedQueue<Event*>* m_robotQueue;

    float waitTimer;

    std::vector<std::string> leagueChampions = {
        "Atrox", "Ahri", "Akali", "Alistar", "Amumu", "Anivia", "Annie", "Aphelios", "Ashe", "Aurelion Sol", "Azir", "Bard",
        "Blitzcrank", "Brand", "Braum", "Caitlyn", "Camille", "Cassiopeia", "Cho'Gath", "Corki", "Darius", "Diana",
        "Dr. Mundo", "Draven", "Ekko", "Elise", "Evelynn", "Ezreal", "Fiddlesticks", "Fiora", "Fizz", "Galio", "Gangplank",
        "Garen", "Gnar", "Gragas", "Graves", "Hecarim", "Heimerdinger", "Illaoi", "Irelia", "Ivern", "Janna", "Jarvan IV",
        "Jax", "Jayce", "Jhin", "Jinx", "KaiSa", "Kalista", "Karma", "Karthus", "Kassadin", "Katarina", "Kayle", "Kayn",
        "Kennen", "Kha'Zix", "Kindred", "Kled", "Kog'Maw", "LeBlanc", "Lee", "Leona", "Lissandra", "Lucian", "Lulu", "Lux",
        "Malphite", "Malzahar", "Maokai", "Master Yi", "Miss Fortune", "Mordekaiser", "Morgana", "Nami", "Nasus", "Nautilus",
        "Neeko", "Nidalee", "Nocturne", "Nunu", "Olaf", "Orianna", "Ornn", "Pantheon", "Poppy", "Pyke", "Qiyana", "Quinn",
        "Rakan", "Rammus", "Rek'Sai", "Renekton", "Rengar", "Riven", "Rumble", "Ryze", "Sejuani", "Senna", "Sett", "Shaco",
        "Shen", "Shyvana", "Singed", "Sion", "Sivir", "Skarner", "Sona", "Soraka", "Swain", "Sylas", "Syndra", "Tahm",
        "Taliyah", "Talon", "Taric", "Teemo", "Thresh", "Tristana", "Trundle", "Tryndamere", "Twisted", "Twitch", "Udyr",
        "Urgot", "Varus", "Vayne", "Veigar", "VelKoz", "Vi", "Viktor", "Vladimir", "Volibear", "Warwick", "Wukong", "Xayah",
        "Xerath", "Xin", "Yasuo", "Yorick", "Yuumi", "Zac", "Zed", "Ziggs", "Zilean", "Zoe", "Zyra"
    };
};

#endif