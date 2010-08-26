//
//
//	
//


class televotescript : public CreatureScript
{
public:
    televotescript() : CreatureScript("televotescript") { }

	bool OnGossipSelect(Player *player, Creature *_Creature, uint32 sender, uint32 action)
	{
		// Main menu
		if (sender == GOSSIP_SENDER_MAIN)
		SendDefaultMenu_televotescript(player, _Creature, action);

		return true;
	}

	bool OnGossipHello(Player *player, Creature *_Creature)
	{
		//Go straight to the main menu
		player->ADD_GOSSIP_ITEM( 5, "Main"                    , GOSSIP_SENDER_MAIN, 6000);
		player->PlayerTalkClass->SendGossipMenu(6000,_Creature->GetGUID());
		return true;
	}
	void creategossiplist(Player *player, Creature *_Creature, uint32 permission, std::string parent)
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

	int checkvote(uint32 accountid,Player *player, Creature *_Creature)
	{
		uint32 type = 0;
		uint32 counter = 0;
		// uint32 nocomplete = 0; // if we find one vote out of date, will send limited menu
		//Check if vote exists in local db
		QueryResult_AutoPtr result = WorldDatabase.PQuery("SELECT (UNIX_TIMESTAMP() - time), account, site, (SELECT count(*) FROM vote_record WHERE account = '%u'), time FROM vote_record WHERE account = '%u'",accountid, accountid);
		if(result) //make sure we have a result
		{		  // past experience if we go any further with no result it will cause the server to crash
		Field *fields = result->Fetch();
			if(fields[3].GetUInt32() < 3)
				return 0; // if we don't have 3 votes in db then we will go no further

			do
			{
				uint32 votetime = fields[0].GetUInt32();
				if(votetime > (24 * 3600) /* 24 hours */)
				{
				//delete record if it still exists
					//we should make the website do this...
					// delete all vote records equal or less then this time since it has expired
					WorldDatabase.PExecute("DELETE FROM vote_record WHERE  time<='%u'", player->GetSession()->GetAccountId(), fields[2].GetUInt32(),fields[4].GetUInt32() );
					//nocomplete = 1;
				}else{
					counter++;
				}
			} while(result->NextRow());
		}else{
				return 0;
		}
		if(counter >= 3)
		{
			return 1;
		}else{
			return 0;
		}
	}
	
	void SendDefaultMenu_televotescript(Player *player, Creature *_Creature, uint32 action )
	{
		uint32 type = 0;
		std::stringstream level_error;
	//we first check if player has voted
		type = checkvote(player->GetSession()->GetAccountId(),player,_Creature);
	if(!type)
	_Creature->MonsterWhisper("You have limited access, to get full access please vote on the site.", player->GetGUID());
			QueryResult_AutoPtr result = WorldDatabase.PQuery("SELECT id,name,permissionflag,parent,folder,x,y,z,mapid,levelreq FROM telelist WHERE id = '%u' AND permissionflag<='%u'",action, type);
			if(result){
				Field* pFields = result->Fetch();
				
				int32 db_id 			= pFields[0].GetInt32(); // don't need
				int32 db_folder 		= pFields[4].GetInt32();
				int32 db_permission 		= pFields[2].GetInt32();			
				std::string db_name 	= pFields[1].GetCppString();

				if(db_folder)// we check if its a folder first
				{
					creategossiplist(player,_Creature,type, db_name.c_str());
				}else{
					int32 db_x 				= pFields[5].GetInt32(); 		
					int32 db_y 				= pFields[6].GetInt32(); 		
					int32 db_z 				= pFields[7].GetInt32();
					int32 db_mapid 			= pFields[8].GetInt32();
					int32 db_levelreq			= pFields[9].GetInt32();
					if(player->getLevel() < db_levelreq)
					{
				_Creature->MonsterWhisper(level_error.str().c_str(), player->GetGUID());
					}else{
				//TELEPORT PLAYER
						player->TeleportTo(db_mapid, db_x, db_y, db_z, 4.919000f);			
					}
					player->CLOSE_GOSSIP_MENU();
				}
				
			}
	}
    struct televotescriptAI : public ScriptedAI
    {
        televotescriptAI(Creature *c) : ScriptedAI(c) {}
    };
	
    CreatureAI* GetAI(Creature* pCreature)
    {
        return new televotescriptAI (pCreature);
    }
};

void AddSC_televotescript()
{
    new televotescript();
}