#ifndef __CREATE_RESULT_H__
#define __CREATE_RESULT_H__

#include <iostream>
#include <string>
#include <vector>

#include "pugixml.hpp"
#include "parsed_create.hpp"
#include "parsed_transcation.hpp"

using namespace std;
class create_result{
    public:
        int flag; //0:symbol 1:account

        long account;
        string resultMsg;//for account creating
        double balance;

        string symbol;//for symbol creating
        vector<pair<long,double>> account_share_list;
        vector<string> result_list;//account not exist

};

#endif