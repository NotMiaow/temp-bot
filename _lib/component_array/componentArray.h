#ifndef COMPONENT_ARRAY_H__
#define COMPONENT_ARRAY_H__

#include <tuple>
#include <array>
#include <iterator>

template<class T, int N, int CheckpointCount>
class ComponentArray
{
public:
    struct Entry
    {
        Entry() { }
        Entry(const int& entityId, const T& data) : entityId(entityId), data(data) { }
        int entityId;
        T data;
    };
    struct Row
    {
        Row() { }
        Row(const int& index, const int& entityId, T* data) : index(index), entityId(entityId), data(data) { }
        int index;
        int entityId;
        T* data;
    };
    struct Iterator
    {
        Iterator(const typename std::array<Entry, N>::iterator& begin, const typename std::array<Entry, N>::iterator& end) :
            m_begin(begin), m_end(end), m_it(begin) { while(!End() && m_it->entityId == -1) m_it++; }
        T* GetData() { return &(m_it->data); }
        Entry* GetEntry() { return &*m_it; }
        Row GetRow() { return Row((int)std::distance(m_begin, m_it), (int)m_it->entityId, (T*)&m_it->data); }
        bool End() { return m_it == m_end; }
        void operator++(int)
        {
            do { m_it++; }
            while(!End() && m_it->entityId == -1);
        }
    private:
        const typename std::array<Entry, N>::iterator m_begin;
        const typename std::array<Entry, N>::iterator m_end;
        typename std::array<Entry, N>::iterator m_it;
    };
    ComponentArray();
    bool Initialise(const std::array<int, CheckpointCount>& sizes);
    bool Add(const T& data, const int& entityId, const int& checkpoint);
    bool Remove(const int& entityId);
    Row GetById(const int& entityId);
    const int GetCheckpointPosition(const int& entityId);
    Iterator GetIterator();
    Iterator GetIterator(const int& checkpoint);
private:
    std::array<Entry, N> m_data;
    std::array<int, CheckpointCount> m_starts;
    std::array<int, CheckpointCount> m_sizes;
    std::array<int, CheckpointCount> m_counts;
};

template<class T, int N, int CheckpointCount>
ComponentArray<T, N, CheckpointCount>::ComponentArray()
{
    T component;
    Entry entry(-1, component);
    m_data.fill(entry);
    m_starts.fill(0);
    m_sizes.fill(0);
    m_counts.fill(0);
}

template<class T, int N, int CheckpointCount>
bool ComponentArray<T, N, CheckpointCount>::Initialise(const std::array<int, CheckpointCount>& sizes)
{
    int total = 0;
    for(int i = 0; i < sizes.size(); i ++)
    {
        m_starts.at(i) = total;
        m_sizes.at(i) = sizes[i];
        total += sizes[i];
    }
    if(total != N)
    {
        m_starts.fill(0);
        m_sizes.fill(0);
        m_counts.fill(0);
        return false;
    }
    return true;
}

template<class T, int N, int CheckpointCount>
bool ComponentArray<T, N, CheckpointCount>::Add(const T& data, const int& entityId, const int& checkpoint)
{
    if(m_counts.at(checkpoint) >= m_sizes.at(checkpoint)) return false;

    for(int i = m_starts.at(checkpoint); i <= m_starts.at(checkpoint) + m_sizes.at(checkpoint); i++)
    {
        if(m_data.at(i).entityId == -1)
        {
            m_data.at(i).entityId = entityId;
            m_data.at(i).data = data;
            m_counts.at(checkpoint)++;
            return true;
        }
    }
    return false;
}

template<class T, int N, int CheckpointCount>
bool ComponentArray<T, N, CheckpointCount>::Remove(const int& entityId)
{
    for(int i = 0; i < m_data.size(); i++)
    {
        if(m_data.at(i).entityId == entityId)
        {
            m_data.at(i).entityId = -1;
            return true;
        }
    }
    return false;
}

template<class T, int N, int CheckpointCount>
typename ComponentArray<T, N, CheckpointCount>::Row ComponentArray<T, N, CheckpointCount>::GetById(const int& entityId)
{
    for(int i = 0; i < m_data.size(); i++)
        if(m_data.at(i).entityId == entityId)
            return Row(i, entityId, &m_data.at(i).data);
    return Row(-1, entityId, (T*)0);
}

template<class T, int N, int CheckpointCount>
const int ComponentArray<T, N, CheckpointCount>::GetCheckpointPosition(const int& entityId)
{
    for(int i = 0; i < m_data.size(); i++)
    {
        if(m_data.at(i).entityId == entityId)
        {
            int index = 0;
            while(i - m_sizes.at(index) >= 0)
            {
                i -= m_sizes.at(index);
                index++;
            }
            return index;
        }
    }
    return -1;
}

template<class T, int N, int CheckpointCount>
typename ComponentArray<T, N, CheckpointCount>::Iterator ComponentArray<T, N, CheckpointCount>::GetIterator()
{
    Iterator iterator(m_data.begin(), m_data.end());
    return iterator;
}

template<class T, int N, int CheckpointCount>
typename ComponentArray<T, N, CheckpointCount>::Iterator ComponentArray<T, N, CheckpointCount>::GetIterator(const int& checkpoint)
{
    Iterator iterator(m_data.begin() + m_starts.at(checkpoint), m_data.begin() + m_starts.at(checkpoint) + m_sizes.at(checkpoint));
    return iterator;
}

#endif