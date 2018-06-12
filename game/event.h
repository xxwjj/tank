#pragma once

#include "game.h"

enum  EVENT_TYPE
{
    E_COOR_ATTACK,
    E_EVENT_TYPE_BUTT
};
class Event
{
public:
    Event(int from, int to, EVENT_TYPE type, int value, int round_id): _from_player_id(from), _to_player_id(to), _type(type),_value(value), _round_id(round_id){};
    EVENT_TYPE  _type;
    int _from_player_id;
    int _to_player_id;
    int _value;
    int _round_id;
};

class EventList :public  vector<Event>
{
public:
    int GetEvent(EVENT_TYPE type, vector<Event> & result)
    {
        int count = 0;
        for (vector<Event>::iterator iter = begin(); iter != end(); iter++)
        {
            if (type == iter->_type)
            {
                result.push_back(*iter);
                count++;
            }
        }
        return count;
    }
    void AddEvent(Event in_put)
    {
        push_back(in_put);
    }
    void Clear(EVENT_TYPE type)
    {
        if (size() ==0)
        {
            return ;
        }
        vector<Event>::iterator next = end() - 1;
        vector<Event>::iterator cur = end() - 1;
        do{
            cur = next;
            next = cur - 1;
            if (cur->_type == type)
            {
                erase(cur);
            }
        } while (cur == begin());

    }
};