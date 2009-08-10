/* Copyright (C) 2006 - 2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef SC_ESCORTAI_H
#define SC_ESCORTAI_H

#define DEFAULT_MAX_PLAYER_DISTANCE 50

extern UNORDERED_MAP<uint32, std::vector<PointMovement> > PointMovementMap;

struct Escort_Waypoint
{
    Escort_Waypoint(uint32 _id, float _x, float _y, float _z, uint32 _w)
    {
        id = _id;
        x = _x;
        y = _y;
        z = _z;
        WaitTimeMs = _w;
    }

    uint32 id;
    float x;
    float y;
    float z;
    uint32 WaitTimeMs;
};

struct TRINITY_DLL_DECL npc_escortAI : public ScriptedAI
{
    public:

        // Pure Virtual Functions
        virtual void WaypointReached(uint32) = 0;

        // CreatureAI functions
        npc_escortAI(Creature *c) : ScriptedAI(c), IsBeingEscorted(false), PlayerTimer(1000), MaxPlayerDistance(DEFAULT_MAX_PLAYER_DISTANCE), CanMelee(true), DespawnAtEnd(true), DespawnAtFar(true)
        {}

        void AttackStart(Unit* who);

        void MoveInLineOfSight(Unit* who);

        void JustRespawned();

        void ReturnToLastPoint();

        void EnterEvadeMode();

        void UpdateAI(const uint32);

        void MovementInform(uint32, uint32);

        // EscortAI functions
        void AddWaypoint(uint32 id, float x, float y, float z, uint32 WaitTimeMs = 0);

        void FillPointMovementListForCreature();

        void Start(bool bAttack, bool bDefend, bool bRun, uint64 pGUID = 0);
        void SetRun(bool bRun = true);

        void SetMaxPlayerDistance(float newMax) { MaxPlayerDistance = newMax; }
        float GetMaxPlayerDistance() { return MaxPlayerDistance; }

        bool IsEscorted() {return IsBeingEscorted;}

        void SetCanMelee(bool usemelee) { CanMelee = usemelee; }
        void SetDespawnAtEnd(bool despawn) { DespawnAtEnd = despawn; }
        void SetDespawnAtFar(bool despawn) { DespawnAtFar = despawn; }
        bool GetAttack() { return Attack; }//used in EnterEvadeMode override
        bool GetIsBeingEscorted() { return IsBeingEscorted; }//used in EnterEvadeMode override
        void SetReturning(bool returning) { Returning = returning; }//used in EnterEvadeMode override
        void SetCanAttack(bool attack) { Attack = attack; }
        void SetCanDefend(bool defend) { Defend = defend; }        
        uint64 GetEventStarterGUID() { return PlayerGUID; }

    // EscortAI variables
    protected:
        uint64 PlayerGUID;
        bool IsBeingEscorted;
        bool IsOnHold;

    private:
        uint32 WaitTimer;
        uint32 PlayerTimer;
        uint32 m_uiNpcFlags;
        float MaxPlayerDistance;

        std::list<Escort_Waypoint> WaypointList;
        std::list<Escort_Waypoint>::iterator CurrentWP;

        bool Attack;
        bool Defend;
        bool Returning;
        bool ReconnectWP;
        bool bIsRunning;
        bool CanMelee;
        bool DespawnAtEnd;
        bool DespawnAtFar;
};
#endif

