#include "game.h"
#include "debug.h"
#include "agent.h"
using namespace std;
//查找指定的位置有没有超级子弹道具
bool GameMap::findStarAtPos(Vector2D &pos)
{
    std::vector<Star>::iterator iter;
    for (iter=_stars.begin(); iter!=_stars.end(); iter++)
    {
        if (iter->_pos==pos)
        {
            return true;
        }
    }
    return false;
}
bool GameMap::findOppositeBulletAtPos(Vector2D &pos, Vector2D &dir, E_BULLET type)
{
    std::vector<Bullet>::iterator iter;
    std::vector<Bullet>::iterator iterEnd;
    if (type = FOE_BULLET)
    {
        iter = _foeBullets.begin();
        iterEnd = _foeBullets.end();
    }
    else
    {
        iter = _ownBullets.begin();
        iterEnd = _ownBullets.end();
    }
    for (;iter!=iterEnd;iter++)
    {
        if (iter->_pos==pos && (iter->_direction+dir).isZero())
        {
            return true;
        }
    }
    return false;
}
bool GameMap::findOppositeBulletAtPos(Vector2D &pos, Vector2D &dir, E_BULLET type, bool isSuperBullet)
{
    std::vector<Bullet>::iterator iter;
    std::vector<Bullet>::iterator iterEnd;
    if (type = FOE_BULLET)
    {
        iter = _foeBullets.begin();
        iterEnd = _foeBullets.end();
    }
    else
    {
        iter = _ownBullets.begin();
        iterEnd = _ownBullets.end();
    }
    for (;iter!=iterEnd;iter++)
    {
        if (iter->_pos==pos && (iter->_direction+dir).isZero() && isSuperBullet==iter->_super_bullet)
        {
            return true;
        }
    }
    return false;
}
#if 0
int GameMap::countOppositeBulletAtPos(Vector2D &pos, Vector2D &dir, E_BULLET type)
{
    int count = 0;
    std::vector<Bullet>::iterator iter;
    std::vector<Bullet>::iterator iterEnd;
    if (type = FOE_BULLET)
    {
        iter = _foeBullets.begin();
        iterEnd = _foeBullets.end();
    }
    else
    {
        iter = _ownBullets.begin();
        iterEnd = _ownBullets.end();
    }
    for (;iter!=iterEnd;iter++)
    {
        if (iter->_pos==pos && (iter->_direction+dir).isZero())
        {
            count++;
        }
    }
    return count;
}
#endif
bool GameMap::HaveSuperBullet()
{
    for (PlayerVecIt iter = _friend_players.begin();iter!=_friend_players.end();iter++)
    {
        if (!iter->_alive)
            continue;
        if (iter->_have_super_bullet)
            return true;
    }
    return false;
}
int GameMap::FriendPlayerCount()
{
    int count = 0;
    for (PlayerVecIt iter = _friend_players.begin();iter!=_friend_players.end();iter++)
    {
        if (!iter->_alive)
            continue;
        count++;
    }
    return count;
}
void Player::SendEventTo(Player& target_player, EVENT_TYPE type, int value)
{
    target_player.event_list.AddEvent(Event(_id, target_player._id,type,value,AGENT->leg._round_id));
}
int Player::GetEvent(EVENT_TYPE type, vector<Event>& result)
{
    result.clear();
    return event_list.GetEvent(type,result);
}
void Player::ClearEvent()
{
    event_list.clear();
}
void Player::ClearEvent(EVENT_TYPE type)
{
    event_list.Clear(type);
}
