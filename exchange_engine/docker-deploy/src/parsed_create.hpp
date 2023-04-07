#ifndef __PARSED_CREATE_H__
#define __PARSED_CREATE_H__

#include <iostream>
#include <string>
#include <fstream>
#include <pqxx/pqxx>
#include <ctime>
#include <vector>

using namespace std;

class parsed_create {
    public:
        int flag; //1: account 0: symbol
        long account_id;
        double balance;
        string sym_name;
        vector<pair<long, double> > account_share_list;
};

#endif