#ifndef __DB_INTERACT_H__
#define __DB_INTERACT_H__

#include <iostream>
#include <string>
#include <fstream>
#include <pqxx/pqxx>
#include <ctime>
using namespace std;
using namespace pqxx;
#define DB_PARAMETER "dbname=MATCH_ENGINE_DB user=postgres password=passw0rd host=db port=5432"
//#define DB_PARAMETER "dbname=MATCH_ENGINE_DB user=postgres password=passw0rd"
void connectDatabase(string db_parameter,connection ** C);

void drop_tables(connection * C);

std::vector<const char *> get_tables_create_sql();

void create_tables(connection * C, std::vector<const char *> tables_create_sql);

#endif