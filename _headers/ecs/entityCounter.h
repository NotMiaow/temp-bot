#ifndef ENTITY_COUNTER_H__
#define ENTITY_COUNTER_H__

class EntityCounter
{
public:
    EntityCounter() { m_entityCounter = 0; }
    const uint GetId() { return m_entityCounter++; }
private:
    uint m_entityCounter;
};

#endif