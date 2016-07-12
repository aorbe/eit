#include "StDatabase.h"

#define DB_FILE "/home/asantos/sqlite3.db"

/* ***** Protocol Overview *****
 **** Acquisition Data
 * Packets are composed from several measures sets
 * Actual Packet Len is 20488.
 * We have: 160 (10 bytes x 16 electrodes) * 128 sets = 20480 bytes
 * 20480 + Header (8 bytes) = 20488 bytes
 * Header: Dest - Src - Mode - ID_H - ID_L - Version ..... CRC_H - CRC_L
 *
 */

// Static members
sqlite3 *StDatabase::db = NULL;
StDatabase::DB_STATE StDatabase::db_state = StDatabase::CLOSED;
StDatabase::DB_STATE StDatabase::db_require = StDatabase::CLOSED;
sqlite3_stmt *StDatabase::stmt = NULL;
wxMutex *StDatabase::s_mutexProtectingState =  new wxMutex();;
Record *StDatabase::buffer = NULL;
Record *StDatabase::st_buffer = NULL;

StDatabase::StDatabase() : wxThread(wxTHREAD_JOINABLE), IStorage::IStorage()
{
    char *zErrMsg = 0;
    db_require = CLOSED;
    if(sqlite3_open(DB_FILE, &db))
    {
        printf("Database Error\n");
        db_state = CLOSED;
        sqlite3_close(db);
    }
    else
    {
        db_state = OPENNING;
        // Initialize database
        if(sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS CONTROL (ts INTEGER, nElectrode INTEGER, tCycle INTERGER, Pattern INTEGER, fSample INTEGER, nUnstable INTEGER, fExcitation INTEGER, dataTable CHAR(25), PRIMARY KEY(ts));", NULL, 0, &zErrMsg)!=SQLITE_OK )
        {
            printf("SQL error: %s\n", zErrMsg);
            db_state = CLOSED;
            sqlite3_free(zErrMsg);
        }
        db_state = OPEN;
        db_require = OPEN;
    }
    block = 0;
    Run();
}

StDatabase::~StDatabase()
{
    Delete();
    sqlite3_close(db);
    printf("Destroying Storage Thread\n");
}

bool StDatabase::InsertData(unsigned char *recvData, unsigned int size, unsigned int n_ele, unsigned int* idx, unsigned char mode)
{
    if (mode == 1)
    {
        if (db_state != READY)
        {
            if((db_require != TRANSACTION) || ((db_state == TRANSACTION) && (db_require == TRANSACTION)))
                db_require = READY;
        }
        if (buffer == NULL)
        {
            printf("Creating new buffer\n");
            buffer = new Record(1024, 16);
        }

        values = buffer->addData(recvData, n_ele, size, idx);
        //values.id = *idx;

        if (buffer->isFull())
        {
            if ((db_state == READY) && (db_require != TRANSACTION))
            {
                db_require = TRANSACTION;
                if (st_buffer == NULL)
                {
                    st_buffer = buffer;
                    printf("Sending buffer %d\n", st_buffer->getSize());
                }
                buffer = NULL;
            }
            else
            {
                delete buffer;
                printf("Buffer FULL\n");
            }
            buffer = new Record(1024,16);
        }
        return true;
    }
    if (mode == 2)
    {
        // TODO: How verify errors in communication

        short data[160]; // Poderia ser 80 para 16 eletrodos
        *idx = ((unsigned int)recvData[3])+((unsigned int)recvData[4])*256;

        //static unsigned int next_id = 0;
        //printf("StDatabase: Registering values (ID %04X).\n", *idx);
        if ((*idx & 0x00FF) == 0x7F && !block)
        {

            memcpy(values.amplitude, tmpvalues.amplitude, sizeof(values.amplitude));
            //if (next_id != (*idx & 0xFF00))
            //{
            //    printf("Erro %04X %04X\n", *idx, next_id);
            //}
            //next_id = ((*idx & 0xFF00) + 0x100) % 0x10000;
        }

        // Packet Size 20488. So 128 frames or 640 values.
        for(unsigned item=0; item<(size-8)/160; item++)
        {
            memcpy((void *)data, &recvData[item*160+6], 160);
            // 16 nodes per frame
            for(unsigned int node=0; node<16; node++)
            {
                // 5 values per frame. So 1250 within two packets
                for(unsigned int x=0; x<5; x++)
                {
                    tmpvalues.amplitude[node][((*idx) & 0xFF)*5+x] = data[node*5 + x];
                }
//                if (node == 0)
//                {
//                    for (int k=4; k>=0; k--)
//                        printf("SINAL: %04X %05d\n", *idx, tmpvalues.amplitude[0][((*idx)%256)*5+k]);
//                }
            }
            (*idx)--;
        }
        values.id = *idx;
        return true;
    }
    return false;
}

void StDatabase::InsertConfig(unsigned int* ids, double* values, int size)
{
    char *zErrMsg = 0;
    time_t tTime = time(NULL);

    if (db_state != OPEN)
    {
        printf(" InsertConfig Error: Database Closed\n");
        return;
    }

    if (size < 1)
    {
        printf(" InsertConfig Error: No Data\n");
        return;
    }

    strcpy(query, "INSERT INTO CONFIG (ts, id, value) VALUES (?001, ?002, ?003);");
    sqlite3_prepare(db, query, 512, &stmt, NULL);

    if (sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &zErrMsg) != SQLITE_OK )
    {
        printf("SQL error: %s\n", zErrMsg);
        db_state = CLOSED;
        sqlite3_free(zErrMsg);
    }

    for (int i=0; i<size; i++)
    {
        sqlite3_bind_int(stmt, 1, tTime);
        sqlite3_bind_int(stmt, 2, ids[i]);
        sqlite3_bind_double(stmt, 3, values[i]);
        sqlite3_step(stmt);
        sqlite3_clear_bindings(stmt);
        sqlite3_reset(stmt);
    }

    printf("Config Storing ********** - %s", asctime(localtime(&tTime)));

    if(sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &zErrMsg) !=SQLITE_OK)
    {
        printf("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }

    for(int i=0; i<size; i++)
    {
        parameters[ids[i]] = values[i];
    }
}

int StDatabase::sqlCallback(void *lastName, int argc, char **argv, char **azColName)
{
    //int i;
    printf("Database Last Name Callback\n");
    long num;
    if ((argc>0) && (argv[0])&& (sscanf(argv[0],"MEASURE%lu", &num) == 1))
    {
        sprintf((char*)lastName, "MEASURE%010lu", num+1);
    }
    else
    {
        strcpy((char*)lastName, "MEASURE0000000001");
    }
    printf("Result: %s\n", (char*)lastName);
    s_mutexProtectingState->Lock();
    db_state = CREATING;
    s_mutexProtectingState->Unlock();
    printf("Responded\n");
    return 0;
}

int StDatabase::GetConfig(unsigned int ids[], double* values, int size)
{
    if (db_state != OPEN)
    {
        return -1;
    }

    for(int i=0; i<size; i++)
    {
        if(0 == parameters[ids[i]])
        {
            printf("Retrying parameter %d\n", ids[i]);

            sqlite3_stmt *statement = NULL;
            sprintf(query, "SELECT value FROM CONFIG WHERE id = %d AND ts = (SELECT MAX(ts) FROM CONFIG WHERE id = %d);", ids[i], ids[i]);
            sqlite3_prepare_v2(db, query, -1, &statement, 0);
            int result = sqlite3_step(statement);
            if (result == SQLITE_ROW)
            {
                parameters[ids[i]] = sqlite3_column_double(statement, 0);
            }
            sqlite3_finalize(statement);
        }
        values[i] = parameters[ids[i]];
        printf("Parameter %d = %e\n", ids[i], values[i]);
    }
    return size;


}

StDatabase::ExitCode StDatabase::Entry()
{
    char *zErrMsg = 0;
    char lastTableName[25];

    printf("Starting StoreData Thread\n");
    while(!TestDestroy())
    {
        wxThread::Sleep(100);
        if (db_state != db_require)
        {
            //printf("DB STATE: Required %d Actual %d\n", db_require, db_state);
            if ((db_state == OPEN) && (db_require == READY))
            {
                s_mutexProtectingState->Lock();
                printf("Executing Query: Finding last measure\n");
                // Find last table
                db_state = SEARCHING;
                if(sqlite3_exec(db, "SELECT MAX(dataTable) FROM CONTROL;", &StDatabase::sqlCallback, (void*)lastTableName, &zErrMsg) != SQLITE_OK )
                {
                    printf("SQL error: %s\n", zErrMsg);
                    db_state = CLOSED;
                    sqlite3_free(zErrMsg);
                }
                s_mutexProtectingState->Unlock();
                continue;
            }
            if ((db_state == CREATING) && (db_require == READY))
            {
                printf("Executing Query: INSERT new config line\n");
                // Find last table
                sprintf(query, "INSERT INTO CONTROL (ts, nElectrode, tCycle, Pattern, \
                        fSample, nUnstable, fExcitation, dataTable) VALUES (%ld, %d, %d, %d, %d, %d, %d, '%s');",
                        time(NULL),8,1,1,1,1,1,lastTableName);
                if(sqlite3_exec(db, query, NULL, 0, &zErrMsg)!=SQLITE_OK )
                {
                    printf("SQL error: %s\n(%s)\n", zErrMsg, query);
                    db_state = CLOSED;
                    sqlite3_free(zErrMsg);
                }

                printf("Executing Query: Creating MEASURE table\n");
                sprintf(query, "CREATE TABLE IF NOT EXISTS %s (ts INTEGER, id INTEGER, V1x REAL, V1y REAL, V1q INTEGER, \
                        V2x REAL, V2y REAL, V2q INTEGER, V3x REAL, V3y REAL, V3q INTEGER, V4x REAL, V4y REAL, V4q INTEGER, \
                        V5x REAL, V5y REAL, V5q INTEGER, V6x REAL, V6y REAL, V6q INTEGER, V7x REAL, V7y REAL, V7q INTEGER, \
                        V8x REAL, V8y REAL, V8q INTEGER, CONSTRAINT pk_ID PRIMARY KEY(id, ts));", lastTableName);
                if(sqlite3_exec(db, query, NULL, 0, &zErrMsg)!=SQLITE_OK )
                {
                    printf("SQL error: %s\n", zErrMsg);
                    db_state = CLOSED;
                    sqlite3_free(zErrMsg);
                }

                printf("Executing Query: Preparing insert values\n");
                sprintf(query, "INSERT INTO %s (ts, id, V1x, V1y, V1q, V2x, V2y, V2q, V3x, V3y, V3q, V4x, V4y, V4q, V5x, V5y, V5q, \
                       V6x, V6y, V6q, V7x, V7y, V7q, V8x, V8y, V8q) VALUES (?001, ?002, ?003, ?004, ?005, ?006, ?007, ?008, ?009, \
                       ?010, ?011, ?012, ?013, ?014, ?015, ?016, ?017, ?018, ?019, ?020, ?021, ?022, ?023, ?024, ?025, ?026);",
                       lastTableName);
                sqlite3_prepare_v2(db, query, 512, &stmt, NULL);

                if (sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &zErrMsg) != SQLITE_OK )
                {
                    printf("SQL error: %s\n", zErrMsg);
                    db_state = CLOSED;
                    sqlite3_free(zErrMsg);
                }

                db_state = READY;
                s_mutexProtectingState->Unlock();
                continue;
            }
            if ((db_state == READY) && (db_require==TRANSACTION))
            {
                //unsigned int len = st_buffer->getSize();
                //unsigned char invalid = 0;
                time_t db_ts;
                unsigned int db_id;
                float db_x[8], db_y[8];
                unsigned short db_q[8];

                while (st_buffer->getSize())
                {
                    st_buffer->getData(&db_ts, &db_id, db_x, db_y,  db_q, 8);
                    sqlite3_bind_int(stmt, 1, db_ts);
                    sqlite3_bind_int(stmt, 2, db_id);
                    for(unsigned int x=0; x<8; x++)
                    {
                        sqlite3_bind_double(stmt, x*3+3, db_x[x]);
                        sqlite3_bind_double(stmt, x*3+4, db_y[x]);
                        sqlite3_bind_int(stmt, x*3+5, db_q[x]);
                    }
                    if (sqlite3_step(stmt) != SQLITE_DONE)
                    {
                        printf("SQL error: %s\n", sqlite3_errmsg(db));
                    }
                    sqlite3_clear_bindings(stmt);
                    sqlite3_reset(stmt);
                }
                delete st_buffer;
                st_buffer = NULL;
                if(sqlite3_exec(db, "END TRANSACTION;", NULL, NULL, &zErrMsg) !=SQLITE_OK)
                {
                    printf("SQL error: %s\n", zErrMsg);
                    sqlite3_free(zErrMsg);
                }
                printf("DATA STORED - %s", asctime(localtime(&db_ts)));
                if(sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, &zErrMsg) !=SQLITE_OK)
                {
                    printf("SQL error: %s\n", zErrMsg);
                    sqlite3_free(zErrMsg);
                }
                db_state = TRANSACTION;
                s_mutexProtectingState->Unlock();
                continue;
            }
            if ((db_state == TRANSACTION) && (db_require==READY))
            {
                db_state = READY;
                s_mutexProtectingState->Unlock();
                continue;
            }
        }
    }
    //threads--;
    printf("Stopping StoreData Thread\n");
    return (wxThread::ExitCode) 0;
}

Voltage StDatabase::getData()
{
    return values;
}

unsigned int StDatabase::getData(short buffer[32][1280])
{
    unsigned int id;
    block = 1;
    for(int i=0; i<16; i++)
        memcpy((void*)buffer[i], (void*)values.amplitude[i], sizeof(values.amplitude[i]));
    id = values.id;
    block = 0;
    return id;
}
