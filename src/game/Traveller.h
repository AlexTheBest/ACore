/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * Copyright (C) 2008-2009 Trinity <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef TRINITY_TRAVELLER_H
#define TRINITY_TRAVELLER_H

#include "Creature.h"
#include "Player.h"
#include <cassert>
#include "CreatureGroups.h"

/** Traveller is a wrapper for units (creatures or players) that
 * travel from point A to point B using the destination holder.
 */
#define PLAYER_FLIGHT_SPEED        32.0f

template<class T>
struct TRINITY_DLL_DECL Traveller
{
    T &i_traveller;
    Traveller(T &t) : i_traveller(t) {}
    Traveller(const Traveller &obj) : i_traveller(obj) {}
    Traveller& operator=(const Traveller &obj)
    {
        this->~Traveller();
        new (this) Traveller(obj);
        return *this;
    }

    operator T&(void) { return i_traveller; }
    operator const T&(void) { return i_traveller; }
    float GetPositionX() const { return i_traveller.GetPositionX(); }
    float GetPositionY() const { return i_traveller.GetPositionY(); }
    float GetPositionZ() const { return i_traveller.GetPositionZ(); }
    T& GetTraveller(void) { return i_traveller; }

    float Speed(void) { assert(false); return 0.0f; }
    float GetMoveDestinationTo(float x, float y, float z);
    uint32 GetTotalTrevelTimeTo(float x, float y, float z);

    void Relocation(float x, float y, float z, float orientation) {}
    void Relocation(float x, float y, float z) { Relocation(x, y, z, i_traveller.GetOrientation()); }
    void MoveTo(float x, float y, float z, uint32 t) {}
};

template<class T>
inline uint32 Traveller<T>::GetTotalTrevelTimeTo(float x, float y, float z)
{
    float dist = GetMoveDestinationTo(x,y,z);
    float speed = 0.001f;
    if(GetTraveller().hasUnitState(UNIT_STAT_CHARGING))
        speed *= SPEED_CHARGE;
    else
        speed *= Speed();   // speed is in seconds so convert from second to millisecond

    return static_cast<uint32>(dist/speed);
}

// specialization for creatures
template<>
inline float Traveller<Creature>::Speed()
{
    if(i_traveller.HasUnitMovementFlag(MOVEMENTFLAG_WALK_MODE))
        return i_traveller.GetSpeed(MOVE_WALK);
    else if(i_traveller.HasUnitMovementFlag(MOVEMENTFLAG_FLYING2))
        return i_traveller.GetSpeed(MOVE_FLIGHT);
    else
        return i_traveller.GetSpeed(MOVE_RUN);
}

template<>
inline void Traveller<Creature>::Relocation(float x, float y, float z, float orientation)
{
    i_traveller.GetMap()->CreatureRelocation(&i_traveller, x, y, z, orientation);
}

template<>
inline float Traveller<Creature>::GetMoveDestinationTo(float x, float y, float z)
{
    float dx = x - GetPositionX();
    float dy = y - GetPositionY();
    float dz = z - GetPositionZ();

    if(i_traveller.hasUnitState(UNIT_STAT_IN_FLIGHT))
        return sqrt((dx*dx) + (dy*dy) + (dz*dz));
    else                                                    //Walking on the ground
        return sqrt((dx*dx) + (dy*dy));
}


template<>
inline void Traveller<Creature>::MoveTo(float x, float y, float z, uint32 t)
{
    //Call for creature group update
    if(i_traveller.IsFormationLeader() && !i_traveller.isInCombat() &&
        CreatureGroupHolder.find(i_traveller.GetFormationID()) != CreatureGroupHolder.end())
        CreatureGroupHolder[i_traveller.GetFormationID()]->LeaderMovedInEvade();

    i_traveller.AI_SendMoveToPacket(x, y, z, t, i_traveller.GetUnitMovementFlags(), 0);
}

// specialization for players
template<>
inline float Traveller<Player>::Speed()
{
    if (i_traveller.isInFlight())
        return PLAYER_FLIGHT_SPEED;
    else
        return i_traveller.GetSpeed(i_traveller.HasUnitMovementFlag(MOVEMENTFLAG_WALK_MODE) ? MOVE_WALK : MOVE_RUN);
}

template<>
inline float Traveller<Player>::GetMoveDestinationTo(float x, float y, float z)
{
    float dx = x - GetPositionX();
    float dy = y - GetPositionY();
    float dz = z - GetPositionZ();

    if (i_traveller.isInFlight())
        return sqrt((dx*dx) + (dy*dy) + (dz*dz));
    else                                                    //Walking on the ground
        return sqrt((dx*dx) + (dy*dy));
}

template<>
inline void Traveller<Player>::Relocation(float x, float y, float z, float orientation)
{
    i_traveller.GetMap()->PlayerRelocation(&i_traveller, x, y, z, orientation);
}

template<>
inline void Traveller<Player>::MoveTo(float x, float y, float z, uint32 t)
{
    //Only send MOVEMENTFLAG_WALK_MODE, client has strange issues with other move flags
    i_traveller.SendMonsterMove(x, y, z, t);
}

typedef Traveller<Creature> CreatureTraveller;
typedef Traveller<Player> PlayerTraveller;
#endif

