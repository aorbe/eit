#ifndef STORAGE_H
#define STORAGE_H

#include "IStorage.h"
#include "Record.h"
#include "Voltage.h"
#include "sqlite/sqlite3.h"

#include <stdio.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wx/thread.h>
#include <map>

class StDatabase : public wxThread, public IStorage
{
    public:
        StDatabase();
        ~StDatabase();
        enum DB_STATE { CLOSED,     // File closed
                        OPENNING,   // Schema creation
                        OPEN,       // Schema created
                        SEARCHING,  // Verifying last measure
                        CREATING,   // Creating measure table and transaction
                        READY,      // Transaction ready for bind
                        TRANSACTION // Storing transaction
                        };
        bool InsertData(unsigned char* data, unsigned int size, unsigned int n_ele, unsigned int* idx, unsigned char mode);
        void InsertConfig(unsigned int* ids, double* values, int size);
        int GetConfig(unsigned int ids[], double* values, int size);
        Voltage getData();
        unsigned int getData(short values[32][1280]);
    protected:
    private:
        static sqlite3 *db;
        static DB_STATE db_state, db_require;
        static sqlite3_stmt *stmt;
        static wxMutex *s_mutexProtectingState;
        static Record *buffer, *st_buffer;
        static int sqlCallback(void *lastName, int argc, char **argv, char **azColName);
        char query[512];
        virtual ExitCode Entry();
        int block;

        std::map<int,double> parameters;
        Voltage values;
        Voltage tmpvalues;
};


#endif // STORAGE_H
