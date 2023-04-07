#ifndef __HANDLE_COMMAND_H__
#define __HANDLE_COMMAND_H__

#include "command.hpp"
#include "create_result.hpp"
#include "trans_result.hpp"
using namespace std;
using namespace pqxx;

string handle_command(const command & cmd, connection * C);

string handle_create(const vector<parsed_create> & creates, connection * C);

string handle_transaction(const vector<parsed_transaction> & trans, connection * C);

create_result create_account(const parsed_create & account, connection * C);

create_result create_symbol(const parsed_create & symbol, connection * C);

string parse_create_results(const vector<create_result> & cr);

string parse_create_result(const create_result  & cr);

void insert_symbol(connection * C, stringstream & insert_query, stringstream & select_query,nontransaction &N);

void show_results(connection * C, nontransaction &N);

string parse_trans_results(const vector<trans_result> & trs);

string parse_trans_result(const trans_result & tr);

trans_result trans_order(const parsed_transaction & order, connection *C);

trans_result trans_query(const parsed_transaction & query, connection *C);

trans_result trans_cancel(const parsed_transaction & cancel, connection *C);
#endif