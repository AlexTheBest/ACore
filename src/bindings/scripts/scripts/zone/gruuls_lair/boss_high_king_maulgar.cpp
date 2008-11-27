/* Copyright (C) 2006 - 2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: Boss_High_King_Maulgar
SD%Complete: 90
SDComment: Correct timers, after whirlwind melee attack bug, prayer of healing
SDCategory: Gruul's Lair
EndScriptData */

#include "precompiled.h"
#include "def_gruuls_lair.h"

//Sounds
#define SOUND_AGGRO             11367
#define SOUND_ENRAGE            11368
#define SOUND_OGRE_DEATH1       11369
#define SOUND_OGRE_DEATH2       11370
#define SOUND_OGRE_DEATH3       11371
#define SOUND_OGRE_DEATH4       11372
#define SOUND_SLAY1             11373
#define SOUND_SLAY2             11374
#define SOUND_SLAY3             11375
#define SOUND_DEATH             11376

//Yells
#define SAY_AGGRO				"Gronn are the real power in Outland!"
#define SAY_ENRAGE				"You will not defeat the Hand of Gruul!"
#define SAY_OGRE_DEATH1			"You not kill next one so easy!"
#define SAY_OGRE_DEATH2			"Does not mean anything!"
#define SAY_OGRE_DEATH3			"I'm not afraid of you!"
#define SAY_OGRE_DEATH4			"Good, now you fight me!"
#define SAY_SLAY1				"You not so tough after all!"
#define SAY_SLAY2				"Ahahahaha!"
#define SAY_SLAY3				"Maulgar is king!"
#define SAY_DEATH				"Gruul will... crush you!"

// High King Maulgar
#define SPELL_ARCING_SMASH      39144
#define SPELL_MIGHTY_BLOW       33230
#define SPELL_WHIRLWIND         33238
#define SPELL_BERSERKER_C       26561
#define SPELL_ROAR              16508
#define SPELL_FLURRY            33232

// Olm the Summoner
#define SPELL_DARK_DECAY        33129
#define SPELL_DEATH_COIL        33130
#define SPELL_SUMMON_WFH        33131

//Kiggler the Craed
#define SPELL_GREATER_POLYMORPH 	33173
#define SPELL_LIGHTNING_BOLT    	36152
#define SPELL_ARCANE_SHOCK      	33175
#define SPELL_ARCANE_EXPLOSION  	33237

//Blindeye the Seer
#define SPELL_GREATER_PW_SHIELD 		33147
#define SPELL_HEAL              		33144
#define SPELL_PRAYER_OH                 33152

//Krosh Firehand
#define SPELL_GREATER_FIREBALL  33051
#define SPELL_SPELLSHIELD       33054
#define SPELL_BLAST_WAVE        33061

//High King Maulgar AI
struct TRINITY_DLL_DECL boss_high_king_maulgarAI : public ScriptedAI
{
    boss_high_king_maulgarAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
        for(uint8 i = 0; i < 4; ++i)
            Council[i] = 0;
        Reset();
    }

    ScriptedInstance* pInstance;

    uint32 ArcingSmash_Timer;
    uint32 MightyBlow_Timer;
    uint32 Whirlwind_Timer;
    uint32 Charging_Timer;
    uint32 Roar_Timer;

    bool Phase2;

    uint64 Council[4];

    void Reset()
    {
        ArcingSmash_Timer = 10000;
        MightyBlow_Timer = 40000;
        Whirlwind_Timer = 30000;
        Charging_Timer = 0;
               Roar_Timer = 0;

        Phase2 = false;

        Creature *pCreature = NULL;
        for(uint8 i = 0; i < 4; i++)
        {
            if(Council[i])
            {
                pCreature = (Creature*)(Unit::GetUnit((*m_creature), Council[i]));
                if(pCreature && !pCreature->isAlive())
                {
                    pCreature->Respawn();
                    pCreature->AI()->EnterEvadeMode();
                }
            }
        }

        //reset encounter
        if (pInstance)
            pInstance->SetData(DATA_MAULGAREVENT, NOT_STARTED);
    }

    void KilledUnit()
    {
        switch(rand()%3)
        {
            case 0:
                DoYell(SAY_SLAY1, LANG_UNIVERSAL, NULL);
                DoPlaySoundToSet(m_creature, SOUND_SLAY1);
                break;
            case 1:
                DoYell(SAY_SLAY2, LANG_UNIVERSAL, NULL);
                DoPlaySoundToSet(m_creature, SOUND_SLAY2);
                break;
            case 2:
                DoYell(SAY_SLAY3, LANG_UNIVERSAL, NULL);
                DoPlaySoundToSet(m_creature, SOUND_SLAY3);
                break;
        }
    }

    void JustDied(Unit* Killer)
    {
        DoPlaySoundToSet(m_creature, SOUND_DEATH);
		DoYell(SAY_DEATH, LANG_UNIVERSAL, NULL);
		
        if (pInstance)
               {
            pInstance->SetData(DATA_MAULGAREVENT, DONE);

                       GameObject* Door = NULL;
                       Door = GameObject::GetGameObject((*m_creature), pInstance->GetData64(DATA_MAULGARDOOR));
                       if(Door)
                               Door->SetGoState(0);
               }
    }

       void AddDeath()
       {
            switch(rand()%4)
			{
				case 0:
					DoYell(SAY_OGRE_DEATH1, LANG_UNIVERSAL, NULL);
					DoPlaySoundToSet(m_creature, SOUND_OGRE_DEATH1);
					break;
				case 1:
					DoYell(SAY_OGRE_DEATH2, LANG_UNIVERSAL, NULL);
					DoPlaySoundToSet(m_creature, SOUND_OGRE_DEATH2);
					break;
				case 2:
					DoYell(SAY_OGRE_DEATH3, LANG_UNIVERSAL, NULL);
					DoPlaySoundToSet(m_creature, SOUND_OGRE_DEATH3);
					break;
				case 3:
					DoYell(SAY_OGRE_DEATH4, LANG_UNIVERSAL, NULL);
					DoPlaySoundToSet(m_creature, SOUND_OGRE_DEATH4);
					break;
			}
       }


    void Aggro(Unit *who)
    {
        StartEvent(who);
    }

    void GetCouncil()
    {
        //get council member's guid to respawn them if needed
        Council[0] = pInstance->GetData64(DATA_KIGGLERTHECRAZED);
        Council[1] = pInstance->GetData64(DATA_BLINDEYETHESEER);
        Council[2] = pInstance->GetData64(DATA_OLMTHESUMMONER);
        Council[3] = pInstance->GetData64(DATA_KROSHFIREHAND);
    }

    void StartEvent(Unit *who)
    {
        if(!pInstance)
            return;

        GetCouncil();

        DoPlaySoundToSet(m_creature, SOUND_AGGRO);
		DoYell(SAY_AGGRO, LANG_UNIVERSAL, NULL);
		
        pInstance->SetData64(DATA_MAULGAREVENT_TANK, who->GetGUID());
        pInstance->SetData(DATA_MAULGAREVENT, IN_PROGRESS);

               DoZoneInCombat();
    }

    void UpdateAI(const uint32 diff)
    {
        //Only if not incombat check if the event is started
        if(!InCombat && pInstance && pInstance->GetData(DATA_MAULGAREVENT))
        {
            Unit* target = Unit::GetUnit((*m_creature), pInstance->GetData64(DATA_MAULGAREVENT_TANK));

            if(target)
            {
                AttackStart(target);
                GetCouncil();
            }
        }

        //Return since we have no target
        if (!m_creature->SelectHostilTarget() || !m_creature->getVictim() )
            return;

        //someone evaded!
        if(pInstance && !pInstance->GetData(DATA_MAULGAREVENT))
        {
            EnterEvadeMode();
            return;
        }

        //ArcingSmash_Timer
        if (ArcingSmash_Timer < diff)
        {
            DoCast(m_creature->getVictim(), SPELL_ARCING_SMASH);
            ArcingSmash_Timer = 10000;
        }else ArcingSmash_Timer -= diff;

        //Whirlwind_Timer
               if (Whirlwind_Timer < diff)
               {
                       DoCast(m_creature->getVictim(), SPELL_WHIRLWIND);
                       Whirlwind_Timer = 55000;
               }else Whirlwind_Timer -= diff;

        //MightyBlow_Timer
        if (MightyBlow_Timer < diff)
        {
            DoCast(m_creature->getVictim(), SPELL_MIGHTY_BLOW);
            MightyBlow_Timer = 30000+rand()%10000;
        }else MightyBlow_Timer -= diff;

        //Entering Phase 2
        if(!Phase2 && (m_creature->GetHealth()*100 / m_creature->GetMaxHealth()) < 50)
        {
            Phase2 = true;
            DoPlaySoundToSet(m_creature, SOUND_ENRAGE);
            DoYell(SAY_ENRAGE, LANG_UNIVERSAL, NULL);
			DoCast(m_creature, SPELL_FLURRY);
			
            m_creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY, 0);
            m_creature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY+1, 0);                
        }

        if(Phase2)
        {
            //Charging_Timer
            if(Charging_Timer < diff)
            {
                Unit* target = NULL;
                target = SelectUnit(SELECT_TARGET_RANDOM, 0);
                if (target)
                    AttackStart(target);
                                       DoCast(target, SPELL_BERSERKER_C);      

                Charging_Timer = 20000;
            }else Charging_Timer -= diff;

                       //Intimidating Roar
                       if(Roar_Timer < diff)
                       {
                               DoCast(m_creature, SPELL_ROAR);
                               Roar_Timer = 40000+(rand()%10000);
                       }else Roar_Timer -= diff;
        }

        DoMeleeAttackIfReady();
    }
};

//Olm The Summoner AI
struct TRINITY_DLL_DECL boss_olm_the_summonerAI : public ScriptedAI
{
    boss_olm_the_summonerAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
        Reset();
    }

    uint32 DarkDecay_Timer;
    uint32 Summon_Timer;
       uint32 DeathCoil_Timer;

    ScriptedInstance* pInstance;

    void Reset()
    {
        DarkDecay_Timer = 10000;
        Summon_Timer = 15000;
               DeathCoil_Timer = 20000;

        //reset encounter
        if (pInstance)
            pInstance->SetData(DATA_MAULGAREVENT, NOT_STARTED);
    }

    void Aggro(Unit *who)
    {
        if(pInstance)
        {
            pInstance->SetData64(DATA_MAULGAREVENT_TANK, who->GetGUID());
            pInstance->SetData(DATA_MAULGAREVENT, IN_PROGRESS);
        }
    }

       void JustDied(Unit* Killer)
       {
               if(pInstance)
        {
            Creature *Maulgar = NULL;
            Maulgar = (Creature*)(Unit::GetUnit((*m_creature), pInstance->GetData64(DATA_MAULGAR)));

            if(Maulgar)
                ((boss_high_king_maulgarAI*)Maulgar->AI())->AddDeath();
        }
       }

    void UpdateAI(const uint32 diff)
    {
        //Only if not incombat check if the event is started
        if(!InCombat && pInstance && pInstance->GetData(DATA_MAULGAREVENT))
        {
            Unit* target = Unit::GetUnit((*m_creature), pInstance->GetData64(DATA_MAULGAREVENT_TANK));

            if(target)
            {
                AttackStart(target);
            }
        }

        //Return since we have no target
        if (!m_creature->SelectHostilTarget() || !m_creature->getVictim() )
            return;

        //someone evaded!
        if(pInstance && !pInstance->GetData(DATA_MAULGAREVENT))
        {
            EnterEvadeMode();
            return;
        }

        //DarkDecay_Timer
        if(DarkDecay_Timer < diff)
        {
            DoCast(m_creature->getVictim(), SPELL_DARK_DECAY);
            DarkDecay_Timer = 20000;
        }else DarkDecay_Timer -= diff;

        //Summon_Timer
        if(Summon_Timer < diff)
        {
                       DoCast(m_creature, SPELL_SUMMON_WFH);
                       Summon_Timer = 30000;
        }else Summon_Timer -= diff;

               //DeathCoil Timer /need correct timer
               if(DeathCoil_Timer < diff)
               {
                       Unit* target = NULL;
                       target = SelectUnit(SELECT_TARGET_RANDOM, 0);
                       if(target)
                       DoCast(target, SPELL_DEATH_COIL);
                       DeathCoil_Timer = 20000;
               }else DeathCoil_Timer -= diff;


        DoMeleeAttackIfReady();
    }
};

//Kiggler The Crazed AI
struct TRINITY_DLL_DECL boss_kiggler_the_crazedAI : public ScriptedAI
{
    boss_kiggler_the_crazedAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
        Reset();
    }

    uint32 GreaterPolymorph_Timer;
    uint32 LightningBolt_Timer;
    uint32 ArcaneShock_Timer;
    uint32 ArcaneExplosion_Timer;

    ScriptedInstance* pInstance;

    void Reset()
    {
        GreaterPolymorph_Timer = 5000;
        LightningBolt_Timer = 10000;
        ArcaneShock_Timer = 20000;
        ArcaneExplosion_Timer = 30000;

        //reset encounter
        if (pInstance)
            pInstance->SetData(DATA_MAULGAREVENT, NOT_STARTED);
    }

    void Aggro(Unit *who)
    {
        if(pInstance)
        {
            pInstance->SetData64(DATA_MAULGAREVENT_TANK, who->GetGUID());
            pInstance->SetData(DATA_MAULGAREVENT, IN_PROGRESS);
        }
    }

       void JustDied(Unit* Killer)
       {
               if(pInstance)
        {
            Creature *Maulgar = NULL;
            Maulgar = (Creature*)(Unit::GetUnit((*m_creature), pInstance->GetData64(DATA_MAULGAR)));

            if(Maulgar)
                ((boss_high_king_maulgarAI*)Maulgar->AI())->AddDeath();
        }
       }
 
    void UpdateAI(const uint32 diff)
    {
        //Only if not incombat check if the event is started
        if(!InCombat && pInstance && pInstance->GetData(DATA_MAULGAREVENT))
        {
            Unit* target = Unit::GetUnit((*m_creature), pInstance->GetData64(DATA_MAULGAREVENT_TANK));

            if(target)
            {
                AttackStart(target);
            }
        }

        //Return since we have no target
        if (!m_creature->SelectHostilTarget() || !m_creature->getVictim() )
            return;

        //someone evaded!
        if(pInstance && !pInstance->GetData(DATA_MAULGAREVENT))
        {
            EnterEvadeMode();
            return;
        }

        //GreaterPolymorph_Timer
        if(GreaterPolymorph_Timer < diff)
        {
            Unit *target = SelectUnit(SELECT_TARGET_RANDOM, 0);
            if(target)
                DoCast(target, SPELL_GREATER_POLYMORPH);

            GreaterPolymorph_Timer = 20000;
        }else GreaterPolymorph_Timer -= diff;

        //LightningBolt_Timer
        if(LightningBolt_Timer < diff)
        {
            DoCast(m_creature->getVictim(), SPELL_LIGHTNING_BOLT);
            LightningBolt_Timer = 15000;
        }else LightningBolt_Timer -= diff;

        //ArcaneShock_Timer
        if(ArcaneShock_Timer < diff)
        {
            DoCast(m_creature->getVictim(), SPELL_ARCANE_SHOCK);
            ArcaneShock_Timer = 20000;
        }else ArcaneShock_Timer -= diff;

        //ArcaneExplosion_Timer
        if(ArcaneExplosion_Timer < diff)
        {
            DoCast(m_creature->getVictim(), SPELL_ARCANE_EXPLOSION);
            ArcaneExplosion_Timer = 30000;
        }else ArcaneExplosion_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};

//Blindeye The Seer AI
struct TRINITY_DLL_DECL boss_blindeye_the_seerAI : public ScriptedAI
{
    boss_blindeye_the_seerAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
        Reset();
    }

    uint32 GreaterPowerWordShield_Timer;
    uint32 Heal_Timer;

    ScriptedInstance* pInstance;

    void Reset()
    {
        GreaterPowerWordShield_Timer = 5000;
        Heal_Timer = 30000;

        //reset encounter
        if (pInstance)
            pInstance->SetData(DATA_MAULGAREVENT, NOT_STARTED);
    }

    void Aggro(Unit *who)
    {
        if(pInstance)
        {
            pInstance->SetData64(DATA_MAULGAREVENT_TANK, who->GetGUID());
            pInstance->SetData(DATA_MAULGAREVENT, IN_PROGRESS);
        }
    }

       void JustDied(Unit* Killer)
       {
               if(pInstance)
        {
            Creature *Maulgar = NULL;
            Maulgar = (Creature*)(Unit::GetUnit((*m_creature), pInstance->GetData64(DATA_MAULGAR)));

            if(Maulgar)
                ((boss_high_king_maulgarAI*)Maulgar->AI())->AddDeath();
        }
       }

     void UpdateAI(const uint32 diff)
    {
        //Only if not incombat check if the event is started
        if(!InCombat && pInstance && pInstance->GetData(DATA_MAULGAREVENT))
        {
            Unit* target = Unit::GetUnit((*m_creature), pInstance->GetData64(DATA_MAULGAREVENT_TANK));

            if(target)
            {
                AttackStart(target);
            }
        }

        //Return since we have no target
        if (!m_creature->SelectHostilTarget() || !m_creature->getVictim() )
            return;

        //someone evaded!
        if(pInstance && !pInstance->GetData(DATA_MAULGAREVENT))
        {
            EnterEvadeMode();
            return;
        }

        //GreaterPowerWordShield_Timer
        if(GreaterPowerWordShield_Timer < diff)
        {
            DoCast(m_creature, SPELL_GREATER_PW_SHIELD);
            GreaterPowerWordShield_Timer = 40000;
        }else GreaterPowerWordShield_Timer -= diff;

        //Heal_Timer
        if(Heal_Timer < diff)
        {
            DoCast(m_creature, SPELL_HEAL);
            Heal_Timer = 60000;
        }else Heal_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};

//Krosh Firehand AI
struct TRINITY_DLL_DECL boss_krosh_firehandAI : public ScriptedAI
{
    boss_krosh_firehandAI(Creature *c) : ScriptedAI(c)
    {
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
        Reset();
    }

    uint32 GreaterFireball_Timer;
    uint32 SpellShield_Timer;
    uint32 BlastWave_Timer;

    ScriptedInstance* pInstance;

    void Reset()
    {
        GreaterFireball_Timer = 1000;
        SpellShield_Timer = 5000;
        BlastWave_Timer = 20000;

        //reset encounter
        if (pInstance)
            pInstance->SetData(DATA_MAULGAREVENT, NOT_STARTED);
    }

    void Aggro(Unit *who)
    {
        if(pInstance)
        {
            pInstance->SetData64(DATA_MAULGAREVENT_TANK, who->GetGUID());
            pInstance->SetData(DATA_MAULGAREVENT, IN_PROGRESS);
        }
    }

       void JustDied(Unit* Killer)
       {
               if(pInstance)
        {
            Creature *Maulgar = NULL;
            Maulgar = (Creature*)(Unit::GetUnit((*m_creature), pInstance->GetData64(DATA_MAULGAR)));

            if(Maulgar)
                ((boss_high_king_maulgarAI*)Maulgar->AI())->AddDeath();
        }
       }

    void UpdateAI(const uint32 diff)
    {
        //Only if not incombat check if the event is started
        if(!InCombat && pInstance && pInstance->GetData(DATA_MAULGAREVENT))
        {
            Unit* target = Unit::GetUnit((*m_creature), pInstance->GetData64(DATA_MAULGAREVENT_TANK));

            if(target)
            {
                AttackStart(target);
            }
        }

        //Return since we have no target
        if (!m_creature->SelectHostilTarget() || !m_creature->getVictim() )
            return;

        //someone evaded!
        if(pInstance && !pInstance->GetData(DATA_MAULGAREVENT))
        {
            EnterEvadeMode();
            return;
        }

        //GreaterFireball_Timer
        if(GreaterFireball_Timer < diff || m_creature->GetDistance(m_creature->getVictim()) < 30)
        {
            DoCast(m_creature->getVictim(), SPELL_GREATER_FIREBALL);
            GreaterFireball_Timer = 2000;
        }else GreaterFireball_Timer -= diff;

        //SpellShield_Timer
        if(SpellShield_Timer < diff)
        {
            m_creature->InterruptNonMeleeSpells(false);
            DoCast(m_creature->getVictim(), SPELL_SPELLSHIELD);
            SpellShield_Timer = 30000;
        }else SpellShield_Timer -= diff;

        //BlastWave_Timer
        if(BlastWave_Timer < diff)
        {
                       Unit *target;
            std::list<HostilReference *> t_list = m_creature->getThreatManager().getThreatList();
            std::vector<Unit *> target_list;
            for(std::list<HostilReference *>::iterator itr = t_list.begin(); itr!= t_list.end(); ++itr)
            {
                target = Unit::GetUnit(*m_creature, (*itr)->getUnitGuid());
                                                            //15 yard radius minimum
                if(target && target->GetDistance2d(m_creature) < 15)
                    target_list.push_back(target);
                target = NULL;
            }
            if(target_list.size())
                target = *(target_list.begin()+rand()%target_list.size());

            m_creature->InterruptNonMeleeSpells(false);
                       DoCast(target, SPELL_BLAST_WAVE);
            BlastWave_Timer = 60000;
        }else BlastWave_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_high_king_maulgar(Creature *_Creature)
{
    return new boss_high_king_maulgarAI (_Creature);
}

CreatureAI* GetAI_boss_olm_the_summoner(Creature *_Creature)
{
    return new boss_olm_the_summonerAI (_Creature);
}

CreatureAI *GetAI_boss_kiggler_the_crazed(Creature *_Creature)
{
    return new boss_kiggler_the_crazedAI (_Creature);
}

CreatureAI *GetAI_boss_blindeye_the_seer(Creature *_Creature)
{
    return new boss_blindeye_the_seerAI (_Creature);
}

CreatureAI *GetAI_boss_krosh_firehand(Creature *_Creature)
{
    return new boss_krosh_firehandAI (_Creature);
}

void AddSC_boss_high_king_maulgar()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="boss_high_king_maulgar";
    newscript->GetAI = GetAI_boss_high_king_maulgar;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_kiggler_the_crazed";
    newscript->GetAI = GetAI_boss_kiggler_the_crazed;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_blindeye_the_seer";
    newscript->GetAI = GetAI_boss_blindeye_the_seer;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_olm_the_summoner";
    newscript->GetAI = GetAI_boss_olm_the_summoner;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="boss_krosh_firehand";
    newscript->GetAI = GetAI_boss_krosh_firehand;
    newscript->RegisterSelf();
}
