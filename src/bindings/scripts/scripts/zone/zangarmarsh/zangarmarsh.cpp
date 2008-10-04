/* Copyright (C) 2006 - 2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: Zangarmarsh
SD%Complete: 100
SDComment: Quest support: 9785, 9803, 10009. Mark Of ... buffs.
SDCategory: Zangarmarsh
EndScriptData */

/* ContentData
npcs_ashyen_and_keleth
npc_cooshcoosh
npc_elder_kuruti
npc_mortog_steamhead
EndContentData */

#include "precompiled.h"

/*######
## npcs_ashyen_and_keleth
######*/

#define GOSSIP_ITEM_BLESS_ASH     "Grant me your mark, wise ancient."
#define GOSSIP_ITEM_BLESS_KEL     "Grant me your mark, mighty ancient."
#define GOSSIP_REWARD_BLESS       "You have my blessing"
//#define TEXT_BLESSINGS        "<You need higher standing with Cenarion Expedition to recive a blessing.>"

bool GossipHello_npcs_ashyen_and_keleth(Player *player, Creature *_Creature )
{
    if (player->GetReputationRank(942) > REP_NEUTRAL)
    {
        if ( _Creature->GetEntry() == 17900)
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM_BLESS_ASH, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
        if ( _Creature->GetEntry() == 17901)
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM_BLESS_KEL, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
    }
    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npcs_ashyen_and_keleth(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_INFO_DEF+1)
    {
        _Creature->setPowerType(POWER_MANA);
        _Creature->SetMaxPower(POWER_MANA,200);             //set a "fake" mana value, we can't depend on database doing it in this case
        _Creature->SetPower(POWER_MANA,200);

        if ( _Creature->GetEntry() == 17900)                //check which creature we are dealing with
        {
            switch (player->GetReputationRank(942))
            {                                               //mark of lore
                case REP_FRIENDLY:
                    _Creature->CastSpell(player, 31808, true);
                    _Creature->Say(GOSSIP_REWARD_BLESS, LANG_UNIVERSAL, 0);
                    break;
                case REP_HONORED:
                    _Creature->CastSpell(player, 31810, true);
                    _Creature->Say(GOSSIP_REWARD_BLESS, LANG_UNIVERSAL, 0);
                    break;
                case REP_REVERED:
                    _Creature->CastSpell(player, 31811, true);
                    _Creature->Say(GOSSIP_REWARD_BLESS, LANG_UNIVERSAL, 0);
                    break;
                case REP_EXALTED:
                    _Creature->CastSpell(player, 31815, true);
                    _Creature->Say(GOSSIP_REWARD_BLESS, LANG_UNIVERSAL, 0);
                    break;
            }
        }

        if ( _Creature->GetEntry() == 17901)
        {
            switch (player->GetReputationRank(942))         //mark of war
            {
                case REP_FRIENDLY:
                    _Creature->CastSpell(player, 31807, true);
                    _Creature->Say(GOSSIP_REWARD_BLESS, LANG_UNIVERSAL, 0);
                    break;
                case REP_HONORED:
                    _Creature->CastSpell(player, 31812, true);
                    _Creature->Say(GOSSIP_REWARD_BLESS, LANG_UNIVERSAL, 0);
                    break;
                case REP_REVERED:
                    _Creature->CastSpell(player, 31813, true);
                    _Creature->Say(GOSSIP_REWARD_BLESS, LANG_UNIVERSAL, 0);
                    break;
                case REP_EXALTED:
                    _Creature->CastSpell(player, 31814, true);
                    _Creature->Say(GOSSIP_REWARD_BLESS, LANG_UNIVERSAL, 0);
                    break;
            }
        }
        player->CLOSE_GOSSIP_MENU();
        player->TalkedToCreature(_Creature->GetEntry(), _Creature->GetGUID());
    }
    return true;
}

/*######
## npc_cooshcoosh
######*/

#define GOSSIP_COOSH            "You owe Sim'salabim money. Hand them over or die!"

#define FACTION_HOSTILE_CO      45
#define FACTION_FRIENDLY_CO     35

#define SPELL_LIGHTNING_BOLT    9532

struct MANGOS_DLL_DECL npc_cooshcooshAI : public ScriptedAI
{
    npc_cooshcooshAI(Creature* c) : ScriptedAI(c) { Reset(); }

    uint32 LightningBolt_Timer;

    void Reset()
    {
        LightningBolt_Timer = 2000;
        m_creature->setFaction(FACTION_FRIENDLY_CO);
    }

    void Aggro(Unit *who) {}

    void UpdateAI(const uint32 diff)
    {
        if(!m_creature->SelectHostilTarget() || !m_creature->getVictim())
            return;

        if( LightningBolt_Timer < diff )
        {
            DoCast(m_creature->getVictim(),SPELL_LIGHTNING_BOLT);
            LightningBolt_Timer = 5000;
        }else LightningBolt_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_npc_cooshcoosh(Creature *_Creature)
{
    return new npc_cooshcooshAI (_Creature);
}

bool GossipHello_npc_cooshcoosh(Player *player, Creature *_Creature )
{
    if( player->GetQuestStatus(10009) == QUEST_STATUS_INCOMPLETE )
        player->ADD_GOSSIP_ITEM(1, GOSSIP_COOSH, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU(9441, _Creature->GetGUID());
    return true;
}

bool GossipSelect_npc_cooshcoosh(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    if( action == GOSSIP_ACTION_INFO_DEF )
    {
        player->CLOSE_GOSSIP_MENU();
        _Creature->setFaction(FACTION_HOSTILE_CO);
        ((npc_cooshcooshAI*)_Creature->AI())->AttackStart(player);
    }
    return true;
}

/*######
## npc_elder_kuruti
######*/

#define GOSSIP_ITEM_KUR1 "Offer treat"
#define GOSSIP_ITEM_KUR2 "Im a messenger for Draenei"
#define GOSSIP_ITEM_KUR3 "Get message"

bool GossipHello_npc_elder_kuruti(Player *player, Creature *_Creature )
{
    if( player->GetQuestStatus(9803) == QUEST_STATUS_INCOMPLETE )
        player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM_KUR1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    player->SEND_GOSSIP_MENU(9226,_Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_elder_kuruti(Player *player, Creature *_Creature, uint32 sender, uint32 action )
{
    switch (action)
    {
        case GOSSIP_ACTION_INFO_DEF:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM_KUR2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(9227, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 1:
            player->ADD_GOSSIP_ITEM( 0, GOSSIP_ITEM_KUR3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU(9229, _Creature->GetGUID());
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
        {
            if( !player->HasItemCount(24573,1) )
            {
                ItemPosCountVec dest;
                uint8 msg = player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, 24573, 1, false);
                if( msg == EQUIP_ERR_OK )
                {
                    player->StoreNewItem( dest, 24573, true);
                }
                else
                    player->SendEquipError( msg,NULL,NULL );
            }
            player->SEND_GOSSIP_MENU(9231, _Creature->GetGUID());
            break;
        }
    }
    return true;
}

/*######
## npc_mortog_steamhead
######*/

bool GossipHello_npc_mortog_steamhead(Player *player, Creature *_Creature)
{
    if (_Creature->isVendor() && player->GetReputationRank(942) == REP_EXALTED)
        player->ADD_GOSSIP_ITEM(1, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

    player->SEND_GOSSIP_MENU(_Creature->GetNpcTextId(), _Creature->GetGUID());

    return true;
}

bool GossipSelect_npc_mortog_steamhead(Player *player, Creature *_Creature, uint32 sender, uint32 action)
{
    if (action == GOSSIP_ACTION_TRADE)
    {
        player->SEND_VENDORLIST( _Creature->GetGUID() );
    }
    return true;
}

/*######
## AddSC
######*/

void AddSC_zangarmarsh()
{
    Script *newscript;

    newscript = new Script;
    newscript->Name="npcs_ashyen_and_keleth";
    newscript->pGossipHello =  &GossipHello_npcs_ashyen_and_keleth;
    newscript->pGossipSelect = &GossipSelect_npcs_ashyen_and_keleth;
    m_scripts[nrscripts++] = newscript;

    newscript = new Script;
    newscript->Name="npc_cooshcoosh";
    newscript->pGossipHello =  &GossipHello_npc_cooshcoosh;
    newscript->pGossipSelect = &GossipSelect_npc_cooshcoosh;
    m_scripts[nrscripts++] = newscript;

    newscript = new Script;
    newscript->Name="npc_elder_kuruti";
    newscript->pGossipHello =  &GossipHello_npc_elder_kuruti;
    newscript->pGossipSelect = &GossipSelect_npc_elder_kuruti;
    m_scripts[nrscripts++] = newscript;

    newscript = new Script;
    newscript->Name="npc_mortog_steamhead";
    newscript->pGossipHello =  &GossipHello_npc_mortog_steamhead;
    newscript->pGossipSelect = &GossipSelect_npc_mortog_steamhead;
    m_scripts[nrscripts++] = newscript;
}
