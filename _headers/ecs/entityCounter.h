#ifndef ENTITY_COUNTER_H__
#define ENTITY_COUNTER_H__

class EntityCounter
{
public:
    EntityCounter() { m_entityCounter = 0; }
    const int GetId() { return m_entityCounter++; }
private:
    int m_entityCounter;
};

#endif