/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * Copyright (C) 2008-2010 Trinity <http://www.trinitycore.org/>
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

#if !defined(DATABASEENV_H)
#define DATABASEENV_H

#include "Common.h"
#include "Logging/Log.h"
#include "Debugging/Errors.h"

#include "Database/Field.h"
#include "Database/QueryResult.h"

#include "Database/Database.h"
typedef Database DatabaseType;
#define _LIKE_           "LIKE"
#define _TABLE_SIM_      "`"
#define _CONCAT3_(A,B,C) "CONCAT( " A " , " B " , " C " )"
#define _OFFSET_         "LIMIT %d,1"

extern DatabaseType WorldDatabase;
extern DatabaseType CharacterDatabase;
extern DatabaseType LoginDatabase;

#endif

