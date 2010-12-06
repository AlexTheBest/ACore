/*
 * Copyright (C) 2008-2010 TrinityCore <http://www.trinitycore.org/>
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

#ifndef _DATABASEWORKERPOOL_H
#define _DATABASEWORKERPOOL_H

#include <ace/Thread_Mutex.h>

#include "Common.h"
#include "Callback.h"
#include "MySQLConnection.h"
#include "Transaction.h"
#include "DatabaseWorker.h"
#include "PreparedStatement.h"
#include "Log.h"
#include "QueryResult.h"
#include "QueryHolder.h"
#include "AdhocStatement.h"

class PingOperation : public SQLOperation
{
    /// Operation for idle delaythreads
    bool Execute()
    {
        if (m_conn->LockIfReady())
        {
            m_conn->Ping();
            m_conn->Unlock();
            return true;
        }

        return false;
    }
};

template <class T>
class DatabaseWorkerPool
{
    template <class Y> friend class DatabaseWorker;

    public:
        DatabaseWorkerPool() :
        m_queue(new ACE_Activation_Queue(new ACE_Message_Queue<ACE_MT_SYNCH>)),
        m_connections(0)
        {
            mysql_library_init(-1, NULL, NULL);
            WPFatal (mysql_thread_safe(), "Used MySQL library isn't thread-safe.");
        }

        ~DatabaseWorkerPool()
        {
            sLog.outSQLDriver("~DatabaseWorkerPool for '%s'.", m_connectionInfo.database.c_str());
            mysql_library_end();
        }

        bool Open(const std::string& infoString, uint8 connections, uint8 delaythreads)
        {
            m_connectionInfo = MySQLConnectionInfo(infoString);

            sLog.outSQLDriver("Opening databasepool '%s'. Delaythreads: %u, connections: %u", m_connectionInfo.database.c_str(), delaythreads, connections);

            m_connections.resize(connections);
            for (uint8 i = 0; i < connections; ++i)
            {
                T* t = new T(m_connectionInfo);
                t->Open();
                m_connections[i] = t;
                ++m_connectionCount;
            }

            m_delaythreads.resize(delaythreads);
            for (uint8 i = 0; i < delaythreads; ++i)
                m_delaythreads[i] = new DatabaseWorker<T>(m_queue, this);

            sLog.outSQLDriver("Databasepool opened succesfuly. %u connections running.", m_connectionCount);
            return true;
        }

        void Close()
        {
            sLog.outSQLDriver("Closing down databasepool '%s'.", m_connectionInfo.database.c_str());

            /// Shuts down delaythreads for this connection pool.
            m_queue->queue()->deactivate();

            for (uint8 i = 0; i < m_delaythreads.size(); ++i)
            {
                DatabaseWorker<T>* worker = m_delaythreads[i];
                worker->wait();
                delete worker;
            }

            /// Shut down the connections
            for (uint8 i = 0; i < m_connectionCount; ++i)
            {
                T* t = m_connections[i];
                while (1)
                    if (t->LockIfReady())
                    {
                        t->Close();
                        --m_connectionCount;
                        break;
                    }
            }
            
            sLog.outSQLDriver("All connections on databasepool %s closed.", m_connectionInfo.database.c_str());
        }

        void Execute(const char* sql)
        {
            if (!sql)
                return;

            BasicStatementTask* task = new BasicStatementTask(sql);
            Enqueue(task);
        }

        void PExecute(const char* sql, ...)
        {
            if (!sql)
                return;

            va_list ap;
            char szQuery[MAX_QUERY_LEN];
            va_start(ap, sql);
            vsnprintf(szQuery, MAX_QUERY_LEN, sql, ap);
            va_end(ap);

            Execute(szQuery);
        }

        void DirectExecute(const char* sql)
        {
            if (!sql)
                return;
            
            T* t = GetFreeConnection();
            t->Execute(sql);
            t->Unlock();
        }

        void DirectPExecute(const char* sql, ...)
        {
            if (!sql)
                return;

            va_list ap;
            char szQuery[MAX_QUERY_LEN];
            va_start(ap, sql);
            vsnprintf(szQuery, MAX_QUERY_LEN, sql, ap);
            va_end(ap);

            return DirectExecute(szQuery);
        }

        QueryResult Query(const char* sql, MySQLConnection* conn = NULL)
        {
            if (!conn)
                conn = GetFreeConnection();

            ResultSet* result = conn->Query(sql);
            conn->Unlock();
            if (!result || !result->GetRowCount())
                return QueryResult(NULL);

            result->NextRow();
            return QueryResult(result);
        }

        QueryResult PQuery(const char* sql, MySQLConnection* conn, ...)
        {
            if (!sql)
                return QueryResult(NULL);

            va_list ap;
            char szQuery[MAX_QUERY_LEN];
            va_start(ap, sql);
            vsnprintf(szQuery, MAX_QUERY_LEN, sql, ap);
            va_end(ap);

            return Query(szQuery, conn);
        }

        QueryResult PQuery(const char* sql, ...)
        {
            if (!sql)
                return QueryResult(NULL);

            va_list ap;
            char szQuery[MAX_QUERY_LEN];
            va_start(ap, sql);
            vsnprintf(szQuery, MAX_QUERY_LEN, sql, ap);
            va_end(ap);

            return Query(szQuery);
        }

        ACE_Future<QueryResult> AsyncQuery(const char* sql)
        {
            QueryResultFuture res;
            BasicStatementTask* task = new BasicStatementTask(sql, res);
            Enqueue(task);
            return res;         //! Actual return value has no use yet
        }

        ACE_Future<QueryResult> AsyncPQuery(const char* sql, ...)
        {
            va_list ap;
            char szQuery[MAX_QUERY_LEN];
            va_start(ap, sql);
            vsnprintf(szQuery, MAX_QUERY_LEN, sql, ap);
            va_end(ap);

            return AsyncQuery(szQuery);
        }

        QueryResultHolderFuture DelayQueryHolder(SQLQueryHolder* holder)
        {
            QueryResultHolderFuture res;
            SQLQueryHolderTask* task = new SQLQueryHolderTask(holder, res);
            Enqueue(task);
            return res;     //! Fool compiler, has no use yet
        }

        SQLTransaction BeginTransaction()
        {
            return SQLTransaction(new Transaction);
        }

        void CommitTransaction(SQLTransaction transaction)
        {
            if (sLog.GetSQLDriverQueryLogging())
            {
                switch (transaction->GetSize())
                {
                    case 0:
                        sLog.outSQLDriver("Transaction contains 0 queries. Not executing.");
                        return;
                    case 1:
                        sLog.outSQLDriver("Warning: Transaction only holds 1 query, consider removing Transaction context in code.");
                        break;
                    default:
                        break;
                }
            }

            Enqueue(new TransactionTask(transaction));
        }

        PreparedStatement* GetPreparedStatement(uint32 index)
        {
            return new PreparedStatement(index);
        }

        void Execute(PreparedStatement* stmt)
        {
            PreparedStatementTask* task = new PreparedStatementTask(stmt);
            Enqueue(task);
        }
		
        void DirectExecute(PreparedStatement* stmt)
        {
            T* t = GetFreeConnection();
            t->Execute(stmt);
            t->Unlock();
        }

        void escape_string(std::string& str)
        {
            if (str.empty())
                return;

            char* buf = new char[str.size()*2+1];
            escape_string(buf,str.c_str(),str.size());
            str = buf;
            delete[] buf;
        }

        PreparedQueryResult Query(PreparedStatement* stmt)
        {
            T* t = GetFreeConnection();
            PreparedResultSet* ret = t->Query(stmt);
            t->Unlock();

            if (!ret || !ret->GetRowCount())
                return PreparedQueryResult(NULL);

            return PreparedQueryResult(ret);
        }

        void KeepAlive()
        {
            /// Ping connections
            for (uint8 i = 0; i < m_connectionCount; ++i)
            {
                T* t = m_connections[i];
                if (t->LockIfReady())
                {
                    t->Ping();
                    t->Unlock();
                }
            }
        }

    protected:
        T* GetFreeConnection()
        {
            uint8 i = 0;
            for (;;)    /// Block forever until a connection is free
            {
                T* t = m_connections[++i % m_connectionCount];
                if (t->LockIfReady())   /// Must be matched with t->Unlock() or you will get deadlocks
                    return t;
            }

            // This will be called when Celine Dion learns to sing
            return NULL;
        }

    private:
        unsigned long escape_string(char *to, const char *from, unsigned long length)
        {
            if (!to || !from || !length)
                return 0;
            
            T* t = GetFreeConnection();
            unsigned long ret = mysql_real_escape_string(t->GetHandle(), to, from, length);
            t->Unlock();
            return ret;
        }

        void Enqueue(SQLOperation* op)
        {
            m_queue->enqueue(op);
        }

    private:

        ACE_Activation_Queue*           m_queue;                    //! Queue shared by async worker threads.
        std::vector<T*>                 m_connections;
        uint8                           m_connectionCount;          //! Counter of MySQL connections;
        MySQLConnectionInfo             m_connectionInfo;
        std::vector<DatabaseWorker<T>*> m_delaythreads;     //! Delaythreads (templatized)
};

#endif
