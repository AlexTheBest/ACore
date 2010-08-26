// NOTES: Planning to redo the database structure which means almost a complete rewrite of this script
//			Features Multi rewards
//				  Change items
//
/* STRUCTURE*************************************
// ID // Name // itemid // cost // class // type // requireslots // flags // itemset // level // folder // skill_id // skill_level  // level_req //  money   //  count //
	0     1	   		2		 3   	 4		  5			  6			  7			8	      9			10			11		//      12  //   13     //  14      //     15  //
class
	You can set what class you want to be able to see it, if you want everyone to see it leave it as default which is 0
	
type
	type is the folder you want it under, so if you want it under Items you put Items :D
	
flags
	  1 = Normal Item
	  2 = Level
	  4 = Profession Skill
	  8 = Itemset
	  16 = money
	 1 + 8 = 9
	 
count
	how many times you want to loop through this
	e.g if you want to add the item 5 times you would be put 5
	
Itemset
		Flags must have an 8 in it!
	Because in Tier 7,8,9 and 10 they use the same itemset ids we must find a way
	to seperate 10man and 25man sets, so we use the name of the set e.g 'Heroes'

Level
	What level do you want the client to level up to?
************************************************************/

class rewardscript : public CreatureScript
{
public:
    rewardscript() : CreatureScript("rewardscript") { }

	bool OnGossipSelect(Player *player, Creature *_Creature, uint32 sender, uint32 action)
	{
		// Main menu
		if (sender == GOSSIP_SENDER_MAIN)
		DonationMenu(player, _Creature, action);

		return true;
	}

bool OnGossipHello(Player *player, Creature *_Creature)
{
    //Go straight to the main menu
    player->ADD_GOSSIP_ITEM( 5, "Main"                    , GOSSIP_SENDER_MAIN, 6000);
    player->PlayerTalkClass->SendGossipMenu(6000,_Creature->GetGUID());
    return true;
}


	int precheck(int32 flagbinary, Creature* _Creature, Player* player, int32 SlotsFree, int32 db_cost, int32 db_level, int32 db_itemid, int32 donationpoints, int32 db_min_level , int32 db_skillid, int32 db_skilllevel, uint32 db_max_level)
	{
	//define
	 uint32 noSpaceForCount = 0;
	 int32 count;
	//first we need to make sure the player has enough points to be able to buy the item
		if(db_cost > donationpoints)
		{
			_Creature->MonsterWhisper("You do not have enough Donation Points!", player->GetGUID()); 
			return 0;
		}
		if(flagbinary >= 16)
		{
			flagbinary = flagbinary - 16;
		}
		if(flagbinary >= 8)				//itemsets
		{
			count = 1;
			uint32 noSpaceForCount = 0;
			ItemPosCountVec dest;
			uint8 msg = player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, 23162, SlotsFree, &noSpaceForCount );

			if( msg != EQUIP_ERR_OK ) // convert to possible store amount
				count -= noSpaceForCount;
				 if( count > 0 ) // can't add any
				{
					flagbinary = flagbinary - 8;
				} else {
					_Creature->MonsterWhisper("Make sure you have enough room in your bags", player->GetGUID());
					return 0;
				}
		}
		if(flagbinary >= 4)				//profession
		{
				if(!player->HasSkill(db_skillid))
				{
					_Creature->MonsterWhisper("You do not have this skill, please go and learn it then come back and try", player->GetGUID());
					return 0;
				}
				if (!player->GetSkillValue(db_skillid) >= db_skilllevel)
				{
					_Creature->MonsterWhisper("Your skill level is already at its max.", player->GetGUID());
					return 0;
				}			
			flagbinary = flagbinary - 4;
		}
		
		if(flagbinary >= 2)				// leveling
		{
			if(player->getLevel() > db_level || player->getLevel() > db_max_level) 
			{	
				_Creature->MonsterWhisper("Your Level is too high.", player->GetGUID());
				return 0;
			}
			if(player->getLevel() < db_min_level)
			{
				_Creature->MonsterWhisper("Your Level is too low.", player->GetGUID());
				return 0;		
			}
			flagbinary = flagbinary - 2;	
		}
		
		if(flagbinary >= 1)				//normale item
		{

			count = 1;
			uint32 noSpaceForCount = 0;
			ItemPosCountVec dest;
			uint8 msg = player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, 23162, SlotsFree, &noSpaceForCount );

			if( msg != EQUIP_ERR_OK ) // convert to possible store amount
				count -= noSpaceForCount;
				 if( count > 0 ) // can't add any
				{
					flagbinary = flagbinary - 8;
				} else {
					_Creature->MonsterWhisper("Make sure you have enough room in your bags", player->GetGUID());
					return 0;
				}
		}
		 _Creature->MonsterWhisper("Debug ALL GOOD", player->GetGUID()); 
		return 1;
	}
	/******* NORMAL ITEM *******/
	void GiveItem(uint32 itemId, Creature* _Creature, Player* player, uint32 howmanytimes)
	{
				uint32 count = 1;
				uint32 noSpaceForCount = 0;
				ItemPosCountVec dest;
				uint8 msg = player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, itemId, howmanytimes, &noSpaceForCount );
				if( msg != EQUIP_ERR_OK )                               // convert to possible store amount
					count -= noSpaceForCount;

				if( count == 0 || dest.empty())                         // can't add any
				{
					_Creature->MonsterWhisper("ERROR: Make sure you have enough room in your bags", player->GetGUID()); 
				} else {
				Item* item = player->StoreNewItem( dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId));
					player->SendNewItem(item,howmanytimes,false,true);
					_Creature->MonsterWhisper("Sent!", player->GetGUID());
				}
			
	}

	/******* ITEMSET *******/
	void GiveItemSet(Creature *_Creature, Player *player, uint32 itemsetid, uint32 itemlevel, int32 SlotsFree)
	{
		_Creature->MonsterWhisper("Getting items", player->GetGUID());

		int32 count = 1;
		uint32 noSpaceForCount = 0;
		ItemPosCountVec dest;
		uint8 msg = player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, 23162, SlotsFree, &noSpaceForCount );
		if( msg != EQUIP_ERR_OK ) // convert to possible store amount
		count -= noSpaceForCount;	 
		if( count > 0 ) // can't add any
		{
			QueryResult_AutoPtr result = WorldDatabase.PQuery("SELECT entry FROM mangos.item_template WHERE itemset = '%u' AND itemlevel LIKE \"\%u\%\"", itemsetid, itemlevel);
			do
			{
				Field *fields = result->Fetch();
				uint32 itemId = fields[0].GetUInt32();
				ItemPosCountVec dest;
				player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, itemId, 1 );
				Item* item = player->StoreNewItem( dest, itemId, true);
				player->SendNewItem(item,1,false,true);
			}
			while(result->NextRow());
			//delete result;
		} else {
			_Creature->MonsterWhisper("ERROR: Make sure you have enough room in your bags", player->GetGUID());
	 
		}
				
	}

	/******* PROFESSION *******/
	void GiveProfessionSkill(Player *player, Creature *_Creature, int32 db_skillid, int32 db_skilllevel)
	{
		//player->SetSkill(db_skillid, db_skilllevel, db_skilllevel); not known in trinity
	}

	/******* LEVEL *******/
	void GiveLevel(Player *player, Creature *_Creature, uint32 WhatLevel)
	{
		uint32 SetPlayerLevel = 0;
		SetPlayerLevel = WhatLevel - player->getLevel();
		player->GiveLevel(player->getLevel() + SetPlayerLevel); 
	}

	/******* FOLDERS *******/
	void createfolders(std::string typedonation, Creature *_Creature, Player *player)
	{
		player->PlayerTalkClass->ClearMenus();
		QueryResult_AutoPtr pResult = WorldDatabase.PQuery("SELECT id,name,class,type,cost,folder,icon,faction FROM donationlist WHERE type=\"%s\" && extension<=1 ORDER BY id", typedonation.c_str());
			// Make sure the result is valid
		if(pResult){
			int32 count = 0;
			do{
				Field* pFields = pResult->Fetch();
				if(pFields[2].GetInt32() == player->getClass() or pFields[2].GetInt32() == 0 && pFields[7].GetInt32() == 0 or pFields[7].GetInt32() == player->GetTeam()){
					std::stringstream menuItempoints;
					menuItempoints << pFields[1].GetCppString() << "(" << pFields[4].GetInt32() << " Points)";
					std::stringstream menuItem;
					menuItem << pFields[1].GetCppString() << " >>";
					if(pFields[5].GetInt32() == 1)
					{
						player->ADD_GOSSIP_ITEM( pFields[6].GetInt32(), menuItem.str().c_str(), GOSSIP_SENDER_MAIN, pFields[0].GetInt32());
					} else {
						player->ADD_GOSSIP_ITEM( pFields[6].GetInt32() , menuItempoints.str().c_str(), GOSSIP_SENDER_MAIN, pFields[0].GetInt32());
					} 
					count++;
				}
			}while(pResult->NextRow());
		}else{
				_Creature->MonsterWhisper("Nothing to display", player->GetGUID());	
		}
		QueryResult_AutoPtr pResult2 = WorldDatabase.PQuery("SELECT id FROM donationlist WHERE name = (SELECT type FROM donationlist WHERE name=\"%s\")", typedonation.c_str());
		if(pResult2){
			Field* pFields2 = pResult2->Fetch();
			if(pFields2[0].GetInt32())
				player->ADD_GOSSIP_ITEM( 5, "<- Back", GOSSIP_SENDER_MAIN, pFields2[0].GetInt32()); 
		}
		player->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE,_Creature->GetGUID());
		return;
	}

	/******* RECORD DONATION *******/
	void record_donation(Creature* _Creature, Player* player, int32 db_id, int32 db_cost)
	{
		int32 playerid = player->GetGUID();
		int32 accountid = player->GetSession()->GetAccountId();
		WorldDatabase.PExecute("UPDATE realmd.account SET donationpoints = donationpoints-'%u' WHERE id = '%u'", db_cost , player->GetSession()->GetAccountId());
		WorldDatabase.PExecute("INSERT INTO realmd.DonationLog (`Character`, `Account`, `ItemName`) VALUES ('%u', '%u', '%u')", playerid , accountid , db_id); 
		player->CLOSE_GOSSIP_MENU();
	}

	//******** EVERYTHING LOOKS GOOD********* //
	//*****SO WE GIVE THE DONATION REWARD ****//
	void giving_proccess(Player *player, Creature *_Creature, uint32 db_id, uint32 db_cost, uint32 itemlevel, uint32 db_requireslots, uint32 flagbinary, uint32 db_skillid, uint32 db_skilllevel, uint32 db_level, uint32 db_itemid, uint32 db_count, uint32 db_money)
	{
		if(flagbinary >= 16)					//money
		{	
			_Creature->MonsterWhisper("Money", player->GetGUID());
			player->ModifyMoney(db_money);
			flagbinary = flagbinary - 16;
		}
		
		if(flagbinary >= 8)					//item set = 8
		{	
			_Creature->MonsterWhisper("Giving", player->GetGUID());
			GiveItemSet(_Creature, player, db_itemid, itemlevel, db_requireslots);
			flagbinary = flagbinary - 8;
		}
			
		if(flagbinary >= 4) // profession = 4
		{
			GiveProfessionSkill(player, _Creature, db_skillid, db_skilllevel);
			flagbinary = flagbinary - 4;
		}
		
		if(flagbinary >= 2) // level = 2
		{
			GiveLevel(player, _Creature, db_level);
			flagbinary = flagbinary - 2;
		}
			//Normal Items is last because its the lowest number
		if(flagbinary >= 1)
		{ //none itemsets
		GiveItem(db_itemid, _Creature, player, db_count);  
		}
	}

	/****************************** MAIN ********************/
	/*********************************************************/
	void DonationMenu(Player *player, Creature *_Creature, uint32 action)
	{
	// DEFINE AND SET
		int32 RequireSlots = 0; //??
		int32 Points = 0; //how many points the player has
		int32 DonationId = 0; //??
		int32 itemid = 0; //itemid or itemsetid
		int32 PlayerLevel = 0; //Find player level?
		int32 RequirePoints = 0; //??
		int32 playerid = player->GetGUID();
		int32 accountid = player->GetSession()->GetAccountId();
		int32 donationpoints;  //??
	//STRINGS
		std::stringstream howmany;
		std::string main;
		main = "main";
		
	//BEGIN Gossip Display
		// Disable npc if in combat
		if(!player->getAttackers().empty())
		{
			player->CLOSE_GOSSIP_MENU();
			_Creature->MonsterSay("You are in combat!", LANG_UNIVERSAL, NULL);
			return;
		}
			QueryResult_AutoPtr result = WorldDatabase.PQuery("SELECT donationpoints FROM realmd.account WHERE id = '%u'", player->GetSession()->GetAccountId());
		Field *Fields = result->Fetch();
		if(result)//prevent a crash maybe?
		{	
			donationpoints = Fields[0].GetUInt32(); //Get x Vote Points from database
			switch(action)
				{
					//***********START MAINMENU*************//			
					case 6000: //Back to Main Menu
					if(donationpoints > 0)
					{
							// Main Menu
					//	player->ADD_GOSSIP_ITEM( 5, "How Many Donations Points Do I Have?"                        , GOSSIP_SENDER_MAIN, 5000);
						createfolders(main.c_str(), _Creature, player);  
						}else{
						_Creature->MonsterWhisper("You have no Donations Points to spend", player->GetGUID());
					}
					break;
					
					case 5000: 
						howmany<< "You Have " << donationpoints << " Donations Points";
						_Creature->MonsterWhisper(howmany.str().c_str(), player->GetGUID());
						player->ADD_GOSSIP_ITEM( 5, "Back" , GOSSIP_SENDER_MAIN, 6000);
						player->PlayerTalkClass->SendGossipMenu(6000,_Creature->GetGUID());
						return;
					break;
				} //End switch	
		} else {
			_Creature->MonsterWhisper("There is a problem with server connecting to the database, please come back later....", player->GetGUID());
			player->CLOSE_GOSSIP_MENU();
		}
		QueryResult_AutoPtr pResult = WorldDatabase.PQuery("SELECT id,name,itemid,cost,itemlevel,requiredslots,flags,type,folder,level,min_level,skill_id,skill_level,count,money,extension,max_level FROM donationlist WHERE extension<=1 AND id='%u'", action);
		/*
		//id // name // itemid // cost // itemlevel // requiredslots // flags // type // folder // level // req_level // skill_id // skill_level //  count   //  money // extension //
		//0 //   1  //    2   //   3  //     4    //        5      //   6   //   7  //   8  //	 9    //	 10     //   11     //    12       //    13    //   14   //  15  //
			
		*/
	uint32 CanPurchase = 0;
		if(pResult){
			//do{
			Field* pFields = pResult->Fetch();
			//ints
			int32 db_id 			= pFields[0].GetInt32(); 
			uint32 db_itemid		= pFields[2].GetInt32();
			int32 db_cost 			= pFields[3].GetInt32();
			int32 db_requireslots 	= pFields[5].GetInt32();
			int32 flagbinary		= pFields[6].GetInt32();
		//	int32 db_type		= pFields[7].GetInt32();
			int32 folder			= pFields[8].GetInt32();
			int32 db_level			= pFields[9].GetInt32();
			int32 db_min_level		= pFields[10].GetInt32();
			int32 db_skillid		= pFields[11].GetInt32();
			int32 db_skilllevel		= pFields[12].GetInt32();
			int32 db_count		= pFields[13].GetInt32();
			int32 db_money		= pFields[14].GetInt32();
			int32 db_extension		= pFields[15].GetInt32();
			int32 db_max_level		= pFields[16].GetInt32();
			int32 db_itemlevel = pFields[4].GetInt32();
			
			//strings
			std::string itemname;
			itemname = pFields[1].GetCppString();
			std::string nametype;
			nametype = pFields[7].GetCppString();

				if(db_id == action && folder == 1 )
				{
					createfolders( itemname.c_str() , _Creature, player);  
				}else if(db_id == action)
				{	// Time to split the flags
					//pre check	
					CanPurchase = precheck(flagbinary, _Creature, player, db_requireslots, db_cost, db_level, db_itemid, donationpoints, db_min_level , db_skillid, db_skilllevel, db_max_level);
					if(CanPurchase == 1)
					{
						flagbinary		= pFields[6].GetInt32();
						giving_proccess(player, _Creature, db_id, db_cost, db_itemlevel, db_requireslots, flagbinary, db_skillid, db_skilllevel, db_level, db_itemid, db_count, db_money);
						record_donation(_Creature, player, db_id, db_cost); //we log it so we know who brought what
					} else {
					//print error
						 _Creature->MonsterWhisper("Problem", player->GetGUID()); 
					}
				player->CLOSE_GOSSIP_MENU();
				}
			//}while(pResult->NextRow());
		}
	}
    struct rewardscriptAI : public ScriptedAI
    {
        rewardscriptAI(Creature *c) : ScriptedAI(c) {}
    };
    CreatureAI* GetAI(Creature* pCreature)
    {
        return new rewardscriptAI (pCreature);
    }
};

void AddSC_rewardscript()
{
    new rewardscript();
}