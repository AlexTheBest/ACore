/*
 * Copyright (C) 2005-2008 MaNGOS <http://www.mangosproject.org/>
 *
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

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "World.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Opcodes.h"
#include "Chat.h"
#include "Log.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "Language.h"
#include "CellImpl.h"
#include "InstanceSaveMgr.h"
#include "Util.h"
#ifdef _DEBUG_VMAPS
#include "VMapFactory.h"
#endif

bool ChatHandler::HandleSayCommand(const char* args)
{
    if(!*args)
        return false;

    Creature* pCreature = getSelectedCreature();
    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    pCreature->Say(args, LANG_UNIVERSAL, 0);

    return true;
}

bool ChatHandler::HandleYellCommand(const char* args)
{
    if(!*args)
        return false;

    Creature* pCreature = getSelectedCreature();
    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    pCreature->Yell(args, LANG_UNIVERSAL, 0);

    return true;
}

//show text emote by creature in chat
bool ChatHandler::HandleTextEmoteCommand(const char* args)
{
    if(!*args)
        return false;

    Creature* pCreature = getSelectedCreature();

    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    pCreature->TextEmote(args, 0);

    return true;
}

// make npc whisper to player
bool ChatHandler::HandleNpcWhisperCommand(const char* args)
{
    if(!*args)
        return false;

    char* receiver_str = strtok((char*)args, " ");
    char* text = strtok(NULL, "");

    uint64 guid = m_session->GetPlayer()->GetSelection();
    Creature* pCreature = ObjectAccessor::GetCreature(*m_session->GetPlayer(), guid);

    if(!pCreature || !receiver_str || !text)
    {
        return false;
    }

    uint64 receiver_guid= atol(receiver_str);

    pCreature->Whisper(text,receiver_guid);

    return true;
}

// global announce
bool ChatHandler::HandleAnnounceCommand(const char* args)
{
    if(!*args)
        return false;

    sWorld.SendWorldText(LANG_SYSTEMMESSAGE,args);
    return true;
}

//notification player at the screen
bool ChatHandler::HandleNotifyCommand(const char* args)
{
    if(!*args)
        return false;

    std::string str = GetMangosString(LANG_GLOBAL_NOTIFY);
    str += args;

    WorldPacket data(SMSG_NOTIFICATION, (str.size()+1));
    data << str;
    sWorld.SendGlobalMessage(&data);

    return true;
}

//Enable\Dissable GM Mode
bool ChatHandler::HandleGMmodeCommand(const char* args)
{
    if(!*args)
    {
        SendSysMessage(LANG_USE_BOL);
        SetSentErrorMessage(true);
        return false;
    }

    std::string argstr = (char*)args;

    if (argstr == "on")
    {
        m_session->GetPlayer()->SetGameMaster(true);
        m_session->SendNotification("GM mode is ON");
        #ifdef _DEBUG_VMAPS
        VMAP::IVMapManager *vMapManager = VMAP::VMapFactory::createOrGetVMapManager();
        vMapManager->processCommand("stoplog");
        #endif
        return true;
    }

    if (argstr == "off")
    {
        m_session->GetPlayer()->SetGameMaster(false);
        m_session->SendNotification("GM mode is OFF");
        #ifdef _DEBUG_VMAPS
        VMAP::IVMapManager *vMapManager = VMAP::VMapFactory::createOrGetVMapManager();
        vMapManager->processCommand("startlog");
        #endif
        return true;
    }

    SendSysMessage(LANG_USE_BOL);
    SetSentErrorMessage(true);
    return false;
}

//Enable\Dissable Invisible mode
bool ChatHandler::HandleVisibleCommand(const char* args)
{
    if (!*args)
    {
        PSendSysMessage(LANG_YOU_ARE, m_session->GetPlayer()->isGMVisible() ?  GetMangosString(LANG_VISIBLE) : GetMangosString(LANG_INVISIBLE));
        return true;
    }

    std::string argstr = (char*)args;

    if (argstr == "on")
    {
        m_session->GetPlayer()->SetGMVisible(true);
        m_session->SendNotification(GetMangosString(LANG_INVISIBLE_VISIBLE));
        return true;
    }

    if (argstr == "off")
    {
        m_session->SendNotification(GetMangosString(LANG_INVISIBLE_INVISIBLE));
        m_session->GetPlayer()->SetGMVisible(false);
        return true;
    }

    SendSysMessage(LANG_USE_BOL);
    SetSentErrorMessage(true);
    return false;
}

bool ChatHandler::HandleGPSCommand(const char* args)
{
    WorldObject *obj = NULL;
    if (*args)
    {
        std::string name = args;
        if(normalizePlayerName(name))
            obj = objmgr.GetPlayer(name.c_str());

        if(!obj)
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }
    }
    else
    {
        obj = getSelectedUnit();

        if(!obj)
        {
            SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            SetSentErrorMessage(true);
            return false;
        }
    }
    CellPair cell_val = MaNGOS::ComputeCellPair(obj->GetPositionX(), obj->GetPositionY());
    Cell cell(cell_val);

    uint32 zone_id = obj->GetZoneId();
    uint32 area_id = obj->GetAreaId();

    MapEntry const* mapEntry = sMapStore.LookupEntry(obj->GetMapId());
    AreaTableEntry const* zoneEntry = GetAreaEntryByAreaID(zone_id);
    AreaTableEntry const* areaEntry = GetAreaEntryByAreaID(area_id);

    float zone_x = obj->GetPositionX();
    float zone_y = obj->GetPositionY();

    Map2ZoneCoordinates(zone_x,zone_y,zone_id);

    Map const *map = MapManager::Instance().GetMap(obj->GetMapId(), obj);
    float ground_z = map->GetHeight(obj->GetPositionX(), obj->GetPositionY(), MAX_HEIGHT);
    float floor_z = map->GetHeight(obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ());

    GridPair p = MaNGOS::ComputeGridPair(obj->GetPositionX(), obj->GetPositionY());

    int gx=63-p.x_coord;
    int gy=63-p.y_coord;

    uint32 have_map = Map::ExistMap(obj->GetMapId(),gx,gy) ? 1 : 0;
    uint32 have_vmap = Map::ExistVMap(obj->GetMapId(),gx,gy) ? 1 : 0;

    PSendSysMessage(LANG_MAP_POSITION,
        obj->GetMapId(), (mapEntry ? mapEntry->name[m_session->GetSessionDbcLocale()] : "<unknown>" ),
        zone_id, (zoneEntry ? zoneEntry->area_name[m_session->GetSessionDbcLocale()] : "<unknown>" ),
        area_id, (areaEntry ? areaEntry->area_name[m_session->GetSessionDbcLocale()] : "<unknown>" ),
        obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), obj->GetOrientation(),
        cell.GridX(), cell.GridY(), cell.CellX(), cell.CellY(), obj->GetInstanceId(),
        zone_x, zone_y, ground_z, floor_z, have_map, have_vmap );

    sLog.outDebug("Player %s GPS call for %s '%s' (%s: %u):",
        m_session->GetPlayer()->GetName(),
        (obj->GetTypeId() == TYPEID_PLAYER ? "player" : "creature"), obj->GetName(),
        (obj->GetTypeId() == TYPEID_PLAYER ? "GUID" : "Entry"), (obj->GetTypeId() == TYPEID_PLAYER ? obj->GetGUIDLow(): obj->GetEntry()) );
    sLog.outDebug(GetMangosString(LANG_MAP_POSITION),
        obj->GetMapId(), (mapEntry ? mapEntry->name[sWorld.GetDefaultDbcLocale()] : "<unknown>" ),
        zone_id, (zoneEntry ? zoneEntry->area_name[sWorld.GetDefaultDbcLocale()] : "<unknown>" ),
        area_id, (areaEntry ? areaEntry->area_name[sWorld.GetDefaultDbcLocale()] : "<unknown>" ),
        obj->GetPositionX(), obj->GetPositionY(), obj->GetPositionZ(), obj->GetOrientation(),
        cell.GridX(), cell.GridY(), cell.CellX(), cell.CellY(), obj->GetInstanceId(),
        zone_x, zone_y, ground_z, floor_z, have_map, have_vmap );

    return true;
}

//Summon Player
bool ChatHandler::HandleNamegoCommand(const char* args)
{
    if(!*args)
        return false;

    std::string name = args;

    if(!normalizePlayerName(name))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = objmgr.GetPlayer(name.c_str());
    if (chr)
    {
        if(chr->IsBeingTeleported()==true)
        {
            PSendSysMessage(LANG_IS_TELEPORTED, chr->GetName());
            SetSentErrorMessage(true);
            return false;
        }

        Map* pMap = MapManager::Instance().GetMap(m_session->GetPlayer()->GetMapId(),m_session->GetPlayer());

        if(pMap->Instanceable())
        {
            Map* cMap = MapManager::Instance().GetMap(chr->GetMapId(),chr);
            if( cMap->Instanceable() && cMap->GetInstanceId() != pMap->GetInstanceId() )
            {
                // cannot summon from instance to instance
                PSendSysMessage(LANG_CANNOT_SUMMON_TO_INST,chr->GetName());
                SetSentErrorMessage(true);
                return false;
            }

            // we are in instance, and can summon only player in our group with us as lead
            if ( !m_session->GetPlayer()->GetGroup() || !chr->GetGroup() ||
                (chr->GetGroup()->GetLeaderGUID() != m_session->GetPlayer()->GetGUID()) ||
                (m_session->GetPlayer()->GetGroup()->GetLeaderGUID() != m_session->GetPlayer()->GetGUID()) )
                // the last check is a bit excessive, but let it be, just in case
            {
                PSendSysMessage(LANG_CANNOT_SUMMON_TO_INST,chr->GetName());
                SetSentErrorMessage(true);
                return false;
            }
        }

        PSendSysMessage(LANG_SUMMONING, chr->GetName(),"");

        if (m_session->GetPlayer()->IsVisibleGloballyFor(chr))
            ChatHandler(chr).PSendSysMessage(LANG_SUMMONED_BY, m_session->GetPlayer()->GetName());

        // stop flight if need
        if(chr->isInFlight())
        {
            chr->GetMotionMaster()->MovementExpired();
            chr->m_taxi.ClearTaxiDestinations();
        }
        // save only in non-flight case
        else
            chr->SaveRecallPosition();

        // before GM
        float x,y,z;
        m_session->GetPlayer()->GetClosePoint(x,y,z,chr->GetObjectSize());
        chr->TeleportTo(m_session->GetPlayer()->GetMapId(),x,y,z,chr->GetOrientation());
    }
    else if (uint64 guid = objmgr.GetPlayerGUIDByName(name))
    {
        PSendSysMessage(LANG_SUMMONING, name.c_str(),GetMangosString(LANG_OFFLINE));

        // in point where GM stay
        Player::SavePositionInDB(m_session->GetPlayer()->GetMapId(),
            m_session->GetPlayer()->GetPositionX(),
            m_session->GetPlayer()->GetPositionY(),
            m_session->GetPlayer()->GetPositionZ(),
            m_session->GetPlayer()->GetOrientation(),
            m_session->GetPlayer()->GetZoneId(),
            guid);
    }
    else
    {
        PSendSysMessage(LANG_NO_PLAYER, args);
        SetSentErrorMessage(true);
    }

    return true;
}

//Teleport to Player
bool ChatHandler::HandleGonameCommand(const char* args)
{
    if(!*args)
        return false;

    Player* _player = m_session->GetPlayer();

    std::string name = args;

    if(!normalizePlayerName(name))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = objmgr.GetPlayer(name.c_str());
    if (chr)
    {
        Map* cMap = MapManager::Instance().GetMap(chr->GetMapId(),chr);
        if(cMap->Instanceable())
        {
            Map* pMap = MapManager::Instance().GetMap(_player->GetMapId(),_player);

            // we have to go to instance, and can go to player only if:
            //   1) we are in his group (either as leader or as member)
            //   2) we are not bound to any group and have GM mode on
            if (_player->GetGroup())
            {
                // we are in group, we can go only if we are in the player group
                if (_player->GetGroup() != chr->GetGroup())
                {
                    PSendSysMessage(LANG_CANNOT_GO_TO_INST_PARTY,chr->GetName());
                    SetSentErrorMessage(true);
                    return false;
                }
            }
            else
            {
                // we are not in group, let's verify our GM mode
                if (!_player->isGameMaster())
                {
                    PSendSysMessage(LANG_CANNOT_GO_TO_INST_GM,chr->GetName());
                    SetSentErrorMessage(true);
                    return false;
                }
            }

            // if the player or the player's group is bound to another instance
            // the player will not be bound to another one
            InstancePlayerBind *pBind = _player->GetBoundInstance(chr->GetMapId(), chr->GetDifficulty());
            if(!pBind)
            {
                Group *group = _player->GetGroup();
                InstanceGroupBind *gBind = group ? group->GetBoundInstance(chr->GetMapId(), chr->GetDifficulty()) : NULL;
                if(!gBind)
                {
                    // if no bind exists, create a solo bind
                    InstanceSave *save = sInstanceSaveManager.GetInstanceSave(chr->GetInstanceId());
                    if(save) _player->BindToInstance(save, !save->CanReset());
                }
            }

            _player->SetDifficulty(chr->GetDifficulty());
        }

        PSendSysMessage(LANG_APPEARING_AT, chr->GetName());

        if (_player->IsVisibleGloballyFor(chr))
            ChatHandler(chr).PSendSysMessage(LANG_APPEARING_TO, _player->GetName());

        // stop flight if need
        if(_player->isInFlight())
        {
            _player->GetMotionMaster()->MovementExpired();
            _player->m_taxi.ClearTaxiDestinations();
        }
        // save only in non-flight case
        else
            _player->SaveRecallPosition();

        // to point to see at target with same orientation
        float x,y,z;
        chr->GetContactPoint(m_session->GetPlayer(),x,y,z);

        _player->TeleportTo(chr->GetMapId(), x, y, z, _player->GetAngle( chr ), TELE_TO_GM_MODE);

        return true;
    }

    if (uint64 guid = objmgr.GetPlayerGUIDByName(name))
    {
        PSendSysMessage(LANG_APPEARING_AT, name.c_str());

        // to point where player stay (if loaded)
        float x,y,z,o;
        uint32 map;
        bool in_flight;
        if(Player::LoadPositionFromDB(map,x,y,z,o,in_flight,guid))
        {
            // stop flight if need
            if(_player->isInFlight())
            {
                _player->GetMotionMaster()->MovementExpired();
                _player->m_taxi.ClearTaxiDestinations();
            }
            // save only in non-flight case
            else
                _player->SaveRecallPosition();

            _player->TeleportTo(map, x, y, z,_player->GetOrientation());
            return true;
        }
    }

    PSendSysMessage(LANG_NO_PLAYER, args);

    SetSentErrorMessage(true);
    return false;
}

// Teleport player to last position
bool ChatHandler::HandleRecallCommand(const char* args)
{
    Player* chr = NULL;

    if(!*args)
    {
        chr = getSelectedPlayer();
        if(!chr)
            chr = m_session->GetPlayer();
    }
    else
    {
        std::string name = args;

        if(!normalizePlayerName(name))
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        chr = objmgr.GetPlayer(name.c_str());

        if(!chr)
        {
            PSendSysMessage(LANG_NO_PLAYER, args);
            SetSentErrorMessage(true);
            return false;
        }
    }

    if(chr->IsBeingTeleported())
    {
        PSendSysMessage(LANG_IS_TELEPORTED, chr->GetName());
        SetSentErrorMessage(true);
        return false;
    }

    // stop flight if need
    if(chr->isInFlight())
    {
        chr->GetMotionMaster()->MovementExpired();
        chr->m_taxi.ClearTaxiDestinations();
    }

    chr->TeleportTo(chr->m_recallMap, chr->m_recallX, chr->m_recallY, chr->m_recallZ, chr->m_recallO);
    return true;
}

//Edit Player KnownTitles
bool ChatHandler::HandleModifyKnownTitlesCommand(const char* args)
{
    if(!*args)
        return false;

    uint64 titles = 0;

    sscanf((char*)args, I64FMTD, &titles);

    Player *chr = getSelectedPlayer();
    if (!chr)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    uint64 titles2 = titles;

    for(int i=1; i < sCharTitlesStore.GetNumRows(); ++i)
        if(CharTitlesEntry const* tEntry = sCharTitlesStore.LookupEntry(i))
            titles2 &= ~(uint64(1) << tEntry->bit_index);

    titles &= ~titles2;                                     // remove not existed titles

    chr->SetUInt64Value(PLAYER__FIELD_KNOWN_TITLES, titles);
    SendSysMessage(LANG_DONE);

    return true;
}

//Edit Player HP
bool ChatHandler::HandleModifyHPCommand(const char* args)
{
    if(!*args)
        return false;

    //    char* pHp = strtok((char*)args, " ");
    //    if (!pHp)
    //        return false;

    //    char* pHpMax = strtok(NULL, " ");
    //    if (!pHpMax)
    //        return false;

    //    int32 hpm = atoi(pHpMax);
    //    int32 hp = atoi(pHp);

    int32 hp = atoi((char*)args);
    int32 hpm = atoi((char*)args);

    if (hp <= 0 || hpm <= 0 || hpm < hp)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_CHANGE_HP, chr->GetName(), hp, hpm);
    ChatHandler(chr).PSendSysMessage(LANG_YOURS_HP_CHANGED, m_session->GetPlayer()->GetName(), hp, hpm);

    chr->SetMaxHealth( hpm );
    chr->SetHealth( hp );

    return true;
}

//Edit Player Mana
bool ChatHandler::HandleModifyManaCommand(const char* args)
{
    if(!*args)
        return false;

    // char* pmana = strtok((char*)args, " ");
    // if (!pmana)
    //     return false;

    // char* pmanaMax = strtok(NULL, " ");
    // if (!pmanaMax)
    //     return false;

    // int32 manam = atoi(pmanaMax);
    // int32 mana = atoi(pmana);
    int32 mana = atoi((char*)args);
    int32 manam = atoi((char*)args);

    if (mana <= 0 || manam <= 0 || manam < mana)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_CHANGE_MANA, chr->GetName(), mana, manam);
    ChatHandler(chr).PSendSysMessage(LANG_YOURS_MANA_CHANGED, m_session->GetPlayer()->GetName(), mana, manam);

    chr->SetMaxPower(POWER_MANA,manam );
    chr->SetPower(POWER_MANA, mana );

    return true;
}

//Edit Player Energy
bool ChatHandler::HandleModifyEnergyCommand(const char* args)
{
    if(!*args)
        return false;

    // char* pmana = strtok((char*)args, " ");
    // if (!pmana)
    //     return false;

    // char* pmanaMax = strtok(NULL, " ");
    // if (!pmanaMax)
    //     return false;

    // int32 manam = atoi(pmanaMax);
    // int32 mana = atoi(pmana);

    int32 energy = atoi((char*)args)*10;
    int32 energym = atoi((char*)args)*10;

    if (energy <= 0 || energym <= 0 || energym < energy)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = getSelectedPlayer();
    if (!chr)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_CHANGE_ENERGY, chr->GetName(), energy/10, energym/10);
    ChatHandler(chr).PSendSysMessage(LANG_YOURS_ENERGY_CHANGED, m_session->GetPlayer()->GetName(), energy/10, energym/10);

    chr->SetMaxPower(POWER_ENERGY,energym );
    chr->SetPower(POWER_ENERGY, energy );

    sLog.outDetail(GetMangosString(LANG_CURRENT_ENERGY),chr->GetMaxPower(POWER_ENERGY));

    return true;
}

//Edit Player Rage
bool ChatHandler::HandleModifyRageCommand(const char* args)
{
    if(!*args)
        return false;

    // char* pmana = strtok((char*)args, " ");
    // if (!pmana)
    //     return false;

    // char* pmanaMax = strtok(NULL, " ");
    // if (!pmanaMax)
    //     return false;

    // int32 manam = atoi(pmanaMax);
    // int32 mana = atoi(pmana);

    int32 rage = atoi((char*)args)*10;
    int32 ragem = atoi((char*)args)*10;

    if (rage <= 0 || ragem <= 0 || ragem < rage)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_CHANGE_RAGE, chr->GetName(), rage/10, ragem/10);
                                                            // Special case: I use GetMangosString here to get local of destination char ;)
    ChatHandler(chr).PSendSysMessage(ChatHandler(chr).GetMangosString(LANG_YOURS_RAGE_CHANGED), m_session->GetPlayer()->GetName(), rage/10, ragem/10);

    chr->SetMaxPower(POWER_RAGE,ragem );
    chr->SetPower(POWER_RAGE, rage );

    return true;
}

//Edit Player Faction
bool ChatHandler::HandleModifyFactionCommand(const char* args)
{
    if(!*args)
        return false;

    char* pfactionid = extractKeyFromLink((char*)args,"Hfaction");

    Creature* chr = getSelectedCreature();
    if(!chr)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    if(!pfactionid)
    {
        if(chr)
        {
            uint32 factionid = chr->getFaction();
            uint32 flag      = chr->GetUInt32Value(UNIT_FIELD_FLAGS);
            uint32 npcflag   = chr->GetUInt32Value(UNIT_NPC_FLAGS);
            uint32 dyflag    = chr->GetUInt32Value(UNIT_DYNAMIC_FLAGS);
            PSendSysMessage(LANG_CURRENT_FACTION,chr->GetGUIDLow(),factionid,flag,npcflag,dyflag);
        }
        return true;
    }

    if( !chr )
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 factionid = atoi(pfactionid);
    uint32 flag;

    char *pflag = strtok(NULL, " ");
    if (!pflag)
        flag = chr->GetUInt32Value(UNIT_FIELD_FLAGS);
    else
        flag = atoi(pflag);

    char* pnpcflag = strtok(NULL, " ");

    uint32 npcflag;
    if(!pnpcflag)
        npcflag   = chr->GetUInt32Value(UNIT_NPC_FLAGS);
    else
        npcflag = atoi(pnpcflag);

    char* pdyflag = strtok(NULL, " ");

    uint32  dyflag;
    if(!pdyflag)
        dyflag   = chr->GetUInt32Value(UNIT_DYNAMIC_FLAGS);
    else
        dyflag = atoi(pdyflag);

    if(!sFactionTemplateStore.LookupEntry(factionid))
    {
        PSendSysMessage(LANG_WRONG_FACTION, factionid);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_CHANGE_FACTION, chr->GetGUIDLow(),factionid,flag,npcflag,dyflag);

    //sprintf((char*)buf,"%s changed your Faction to %i.", m_session->GetPlayer()->GetName(), factionid);
    //FillSystemMessageData(&data, m_session, buf);

    //chr->GetSession()->SendPacket(&data);

    chr->setFaction(factionid);
    chr->SetUInt32Value(UNIT_FIELD_FLAGS,flag);
    chr->SetUInt32Value(UNIT_NPC_FLAGS,npcflag);
    chr->SetUInt32Value(UNIT_DYNAMIC_FLAGS,dyflag);

    return true;
}

//Edit Player Spell
bool ChatHandler::HandleModifySpellCommand(const char* args)
{
    if(!*args) return false;
    char* pspellflatid = strtok((char*)args, " ");
    if (!pspellflatid)
        return false;

    char* pop = strtok(NULL, " ");
    if (!pop)
        return false;

    char* pval = strtok(NULL, " ");
    if (!pval)
        return false;

    uint16 mark;

    char* pmark = strtok(NULL, " ");

    uint8 spellflatid = atoi(pspellflatid);
    uint8 op   = atoi(pop);
    uint16 val = atoi(pval);
    if(!pmark)
        mark = 65535;
    else
        mark = atoi(pmark);

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_CHANGE_SPELLFLATID, spellflatid, val, mark, chr->GetName());
    if(chr != m_session->GetPlayer())
        ChatHandler(chr).PSendSysMessage(LANG_YOURS_SPELLFLATID_CHANGED, m_session->GetPlayer()->GetName(), spellflatid, val, mark);

    WorldPacket data(SMSG_SET_FLAT_SPELL_MODIFIER, (1+1+2+2));
    data << uint8(spellflatid);
    data << uint8(op);
    data << uint16(val);
    data << uint16(mark);
    chr->GetSession()->SendPacket(&data);

    return true;
}

//Edit Player TP
bool ChatHandler::HandleModifyTalentCommand (const char* args)
{
    if (!*args)
        return false;

    int tp = atoi((char*)args);
    if (tp>0)
    {
        Player* player = getSelectedPlayer();
        if(!player)
        {
            SendSysMessage(LANG_NO_CHAR_SELECTED);
            SetSentErrorMessage(true);
            return false;
        }
        player->SetFreeTalentPoints(tp);
        return true;
    }
    return false;
}

//Enable On\OFF all taxi paths
bool ChatHandler::HandleTaxiCheatCommand(const char* args)
{
    if (!*args)
    {
        SendSysMessage(LANG_USE_BOL);
        SetSentErrorMessage(true);
        return false;
    }

    std::string argstr = (char*)args;

    Player *chr = getSelectedPlayer();
    if (!chr)
    {
        chr=m_session->GetPlayer();
    }

    if (argstr == "on")
    {
        chr->SetTaxiCheater(true);
        PSendSysMessage(LANG_YOU_GIVE_TAXIS, chr->GetName());

        if(chr != m_session->GetPlayer())
            // to send localized data to target
            ChatHandler(chr).PSendSysMessage(ChatHandler(chr).GetMangosString(LANG_YOURS_TAXIS_ADDED), m_session->GetPlayer()->GetName());
        return true;
    }

    if (argstr == "off")
    {
        chr->SetTaxiCheater(false);
        PSendSysMessage(LANG_YOU_REMOVE_TAXIS, chr->GetName());

        if(chr != m_session->GetPlayer())
            ChatHandler(chr).PSendSysMessage(ChatHandler(chr).GetMangosString(LANG_YOURS_TAXIS_REMOVED), m_session->GetPlayer()->GetName());

        return true;
    }

    SendSysMessage(LANG_USE_BOL);
    SetSentErrorMessage(true);
    return false;
}

//Edit Player Aspeed
bool ChatHandler::HandleModifyASpeedCommand(const char* args)
{
    if (!*args)
        return false;

    float ASpeed = (float)atof((char*)args);

    if (ASpeed > 10 || ASpeed < 0.1)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if(chr->isInFlight())
    {
        PSendSysMessage(LANG_CHAR_IN_FLIGHT,chr->GetName());
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_CHANGE_ASPEED, ASpeed, chr->GetName());

    if(chr != m_session->GetPlayer())
        ChatHandler(chr).PSendSysMessage(ChatHandler(chr).GetMangosString(LANG_YOURS_ASPEED_CHANGED), m_session->GetPlayer()->GetName(), ASpeed);

    chr->SetSpeed(MOVE_WALK,    ASpeed,true);
    chr->SetSpeed(MOVE_RUN,     ASpeed,true);
    chr->SetSpeed(MOVE_SWIM,    ASpeed,true);
    //chr->SetSpeed(MOVE_TURN,    ASpeed,true);
    chr->SetSpeed(MOVE_FLY,     ASpeed,true);
    return true;
}

//Edit Player Speed
bool ChatHandler::HandleModifySpeedCommand(const char* args)
{
    if (!*args)
        return false;

    float Speed = (float)atof((char*)args);

    if (Speed > 10 || Speed < 0.1)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if(chr->isInFlight())
    {
        PSendSysMessage(LANG_CHAR_IN_FLIGHT,chr->GetName());
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_CHANGE_SPEED, Speed, chr->GetName());

    if(chr != m_session->GetPlayer())
        ChatHandler(chr).PSendSysMessage(ChatHandler(chr).GetMangosString(LANG_YOURS_SPEED_CHANGED), m_session->GetPlayer()->GetName(), Speed);

    chr->SetSpeed(MOVE_RUN,Speed,true);

    return true;
}

//Edit Player Swim Speed
bool ChatHandler::HandleModifySwimCommand(const char* args)
{
    if (!*args)
        return false;

    float Swim = (float)atof((char*)args);

    if (Swim > 10.0f || Swim < 0.01f)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if(chr->isInFlight())
    {
        PSendSysMessage(LANG_CHAR_IN_FLIGHT,chr->GetName());
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_CHANGE_SWIM_SPEED, Swim, chr->GetName());

    if(chr != m_session->GetPlayer())
        ChatHandler(chr).PSendSysMessage(ChatHandler(chr).GetMangosString(LANG_YOURS_SWIM_SPEED_CHANGED), m_session->GetPlayer()->GetName(), Swim);

    chr->SetSpeed(MOVE_SWIM,Swim,true);

    return true;
}

//Edit Player Walk Speed
bool ChatHandler::HandleModifyBWalkCommand(const char* args)
{
    if (!*args)
        return false;

    float BSpeed = (float)atof((char*)args);

    if (BSpeed > 10.0f || BSpeed < 0.1f)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if(chr->isInFlight())
    {
        PSendSysMessage(LANG_CHAR_IN_FLIGHT,chr->GetName());
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_CHANGE_BACK_SPEED, BSpeed, chr->GetName());

    if(chr != m_session->GetPlayer())
        ChatHandler(chr).PSendSysMessage(ChatHandler(chr).GetMangosString(LANG_YOURS_BACK_SPEED_CHANGED), m_session->GetPlayer()->GetName(), BSpeed);

    chr->SetSpeed(MOVE_WALKBACK,BSpeed,true);

    return true;
}

//Edit Player Fly
bool ChatHandler::HandleModifyFlyCommand(const char* args)
{
    if (!*args)
        return false;

    float FSpeed = (float)atof((char*)args);

    if (FSpeed > 10.0f || FSpeed < 0.1f)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_CHANGE_FLY_SPEED, FSpeed, chr->GetName());

    if(chr != m_session->GetPlayer())
        ChatHandler(chr).PSendSysMessage(ChatHandler(chr).GetMangosString(LANG_YOURS_FLY_SPEED_CHANGED), m_session->GetPlayer()->GetName(), FSpeed);

    chr->SetSpeed(MOVE_FLY,FSpeed,true);

    return true;
}

//Edit Player Scale
bool ChatHandler::HandleModifyScaleCommand(const char* args)
{
    if (!*args)
        return false;

    float Scale = (float)atof((char*)args);
    if (Scale > 3.0f || Scale <= 0.0f)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_CHANGE_SIZE, Scale, chr->GetName());

    if(chr != m_session->GetPlayer())
        ChatHandler(chr).PSendSysMessage(ChatHandler(chr).GetMangosString(LANG_YOURS_SIZE_CHANGED), m_session->GetPlayer()->GetName(), Scale);

    chr->SetFloatValue(OBJECT_FIELD_SCALE_X, Scale);

    return true;
}

//Enable Player mount
bool ChatHandler::HandleModifyMountCommand(const char* args)
{
    if(!*args)
        return false;

    uint16 mId = 1147;
    float speed = (float)15;
    uint32 num = 0;

    num = atoi((char*)args);
    switch(num)
    {
        case 1:
            mId=14340;
            break;
        case 2:
            mId=4806;
            break;
        case 3:
            mId=6471;
            break;
        case 4:
            mId=12345;
            break;
        case 5:
            mId=6472;
            break;
        case 6:
            mId=6473;
            break;
        case 7:
            mId=10670;
            break;
        case 8:
            mId=10719;
            break;
        case 9:
            mId=10671;
            break;
        case 10:
            mId=10672;
            break;
        case 11:
            mId=10720;
            break;
        case 12:
            mId=14349;
            break;
        case 13:
            mId=11641;
            break;
        case 14:
            mId=12244;
            break;
        case 15:
            mId=12242;
            break;
        case 16:
            mId=14578;
            break;
        case 17:
            mId=14579;
            break;
        case 18:
            mId=14349;
            break;
        case 19:
            mId=12245;
            break;
        case 20:
            mId=14335;
            break;
        case 21:
            mId=207;
            break;
        case 22:
            mId=2328;
            break;
        case 23:
            mId=2327;
            break;
        case 24:
            mId=2326;
            break;
        case 25:
            mId=14573;
            break;
        case 26:
            mId=14574;
            break;
        case 27:
            mId=14575;
            break;
        case 28:
            mId=604;
            break;
        case 29:
            mId=1166;
            break;
        case 30:
            mId=2402;
            break;
        case 31:
            mId=2410;
            break;
        case 32:
            mId=2409;
            break;
        case 33:
            mId=2408;
            break;
        case 34:
            mId=2405;
            break;
        case 35:
            mId=14337;
            break;
        case 36:
            mId=6569;
            break;
        case 37:
            mId=10661;
            break;
        case 38:
            mId=10666;
            break;
        case 39:
            mId=9473;
            break;
        case 40:
            mId=9476;
            break;
        case 41:
            mId=9474;
            break;
        case 42:
            mId=14374;
            break;
        case 43:
            mId=14376;
            break;
        case 44:
            mId=14377;
            break;
        case 45:
            mId=2404;
            break;
        case 46:
            mId=2784;
            break;
        case 47:
            mId=2787;
            break;
        case 48:
            mId=2785;
            break;
        case 49:
            mId=2736;
            break;
        case 50:
            mId=2786;
            break;
        case 51:
            mId=14347;
            break;
        case 52:
            mId=14346;
            break;
        case 53:
            mId=14576;
            break;
        case 54:
            mId=9695;
            break;
        case 55:
            mId=9991;
            break;
        case 56:
            mId=6448;
            break;
        case 57:
            mId=6444;
            break;
        case 58:
            mId=6080;
            break;
        case 59:
            mId=6447;
            break;
        case 60:
            mId=4805;
            break;
        case 61:
            mId=9714;
            break;
        case 62:
            mId=6448;
            break;
        case 63:
            mId=6442;
            break;
        case 64:
            mId=14632;
            break;
        case 65:
            mId=14332;
            break;
        case 66:
            mId=14331;
            break;
        case 67:
            mId=8469;
            break;
        case 68:
            mId=2830;
            break;
        case 69:
            mId=2346;
            break;
        default:
            SendSysMessage(LANG_NO_MOUNT);
            SetSentErrorMessage(true);
            return false;
    }

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_YOU_GIVE_MOUNT, chr->GetName());

    if(chr != m_session->GetPlayer())
        ChatHandler(chr).PSendSysMessage(ChatHandler(chr).GetMangosString(LANG_MOUNT_GIVED), m_session->GetPlayer()->GetName());

    chr->SetUInt32Value( UNIT_FIELD_FLAGS , 0x001000 );
    chr->Mount(mId);

    WorldPacket data( SMSG_FORCE_RUN_SPEED_CHANGE, (8+4+1+4) );
    data.append(chr->GetPackGUID());
    data << (uint32)0;
    data << (uint8)0;                                       //new 2.1.0
    data << float(speed);
    chr->SendMessageToSet( &data, true );

    data.Initialize( SMSG_FORCE_SWIM_SPEED_CHANGE, (8+4+4) );
    data.append(chr->GetPackGUID());
    data << (uint32)0;
    data << float(speed);
    chr->SendMessageToSet( &data, true );

    return true;
}

//Edit Player money
bool ChatHandler::HandleModifyMoneyCommand(const char* args)
{
    if (!*args)
        return false;

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    int32 addmoney = atoi((char*)args);

    uint32 moneyuser = chr->GetMoney();

    if(addmoney < 0)
    {
        int32 newmoney = moneyuser + addmoney;

        sLog.outDetail(GetMangosString(LANG_CURRENT_MONEY), moneyuser, addmoney, newmoney);
        if(newmoney <= 0 )
        {
            PSendSysMessage(LANG_YOU_TAKE_ALL_MONEY, chr->GetName());

            if(chr != m_session->GetPlayer())
                ChatHandler(chr).PSendSysMessage(ChatHandler(chr).GetMangosString(LANG_YOURS_ALL_MONEY_GONE), m_session->GetPlayer()->GetName());

            chr->SetMoney(0);
        }
        else
        {
            PSendSysMessage(LANG_YOU_TAKE_MONEY, abs(addmoney), chr->GetName());
            if(chr != m_session->GetPlayer())
                ChatHandler(chr).PSendSysMessage(ChatHandler(chr).GetMangosString(LANG_YOURS_MONEY_TAKEN), m_session->GetPlayer()->GetName(), abs(addmoney));
            chr->SetMoney( newmoney );
        }
    }
    else
    {
        PSendSysMessage(LANG_YOU_GIVE_MONEY, addmoney, chr->GetName());
        if(chr != m_session->GetPlayer())
            ChatHandler(chr).PSendSysMessage(ChatHandler(chr).GetMangosString(LANG_YOURS_MONEY_GIVEN), m_session->GetPlayer()->GetName(), addmoney);
        chr->ModifyMoney( addmoney );
    }

    sLog.outDetail(GetMangosString(LANG_NEW_MONEY), moneyuser, addmoney, chr->GetMoney() );

    return true;
}

//Edit Player field
bool ChatHandler::HandleModifyBitCommand(const char* args)
{
    if( !*args )
        return false;

    Player *chr = getSelectedPlayer();
    if (chr == NULL)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    char* pField = strtok((char*)args, " ");
    if (!pField)
        return false;

    char* pBit = strtok(NULL, " ");
    if (!pBit)
        return false;

    uint16 field = atoi(pField);
    uint32 bit   = atoi(pBit);

    if (field < 1 || field >= PLAYER_END)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    if (bit < 1 || bit > 32)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    if ( chr->HasFlag( field, (1<<(bit-1)) ) )
    {
        chr->RemoveFlag( field, (1<<(bit-1)) );
        PSendSysMessage(LANG_REMOVE_BIT, bit, field);
    }
    else
    {
        chr->SetFlag( field, (1<<(bit-1)) );
        PSendSysMessage(LANG_SET_BIT, bit, field);
    }

    return true;
}

//Teleport by game_tele entry
bool ChatHandler::HandleModifyHonorCommand (const char* args)
{
    if (!*args)
        return false;

    Player *target = getSelectedPlayer();
    if(!target)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    int32 amount = (uint32)atoi(args);

    target->ModifyHonorPoints(amount);

    PSendSysMessage(LANG_COMMAND_MODIFY_HONOR, target->GetName(), target->GetHonorPoints());

    return true;
}

bool ChatHandler::HandleTeleCommand(const char * args)
{
    if(!*args)
        return false;

    Player* _player = m_session->GetPlayer();

    char* cId = extractKeyFromLink((char*)args,"Htele");    // string or [name] Shift-click form |color|Htele:name|h[name]|h|r
    if(!cId)
        return false;

    std::string name = cId;
    WorldDatabase.escape_string(name);

    QueryResult *result = WorldDatabase.PQuery("SELECT position_x,position_y,position_z,orientation,map FROM game_tele WHERE name = '%s'",name.c_str());
    if (!result)
    {
        SendSysMessage(LANG_COMMAND_TELE_NOTFOUND);
        SetSentErrorMessage(true);
        return false;
    }
    Field *fields = result->Fetch();
    float x = fields[0].GetFloat();
    float y = fields[1].GetFloat();
    float z = fields[2].GetFloat();
    float ort = fields[3].GetFloat();
    int mapid = fields[4].GetUInt16();
    delete result;

    if(!MapManager::IsValidMapCoord(mapid,x,y,x,ort))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD,x,y,mapid);
        SetSentErrorMessage(true);
        return false;
    }

    // stop flight if need
    if(_player->isInFlight())
    {
        _player->GetMotionMaster()->MovementExpired();
        _player->m_taxi.ClearTaxiDestinations();
    }
    // save only in non-flight case
    else
        _player->SaveRecallPosition();

    _player->TeleportTo(mapid, x, y, z, ort);
    return true;
}

bool ChatHandler::HandleLookupAreaCommand(const char* args)
{
    if(!*args)
        return false;

    std::string namepart = args;
    std::wstring wnamepart;

    if(!Utf8toWStr(namepart,wnamepart))
        return false;

    uint32 counter = 0;                                     // Counter for figure out that we found smth.

    // converting string that we try to find to lower case
    wstrToLower( wnamepart );

    // Search in AreaTable.dbc
    for (uint32 areaflag = 0; areaflag < sAreaStore.GetNumRows(); ++areaflag)
    {
        AreaTableEntry const *areaEntry = sAreaStore.LookupEntry(areaflag);
        if(areaEntry)
        {
            int loc = m_session->GetSessionDbcLocale();
            std::string name = areaEntry->area_name[loc];
            if(name.empty())
                continue;

            if(!Utf8FitTo(name, wnamepart))
            {
                loc = 0;
                for(; loc < MAX_LOCALE; ++loc)
                {
                    if(loc==m_session->GetSessionDbcLocale())
                        continue;

                    name = areaEntry->area_name[loc];
                    if(name.empty())
                        continue;

                    if (Utf8FitTo(name, wnamepart))
                        break;
                }
            }

            if(loc < MAX_LOCALE)
            {
                // send area in "id - [name]" format
                std::ostringstream ss;
                ss << areaEntry->ID << " - |cffffffff|Harea:" << areaEntry->ID << "|h[" << name << " " << localeNames[loc]<< "]|h|r";

                SendSysMessage(ss.str().c_str());

                ++counter;
            }
        }
    }
    if (counter == 0)                                       // if counter == 0 then we found nth
        SendSysMessage(LANG_COMMAND_NOAREAFOUND);
    return true;
}

//Find tele in game_tele order by name
bool ChatHandler::HandleLookupTeleCommand(const char * args)
{
    if(!*args)
    {
        SendSysMessage(LANG_COMMAND_TELE_PARAMETER);
        SetSentErrorMessage(true);
        return false;
    }
    char const* str = strtok((char*)args, " ");
    if(!str)
        return false;

    std::string namepart = str;
    WorldDatabase.escape_string(namepart);
    QueryResult *result = WorldDatabase.PQuery("SELECT name FROM game_tele WHERE name "_LIKE_" '""%%%s%%""'",namepart.c_str());
    if (!result)
    {
        SendSysMessage(LANG_COMMAND_TELE_NOREQUEST);
        SetSentErrorMessage(true);
        return false;
    }
    std::string reply;
    for (uint64 i=0; i < result->GetRowCount(); i++)
    {
        Field *fields = result->Fetch();
        reply += "  |cffffffff|Htele:";
        reply += fields[0].GetCppString();
        reply += "|h[";
        reply += fields[0].GetCppString();
        reply += "]|h|r\n";
        result->NextRow();
    }
    delete result;

    if(reply.empty())
        SendSysMessage(LANG_COMMAND_TELE_NOLOCATION);
    else
    {
        reply = GetMangosString(LANG_COMMAND_TELE_LOCATION) + reply;
        SendSysMessage(reply.c_str());
    }
    return true;
}

//Enable\Dissable accept whispers (for GM)
bool ChatHandler::HandleWhispersCommand(const char* args)
{
    if(!*args)
    {
        PSendSysMessage(LANG_COMMAND_WHISPERACCEPTING, m_session->GetPlayer()->isAcceptWhispers() ?  GetMangosString(LANG_ON) : GetMangosString(LANG_OFF));
        return true;
    }

    std::string argstr = (char*)args;
    // whisper on
    if (argstr == "on")
    {
        m_session->GetPlayer()->SetAcceptWhispers(true);
        SendSysMessage(LANG_COMMAND_WHISPERON);
        return true;
    }

    // whisper off
    if (argstr == "off")
    {
        m_session->GetPlayer()->SetAcceptWhispers(false);
        SendSysMessage(LANG_COMMAND_WHISPEROFF);
        return true;
    }

    SendSysMessage(LANG_USE_BOL);
    SetSentErrorMessage(true);
    return false;
}

//Play sound
bool ChatHandler::HandlePlaySoundCommand(const char* args)
{
    // USAGE: .debug playsound #soundid
    // #soundid - ID decimal number from SoundEntries.dbc (1st column)
    // this file have about 5000 sounds.
    // In this realization only caller can hear this sound.
    if( *args )
    {
        uint32 dwSoundId = atoi((char*)args);

        if( !sSoundEntriesStore.LookupEntry(dwSoundId) )
        {
            PSendSysMessage(LANG_SOUND_NOT_EXIST, dwSoundId);
            SetSentErrorMessage(true);
            return false;
        }

        WorldPacket data(SMSG_PLAY_OBJECT_SOUND,4+8);
        data << uint32(dwSoundId) << m_session->GetPlayer()->GetGUID();
        m_session->SendPacket(&data);

        PSendSysMessage(LANG_YOU_HEAR_SOUND, dwSoundId);
        return true;
    }

    return false;
}

//Save all players in the world
bool ChatHandler::HandleSaveAllCommand(const char* /*args*/)
{
    ObjectAccessor::Instance().SaveAllPlayers();
    SendSysMessage(LANG_PLAYERS_SAVED);
    return true;
}

//Send mail by command
bool ChatHandler::HandleSendMailCommand(const char* args)
{
    if(!*args)
        return false;

    char* pName = strtok((char*)args, " ");
    char* msgSubject = strtok(NULL, " ");
    char* msgText = strtok(NULL, "");

    if (!msgText)
        return false;

    // pName, msgSubject, msgText isn't NUL after prev. check

    std::string name    = pName;
    std::string subject = msgSubject;
    std::string text    = msgText;

    if(!normalizePlayerName(name))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    uint64 receiver_guid = objmgr.GetPlayerGUIDByName(name);

    if(!receiver_guid)
        return false;

    uint32 mailId = objmgr.GenerateMailID();
    uint32 sender_guidlo = m_session->GetPlayer()->GetGUIDLow();
    uint32 messagetype = MAIL_NORMAL;
    uint32 stationery = MAIL_STATIONERY_GM;
    uint32 itemTextId = 0;
    if (!text.empty())
    {
        itemTextId = objmgr.CreateItemText( text );
    }

    Player *receiver = objmgr.GetPlayer(receiver_guid);

    WorldSession::SendMailTo(receiver,messagetype, stationery, sender_guidlo, GUID_LOPART(receiver_guid), subject, itemTextId, NULL, 0, 0, MAIL_CHECK_MASK_NONE);

    PSendSysMessage(LANG_MAIL_SENT, name.c_str());
    return true;
}

// teleport player to given game_tele.entry
bool ChatHandler::HandleNameTeleCommand(const char * args)
{
    if(!*args)
        return false;

    char* pName = strtok((char*)args, " ");

    if(!pName)
        return false;

    char* tail = strtok(NULL, "");
    if(!tail)
        return false;

    char* cId = extractKeyFromLink((char*)tail,"Htele");    // string or [name] Shift-click form |color|Htele:name|h[name]|h|r
    if(!cId)
        return false;

    std::string location = cId;

    std::string name = pName;

    if(!normalizePlayerName(name))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    WorldDatabase.escape_string(location);
    QueryResult *result = WorldDatabase.PQuery("SELECT position_x,position_y,position_z,orientation,map FROM game_tele WHERE name = '%s'",location.c_str());
    if (!result)
    {
        SendSysMessage(LANG_COMMAND_TELE_NOTFOUND);
        SetSentErrorMessage(true);
        return false;
    }

    Field *fields = result->Fetch();
    float x = fields[0].GetFloat();
    float y = fields[1].GetFloat();
    float z = fields[2].GetFloat();
    float ort = fields[3].GetFloat();
    int mapid = fields[4].GetUInt16();
    delete result;

    if(!MapManager::IsValidMapCoord(mapid,x,y,x,ort))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD,x,y,mapid);
        SetSentErrorMessage(true);
        return false;
    }

    Player *chr = objmgr.GetPlayer(name.c_str());
    if (chr)
    {

        if(chr->IsBeingTeleported()==true)
        {
            PSendSysMessage(LANG_IS_TELEPORTED, chr->GetName());
            SetSentErrorMessage(true);
            return false;
        }

        PSendSysMessage(LANG_TELEPORTING_TO, chr->GetName(),"", location.c_str());

        if (m_session->GetPlayer()->IsVisibleGloballyFor(chr))
            ChatHandler(chr).PSendSysMessage(LANG_TELEPORTED_TO_BY, m_session->GetPlayer()->GetName());

        // stop flight if need
        if(chr->isInFlight())
        {
            chr->GetMotionMaster()->MovementExpired();
            chr->m_taxi.ClearTaxiDestinations();
        }
        // save only in non-flight case
        else
            chr->SaveRecallPosition();

        chr->TeleportTo(mapid,x,y,z,chr->GetOrientation());
    }
    else if (uint64 guid = objmgr.GetPlayerGUIDByName(name.c_str()))
    {
        PSendSysMessage(LANG_TELEPORTING_TO, name.c_str(), GetMangosString(LANG_OFFLINE), location.c_str());
        Player::SavePositionInDB(mapid,x,y,z,ort,MapManager::Instance().GetZoneId(mapid,x,y),guid);
    }
    else
        PSendSysMessage(LANG_NO_PLAYER, name.c_str());

    return true;
}

//Teleport group to given game_tele.entry
bool ChatHandler::HandleGroupTeleCommand(const char * args)
{
    if(!*args)
        return false;

    Player *player = getSelectedPlayer();
    if (!player)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    char* cId = extractKeyFromLink((char*)args,"Htele");    // string or [name] Shift-click form |color|Htele:name|h[name]|h|r
    if(!cId)
        return false;

    std::string location = cId;

    WorldDatabase.escape_string(location);
    QueryResult *result = WorldDatabase.PQuery("SELECT position_x,position_y,position_z,orientation,map FROM game_tele WHERE name = '%s'",location.c_str());
    if (!result)
    {
        SendSysMessage(LANG_COMMAND_TELE_NOTFOUND);
        SetSentErrorMessage(true);
        return false;
    }
    Field *fields = result->Fetch();
    float x = fields[0].GetFloat();
    float y = fields[1].GetFloat();
    float z = fields[2].GetFloat();
    float ort = fields[3].GetFloat();
    int mapid = fields[4].GetUInt16();
    delete result;

    if(!MapManager::IsValidMapCoord(mapid,x,y,z,ort))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD,x,y,mapid);
        SetSentErrorMessage(true);
        return false;
    }

    Group *grp = player->GetGroup();

    if(!grp)
    {
        PSendSysMessage(LANG_NOT_IN_GROUP,player->GetName());
        SetSentErrorMessage(true);
        return false;
    }

    for(GroupReference *itr = grp->GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player *pl = itr->getSource();

        if(!pl || !pl->GetSession() )
            continue;

        if(pl->IsBeingTeleported())
        {
            PSendSysMessage(LANG_IS_TELEPORTED, pl->GetName());
            continue;
        }

        PSendSysMessage(LANG_TELEPORTING_TO, pl->GetName(),"", location.c_str());

        if (m_session->GetPlayer() != pl && m_session->GetPlayer()->IsVisibleGloballyFor(pl))
            ChatHandler(pl).PSendSysMessage(LANG_TELEPORTED_TO_BY, m_session->GetPlayer()->GetName());

        // stop flight if need
        if(pl->isInFlight())
        {
            pl->GetMotionMaster()->MovementExpired();
            pl->m_taxi.ClearTaxiDestinations();
        }
        // save only in non-flight case
        else
            pl->SaveRecallPosition();

        pl->TeleportTo(mapid, x, y, z, ort);
    }

    return true;
}

//Summon group of player
bool ChatHandler::HandleGroupgoCommand(const char* args)
{
    if(!*args)
        return false;

    std::string name = args;

    if(!normalizePlayerName(name))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    Player *player = objmgr.GetPlayer(name.c_str());
    if (!player)
    {
        PSendSysMessage(LANG_NO_PLAYER, args);
        SetSentErrorMessage(true);
        return false;
    }

    Group *grp = player->GetGroup();

    if(!grp)
    {
        PSendSysMessage(LANG_NOT_IN_GROUP,player->GetName());
        SetSentErrorMessage(true);
        return false;
    }

    Map* gmMap = MapManager::Instance().GetMap(m_session->GetPlayer()->GetMapId(),m_session->GetPlayer());
    bool to_instance =  gmMap->Instanceable();

    // we are in instance, and can summon only player in our group with us as lead
    if ( to_instance && (
        !m_session->GetPlayer()->GetGroup() || (grp->GetLeaderGUID() != m_session->GetPlayer()->GetGUID()) ||
        (m_session->GetPlayer()->GetGroup()->GetLeaderGUID() != m_session->GetPlayer()->GetGUID()) ) )
        // the last check is a bit excessive, but let it be, just in case
    {
        SendSysMessage(LANG_CANNOT_SUMMON_TO_INST);
        SetSentErrorMessage(true);
        return false;
    }

    for(GroupReference *itr = grp->GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player *pl = itr->getSource();

        if(!pl || pl==m_session->GetPlayer() || !pl->GetSession() )
            continue;

        if(pl->IsBeingTeleported()==true)
        {
            PSendSysMessage(LANG_IS_TELEPORTED, pl->GetName());
            SetSentErrorMessage(true);
            return false;
        }

        if (to_instance)
        {
            Map* plMap = MapManager::Instance().GetMap(pl->GetMapId(),pl);

            if ( plMap->Instanceable() && plMap->GetInstanceId() != gmMap->GetInstanceId() )
            {
                // cannot summon from instance to instance
                PSendSysMessage(LANG_CANNOT_SUMMON_TO_INST,pl->GetName());
                SetSentErrorMessage(true);
                return false;
            }
        }

        PSendSysMessage(LANG_SUMMONING, pl->GetName(),"");

        if (m_session->GetPlayer()->IsVisibleGloballyFor(pl))
            ChatHandler(pl).PSendSysMessage(LANG_SUMMONED_BY, m_session->GetPlayer()->GetName());

        // stop flight if need
        if(pl->isInFlight())
        {
            pl->GetMotionMaster()->MovementExpired();
            pl->m_taxi.ClearTaxiDestinations();
        }
        // save only in non-flight case
        else
            pl->SaveRecallPosition();

        // before GM
        float x,y,z;
        m_session->GetPlayer()->GetClosePoint(x,y,z,pl->GetObjectSize());
        pl->TeleportTo(m_session->GetPlayer()->GetMapId(),x,y,z,pl->GetOrientation());
    }

    return true;
}

//teleport at coordinates
bool ChatHandler::HandleGoXYCommand(const char* args)
{
    if(!*args)
        return false;

    Player* _player = m_session->GetPlayer();

    char* px = strtok((char*)args, " ");
    char* py = strtok(NULL, " ");
    char* pmapid = strtok(NULL, " ");

    if (!px || !py)
        return false;

    float x = (float)atof(px);
    float y = (float)atof(py);
    uint32 mapid;
    if (pmapid)
        mapid = (uint32)atoi(pmapid);
    else mapid = _player->GetMapId();

    if(!MapManager::IsValidMapCoord(mapid,x,y))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD,x,y,mapid);
        SetSentErrorMessage(true);
        return false;
    }

    // stop flight if need
    if(_player->isInFlight())
    {
        _player->GetMotionMaster()->MovementExpired();
        _player->m_taxi.ClearTaxiDestinations();
    }
    // save only in non-flight case
    else
        _player->SaveRecallPosition();

    Map const *map = MapManager::Instance().GetBaseMap(mapid);
    float z = std::max(map->GetHeight(x, y, MAX_HEIGHT), map->GetWaterLevel(x, y));

    _player->TeleportTo(mapid, x, y, z, _player->GetOrientation());

    return true;
}

//teleport at coordinates, including Z
bool ChatHandler::HandleGoXYZCommand(const char* args)
{
    if(!*args)
        return false;

    Player* _player = m_session->GetPlayer();

    char* px = strtok((char*)args, " ");
    char* py = strtok(NULL, " ");
    char* pz = strtok(NULL, " ");
    char* pmapid = strtok(NULL, " ");

    if (!px || !py || !pz)
        return false;

    float x = (float)atof(px);
    float y = (float)atof(py);
    float z = (float)atof(pz);
    uint32 mapid;
    if (pmapid)
        mapid = (uint32)atoi(pmapid);
    else
        mapid = _player->GetMapId();

    if(!MapManager::IsValidMapCoord(mapid,x,y,z))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD,x,y,mapid);
        SetSentErrorMessage(true);
        return false;
    }

    // stop flight if need
    if(_player->isInFlight())
    {
        _player->GetMotionMaster()->MovementExpired();
        _player->m_taxi.ClearTaxiDestinations();
    }
    // save only in non-flight case
    else
        _player->SaveRecallPosition();

    _player->TeleportTo(mapid, x, y, z, _player->GetOrientation());

    return true;
}

//teleport at coordinates
bool ChatHandler::HandleGoZoneXYCommand(const char* args)
{
    if(!*args)
        return false;

    Player* _player = m_session->GetPlayer();

    char* px = strtok((char*)args, " ");
    char* py = strtok(NULL, " ");
    char* tail = strtok(NULL,"");

    char* cAreaId = extractKeyFromLink(tail,"Harea");       // string or [name] Shift-click form |color|Harea:area_id|h[name]|h|r

    if (!px || !py)
        return false;

    float x = (float)atof(px);
    float y = (float)atof(py);
    uint32 areaid = cAreaId ? (uint32)atoi(cAreaId) : _player->GetZoneId();

    AreaTableEntry const* areaEntry = GetAreaEntryByAreaID(areaid);

    if( x<0 || x>100 || y<0 || y>100 || !areaEntry )
    {
        PSendSysMessage(LANG_INVALID_ZONE_COORD,x,y,areaid);
        SetSentErrorMessage(true);
        return false;
    }

    // update to parent zone if exist (client map show only zones without parents)
    AreaTableEntry const* zoneEntry = areaEntry->zone ? GetAreaEntryByAreaID(areaEntry->zone) : areaEntry;

    Map const *map = MapManager::Instance().GetBaseMap(zoneEntry->mapid);

    if(map->Instanceable())
    {
        PSendSysMessage(LANG_INVALID_ZONE_MAP,areaEntry->ID,areaEntry->area_name[m_session->GetSessionDbcLocale()],map->GetId(),map->GetMapName());
        SetSentErrorMessage(true);
        return false;
    }

    Zone2MapCoordinates(x,y,zoneEntry->ID);

    if(!MapManager::IsValidMapCoord(zoneEntry->mapid,x,y))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD,x,y,zoneEntry->mapid);
        SetSentErrorMessage(true);
        return false;
    }

    // stop flight if need
    if(_player->isInFlight())
    {
        _player->GetMotionMaster()->MovementExpired();
        _player->m_taxi.ClearTaxiDestinations();
    }
    // save only in non-flight case
    else
        _player->SaveRecallPosition();

    float z = std::max(map->GetHeight(x, y, MAX_HEIGHT), map->GetWaterLevel(x, y));
    _player->TeleportTo(zoneEntry->mapid, x, y, z, _player->GetOrientation());

    return true;
}

//teleport to grid
bool ChatHandler::HandleGoGridCommand(const char* args)
{
    if(!*args)    return false;
    Player* _player = m_session->GetPlayer();

    char* px = strtok((char*)args, " ");
    char* py = strtok(NULL, " ");
    char* pmapid = strtok(NULL, " ");

    if (!px || !py)
        return false;

    float grid_x = (float)atof(px);
    float grid_y = (float)atof(py);
    uint32 mapid;
    if (pmapid)
        mapid = (uint32)atoi(pmapid);
    else mapid = _player->GetMapId();

    // center of grid
    float x = (grid_x-CENTER_GRID_ID+0.5f)*SIZE_OF_GRIDS;
    float y = (grid_y-CENTER_GRID_ID+0.5f)*SIZE_OF_GRIDS;

    if(!MapManager::IsValidMapCoord(mapid,x,y))
    {
        PSendSysMessage(LANG_INVALID_TARGET_COORD,x,y,mapid);
        SetSentErrorMessage(true);
        return false;
    }

    // stop flight if need
    if(_player->isInFlight())
    {
        _player->GetMotionMaster()->MovementExpired();
        _player->m_taxi.ClearTaxiDestinations();
    }
    // save only in non-flight case
    else
        _player->SaveRecallPosition();

    Map const *map = MapManager::Instance().GetBaseMap(mapid);
    float z = std::max(map->GetHeight(x, y, MAX_HEIGHT), map->GetWaterLevel(x, y));
    _player->TeleportTo(mapid, x, y, z, _player->GetOrientation());

    return true;
}

bool ChatHandler::HandleDrunkCommand(const char* args)
{
    if(!*args)    return false;

    uint32 drunklevel = (uint32)atoi(args);
    if(drunklevel > 100)
        drunklevel = 100;

    uint16 drunkMod = drunklevel * 0xFFFF / 100;

    m_session->GetPlayer()->SetDrunkValue(drunkMod);

    return true;
}
