/*
 * Copyright (C) 2008-2010 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/* ScriptData
SDName: Icecrown
SD%Complete: 100
SDComment: Quest support: 12807
SDCategory: Icecrown
EndScriptData */

/* ContentData
npc_arete
EndContentData */

#include "ScriptPCH.h"

/*######
## npc_arete
######*/

#define GOSSIP_ARETE_ITEM1 "Lord-Commander, I would hear your tale."
#define GOSSIP_ARETE_ITEM2 "<You nod slightly but do not complete the motion as the Lord-Commander narrows his eyes before he continues.>"
#define GOSSIP_ARETE_ITEM3 "I thought that they now called themselves the Scarlet Onslaught?"
#define GOSSIP_ARETE_ITEM4 "Where did the grand admiral go?"
#define GOSSIP_ARETE_ITEM5 "That's fine. When do I start?"
#define GOSSIP_ARETE_ITEM6 "Let's finish this!"
#define GOSSIP_ARETE_ITEM7 "That's quite a tale, Lord-Commander."

enum eArete
{
    GOSSIP_TEXTID_ARETE1        = 13525,
    GOSSIP_TEXTID_ARETE2        = 13526,
    GOSSIP_TEXTID_ARETE3        = 13527,
    GOSSIP_TEXTID_ARETE4        = 13528,
    GOSSIP_TEXTID_ARETE5        = 13529,
    GOSSIP_TEXTID_ARETE6        = 13530,
    GOSSIP_TEXTID_ARETE7        = 13531,

    QUEST_THE_STORY_THUS_FAR    = 12807
};

class npc_arete : public CreatureScript
{
public:
    npc_arete() : CreatureScript("npc_arete") { }

    bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
    {
        switch(uiAction)
        {
            case GOSSIP_ACTION_INFO_DEF+1:
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ARETE_ITEM2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_ARETE2, pCreature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+2:
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ARETE_ITEM3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
                pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_ARETE3, pCreature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+3:
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ARETE_ITEM4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
                pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_ARETE4, pCreature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+4:
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ARETE_ITEM5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
                pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_ARETE5, pCreature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+5:
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ARETE_ITEM6, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
                pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_ARETE6, pCreature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+6:
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ARETE_ITEM7, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7);
                pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_ARETE7, pCreature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+7:
                pPlayer->CLOSE_GOSSIP_MENU();
                pPlayer->AreaExploredOrEventHappens(QUEST_THE_STORY_THUS_FAR);
                break;
        }

        return true;
    }

    bool OnGossipHello(Player* pPlayer, Creature* pCreature)
    {
        if (pCreature->isQuestGiver())
            pPlayer->PrepareQuestMenu(pCreature->GetGUID());

        if (pPlayer->GetQuestStatus(QUEST_THE_STORY_THUS_FAR) == QUEST_STATUS_INCOMPLETE)
        {
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ARETE_ITEM1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_ARETE1, pCreature->GetGUID());
            return true;
        }

        pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());
        return true;
    }

};


/*######
## npc_dame_evniki_kapsalis
######*/

enum eDameEnvikiKapsalis
{
    TITLE_CRUSADER    = 123,
    ACHI_CRUSADER_A   = 2817,
    ACHI_CRUSADER_H   = 2816
};

class npc_dame_evniki_kapsalis : public CreatureScript
{
public:
    npc_dame_evniki_kapsalis() : CreatureScript("npc_dame_evniki_kapsalis") { }

    bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
    {
        if (uiAction == GOSSIP_ACTION_TRADE)
            pPlayer->SEND_VENDORLIST(pCreature->GetGUID());
        return true;
    }

    bool OnGossipHello(Player* pPlayer, Creature* pCreature)
    {
        const AchievementEntry * achiCrusader = GetAchievementStore()->LookupEntry(pPlayer->GetTeam() == TEAM_HORDE ? ACHI_CRUSADER_H : ACHI_CRUSADER_A);
        if (pPlayer->HasTitle(TITLE_CRUSADER) || pPlayer->GetAchievementMgr().HasAchieved(achiCrusader))
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

        pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());
        return true;
    }

};


/*######
## npc_squire_david
######*/

enum eSquireDavid
{
    QUEST_THE_ASPIRANT_S_CHALLENGE_H                    = 13680,
    QUEST_THE_ASPIRANT_S_CHALLENGE_A                    = 13679,

    NPC_ARGENT_VALIANT                                  = 33448,

    GOSSIP_TEXTID_SQUIRE_DAVID                          = 14407
};

#define GOSSIP_SQUIRE_ITEM_1 "I am ready to fight!"
#define GOSSIP_SQUIRE_ITEM_2 "How do the Argent Crusader raiders fight?"

class npc_squire_david : public CreatureScript
{
public:
    npc_squire_david() : CreatureScript("npc_squire_david") { }

    bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
    {
        if (uiAction == GOSSIP_ACTION_INFO_DEF+1)
        {
            pPlayer->CLOSE_GOSSIP_MENU();
            pCreature->SummonCreature(NPC_ARGENT_VALIANT,8575.451,952.472,547.554,0.38);
        }
        //else
            //pPlayer->SEND_GOSSIP_MENU(???, pCreature->GetGUID()); Missing text
        return true;
    }

    bool OnGossipHello(Player* pPlayer, Creature* pCreature)
    {
        if (pPlayer->GetQuestStatus(QUEST_THE_ASPIRANT_S_CHALLENGE_H) == QUEST_STATUS_INCOMPLETE ||
            pPlayer->GetQuestStatus(QUEST_THE_ASPIRANT_S_CHALLENGE_A) == QUEST_STATUS_INCOMPLETE)//We need more info about it.
        {
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SQUIRE_ITEM_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SQUIRE_ITEM_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
        }

        pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_SQUIRE_DAVID, pCreature->GetGUID());
        return true;
    }

};


/*######
## npc_squire_danny
######*/

enum eSquireDanny
{
    QUEST_THE_VALIANT_S_CHALLENGE_HORDE_UNDERCITY      = 13729,
    QUEST_THE_VALIANT_S_CHALLENGE_HORDE_SENJIN         = 13727,
    QUEST_THE_VALIANT_S_CHALLENGE_HORDE_THUNDERBLUFF   = 13728,
    QUEST_THE_VALIANT_S_CHALLENGE_HORDE_SILVERMOON     = 13731,
    QUEST_THE_VALIANT_S_CHALLENGE_HORDE_ORGRIMMAR      = 13726,
    QUEST_THE_VALIANT_S_CHALLENGE_ALLIANCE_DARNASSUS   = 13725,
    QUEST_THE_VALIANT_S_CHALLENGE_ALLIANCE_IRONFORGE   = 13713,
    QUEST_THE_VALIANT_S_CHALLENGE_ALLIANCE_GNOMEREGAN  = 13723,
    QUEST_THE_VALIANT_S_CHALLENGE_ALLIANCE_EXODAR      = 13724,
    QUEST_THE_VALIANT_S_CHALLENGE_ALLIANCE_STORMWIND   = 13699,

    NPC_ARGENT_CHAMPION                                = 33707,

    GOSSIP_TEXTID_SQUIRE_DANNY                         = 14407
};

class npc_squire_danny : public CreatureScript
{
public:
    npc_squire_danny() : CreatureScript("npc_squire_danny") { }

    bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
    {
        if (uiAction == GOSSIP_ACTION_INFO_DEF+1)
        {
            pPlayer->CLOSE_GOSSIP_MENU();
            pCreature->SummonCreature(NPC_ARGENT_CHAMPION,8562.451,1095.72,556.784,1.76);
        }
        //else
            //pPlayer->SEND_GOSSIP_MENU(???, pCreature->GetGUID()); Missing text
        return true;
    }

    bool OnGossipHello(Player* pPlayer, Creature* pCreature)
    {
        if (pPlayer->GetQuestStatus(QUEST_THE_VALIANT_S_CHALLENGE_HORDE_UNDERCITY) == QUEST_STATUS_INCOMPLETE
            || pPlayer->GetQuestStatus(QUEST_THE_VALIANT_S_CHALLENGE_HORDE_SENJIN) == QUEST_STATUS_INCOMPLETE
            || pPlayer->GetQuestStatus(QUEST_THE_VALIANT_S_CHALLENGE_HORDE_THUNDERBLUFF) == QUEST_STATUS_INCOMPLETE
            || pPlayer->GetQuestStatus(QUEST_THE_VALIANT_S_CHALLENGE_HORDE_SILVERMOON) == QUEST_STATUS_INCOMPLETE
            || pPlayer->GetQuestStatus(QUEST_THE_VALIANT_S_CHALLENGE_HORDE_ORGRIMMAR) == QUEST_STATUS_INCOMPLETE
            || pPlayer->GetQuestStatus(QUEST_THE_VALIANT_S_CHALLENGE_ALLIANCE_DARNASSUS) == QUEST_STATUS_INCOMPLETE
            || pPlayer->GetQuestStatus(QUEST_THE_VALIANT_S_CHALLENGE_ALLIANCE_IRONFORGE) == QUEST_STATUS_INCOMPLETE
            || pPlayer->GetQuestStatus(QUEST_THE_VALIANT_S_CHALLENGE_ALLIANCE_GNOMEREGAN) == QUEST_STATUS_INCOMPLETE
            || pPlayer->GetQuestStatus(QUEST_THE_VALIANT_S_CHALLENGE_ALLIANCE_EXODAR) == QUEST_STATUS_INCOMPLETE
            || pPlayer->GetQuestStatus(QUEST_THE_VALIANT_S_CHALLENGE_ALLIANCE_STORMWIND) == QUEST_STATUS_INCOMPLETE) //We need more info about it.
        {
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SQUIRE_ITEM_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SQUIRE_ITEM_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
        }

        pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXTID_SQUIRE_DANNY, pCreature->GetGUID());
        return true;
    }

};


/*######
## npc_argent_valiant
######*/

enum eArgentValiant
{
    SPELL_CHARGE                = 63010,
    SPELL_SHIELD_BREAKER        = 65147,
    SPELL_DEFEND		= 62719,
    SPELL_THRUST		= 62544,

    NPC_ARGENT_VALIANT_CREDIT   = 38595
};

enum eValiantText
{

	NPC_FACTION_VAILIANT_TEXT_SAY_START_1 	= -1850004,//	Tenez-vous pr�t !
	NPC_FACTION_VAILIANT_TEXT_SAY_START_2 	= -1850005,//	Que le combat commence !
	NPC_FACTION_VAILIANT_TEXT_SAY_START_3 	= -1850006,//	Pr�parez-vous !
	NPC_ARGENT_VAILIANT_TEXT_SAY_START 		= -1850007,//	Vous pensez avoir la vaillance en vous ? Nous verrons.
	NPC_ARGENT_VAILIANT_TEXT_SAY_WIN 		= -1850008,//	Impressionnante d�monstration. Je pense que vous �tes tout � fait en mesure de rejoindre les rangs des vaillants.
	NPC_ARGENT_VAILIANT_TEXT_SAY_LOOSE 		= -1850009,//	J'ai gagn�. Vous aurez sans doute plus de chance la prochaine fois.
	NPC_FACTION_VAILIANT_TEXT_SAY_WIN_1 	= -1850010,//	Je suis vaincue. Joli combat !
	NPC_FACTION_VAILIANT_TEXT_SAY_WIN_2 	= -1850011,//	On dirait que j'ai sous-estim� vos comp�tences. Bien jou�.
	NPC_FACTION_VAILIANT_TEXT_SAY_LOOSE 	= -1850012,//	J'ai gagn�. Vous aurez sans doute plus de chance la prochaine fois.
};

class npc_argent_valiant : public CreatureScript
{
public:
    npc_argent_valiant() : CreatureScript("npc_argent_valiant") { }

    struct npc_argent_valiantAI : public ScriptedAI
    {
        npc_argent_valiantAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
    	me->CastSpell(me, SPELL_DEFEND, true);
    	me->CastSpell(me, SPELL_DEFEND, true);
            pCreature->GetMotionMaster()->MovePoint(0,8599.258,963.951,547.553);
            pCreature->setFaction(35); //wrong faction in db?
        }

        uint32 uiChargeTimer;
        uint32 uiShieldBreakerTimer;
        uint32 uiDefendTimer;

        void Reset()
        {
            uiChargeTimer = 7000;
            uiShieldBreakerTimer = 10000;
        }

        void MovementInform(uint32 uiType, uint32 /*uiId*/)
        {
            if (uiType != POINT_MOTION_TYPE)
                return;

            me->setFaction(14);
        }

        void DamageTaken(Unit* pDoneBy, uint32& uiDamage)
        {
    		if(pDoneBy)
    		{
    			if (uiDamage > me->GetHealth() && (pDoneBy->GetTypeId() == TYPEID_PLAYER || pDoneBy->GetOwner()))
    			{
    				DoScriptText(NPC_ARGENT_VAILIANT_TEXT_SAY_WIN, me);
    				uiDamage = 0;

    				if(pDoneBy->GetOwner())
    					(pDoneBy->GetOwner())->ToPlayer()->KilledMonsterCredit(NPC_ARGENT_VALIANT_CREDIT,0);
    				if(pDoneBy->GetTypeId() == TYPEID_PLAYER)
    					pDoneBy->ToPlayer()->KilledMonsterCredit(NPC_ARGENT_VALIANT_CREDIT,0);

    				me->setFaction(35);
    				me->ForcedDespawn(5000);
    				me->SetHomePosition(me->GetPositionX(),me->GetPositionY(),me->GetPositionZ(),me->GetOrientation());
    				EnterEvadeMode();
    			}
    		}
        }

        void KilledUnit(Unit* /*victim*/)
        {
        	me->setFaction(35);
        	me->ForcedDespawn(5000);
        	DoScriptText(NPC_ARGENT_VAILIANT_TEXT_SAY_LOOSE, me);
            me->CombatStop(true);
        }

        void DoMeleeAttackIfReady()
    	{
    		if (me->hasUnitState(UNIT_STAT_CASTING))
    		    return;

    		//Make sure our attack is ready and we aren't currently casting before checking distance
    		if (me->isAttackReady())
    		{
    		    //If we are within range melee the target
    		    if (me->IsWithinMeleeRange(me->getVictim()))
    		    {
    		        DoCastVictim(SPELL_THRUST);
    		        me->resetAttackTimer();
    		    }
    		}
    	}

    	void EnterCombat(Unit* /*who*/)
        {
    		DoScriptText(NPC_ARGENT_VAILIANT_TEXT_SAY_START, me);
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim())
                return;

            if (uiChargeTimer <= uiDiff)
            {
                DoCastVictim(SPELL_CHARGE);
                uiChargeTimer = 7000;
            } else uiChargeTimer -= uiDiff;

            if (uiShieldBreakerTimer <= uiDiff)
            {
                DoCastVictim(SPELL_SHIELD_BREAKER);
                uiShieldBreakerTimer = 10000;
            } else uiShieldBreakerTimer -= uiDiff;

            if (uiDefendTimer <= uiDiff)
            {
    	    me->CastSpell(me, SPELL_DEFEND, true);
    	    uiDefendTimer = 10000;
            } else uiDefendTimer -= uiDiff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_argent_valiantAI (pCreature);
    }

};


/*######
## npc_argent_champion
######*/

enum eArgentChampion
{
    SPELL_CHARGE_CHAMPION           = 63010,
    SPELL_SHIELD_BREAKER_CHAMPION   = 65147,

    NPC_ARGENT_CHAMPION_CREDIT      = 33708
};

class npc_argent_champion : public CreatureScript
{
public:
    npc_argent_champion() : CreatureScript("npc_argent_champion") { }

    struct npc_argent_championAI : public ScriptedAI
    {
        npc_argent_championAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            pCreature->GetMotionMaster()->MovePoint(0,8552.43,1124.95,556.786);
            pCreature->setFaction(35); //wrong faction in db?
        }

        uint32 uiChargeTimer;
        uint32 uiShieldBreakerTimer;

        void Reset()
        {
            uiChargeTimer = 7000;
            uiShieldBreakerTimer = 10000;
        }

        void MovementInform(uint32 uiType, uint32 uiId)
        {
            if (uiType != POINT_MOTION_TYPE)
                return;

            me->setFaction(14);
        }

        void DamageTaken(Unit* pDoneBy, uint32& uiDamage)
        {
            if (uiDamage > me->GetHealth() && pDoneBy->GetTypeId() == TYPEID_PLAYER)
            {
                uiDamage = 0;
                CAST_PLR(pDoneBy)->KilledMonsterCredit(NPC_ARGENT_CHAMPION_CREDIT,0);
                me->setFaction(35);
                me->ForcedDespawn(5000);
                me->SetHomePosition(me->GetPositionX(),me->GetPositionY(),me->GetPositionZ(),me->GetOrientation());
                EnterEvadeMode();
            }
        }

        void UpdateAI(const uint32 uiDiff)
        {
            if (!UpdateVictim())
                return;

            if (uiChargeTimer <= uiDiff)
            {
                DoCastVictim(SPELL_CHARGE);
                uiChargeTimer = 7000;
            } else uiChargeTimer -= uiDiff;

            if (uiShieldBreakerTimer <= uiDiff)
            {
                DoCastVictim(SPELL_SHIELD_BREAKER);
                uiShieldBreakerTimer = 10000;
            } else uiShieldBreakerTimer -= uiDiff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_argent_championAI (pCreature);
    }

};


/*######
## npc_argent_tournament_post
######*/

enum eArgentTournamentPost
{
    SPELL_ROPE_BEAM                 = 63413,
    NPC_GORMOK_THE_IMPALER          = 35469,
    NPC_ICEHOWL                     = 35470
};

class npc_argent_tournament_post : public CreatureScript
{
public:
    npc_argent_tournament_post() : CreatureScript("npc_argent_tournament_post") { }

    struct npc_argent_tournament_postAI : public ScriptedAI
    {
        npc_argent_tournament_postAI(Creature* pCreature) : ScriptedAI(pCreature) {}

        void UpdateAI(const uint32 /*uiDiff*/)
        {
            if (me->IsNonMeleeSpellCasted(false))
                return;

            if (Creature* pTarget = me->FindNearestCreature(NPC_GORMOK_THE_IMPALER, 6.0f))
                DoCast(pTarget, SPELL_ROPE_BEAM);

            if (Creature* pTarget2 = me->FindNearestCreature(NPC_ICEHOWL, 6.0f))
                DoCast(pTarget2, SPELL_ROPE_BEAM);

            if (!UpdateVictim())
                return;
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_argent_tournament_postAI (pCreature);
    }

};


/*######
## npc_alorah_and_grimmin
######*/

enum ealorah_and_grimmin
{
    SPELL_CHAIN                     = 68341,
    NPC_FJOLA_LIGHTBANE             = 36065,
    NPC_EYDIS_DARKBANE              = 36066,
    NPC_PRIESTESS_ALORAH            = 36101,
    NPC_PRIEST_GRIMMIN              = 36102
};

class npc_alorah_and_grimmin : public CreatureScript
{
public:
    npc_alorah_and_grimmin() : CreatureScript("npc_alorah_and_grimmin") { }

    struct npc_alorah_and_grimminAI : public ScriptedAI
    {
        npc_alorah_and_grimminAI(Creature* pCreature) : ScriptedAI(pCreature) {}

        bool uiCast;

        void Reset()
        {
            uiCast = false;
        }

        void UpdateAI(const uint32 /*uiDiff*/)
        {
            if (uiCast)
                return;
            uiCast = true;
            Creature* pTarget = NULL;

            switch(me->GetEntry())
            {
                case NPC_PRIESTESS_ALORAH:
                    pTarget = me->FindNearestCreature(NPC_EYDIS_DARKBANE, 10.0f);
                    break;
                case NPC_PRIEST_GRIMMIN:
                    pTarget = me->FindNearestCreature(NPC_FJOLA_LIGHTBANE, 10.0f);
                    break;
            }
            if (pTarget)
                DoCast(pTarget, SPELL_CHAIN);

            if (!UpdateVictim())
                return;
        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_alorah_and_grimminAI (pCreature);
    }

};


/*######
## npc_guardian_pavilion
######*/

enum eGuardianPavilion
{
    SPELL_TRESPASSER_H                            = 63987,
    AREA_SUNREAVER_PAVILION                       = 4676,

    AREA_SILVER_COVENANT_PAVILION                 = 4677,
    SPELL_TRESPASSER_A                            = 63986,
};

class npc_guardian_pavilion : public CreatureScript
{
public:
    npc_guardian_pavilion() : CreatureScript("npc_guardian_pavilion") { }

    struct npc_guardian_pavilionAI : public Scripted_NoMovementAI
    {
        npc_guardian_pavilionAI(Creature* pCreature) : Scripted_NoMovementAI(pCreature) {}

        void MoveInLineOfSight(Unit* pWho)
        {
            if (me->GetAreaId() != AREA_SUNREAVER_PAVILION && me->GetAreaId() != AREA_SILVER_COVENANT_PAVILION)
                return;

            if (!pWho || pWho->GetTypeId() != TYPEID_PLAYER || !me->IsHostileTo(pWho) || !me->isInBackInMap(pWho, 5.0f))
                return;

            if (pWho->HasAura(SPELL_TRESPASSER_H) || pWho->HasAura(SPELL_TRESPASSER_A))
                return;

            if (pWho->ToPlayer()->GetTeamId() == TEAM_ALLIANCE)
                pWho->CastSpell(pWho, SPELL_TRESPASSER_H, true);
            else
                pWho->CastSpell(pWho, SPELL_TRESPASSER_A, true);

        }
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_guardian_pavilionAI (pCreature);
    }

};


/*######
## npc_vendor_argent_tournament
######*/
const uint32 ArgentTournamentVendor[10][4] =
{
	{33553,13726,2,14460}, // Orc
	{33554,13726,8,14464}, // Troll
	{33556,13728,6,14458}, // Tauren
	{33555,13729,5,14459}, // Undead
	{33557,13731,10,14465}, // Blood Elf
	{33307,13699,1,14456}, // Human
	{33310,13713,3,14457}, // Dwarf
	{33653,13725,4,14463}, // Night Elf
	{33650,13723,7,14462}, // Gnome
	{33657,13724,11,14461} // Draenei
};

class npc_vendor_argent_tournament : public CreatureScript
{
public:
    npc_vendor_argent_tournament() : CreatureScript("npc_vendor_argent_tournament") { }

    bool OnGossipHello(Player* pPlayer, Creature* pCreature)
    {
    	bool npcCheck = false;
    	bool questCheck = false;
    	bool raceCheck = false;
    	uint32 textId = 0;
    	
    	for(int i = 0; (i < 10) && !npcCheck; i++)
    	{
    		if(pCreature->GetEntry() == ArgentTournamentVendor[i][0])
    		{
    			npcCheck = true;
    			questCheck = pPlayer->GetQuestStatus(ArgentTournamentVendor[i][1]) == QUEST_STATUS_COMPLETE;
    			raceCheck = pPlayer->getRace() == ArgentTournamentVendor[i][2];
    			textId = ArgentTournamentVendor[i][3];
    		}
    	}
    	
    	if(questCheck || raceCheck)
    		pPlayer->SEND_VENDORLIST(pCreature->GetGUID()); 
    	else
    	    pPlayer->SEND_GOSSIP_MENU(textId, pCreature->GetGUID());
    	return true;
    }

};

/*
	npc_training_dummy_argent
*/
#define SPELL_DEFEND_AURA 62719
#define SPELL_DEFEND_AURA_1 64100
#define SPELL_ARGENT_CHARGE 63010
#define SPELL_ARGENT_BREAK_SHIELD 62575
#define SPELL_ARGENT_MELEE 62544 
class npc_training_dummy_argent : public CreatureScript
{
public:
    npc_training_dummy_argent() : CreatureScript("npc_training_dummy_argent") { }

    struct npc_training_dummy_argentAI : Scripted_NoMovementAI
    {
        npc_training_dummy_argentAI(Creature *c) : Scripted_NoMovementAI(c)
        {
            m_Entry = c->GetEntry();
        }

        uint64 m_Entry;
        uint32 ResetTimer;
        uint32 DespawnTimer;
    	uint32 ShielTimer;
        void Reset()
        {
            me->SetControlled(true,UNIT_STAT_STUNNED);//disable rotate
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);//imune to knock aways like blast wave
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
            ResetTimer = 10000;
            DespawnTimer = 15000;
    		ShielTimer=0;
        }

        void EnterEvadeMode()
        {
            if (!_EnterEvadeMode())
                return;

            Reset();
        }

        void DamageTaken(Unit * /*done_by*/, uint32 &damage)
        {
            ResetTimer = 10000;
            damage = 0;
        }

        void EnterCombat(Unit * /*who*/)
        {
            if (m_Entry != 2674 && m_Entry != 2673)
                return;
        }

    	void SpellHit(Unit* caster,const SpellEntry* spell)
    	{
	Unit *owner = caster->GetCharmerOrOwnerOrSelf();
	if(owner) {

    			if(m_Entry==33272)
    				if(spell->Id==SPELL_ARGENT_CHARGE)
    					if(me->GetAura(SPELL_DEFEND_AURA_1) && me->GetAura(SPELL_DEFEND_AURA_1)->GetStackAmount() == 1)
    						owner->ToPlayer()->KilledMonsterCredit(33340, 0);
   					else me->SetAuraStack(SPELL_DEFEND_AURA_1,me,me->GetAura(SPELL_DEFEND_AURA_1)->GetStackAmount()-1);
    			if(m_Entry==33229){
    				if(spell->Id==SPELL_ARGENT_MELEE)
    				{
    					owner->ToPlayer()->KilledMonsterCredit(33341, 0);
    					me->CastSpell(caster,62709,true);
    				}
    			}
    			
    		
    			
    		if(m_Entry==33243)
    				if(spell->Id==SPELL_ARGENT_BREAK_SHIELD)
    					if(me->GetAura(SPELL_DEFEND_AURA) && me->GetAura(SPELL_DEFEND_AURA)->GetStackAmount() == 1)
  						owner->ToPlayer()->KilledMonsterCredit(33339, 0);
   					else me->SetAuraStack(SPELL_DEFEND_AURA,me,me->GetAura(SPELL_DEFEND_AURA)->GetStackAmount()-1);
	}
    	   }


        void UpdateAI(const uint32 diff)
        {
    		if (ShielTimer <= diff)
    		{
    			if(m_Entry==33243)
    				me->CastSpell(me,SPELL_DEFEND_AURA,true);

    			if(m_Entry==33272 && !me->GetAura(SPELL_DEFEND_AURA_1))
    					me->CastSpell(me,SPELL_DEFEND_AURA_1,true);
    			ShielTimer = 8000;
    		}
    		else
    			ShielTimer -= diff;

            if (!UpdateVictim())
                return;
            if (!me->hasUnitState(UNIT_STAT_STUNNED))
                me->SetControlled(true,UNIT_STAT_STUNNED);//disable rotate

            if (m_Entry != 2674 && m_Entry != 2673)
            {
                if (ResetTimer <= diff)
                {
                    EnterEvadeMode();
                    ResetTimer = 10000;
                }
                else
                    ResetTimer -= diff;
                return;
            }
            else
            {
                if (DespawnTimer <= diff)
                    me->ForcedDespawn();
                else
                    DespawnTimer -= diff;
            }
        }
        void MoveInLineOfSight(Unit * /*who*/){return;}
    };

    CreatureAI* GetAI(Creature* pCreature) const
    {
        return new npc_training_dummy_argentAI(pCreature);
    }

};

class npc_quest_givers_for_crusaders : public CreatureScript
{
public:
    npc_quest_givers_for_crusaders() : CreatureScript("npc_quest_givers_for_crusaders") { }

    bool OnGossipHello(Player* pPlayer, Creature* pCreature)
    {
    	if (pPlayer->HasTitle(TITLE_CRUSADER))
    		if (pCreature->isQuestGiver())
    			pPlayer->PrepareQuestMenu(pCreature->GetGUID());

    	pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());
    	return true;
    }

};

/*
* Npc Jeran Lockwood (33973)
*/
#define JERAN_DEFAULT_TEXTID 14453
#define JERAN_QUEST_TEXTID 14431
#define JERAN_RP_TEXTID 14434
#define GOSSIP_HELLO_JERAN_1 "Montrez-moi comment m'entra�ner sur une cible de m�l�e."
#define GOSSIP_HELLO_JERAN_2 "Parlez-moi de la d�fense et du coup de lance."
#define SPELL_CREDIT_JERAN 64113
class npc_jeran_lockwood : public CreatureScript
{
public:
    npc_jeran_lockwood() : CreatureScript("npc_jeran_lockwood") { }

    bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
    {
    	switch(uiAction)
    	{
    		case GOSSIP_ACTION_INFO_DEF+1:
    			pPlayer->CastSpell(pPlayer,SPELL_CREDIT_JERAN,true);
    			pPlayer->CLOSE_GOSSIP_MENU();
    			break;
    		case GOSSIP_ACTION_INFO_DEF+2:
    			pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_JERAN_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    			pPlayer->SEND_GOSSIP_MENU(JERAN_RP_TEXTID, pCreature->GetGUID());
    			break;
    	}
    	return true;
    }

    bool OnGossipHello(Player* pPlayer, Creature* pCreature)
    {
    	if((pPlayer->GetQuestStatus(13828) == QUEST_STATUS_INCOMPLETE) || (pPlayer->GetQuestStatus(13829) == QUEST_STATUS_INCOMPLETE))
    	{
    		pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_JERAN_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    		pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_JERAN_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            pPlayer->SEND_GOSSIP_MENU(JERAN_QUEST_TEXTID, pCreature->GetGUID());
    	}
    	else
    	{
    		pPlayer->SEND_GOSSIP_MENU(JERAN_DEFAULT_TEXTID, pCreature->GetGUID());
    	}
    	return true;
    }

};


/*
* Npc Rugan Steelbelly (33972)
*/
#define RUGAN_DEFAULT_TEXTID 14453
#define RUGAN_QUEST_TEXTID 14436
#define RUGAN_RP_TEXTID 14437
#define GOSSIP_HELLO_RUGAN_1 "Montrez-moi comment m'entra�ner sur une cible de charge."
#define GOSSIP_HELLO_RUGAN_2 "Parlez-moi de la charge"
#define SPELL_CREDIT_RUGAN 64114
class npc_rugan_steelbelly : public CreatureScript
{
public:
    npc_rugan_steelbelly() : CreatureScript("npc_rugan_steelbelly") { }

    bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
    {
    	switch(uiAction)
    	{
    		case GOSSIP_ACTION_INFO_DEF+1:
    			pPlayer->CastSpell(pPlayer,SPELL_CREDIT_RUGAN,true);
    			pPlayer->CLOSE_GOSSIP_MENU();
    			break;
    		case GOSSIP_ACTION_INFO_DEF+2:
    			pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_RUGAN_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    			pPlayer->SEND_GOSSIP_MENU(RUGAN_RP_TEXTID, pCreature->GetGUID());
    			break;
    	}
    	return true;
    }

    bool OnGossipHello(Player* pPlayer, Creature* pCreature)
    {
    	if((pPlayer->GetQuestStatus(13837) == QUEST_STATUS_INCOMPLETE) || (pPlayer->GetQuestStatus(13839) == QUEST_STATUS_INCOMPLETE))
    	{
    		pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_RUGAN_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    		pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_RUGAN_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            pPlayer->SEND_GOSSIP_MENU(RUGAN_QUEST_TEXTID, pCreature->GetGUID());
    	}
    	else
    	{
    		pPlayer->SEND_GOSSIP_MENU(RUGAN_DEFAULT_TEXTID, pCreature->GetGUID());
    	}
    	return true;
    }

};


/*
* Npc Valis Windchaser
*/
#define VALIS_DEFAULT_TEXTID 14453
#define VALIS_QUEST_TEXTID 14438
#define VALIS_RP_TEXTID 14439
#define GOSSIP_HELLO_VALIS_1 "Montrez-moi comment m'entra�ner sur une cible � distance."
#define GOSSIP_HELLO_VALIS_2 "Expliquez-moi comment utiliser le brise-bouclier."
#define SPELL_CREDIT_VALIS 64115
class npc_valis_windchaser : public CreatureScript
{
public:
    npc_valis_windchaser() : CreatureScript("npc_valis_windchaser") { }

    bool OnGossipSelect(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
    {
    	switch (uiAction)
    	{
    		case GOSSIP_ACTION_INFO_DEF+1:
    			pPlayer->CastSpell(pPlayer,SPELL_CREDIT_VALIS,true);//Cast du sort de credit quest (valide l'objectif)
    			pPlayer->CLOSE_GOSSIP_MENU();//Ferme la fenetre du gossip cot� client
    		break;
    		case GOSSIP_ACTION_INFO_DEF+2:
    			//Raconte un blabla
    			pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_VALIS_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            	pPlayer->SEND_GOSSIP_MENU(VALIS_RP_TEXTID, pCreature->GetGUID());
    		break;
    	}
    	return true;
    }

    bool OnGossipHello(Player* pPlayer, Creature* pCreature)
    {
    	//Si il a la quete
    	if((pPlayer->GetQuestStatus(13835) == QUEST_STATUS_INCOMPLETE) || 
    		(pPlayer->GetQuestStatus(13838) == QUEST_STATUS_INCOMPLETE))
    	{
    		pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_VALIS_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    		pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_VALIS_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            pPlayer->SEND_GOSSIP_MENU(VALIS_QUEST_TEXTID, pCreature->GetGUID());
    	}
    	//Sinon Texte par d�faut
    	else
    		pPlayer->SEND_GOSSIP_MENU(VALIS_DEFAULT_TEXTID, pCreature->GetGUID());
    	return true;
    }

};

void AddSC_icecrown()
{
    new npc_arete;
    new npc_dame_evniki_kapsalis;
    new npc_squire_david;
    new npc_squire_danny;
    new npc_argent_valiant;
    new npc_argent_champion;
    new npc_argent_tournament_post;
    new npc_alorah_and_grimmin;
    new npc_guardian_pavilion;
    new npc_vendor_argent_tournament;
    new npc_training_dummy_argent;
    new npc_quest_givers_for_crusaders;
    new npc_valis_windchaser;
    new npc_rugan_steelbelly;
    new npc_jeran_lockwood;
}