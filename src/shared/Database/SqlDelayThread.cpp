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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "Database/SqlDelayThread.h"
#include "Database/SqlOperations.h"
#include "DatabaseEnv.h"

SqlDelayThread::SqlDelayThread(Database* db) : m_dbEngine(db), m_running(true)
{
}

void SqlDelayThread::run()
{
    mysql_thread_init();

    SqlAsyncTask * s = NULL;

    ACE_Time_Value _time(2);
    while (m_running)
    {
        // if the running state gets turned off while sleeping
        // empty the queue before exiting
        s = dynamic_cast<SqlAsyncTask*> (m_sqlQueue.dequeue());
        if (s)
        {
            s->call();
            delete s;
        }
    }

    mysql_thread_end();
}

void SqlDelayThread::Stop()
{
    m_running = false;
    m_sqlQueue.queue()->deactivate();
}

bool SqlDelayThread::Delay(SqlOperation* sql)
{
    int res = m_sqlQueue.enqueue(new SqlAsyncTask(m_dbEngine, sql));
    return (res != -1);
}
