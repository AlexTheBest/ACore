
class votestonenpc : public CreatureScript
{
public:
    votestonenpc() : CreatureScript("votestonenpc") { }

	bool OnGossipSelect(Player *player, Creature *_Creature, uint32 sender, uint32 action)
	{
		// Main menu
		if (sender == GOSSIP_SENDER_MAIN)
		SendDefaultMenu_vote_stone_npc(player, _Creature, action);
		return true;
	}

	bool OnGossipHello(Player *player, Creature *_Creature)
	{
		// Make sure player is not in combat
		if(!player->getAttackers().empty()){
			player->CLOSE_GOSSIP_MENU();
			_Creature->MonsterWhisper("You are in combat!", player->GetGUID());
			return false;
		}
		// Main Menu 
		//Go straight to the main menu, hacking approach but works
		player->ADD_GOSSIP_ITEM( 5, "Main"                    , GOSSIP_SENDER_MAIN, 6000);
		player->PlayerTalkClass->SendGossipMenu(6000,_Creature->GetGUID());
	}
	
	void creategossiplist_2(Player *player, Creature *_Creature, uint32 permission, std::string parent)
	{
		player->PlayerTalkClass->ClearMenus();
		QueryResult_AutoPtr result = WorldDatabase.PQuery("SELECT id,name,folder, faction FROM telelist WHERE parent='%s' && permissionflag<='%u'", parent.c_str(), permission);
			if(result) //check if we got a result
			{
					Field *fields = result->Fetch();  
				do{
					if(player->GetTeam() == fields[3].GetUInt32() || fields[3].GetUInt32() == 0)
					{
						if(fields[2].GetUInt32() == 1)
						{
							std::stringstream folder;
							folder << fields[1].GetCppString() << " >>";
							player->ADD_GOSSIP_ITEM( 5, folder.str().c_str() , GOSSIP_SENDER_MAIN, fields[0].GetInt32());
						}else{
							std::stringstream teleport;
							teleport << fields[1].GetCppString();
							player->ADD_GOSSIP_ITEM( 3, teleport.str().c_str() , GOSSIP_SENDER_MAIN, fields[0].GetInt32());					
						}
					}
				}while(result->NextRow());
			}else{ // if we don't then we will display a message
				_Creature->MonsterWhisper("Nothing to display", player->GetGUID());
			}
			
		QueryResult_AutoPtr BackResult = WorldDatabase.PQuery("SELECT id FROM telelist WHERE name IN (SELECT parent FROM telelist WHERE name='%s')", parent.c_str());
		if(BackResult)
		{
			Field *BackFields = BackResult->Fetch();
			player->ADD_GOSSIP_ITEM( 5, "<- Back", GOSSIP_SENDER_MAIN, BackFields[0].GetUInt32());
		}
		player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,_Creature->GetGUID());
	}

	void SendDefaultMenu_vote_stone_npc(Player *player, Creature *_Creature, uint32 action)
	{
		uint32 type = 1;
		std::stringstream level_error;
		if(player->HasItemCount(100180,1,false))
			{

			QueryResult_AutoPtr result = WorldDatabase.PQuery("SELECT id,name,permissionflag,parent,folder,x,y,z,mapid,levelreq FROM telelist WHERE id = '%u' AND permissionflag<='%u'",action, type);
			if(result){
				Field* pFields = result->Fetch();
				
				int32 db_id 			= pFields[0].GetInt32(); // don't need
				int32 db_folder 		= pFields[4].GetInt32();
				int32 db_permission 		= pFields[2].GetInt32();			
				std::string db_name 	= pFields[1].GetCppString();

				if(db_folder)// we check if its a folder first
				{
					creategossiplist_2(player,_Creature,type, db_name.c_str());
				}else{
					int32 db_x 				= pFields[5].GetInt32(); 		
					int32 db_y 				= pFields[6].GetInt32(); 		
					int32 db_z 				= pFields[7].GetInt32();
					int32 db_mapid 			= pFields[8].GetInt32();
					int32 db_levelreq			= pFields[9].GetInt32();
					if(player->getLevel() < db_levelreq)
					{
				level_error << "Your level must be " << db_levelreq << " or higher.";
				_Creature->MonsterWhisper(level_error.str().c_str(), player->GetGUID());
					}else{
				//TELEPORT PLAYER
						player->TeleportTo(db_mapid, db_x, db_y, db_z, 4.919000f);			
					}
					player->CLOSE_GOSSIP_MENU();
				}
				
			}
		}else {
			_Creature->MonsterWhisper("You do not have the stone!", player->GetGUID());
		}
	}	

};

class item_votestone : public ItemScript
{
public:
    item_votestone() : ItemScript("item_votestone") { }
    bool OnUse(Player* player, Item* _Item, SpellCastTargets const& /*targets*/)
    {

		if ( (player->isInCombat()) || (player->isDead()) || (player->IsMounted()))
		{
			player->SendEquipError(EQUIP_ERR_NOT_IN_COMBAT,_Item,NULL );
			return false;
		}

		 if (player->HasSpellCooldown(40662) or /*battle grounds */player->GetMapId() ==  529 or player->GetMapId() == 489 or player->GetMapId() == 30 or player->GetMapId() == 566)
		{
			//	_Creature->MonsterWhisper("Please wait for the cooldown", player->GetGUID());
		} else {
			player->CastSpell(player, 40662, false);
			player->AddSpellCooldown(40662,0,time(NULL) + 120);

			player->SummonCreature(100080,player->GetPositionX() ,player->GetPositionY()+1, player->GetPositionZ()+1, 0,TEMPSUMMON_TIMED_DESPAWN,180000);
			return true;
		}
    }
};

class item_rewardstone : public ItemScript
{
public:
    item_rewardstone() : ItemScript("item_rewardstone") { }
    bool OnUse(Player* player, Item* _Item, SpellCastTargets const& /*targets*/)
    {

		if ( (player->isInCombat()) || (player->isDead()) || (player->IsMounted()))
		{
			player->SendEquipError(EQUIP_ERR_NOT_IN_COMBAT,_Item,NULL );
			return false;
		}

		 if (player->HasSpellCooldown(40662) or /*battle grounds */player->GetMapId() ==  529 or player->GetMapId() == 489 or player->GetMapId() == 30 or player->GetMapId() == 566)
		{
			//	_Creature->MonsterWhisper("Please wait for the cooldown", player->GetGUID());
		} else {
			player->CastSpell(player, 40662, false);
			player->AddSpellCooldown(40662,0,time(NULL) + 120);

			player->SummonCreature(100012,player->GetPositionX() ,player->GetPositionY()+1, player->GetPositionZ()+1, 0,TEMPSUMMON_TIMED_DESPAWN,180000);
			return true;
		}
    }
};

void AddSC_telestone()
{
    new votestonenpc();
	new item_votestone();
	new item_rewardstone();
}