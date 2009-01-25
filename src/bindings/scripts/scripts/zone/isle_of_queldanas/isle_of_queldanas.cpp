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
SDName: Isle_of_Queldanas
SD%Complete: 100
SDComment: Quest support: 11524, 11525, 11532, 11533, 11542, 11543, 11541
SDCategory: Isle Of Quel'Danas
EndScriptData */

/* ContentData
npc_ayren_cloudbreaker
npc_converted_sentry
npc_unrestrained_dragonhawk
npc_greengill_slave
EndContentData */

#include "precompiled.h"

/*######
## npc_ayren_cloudbreaker
######*/

#define GOSSIP_FLY1 "Speaking of action, I've been ordered to undertake an air strike."
#define GOSSIP_FLY2 "I need to intercept the Dawnblade reinforcements."
bool GossipHello_npc_ayren_cloudbreaker(Player *player, Creature *_Creature)
{
    if( player->GetQuestStatus(11532) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(11533) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(0, GOSSIP_FLY1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    if( player->GetQuestStatus(11542) == QUEST_STATUS_INCOMPLETE || player->GetQuestStatus(11543) == QUEST_STATUS_INCOMPLETE)
        player->ADD_GOSSIP_ITEM(0, GOSSIP_FLY2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(),_Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_ayren_cloudbreaker(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player,45071,true);               //TaxiPath 779
    }
    if (action == GOSSIP_ACTION_INFO_DEF+2)
    {
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player,45113,true);               //TaxiPath 784
    }
    return true;
}

/*######
## npc_converted_sentry
######*/

#define SAY_CONVERTED_1         -1000284
#define SAY_CONVERTED_2         -1000284

#define SPELL_CONVERT_CREDIT    45009

struct TRINITY_DLL_DECL npc_converted_sentryAI : public ScriptedAI
{
    npc_converted_sentryAI(Creature *c) : ScriptedAI(c) { Reset(); }

    bool Credit;
    uint32 Timer;

    void Reset()
    {
        Credit = false;
        Timer = 2500;
    }

    void MoveInLineOfSight(Unit *who)
        { return; }
    void Aggro(Unit* who)
        { }

    void UpdateAI(const uint32 diff)
    {
        if( !Credit )
        {
            if( Timer <= diff )
            {
                uint32 i = urand(1,2);
                if( i=1 ) DoScriptText(SAY_CONVERTED_1, m_creature);
                else DoScriptText(SAY_CONVERTED_2, m_creature);

                DoCast(m_creature, SPELL_CONVERT_CREDIT);
                ((Pet*)m_creature)->SetDuration(7500);
                Credit = true;
            }else Timer -= diff;
        }
    }
};
CreatureAI* GetAI_npc_converted_sentry(Creature *_Creature)
{
    return new npc_converted_sentryAI (_Creature);
}

/*######
## npc_unrestrained_dragonhawk
######*/

#define GOSSIP_UD "<Ride the dragonhawk to Sun's Reach>"

bool GossipHello_npc_unrestrained_dragonhawk(Player *player, Creature *_Creature)
{
    if( player->GetQuestStatus(11542) == QUEST_STATUS_COMPLETE || player->GetQuestStatus(11543) == QUEST_STATUS_COMPLETE )
        player->ADD_GOSSIP_ITEM(0, GOSSIP_UD, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(),_Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_unrestrained_dragonhawk(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        player->CLOSE_GOSSIP_MENU();
        player->CastSpell(player,45353,true);               //TaxiPath 788
    }
    return true;
}

/*######
## npc_greengill_slave
######*/

#define ENRAGE	45111
#define ORB		45109
#define QUESTG	11541
#define DM		25060

struct TRINITY_DLL_DECL npc_greengill_slaveAI : public ScriptedAI
{
	npc_greengill_slaveAI(Creature* c) : ScriptedAI(c) {Reset();}

	uint64 PlayerGUID;

	void Aggro(Unit* who){}

	void Reset()
	{
	PlayerGUID = 0;
	}

	void SpellHit(Unit* caster, const SpellEntry* spell)
	{
		if(!caster)
            return;

		if(caster->GetTypeId() == TYPEID_PLAYER && spell->Id == ORB && !m_creature->HasAura(ENRAGE, 0))
		{
			PlayerGUID = caster->GetGUID();
			if(PlayerGUID)
            {
                Unit* plr = Unit::GetUnit((*m_creature), PlayerGUID);
                if(plr && ((Player*)plr)->GetQuestStatus(QUESTG) == QUEST_STATUS_INCOMPLETE)				
					((Player*)plr)->KilledMonster(25086, m_creature->GetGUID());
			}
			DoCast(m_creature, ENRAGE);
			Unit* Myrmidon = FindCreature(DM, 70, m_creature);
			if(Myrmidon)
			{
				m_creature->AddThreat(Myrmidon, 100000.0f);
                AttackStart(Myrmidon);
			}
		}
	}

	void UpdateAI(const uint32 diff)
	{
		DoMeleeAttackIfReady();
	}
};

CreatureAI* GetAI_npc_greengill_slaveAI(Creature* _Creature)
{
    return new npc_greengill_slaveAI(_Creature);
}

void AddSC_isle_of_queldanas()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npc_ayren_cloudbreaker";
    newscript->pGossipHello = &GossipHello_npc_ayren_cloudbreaker;
    newscript->pGossipSelect = &GossipSelect_npc_ayren_cloudbreaker;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_converted_sentry";
    newscript->GetAI = &GetAI_npc_converted_sentry;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name="npc_unrestrained_dragonhawk";
    newscript->pGossipHello = &GossipHello_npc_unrestrained_dragonhawk;
    newscript->pGossipSelect = &GossipSelect_npc_unrestrained_dragonhawk;
    newscript->RegisterSelf();

	newscript = new Script;
	newscript->Name="npc_greengill_slave";
	newscript->GetAI = &GetAI_npc_greengill_slaveAI;
	newscript->RegisterSelf();
}
