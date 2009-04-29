/* Copyright (C) 2009 Trinity <http://www.trinitycore.org/>
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

/* ScriptData
SDName: Boss_Eredar_Twins
SD%Complete: 100
SDComment:
EndScriptData */

#include "precompiled.h"
#include "def_sunwell_plateau.h"

enum Quotes
{
    //Alytesh
    YELL_CANFLAGRATION          =   -1580044,
    YELL_SISTER_SACROLASH_DEAD  =   -1580045,
    YELL_ALY_KILL_1             =   -1580046,
    YELL_ALY_KILL_2             =   -1580047,
    YELL_ALY_DEAD               =   -1580048,
    YELL_BERSERK                =   -1580049,

    //Sacrolash
    YELL_SHADOW_NOVA            =   -1580050,
    YELL_SISTER_ALYTHESS_DEAD   =   -1580051,
    YELL_SAC_KILL_1             =   -1580052,
    YELL_SAC_KILL_2             =   -1580053,
    SAY_SAC_DEAD                =   -1580054,
    YELL_ENRAGE                 =   -1580055,

    //Intro
    YELL_INTRO_SAC_1            =   -1580056,
    YELL_INTRO_ALY_2            =   -1580057,
    YELL_INTRO_SAC_3            =   -1580058,
    YELL_INTRO_ALY_4            =   -1580059,
    YELL_INTRO_SAC_5            =   -1580060,
    YELL_INTRO_ALY_6            =   -1580061,
    YELL_INTRO_SAC_7            =   -1580062,
    YELL_INTRO_ALY_8            =   -1580063,

    //Emote
    EMOTE_SHADOW_NOVA           =   -1580064,
    EMOTE_CONFLAGRATION         =   -1580065
};

enum Spells
{
    //Lady Sacrolash spells
    SPELL_DARK_TOUCHED      =   45347,
    SPELL_SHADOW_BLADES     =   45248, //10 secs
    SPELL_DARK_STRIKE       =   45271,
    SPELL_SHADOW_NOVA       =   45329, //30-35 secs
    SPELL_CONFOUNDING_BLOW  =   45256, //25 secs

    //Shadow Image spells
    SPELL_SHADOW_FURY       =   45270,
    SPELL_IMAGE_VISUAL      =   45263,

    //Misc spells
    SPELL_ENRAGE            =   46587,
    SPELL_EMPOWER           =   45366,
    SPELL_DARK_FLAME        =   45345,

    //Grand Warlock Alythess spells
    SPELL_PYROGENICS        =   45230, //15secs
    SPELL_FLAME_TOUCHED     =   45348,
    SPELL_CONFLAGRATION     =   45342, //30-35 secs
    SPELL_BLAZE             =   45235, //on main target every 3 secs
    SPELL_FLAME_SEAR        =   46771,
    SPELL_BLAZE_SUMMON      =   45236, //187366 GO
    SPELL_BLAZE_BURN        =   45246
};

enum Creatures
{
    GRAND_WARLOCK_ALYTHESS  =   25166,
    MOB_SHADOW_IMAGE        =   25214,
    LADY_SACROLASH          =   25165
};

struct TRINITY_DLL_DECL boss_sacrolashAI : public ScriptedAI
{
    boss_sacrolashAI(Creature *c) : ScriptedAI(c){
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
    }

    ScriptedInstance *pInstance;

    bool InCombat;
    bool SisterDeath;
    bool Enraged;

    uint32 ShadowbladesTimer;
    uint32 ShadownovaTimer;
    uint32 ConfoundingblowTimer;
    uint32 ShadowimageTimer;
    uint32 ConflagrationTimer;
    uint32 EnrageTimer;

    void Reset()
    {
        InCombat = false;
        Enraged = false;

        if(pInstance)
        {
            Unit* Temp =  Unit::GetUnit((*m_creature),pInstance->GetData64(DATA_ALYTHESS));
            if (Temp)
                if (Temp->isDead())
                {
                    ((Creature*)Temp)->Respawn();
                }else
                {
                    if(Temp->getVictim())
                    {
                        m_creature->getThreatManager().addThreat(Temp->getVictim(),0.0f);
                        InCombat = true;
                    }
                }
        }

        if(!InCombat)
        {
            ShadowbladesTimer = 10000;
            ShadownovaTimer = 30000;
            ConfoundingblowTimer = 25000;
            ShadowimageTimer = 20000;
            ConflagrationTimer = 30000;
            EnrageTimer = 360000;

            SisterDeath = false;
        }

        if(pInstance)
            pInstance->SetData(DATA_EREDAR_TWINS_EVENT, NOT_STARTED);
    }
    
    void EnterCombat(Unit *who)
    {
        DoZoneInCombat();

        if(pInstance)
        {
            Unit* Temp =  Unit::GetUnit((*m_creature),pInstance->GetData64(DATA_ALYTHESS));
            if (Temp && Temp->isAlive() && !(Temp->getVictim()))
                ((Creature*)Temp)->AI()->AttackStart(who);
        }

        if(pInstance)
            pInstance->SetData(DATA_EREDAR_TWINS_EVENT, IN_PROGRESS);
    }

    void KilledUnit(Unit *victim)
    {
        if(rand()%4 == 0)
        {
            switch (rand()%2)
            {
            case 0: DoScriptText(YELL_SAC_KILL_1, m_creature); break;
            case 1: DoScriptText(YELL_SAC_KILL_2, m_creature); break;
            }
        }
    }

    void JustDied(Unit* Killer)
    {
        // only if ALY death
        if (SisterDeath)
        {
            DoScriptText(SAY_SAC_DEAD, m_creature);

            if(pInstance)
                pInstance->SetData(DATA_EREDAR_TWINS_EVENT, DONE);
        }
        else
            m_creature->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
    }

    void SpellHitTarget(Unit* target,const SpellEntry* spell)
    {
        switch(spell->Id)
        {
        case SPELL_SHADOW_BLADES:
        case SPELL_SHADOW_NOVA:
        case SPELL_CONFOUNDING_BLOW:
        case SPELL_SHADOW_FURY:
            HandleTouchedSpells(target, SPELL_DARK_TOUCHED);
            break;
        case SPELL_CONFLAGRATION:
            HandleTouchedSpells(target, SPELL_FLAME_TOUCHED);
            break;
        }
    }

    void HandleTouchedSpells(Unit* target, uint32 TouchedType)
    {
        switch(TouchedType)
        {
        case SPELL_FLAME_TOUCHED:
            if(!target->HasAura(SPELL_DARK_FLAME))
            {
                if(target->HasAura(SPELL_DARK_TOUCHED))
                {
                    target->RemoveAurasDueToSpell(SPELL_DARK_TOUCHED);
                    target->CastSpell(target, SPELL_DARK_FLAME, true);
                }else target->CastSpell(target, SPELL_FLAME_TOUCHED, true);
            }
            break;
        case SPELL_DARK_TOUCHED:
            if(!target->HasAura(SPELL_DARK_FLAME))
            {
                if(target->HasAura(SPELL_FLAME_TOUCHED))
                {
                    target->RemoveAurasDueToSpell(SPELL_FLAME_TOUCHED);
                    target->CastSpell(target, SPELL_DARK_FLAME, true);
                }else target->CastSpell(target, SPELL_DARK_TOUCHED, true);
            }
            break;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!SisterDeath)
        {
            if (pInstance)
            {
                Unit* Temp = NULL;
                Temp = Unit::GetUnit((*m_creature),pInstance->GetData64(DATA_ALYTHESS));
                if (Temp && Temp->isDead())
                {
                    DoScriptText(YELL_SISTER_ALYTHESS_DEAD, m_creature);
                    DoCast(m_creature,SPELL_EMPOWER);
                    m_creature->InterruptSpell(CURRENT_GENERIC_SPELL);
                    SisterDeath = true;
                }
            }
        }

        if (!UpdateVictim())
            return;

        if(SisterDeath)
        {
            if (ConflagrationTimer < diff)
            {
                if (!m_creature->IsNonMeleeSpellCasted(false))
                {
                    m_creature->InterruptSpell(CURRENT_GENERIC_SPELL);
                    Unit* target = NULL;
                    target = SelectUnit(SELECT_TARGET_RANDOM, 0);
                    DoCast(target, SPELL_CONFLAGRATION);
                    ConflagrationTimer = 30000+(rand()%5000);
                }
            }else ConflagrationTimer -= diff;
        }
        else
        {
            if(ShadownovaTimer < diff)
            {
                if (!m_creature->IsNonMeleeSpellCasted(false))
                {
                    Unit* target = NULL;
                    target = SelectUnit(SELECT_TARGET_RANDOM, 0);
                    DoCast(target, SPELL_SHADOW_NOVA);

                    if(!SisterDeath)
                    {
                        DoScriptText(EMOTE_SHADOW_NOVA, m_creature, target);
                        DoScriptText(YELL_SHADOW_NOVA, m_creature);
                    }
                    ShadownovaTimer = 30000+(rand()%5000);
                }
            }else ShadownovaTimer -=diff;
        }

        if(ConfoundingblowTimer < diff)
        {
            if (!m_creature->IsNonMeleeSpellCasted(false))
            {
                Unit* target = NULL;
                target = SelectUnit(SELECT_TARGET_RANDOM, 0);
                DoCast(target, SPELL_CONFOUNDING_BLOW);
                ConfoundingblowTimer = 20000 + (rand()%5000);
            }
        }else ConfoundingblowTimer -=diff;

        if(ShadowimageTimer < diff)
        {
            Unit* target = NULL;
            Creature* temp = NULL;
            for(int i = 0;i<3;i++)
            {
                target = SelectUnit(SELECT_TARGET_RANDOM, 0);
                temp = DoSpawnCreature(MOB_SHADOW_IMAGE,0,0,0,0,TEMPSUMMON_CORPSE_DESPAWN,10000);
                temp->AI()->AttackStart(target);
            }
            ShadowimageTimer = 20000;
        }else ShadowimageTimer -=diff;

        if(ShadowbladesTimer < diff)
        {
            if (!m_creature->IsNonMeleeSpellCasted(false))
            {
                DoCast(m_creature, SPELL_SHADOW_BLADES);
                ShadowbladesTimer = 10000;
            }
        }else ShadowbladesTimer -=diff;

        if (EnrageTimer < diff && !Enraged)
        {
            m_creature->InterruptSpell(CURRENT_GENERIC_SPELL);
            DoScriptText(YELL_ENRAGE, m_creature);
            DoCast(m_creature,SPELL_ENRAGE);
            Enraged = true;
        }else EnrageTimer -= diff;

        if( m_creature->isAttackReady() && !m_creature->IsNonMeleeSpellCasted(false))
        {
            //If we are within range melee the target
            if( m_creature->IsWithinMeleeRange(m_creature->getVictim()))
            {
                HandleTouchedSpells(m_creature->getVictim(), SPELL_DARK_TOUCHED);
                m_creature->AttackerStateUpdate(m_creature->getVictim());
                m_creature->resetAttackTimer();
            }
        }
    }
};

CreatureAI* GetAI_boss_sacrolash(Creature *_Creature)
{
    return new boss_sacrolashAI (_Creature);
};

struct TRINITY_DLL_DECL boss_alythessAI : public Scripted_NoMovementAI
{
    boss_alythessAI(Creature *c) : Scripted_NoMovementAI(c){
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
        IntroStepCounter = 10;
    }

    ScriptedInstance *pInstance;

    bool InCombat;
    bool SisterDeath;
    bool Enraged;

    uint32 IntroStepCounter;
    uint32 IntroYellTimer;

    uint32 ConflagrationTimer;
    uint32 BlazeTimer;
    uint32 PyrogenicsTimer;
    uint32 ShadownovaTimer;
    uint32 FlamesearTimer;
    uint32 EnrageTimer;

    void Reset()
    {
        InCombat = false;
        Enraged = false;

        if(pInstance)
        {
            Unit* Temp =  Unit::GetUnit((*m_creature),pInstance->GetData64(DATA_SACROLASH));
            if (Temp)
                if (Temp->isDead())
                {
                    ((Creature*)Temp)->Respawn();
                }else
                {
                    if(Temp->getVictim())
                    {
                        m_creature->getThreatManager().addThreat(Temp->getVictim(),0.0f);
                        InCombat = true;
                    }
                }
        }

        if(!InCombat)
        {
            ConflagrationTimer = 45000;
            BlazeTimer = 100;
            PyrogenicsTimer = 15000;
            ShadownovaTimer = 40000;
            EnrageTimer = 360000;
            FlamesearTimer = 15000;
            IntroYellTimer = 10000;

            SisterDeath = false;
        }

        if(pInstance)
            pInstance->SetData(DATA_EREDAR_TWINS_EVENT, NOT_STARTED);
    }
    
    void EnterCombat(Unit *who)
    {
        DoZoneInCombat();

        if(pInstance)
        {
            Unit* Temp =  Unit::GetUnit((*m_creature),pInstance->GetData64(DATA_SACROLASH));
            if (Temp && Temp->isAlive() && !(Temp->getVictim()))
                ((Creature*)Temp)->AI()->AttackStart(who);
        }

        if(pInstance)
            pInstance->SetData(DATA_EREDAR_TWINS_EVENT, IN_PROGRESS);
    }

    void AttackStart(Unit *who)
    {
        if (!InCombat)
        {
            Scripted_NoMovementAI::AttackStart(who);
        }
    }

    void MoveInLineOfSight(Unit *who)
    {
        if (!who || m_creature->getVictim())
            return;

        if (who->isTargetableForAttack() && who->isInAccessiblePlaceFor(m_creature) && m_creature->IsHostileTo(who))
        {

            float attackRadius = m_creature->GetAttackDistance(who);
            if (m_creature->IsWithinDistInMap(who, attackRadius) && m_creature->GetDistanceZ(who) <= CREATURE_Z_ATTACK_RANGE && m_creature->IsWithinLOSInMap(who))
            {
                if (!m_creature->isInCombat())
                {
                    DoStartNoMovement(who);
                }
            }
        }
        else if (IntroStepCounter == 10 && m_creature->IsWithinLOSInMap(who)&& m_creature->IsWithinDistInMap(who, 30) )
        {
            IntroStepCounter = 0;
        }
    }

    void KilledUnit(Unit *victim)
    {
        if(rand()%4 == 0)
        {
            switch (rand()%2)
            {
            case 0: DoScriptText(YELL_ALY_KILL_1, m_creature); break;
            case 1: DoScriptText(YELL_ALY_KILL_2, m_creature); break;
            }
        }
    }

    void JustDied(Unit* Killer)
    {
        if (SisterDeath)
        {
            DoScriptText(YELL_ALY_DEAD, m_creature);

            if(pInstance)
                pInstance->SetData(DATA_EREDAR_TWINS_EVENT, DONE);
        }
        else
            m_creature->RemoveFlag(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_LOOTABLE);
    }

    void SpellHitTarget(Unit* target,const SpellEntry* spell)
    {
        switch(spell->Id)
        {

        case SPELL_BLAZE:
            target->CastSpell(target, SPELL_BLAZE_SUMMON, true);
        case SPELL_CONFLAGRATION:
        case SPELL_FLAME_SEAR:
            HandleTouchedSpells(target, SPELL_FLAME_TOUCHED);
            break;
        case SPELL_SHADOW_NOVA:
            HandleTouchedSpells(target, SPELL_DARK_TOUCHED);
            break;
        }
    }

    void HandleTouchedSpells(Unit* target, uint32 TouchedType)
    {
        switch(TouchedType)
        {
        case SPELL_FLAME_TOUCHED:
            if(!target->HasAura(SPELL_DARK_FLAME))
            {
                if(target->HasAura(SPELL_DARK_TOUCHED))
                {
                    target->RemoveAurasDueToSpell(SPELL_DARK_TOUCHED);
                    target->CastSpell(target, SPELL_DARK_FLAME, true);
                }else
                {
                    target->CastSpell(target, SPELL_FLAME_TOUCHED, true);
                }
            }
            break;
        case SPELL_DARK_TOUCHED:
            if(!target->HasAura(SPELL_DARK_FLAME))
            {
                if(target->HasAura(SPELL_FLAME_TOUCHED))
                {
                    target->RemoveAurasDueToSpell(SPELL_FLAME_TOUCHED);
                    target->CastSpell(target, SPELL_DARK_FLAME, true);
                }else target->CastSpell(target, SPELL_DARK_TOUCHED, true);
            }
            break;
        }
    }

    uint32 IntroStep(uint32 step)
    {
        Creature* Sacrolash = (Creature*)Unit::GetUnit((*m_creature),pInstance->GetData64(DATA_SACROLASH));
        switch (step)
        {
        case 0: return 0;
        case 1:
            if(Sacrolash)
                DoScriptText(YELL_INTRO_SAC_1, Sacrolash);
            return 1000;
        case 2: DoScriptText(YELL_INTRO_ALY_2, m_creature); return 1000;
        case 3:
            if(Sacrolash)
                DoScriptText(YELL_INTRO_SAC_3, Sacrolash);
            return 2000;
        case 4: DoScriptText(YELL_INTRO_ALY_4, m_creature); return 1000;
        case 5:
            if(Sacrolash)
                DoScriptText(YELL_INTRO_SAC_5, Sacrolash);
            return 2000;
        case 6: DoScriptText(YELL_INTRO_ALY_6, m_creature); return 1000;
        case 7:
            if(Sacrolash)
                DoScriptText(YELL_INTRO_SAC_7, Sacrolash);
            return 3000;
        case 8: DoScriptText(YELL_INTRO_ALY_8, m_creature); return 900000;
        }
        return 10000;
    }

    void UpdateAI(const uint32 diff)
    {
        if(IntroStepCounter < 9)
        {
            if(IntroYellTimer < diff)
            {
                IntroYellTimer = IntroStep(++IntroStepCounter);
            }else IntroYellTimer -= diff;
        }

        if(!SisterDeath)
        {
            if (pInstance)
            {
                Unit* Temp = NULL;
                Temp = Unit::GetUnit((*m_creature),pInstance->GetData64(DATA_SACROLASH));
                if (Temp && Temp->isDead())
                {
                    DoScriptText(YELL_SISTER_SACROLASH_DEAD, m_creature);
                    DoCast(m_creature, SPELL_EMPOWER);
                    m_creature->InterruptSpell(CURRENT_GENERIC_SPELL);
                    SisterDeath = true;
                }
            }
        }

        if (!UpdateVictim())
            return;

        if(SisterDeath)
        {
            if(ShadownovaTimer < diff)
            {
                if (!m_creature->IsNonMeleeSpellCasted(false))
                {
                    Unit* target = NULL;
                    target = SelectUnit(SELECT_TARGET_RANDOM, 0);
                    DoCast(target, SPELL_SHADOW_NOVA);
                    ShadownovaTimer= 30000+(rand()%5000);
                }
            }else ShadownovaTimer -=diff;
        }
        else
        {
            if(ConflagrationTimer < diff)
            {
                if (!m_creature->IsNonMeleeSpellCasted(false))
                {
                    m_creature->InterruptSpell(CURRENT_GENERIC_SPELL);
                    Unit* target = NULL;
                    target = SelectUnit(SELECT_TARGET_RANDOM, 0);
                    DoCast(target, SPELL_CONFLAGRATION);
                    ConflagrationTimer = 30000+(rand()%5000);

                    if(!SisterDeath)
                    {
                        DoScriptText(EMOTE_CONFLAGRATION, m_creature, target);
                        DoScriptText(YELL_CANFLAGRATION, m_creature);
                    }

                    BlazeTimer = 4000;
                }
            }else ConflagrationTimer -= diff;
        }

        if(FlamesearTimer < diff)
        {
            if(!m_creature->IsNonMeleeSpellCasted(false))
            {
                DoCast(m_creature, SPELL_FLAME_SEAR);
                FlamesearTimer = 15000;
            }
        }else FlamesearTimer -=diff;

        if (PyrogenicsTimer < diff)
        {
            if(!m_creature->IsNonMeleeSpellCasted(false))
            {
                DoCast(m_creature, SPELL_PYROGENICS,true);
                PyrogenicsTimer = 15000;
            }
        }else PyrogenicsTimer -= diff;

        if (BlazeTimer < diff)
        {
            if(!m_creature->IsNonMeleeSpellCasted(false))
            {
                DoCast(m_creature->getVictim(), SPELL_BLAZE);
                BlazeTimer = 3800;
            }
        }else BlazeTimer -= diff;

        if (EnrageTimer < diff && !Enraged)
        {
            m_creature->InterruptSpell(CURRENT_GENERIC_SPELL);
            DoScriptText(YELL_BERSERK, m_creature);
            DoCast(m_creature, SPELL_ENRAGE);
            Enraged = true;
        }else EnrageTimer -= diff;
    }
};

CreatureAI* GetAI_boss_alythess(Creature *_Creature)
{
    return new boss_alythessAI (_Creature);
};

struct TRINITY_DLL_DECL mob_shadow_imageAI : public ScriptedAI
{
    mob_shadow_imageAI(Creature *c) : ScriptedAI(c) {}

    uint32 ShadowfuryTimer;
    uint32 KillTimer;
    uint32 DarkstrikeTimer;

    void Reset()
    {
        ShadowfuryTimer = 5000 + (rand()%15000);
        DarkstrikeTimer = 3000;
        KillTimer = 15000;
    }

    void EnterCombat(Unit *who){}

    void SpellHitTarget(Unit* target,const SpellEntry* spell)
    {
        switch(spell->Id)
        {

        case SPELL_SHADOW_FURY:
        case SPELL_DARK_STRIKE:
            if(!target->HasAura(SPELL_DARK_FLAME))
            {
                if(target->HasAura(SPELL_FLAME_TOUCHED))
                {
                    target->RemoveAurasDueToSpell(SPELL_FLAME_TOUCHED);
                    target->CastSpell(target, SPELL_DARK_FLAME, true);
                }else target->CastSpell(target,SPELL_DARK_TOUCHED,true);
            }
            break;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if(!m_creature->HasAura(SPELL_IMAGE_VISUAL))
            DoCast(m_creature, SPELL_IMAGE_VISUAL);

        if(KillTimer < diff)
        {
            m_creature->DealDamage(m_creature, m_creature->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            KillTimer = 9999999;
        }else KillTimer -=diff;

        if (!UpdateVictim())
            return;

        if(ShadowfuryTimer < diff)
        {
            DoCast(m_creature, SPELL_SHADOW_FURY);
            ShadowfuryTimer = 10000;
        }else ShadowfuryTimer -=diff;

        if(DarkstrikeTimer < diff)
        {
            if(!m_creature->IsNonMeleeSpellCasted(false))
            {
                //If we are within range melee the target
                if( m_creature->IsWithinMeleeRange(m_creature->getVictim()))
                    DoCast(m_creature->getVictim(), SPELL_DARK_STRIKE);
            }
            DarkstrikeTimer = 3000;
        }
        else DarkstrikeTimer -= diff;
    }
};

CreatureAI* GetAI_mob_shadow_image(Creature *_Creature)
{
    return new mob_shadow_imageAI (_Creature);
};

void AddSC_boss_eredar_twins()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_sacrolash";
    newscript->GetAI = &GetAI_boss_sacrolash;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_alythess";
    newscript->GetAI = &GetAI_boss_alythess;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="mob_shadow_image";
    newscript->GetAI = &GetAI_mob_shadow_image;
    newscript->RegisterSelf();
}
