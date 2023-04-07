#ifndef __PARSED_TRANSACTION_H__
#define __PARSED_TRANSACTION_H__

#include <iostream>
#include <string>
#include <fstream>
#include <pqxx/pqxx>
#include <ctime>
#include <vector>

using namespace std;

class parsed_transaction {
    public:
        int flag; //0: order 1: query 2: cancel
        long account_id;
        string sym_name;
        double amount;
        double limit;
        int trans_id;
};

#endif