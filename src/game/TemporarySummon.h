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

#ifndef TRINITYCORE_TEMPSUMMON_H
#define TRINITYCORE_TEMPSUMMON_H

#include "Creature.h"

class TempSummon : public Creature
{
    public:
        explicit TempSummon(SummonPropertiesEntry const *properties, Unit *owner);
        virtual ~TempSummon(){};
        void Update(uint32 time);
        virtual void InitSummon(uint32 lifetime);
        void UnSummon();
        void RemoveFromWorld();
        void SetTempSummonType(TempSummonType type);
        void SaveToDB();
        Unit* GetSummoner() const;

        SummonPropertiesEntry const *m_Properties;

        bool SetOwner(Unit *owner, bool apply);
    private:
        TempSummonType m_type;
        uint32 m_timer;
        uint32 m_lifetime;
        uint64 m_summonerGUID;
};

class Guardian : public TempSummon
{
    public:
        Guardian(SummonPropertiesEntry const *properties, Unit *owner);
        bool Create(uint32 guidlow, Map *map, uint32 phaseMask, uint32 Entry, uint32 pet_number);
        void InitSummon(uint32 duration);
        void RemoveFromWorld();
        bool InitStatsForLevel(uint32 level);

        int32 GetBonusDamage() { return m_bonusdamage; }
        void SetBonusDamage(int32 damage) { m_bonusdamage = damage; }

        Unit *GetOwner() { return m_owner; }
    protected:
        Unit *m_owner;
        int32   m_bonusdamage;
};

#endif

