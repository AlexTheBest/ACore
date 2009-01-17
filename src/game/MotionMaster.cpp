/*
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
 *
 * Copyright (C) 2008 Trinity <http://www.trinitycore.org/>
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

#include "MotionMaster.h"
#include "CreatureAISelector.h"
#include "Creature.h"
#include "Traveller.h"

#include "ConfusedMovementGenerator.h"
#include "FleeingMovementGenerator.h"
#include "HomeMovementGenerator.h"
#include "IdleMovementGenerator.h"
#include "PointMovementGenerator.h"
#include "TargetedMovementGenerator.h"
#include "WaypointMovementGenerator.h"
#include "RandomMovementGenerator.h"

#include <cassert>

inline bool isStatic(MovementGenerator *mv)
{
    return (mv == &si_idleMovement);
}

void
MotionMaster::Initialize()
{
    // clear ALL movement generators (including default)
    while(!empty())
    {
        MovementGenerator *curr = top();
        curr->Finalize(*i_owner);
        pop();
        if( !isStatic( curr ) )
            delete curr;
    }

    // set new default movement generator
    if(i_owner->GetTypeId() == TYPEID_UNIT)
    {
        MovementGenerator* movement = FactorySelector::selectMovementGenerator((Creature*)i_owner);
        push(  movement == NULL ? &si_idleMovement : movement );
        top()->Initialize(*i_owner);
    }
    else
        push(&si_idleMovement);
}

MotionMaster::~MotionMaster()
{
    // clear ALL movement generators (including default)
    while(!empty())
    {
        MovementGenerator *curr = top();
        curr->Finalize(*i_owner);
        pop();
        if( !isStatic( curr ) )
            delete curr;
    }
}

void
MotionMaster::UpdateMotion(const uint32 &diff)
{
    if( i_owner->hasUnitState(UNIT_STAT_ROOT | UNIT_STAT_STUNNED) )
        return;
    assert( !empty() );
    if (!top()->Update(*i_owner, diff))
        MovementExpired();
}

void
MotionMaster::Clear(bool reset)
{
    while( !empty() && size() > 1 )
    {
        MovementGenerator *curr = top();
        curr->Finalize(*i_owner);
        pop();
        if( !isStatic( curr ) )
            delete curr;
    }

    if (reset)
    {
        assert( !empty() );
        top()->Reset(*i_owner);
    }
}

void
MotionMaster::MoveRandom(float spawndist)
{
	if(i_owner->GetTypeId()==TYPEID_UNIT)
    {
        DEBUG_LOG("Creature (GUID: %u) start moving random", i_owner->GetGUIDLow() );
        Mutate(new RandomMovementGenerator<Creature>(spawndist), MOTION_SLOT_IDLE);
    }
}

void
MotionMaster::MovementExpired(bool reset)
{
    if( empty() || size() == 1 )
        return;

    MovementGenerator *curr = top();
    curr->Finalize(*i_owner);
    pop();

    if( !isStatic(curr) )
        delete curr;

    assert( !empty() );
    while(!top())
        --i_top;
    /*while( !empty() && top()->GetMovementGeneratorType() == TARGETED_MOTION_TYPE )
    {
        // Should check if target is still valid? If not valid it will crash.
        curr = top();
        curr->Finalize(*i_owner);
        pop();
        delete curr;
    }*/
    if( empty() )
        Initialize();
    if (reset) top()->Reset(*i_owner);
}

void MotionMaster::MoveIdle(MovementSlot slot)
{
    //if( empty() || !isStatic( top() ) )
    //    push( &si_idleMovement );
    if(!isStatic(Impl[slot]))
        Mutate(&si_idleMovement, slot);
}

void
MotionMaster::MoveTargetedHome()
{
    if(i_owner->hasUnitState(UNIT_STAT_FLEEING))
        return;

    Clear(false);

    if(i_owner->GetTypeId()==TYPEID_UNIT && !((Creature*)i_owner)->GetCharmerOrOwnerGUID())
    {
        DEBUG_LOG("Creature (Entry: %u GUID: %u) targeted home", i_owner->GetEntry(), i_owner->GetGUIDLow());
        Mutate(new HomeMovementGenerator<Creature>(), MOTION_SLOT_ACTIVE);
    }
    else if(i_owner->GetTypeId()==TYPEID_UNIT && ((Creature*)i_owner)->GetCharmerOrOwnerGUID())
    {
        DEBUG_LOG("Pet or controlled creature (Entry: %u GUID: %u) targeting home",
            i_owner->GetEntry(), i_owner->GetGUIDLow() );
        Unit *target = ((Creature*)i_owner)->GetCharmerOrOwner();
        if(target)
        {
            i_owner->addUnitState(UNIT_STAT_FOLLOW);
            DEBUG_LOG("Following %s (GUID: %u)",
                target->GetTypeId()==TYPEID_PLAYER ? "player" : "creature",
                target->GetTypeId()==TYPEID_PLAYER ? target->GetGUIDLow() : ((Creature*)target)->GetDBTableGUIDLow() );
            Mutate(new TargetedMovementGenerator<Creature>(*target,PET_FOLLOW_DIST,PET_FOLLOW_ANGLE), MOTION_SLOT_ACTIVE);
        }
    }
    else
    {
        sLog.outError("Player (GUID: %u) attempt targeted home", i_owner->GetGUIDLow() );
    }
}

void
MotionMaster::MoveConfused()
{
    if(i_owner->GetTypeId()==TYPEID_PLAYER)
    {
        DEBUG_LOG("Player (GUID: %u) move confused", i_owner->GetGUIDLow() );
        Mutate(new ConfusedMovementGenerator<Player>(), MOTION_SLOT_CONTROLLED);
    }
    else
    {
        DEBUG_LOG("Creature (Entry: %u GUID: %u) move confused",
            i_owner->GetEntry(), i_owner->GetGUIDLow() );
        Mutate(new ConfusedMovementGenerator<Creature>(), MOTION_SLOT_CONTROLLED);
    }
}

void
MotionMaster::MoveChase(Unit* target, float dist, float angle)
{
    // ignore movement request if target not exist
    if(!target)
        return;

    i_owner->clearUnitState(UNIT_STAT_FOLLOW);
    if(i_owner->GetTypeId()==TYPEID_PLAYER)
    {
        DEBUG_LOG("Player (GUID: %u) chase to %s (GUID: %u)",
            i_owner->GetGUIDLow(),
            target->GetTypeId()==TYPEID_PLAYER ? "player" : "creature",
            target->GetTypeId()==TYPEID_PLAYER ? i_owner->GetGUIDLow() : ((Creature*)i_owner)->GetDBTableGUIDLow() );
        Mutate(new TargetedMovementGenerator<Player>(*target,dist,angle), MOTION_SLOT_ACTIVE);
    }
    else
    {
        DEBUG_LOG("Creature (Entry: %u GUID: %u) chase to %s (GUID: %u)",
            i_owner->GetEntry(), i_owner->GetGUIDLow(),
            target->GetTypeId()==TYPEID_PLAYER ? "player" : "creature",
            target->GetTypeId()==TYPEID_PLAYER ? target->GetGUIDLow() : ((Creature*)target)->GetDBTableGUIDLow() );
        Mutate(new TargetedMovementGenerator<Creature>(*target,dist,angle), MOTION_SLOT_ACTIVE);
    }
}

void
MotionMaster::MoveFollow(Unit* target, float dist, float angle)
{
    // ignore movement request if target not exist
    if(!target)
        return;

    i_owner->addUnitState(UNIT_STAT_FOLLOW);
    if(i_owner->GetTypeId()==TYPEID_PLAYER)
    {
        DEBUG_LOG("Player (GUID: %u) follow to %s (GUID: %u)", i_owner->GetGUIDLow(),
            target->GetTypeId()==TYPEID_PLAYER ? "player" : "creature",
            target->GetTypeId()==TYPEID_PLAYER ? i_owner->GetGUIDLow() : ((Creature*)i_owner)->GetDBTableGUIDLow() );
        Mutate(new TargetedMovementGenerator<Player>(*target,dist,angle), MOTION_SLOT_ACTIVE);
    }
    else
    {
        DEBUG_LOG("Creature (Entry: %u GUID: %u) follow to %s (GUID: %u)",
            i_owner->GetEntry(), i_owner->GetGUIDLow(),
            target->GetTypeId()==TYPEID_PLAYER ? "player" : "creature",
            target->GetTypeId()==TYPEID_PLAYER ? target->GetGUIDLow() : ((Creature*)target)->GetDBTableGUIDLow() );
        Mutate(new TargetedMovementGenerator<Creature>(*target,dist,angle), MOTION_SLOT_ACTIVE);
    }
}

void
MotionMaster::MovePoint(uint32 id, float x, float y, float z)
{
    if(i_owner->GetTypeId()==TYPEID_PLAYER)
    {
        DEBUG_LOG("Player (GUID: %u) targeted point (Id: %u X: %f Y: %f Z: %f)", i_owner->GetGUIDLow(), id, x, y, z );
        Mutate(new PointMovementGenerator<Player>(id,x,y,z), MOTION_SLOT_ACTIVE);
    }
    else
    {
        DEBUG_LOG("Creature (Entry: %u GUID: %u) targeted point (ID: %u X: %f Y: %f Z: %f)",
            i_owner->GetEntry(), i_owner->GetGUIDLow(), id, x, y, z );
        Mutate(new PointMovementGenerator<Creature>(id,x,y,z), MOTION_SLOT_ACTIVE);
    }
}

void
MotionMaster::MoveFleeing(Unit* enemy)
{
    if(!enemy)
        return;

    if(i_owner->HasAuraType(SPELL_AURA_PREVENTS_FLEEING))
        return;

    if(i_owner->GetTypeId()==TYPEID_PLAYER)
    {
        DEBUG_LOG("Player (GUID: %u) flee from %s (GUID: %u)", i_owner->GetGUIDLow(),
            enemy->GetTypeId()==TYPEID_PLAYER ? "player" : "creature",
            enemy->GetTypeId()==TYPEID_PLAYER ? enemy->GetGUIDLow() : ((Creature*)enemy)->GetDBTableGUIDLow() );
        Mutate(new FleeingMovementGenerator<Player>(enemy->GetGUID()), MOTION_SLOT_CONTROLLED);
    }
    else
    {
        DEBUG_LOG("Creature (Entry: %u GUID: %u) flee from %s (GUID: %u)",
            i_owner->GetEntry(), i_owner->GetGUIDLow(),
            enemy->GetTypeId()==TYPEID_PLAYER ? "player" : "creature",
            enemy->GetTypeId()==TYPEID_PLAYER ? enemy->GetGUIDLow() : ((Creature*)enemy)->GetDBTableGUIDLow() );
        Mutate(new FleeingMovementGenerator<Creature>(enemy->GetGUID()), MOTION_SLOT_CONTROLLED);
    }
}

void
MotionMaster::MoveTaxiFlight(uint32 path, uint32 pathnode)
{
    if(i_owner->GetTypeId()==TYPEID_PLAYER)
    {
        DEBUG_LOG("Player (GUID: %u) taxi to (Path %u node %u)", i_owner->GetGUIDLow(), path, pathnode);
        FlightPathMovementGenerator* mgen = new FlightPathMovementGenerator(path,pathnode);
        Mutate(mgen, MOTION_SLOT_CONTROLLED);
    }
    else
    {
        sLog.outError("Creature (Entry: %u GUID: %u) attempt taxi to (Path %u node %u)",
            i_owner->GetEntry(), i_owner->GetGUIDLow(), path, pathnode );
    }
}

void
MotionMaster::MoveDistract(uint32 timer)
{
    if(Impl[MOTION_SLOT_CONTROLLED])
        return;

    if(i_owner->GetTypeId()==TYPEID_PLAYER)
    {
        DEBUG_LOG("Player (GUID: %u) distracted (timer: %u)", i_owner->GetGUIDLow(), timer);
    }
    else
    {
        DEBUG_LOG("Creature (Entry: %u GUID: %u) (timer: %u)",
            i_owner->GetEntry(), i_owner->GetGUIDLow(), timer);
    }

    DistractMovementGenerator* mgen = new DistractMovementGenerator(timer);
    Mutate(mgen, MOTION_SLOT_CONTROLLED);
}

void MotionMaster::Mutate(MovementGenerator *m, MovementSlot slot)
{
    if(MovementGenerator *curr = Impl[slot])
    {
        curr->Finalize(*i_owner);
        if( !isStatic( curr ) )
            delete curr;
    }
    else if(i_top < slot)
    {
        i_top = slot;        
    }
    m->Initialize(*i_owner);
    Impl[slot] = m;

    /*if (!empty())
    {
        switch(top()->GetMovementGeneratorType())
        {
            // HomeMovement is not that important, delete it if meanwhile a new comes
            case HOME_MOTION_TYPE:
            // DistractMovement interrupted by any other movement
            case DISTRACT_MOTION_TYPE:
                MovementExpired(false);
        }
    }
    m->Initialize(*i_owner);
    push(m);*/
}

void MotionMaster::MovePath(uint32 path_id, bool repeatable)
{
	if(!path_id)
		return;
	//We set waypoint movement as new default movement generator
	// clear ALL movement generators (including default)
    while(!empty())
    {
        MovementGenerator *curr = top();
        curr->Finalize(*i_owner);
        pop();
        if( !isStatic( curr ) )
            delete curr;
    }
	
	//i_owner->GetTypeId()==TYPEID_PLAYER ?
		//Mutate(new WaypointMovementGenerator<Player>(path_id, repeatable)):
		Mutate(new WaypointMovementGenerator<Creature>(path_id, repeatable), MOTION_SLOT_IDLE);
	
	DEBUG_LOG("%s (GUID: %u) start moving over path(Id:%u, repeatable: %s)", 
		i_owner->GetTypeId()==TYPEID_PLAYER ? "Player" : "Creature", 
		i_owner->GetGUIDLow(), path_id, repeatable ? "YES" : "NO" );
}

void MotionMaster::propagateSpeedChange()
{
    /*Impl::container_type::iterator it = Impl::c.begin();
    for ( ;it != end(); ++it)
    {
        (*it)->unitSpeedChanged();
    }*/
    for(int i = 0; i <= i_top; ++i)
        Impl[i]->unitSpeedChanged();
}

MovementGeneratorType MotionMaster::GetCurrentMovementGeneratorType() const
{
   if(empty())
       return IDLE_MOTION_TYPE;

   return top()->GetMovementGeneratorType();
}

bool MotionMaster::GetDestination(float &x, float &y, float &z)
{
   if(empty())
       return false;

   return top()->GetDestination(x,y,z);
}
