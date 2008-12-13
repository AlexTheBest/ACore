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
SDName: Karazhan
SD%Complete: 100
SDComment: Support for Barnes (Opera controller) and Berthold (Doorman).
SDCategory: Karazhan
EndScriptData */

/* ContentData
npc_barnes
npc_berthold
EndContentData */

#include "precompiled.h"
#include "def_karazhan.h"
#include "../../npc/npc_escortAI.h"

/*######
# npc_barnesAI
######*/

#define GOSSIP_READY        "I'm not an actor."

#define SAY_READY           "Splendid, I'm going to get the audience ready. Break a leg!"
#define SAY_OZ_INTRO1       "Finally, everything is in place. Are you ready for your big stage debut?"
#define OZ_GOSSIP1          "I'm not an actor."
#define SAY_OZ_INTRO2       "Don't worry, you'll be fine. You look like a natural!"
#define OZ_GOSSIP2          "Ok, I'll give it a try, then."

#define SAY_RAJ_INTRO1      "The romantic plays are really tough, but you'll do better this time. You have TALENT. Ready?"
#define RAJ_GOSSIP1         "I've never been more ready."

struct Dialogue
{
    int32 textid;
	uint32 timer;
};

static Dialogue OzDialogue[]=
{
	{-1532103, 6000},
	{-1532104, 18000},
	{-1532105, 9000},
	{-1532106, 15000}
};

static Dialogue HoodDialogue[]=
{
	{-1532107, 6000},
	{-1532108, 10000},
	{-1532109, 14000},
	{-1532110, 15000}
};

static Dialogue RAJDialogue[]=
{
	{-1532111, 5000},
	{-1532112, 7000},
	{-1532113, 14000},
	{-1532114, 14000}
};

// Entries and spawn locations for creatures in Oz event
float Spawns[6][2]=
{
    {17535, -10896},                                        // Dorothee
    {17546, -10891},                                        // Roar
    {17547, -10884},                                        // Tinhead
    {17543, -10902},                                        // Strawman
    {17603, -10892},                                        // Grandmother
    {17534, -10900},                                        // Julianne
};

float StageLocations[6][2]=
{
    {-10866.711, -1779.816},                                // Open door, begin walking (0)
    {-10894.917, -1775.467},                                // (1)
    {-10896.044, -1782.619},                                // Begin Speech after this (2)
    {-10894.917, -1775.467},                                // Resume walking (back to spawn point now) after speech (3)
    {-10866.711, -1779.816},                                // (4)
    {-10866.700, -1781.030}                                 // Summon mobs, open curtains, close door (5)
};

#define CREATURE_SPOTLIGHT  19525

#define SPELL_SPOTLIGHT     25824
#define SPELL_TUXEDO        32616

#define SPAWN_Z             90.5
#define SPAWN_Y             -1758
#define SPAWN_O             4.738

struct TRINITY_DLL_DECL npc_barnesAI : public npc_escortAI
{
    npc_barnesAI(Creature* c) : npc_escortAI(c)
    {
        RaidWiped = false;
        pInstance = ((ScriptedInstance*)c->GetInstanceData());
        Reset();
    }

    ScriptedInstance* pInstance;

    uint64 SpotlightGUID;

    uint32 TalkCount;
    uint32 TalkTimer;
    uint32 CurtainTimer;
    uint32 WipeTimer;
    uint32 Event;

    bool PerformanceReady;
    bool RaidWiped;
    bool IsTalking;

    void Reset()
    {
        TalkCount = 0;
        TalkTimer = 2000;
        CurtainTimer = 5000;
        WipeTimer = 5000;

        PerformanceReady = false;
        IsTalking = false;

        if(pInstance)
        {
            pInstance->SetData(DATA_OPERA_EVENT, NOT_STARTED);

            Event = pInstance->GetData(DATA_OPERA_PERFORMANCE);

             if (GameObject* Door = GameObject::GetGameObject((*m_creature), pInstance->GetData64(DATA_GAMEOBJECT_STAGEDOORLEFT)))
                Door->SetGoState(1);

             if (GameObject* Curtain = GameObject::GetGameObject((*m_creature), pInstance->GetData64(DATA_GAMEOBJECT_CURTAINS)))
                Curtain->SetGoState(1);
        }
    }

    void Aggro(Unit* who) {}

    void WaypointReached(uint32 i)
    {
        switch(i)
        {
            case 2:
                IsBeingEscorted = false;
                TalkCount = 0;
                IsTalking = true;

                float x,y,z;
                m_creature->GetPosition(x, y, z);
				if (Creature* Spotlight = m_creature->SummonCreature(CREATURE_SPOTLIGHT, x, y, z, 0, TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 50000))
                {
                    Spotlight->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                    Spotlight->CastSpell(Spotlight, SPELL_SPOTLIGHT, false);
                    SpotlightGUID = Spotlight->GetGUID();
                }
                break;

            case 5:
                if(pInstance)
                {
					if (GameObject* Door = GameObject::GetGameObject((*m_creature), pInstance->GetData64(DATA_GAMEOBJECT_STAGEDOORLEFT)))
                        Door->SetGoState(1);
                }
                IsBeingEscorted = false;
                PerformanceReady = true;
                break;
        }
    }

    void Talk(uint32 count)
    {
        int32 text = 0;

        switch(Event)
        {
            case EVENT_OZ:
                if (OzDialogue[count].textid)
					 text = OzDialogue[count].textid;
                if(OzDialogue[count].timer)
                    TalkTimer = OzDialogue[count].timer;
                break;

            case EVENT_HOOD:
				if (HoodDialogue[count].textid)
					text = HoodDialogue[count].textid;
                if(HoodDialogue[count].timer)
                    TalkTimer = HoodDialogue[count].timer;
                break;

            case EVENT_RAJ:
                 if (RAJDialogue[count].textid)
					 text = RAJDialogue[count].textid;
                if(RAJDialogue[count].timer)
                    TalkTimer = RAJDialogue[count].timer;
                break;
        }

        if(text)
             DoScriptText(text, m_creature);
    }

    void UpdateAI(const uint32 diff)
    {
        npc_escortAI::UpdateAI(diff);

        if(IsTalking)
        {
            if(TalkTimer < diff)
            {
                if(TalkCount > 3)
                {
                    if (Unit* Spotlight = Unit::GetUnit((*m_creature), SpotlightGUID))
                    {
                        Spotlight->RemoveAllAuras();
                        Spotlight->SetVisibility(VISIBILITY_OFF);
                    }

                    m_creature->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_STAND);
                    IsTalking = false;
                    IsBeingEscorted = true;
                    return;
                }

                m_creature->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_TALK);
                Talk(TalkCount);
                ++TalkCount;
            }else TalkTimer -= diff;
        }

        if(PerformanceReady)
        {
            if(CurtainTimer)
			{
                if(CurtainTimer <= diff)
            {
                PrepareEncounter();

                if(!pInstance)
                    return;

				if (GameObject* Curtain = GameObject::GetGameObject((*m_creature), pInstance->GetData64(DATA_GAMEOBJECT_CURTAINS)))
                    Curtain->SetGoState(0);

                CurtainTimer = 0;
            }else CurtainTimer -= diff;
			}

            if(!RaidWiped)
            {
                if(WipeTimer < diff)
                {
                    Map *map = m_creature->GetMap();
                    if(!map->IsDungeon()) return;

                    Map::PlayerList const &PlayerList = map->GetPlayers();
                    if(PlayerList.isEmpty())
                        return;

                    RaidWiped = true;
                    for(Map::PlayerList::const_iterator i = PlayerList.begin();i != PlayerList.end(); ++i)
                    {
                        if (i->getSource()->isAlive() && !i->getSource()->isGameMaster())
                        {
                            RaidWiped = false;
                            break;
                        }
                    }

                    if(RaidWiped)
                    {
                        RaidWiped = true;
                        EnterEvadeMode();
                    }

                    WipeTimer = 15000;
                }else WipeTimer -= diff;
            }

        }

        if(!m_creature->SelectHostilTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }

    void StartEvent()
    {
        if(!pInstance)
            return;

        pInstance->SetData(DATA_OPERA_EVENT, IN_PROGRESS);

		if (GameObject* Door = GameObject::GetGameObject((*m_creature), pInstance->GetData64(DATA_GAMEOBJECT_STAGEDOORLEFT)))
            Door->SetGoState(0);

        m_creature->CastSpell(m_creature, SPELL_TUXEDO, true);
        m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

        Start(false, false, false);
    }

    void PrepareEncounter()
    {
        debug_log("SD2: Barnes Opera Event - Introduction complete - preparing encounter %d", Event);
        uint8 index = 0;
        uint8 count = 0;
        switch(Event)
        {
            case EVENT_OZ:
                index = 0;
                count = 4;
                break;

            case EVENT_HOOD:
                index = 4;
                count = index+1;
                break;

            case EVENT_RAJ:
                index = 5;
                count = index+1;
                break;
        }

        for( ; index < count; ++index)
        {
            uint32 entry = ((uint32)Spawns[index][0]);
            float PosX = Spawns[index][1];
            if (Creature* pCreature = m_creature->SummonCreature(entry, PosX, SPAWN_Y, SPAWN_Z, SPAWN_O, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 120000))
            {
                                                            // In case database has bad flags
                pCreature->SetUInt32Value(UNIT_FIELD_FLAGS, 0);
                pCreature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }
        }

        CurtainTimer = 10000;
        PerformanceReady = true;
        RaidWiped = false;
    }
};

CreatureAI* GetAI_npc_barnesAI(Creature* _Creature)
{
    npc_barnesAI* Barnes_AI = new npc_barnesAI(_Creature);

    for(uint8 i = 0; i < 6; ++i)
        Barnes_AI->AddWaypoint(i, StageLocations[i][0], StageLocations[i][1], 90.465);

    return ((CreatureAI*)Barnes_AI);
}

bool GossipHello_npc_barnes(Player* player, Creature* _Creature)
{
    // Check for death of Moroes.
    ScriptedInstance* pInstance = ((ScriptedInstance*)_Creature->GetInstanceData());
    if(pInstance && (pInstance->GetData(DATA_MOROES_EVENT) >= DONE))
    {
        player->ADD_GOSSIP_ITEM(0, OZ_GOSSIP1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        if(!((npc_barnesAI*)_Creature->AI())->RaidWiped)
            player->SEND_GOSSIP_MENU(8970, _Creature->GetGUID());
        else
            player->SEND_GOSSIP_MENU(8975, _Creature->GetGUID());
    }else player->SEND_GOSSIP_MENU(8978, _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_barnes(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    switch(action)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            player->ADD_GOSSIP_ITEM(0, OZ_GOSSIP2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->SEND_GOSSIP_MENU(8971, _Creature->GetGUID());
            break;

        case GOSSIP_ACTION_INFO_DEF+2:
            player->CLOSE_GOSSIP_MENU();
            ((npc_barnesAI*)_Creature->AI())->StartEvent();
            break;
    }

    return true;
}

/*###
# npc_berthold
####*/

#define SPELL_TELEPORT           39567

#define GOSSIP_ITEM_TELEPORT     "Teleport me to the Guardian's Library"

bool GossipHello_npc_berthold(Player* player, Creature* _Creature)
{
    ScriptedInstance* pInstance = ((ScriptedInstance*)_Creature->GetInstanceData());
                                                            // Check if Shade of Aran is dead or not
    if(pInstance && (pInstance->GetData(DATA_SHADEOFARAN_EVENT) >= DONE))
        player->ADD_GOSSIP_ITEM(0, GOSSIP_ITEM_TELEPORT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_berthold(Player* player, Creature* _Creature, uint32 sender, uint32 action)
{
    if(action == GOSSIP_ACTION_INFO_DEF + 1)
        player->CastSpell(player, SPELL_TELEPORT, true);

    player->CLOSE_GOSSIP_MENU();
    return true;
}

void AddSC_karazhan()
{
    Script* newscript;

    newscript = new Script;
    newscript->GetAI = GetAI_npc_barnesAI;
    newscript->Name = "npc_barnes";
    newscript->pGossipHello = GossipHello_npc_barnes;
    newscript->pGossipSelect = GossipSelect_npc_barnes;
    newscript->RegisterSelf();

    newscript = new Script;
    newscript->Name = "npc_berthold";
    newscript->pGossipHello = GossipHello_npc_berthold;
    newscript->pGossipSelect = GossipSelect_npc_berthold;
    newscript->RegisterSelf();
}
