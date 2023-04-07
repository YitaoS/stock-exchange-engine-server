#ifndef __TRANS_RESULT_H__
#define __TRANS_RESULT_H__

#include <iostream>
#include <string>
#include <vector>

#include "pugixml.hpp"
#include "parsed_create.hpp"
#include "parsed_transcation.hpp"

using namespace std;

class cancel_info{
    public:
        double shares;
        long time;
};

class executed_info {
    public:
        double shares;
        double price;
        long time;
};
class order_result{
    public:
        //order
        double amount;
        string sym_name;
        double limit_price;
};

class query_result{
    public:
        //query
        int query_flag; //0: open 1: canceled 2: executed

        double open_shares;

        cancel_info cancel_information;

        vector<executed_info> executed_info_list;

};

class cancel_result{
    public:
        cancel_info cancel_information;

        vector<executed_info> executed_info_list;

};


class trans_result{
    public:
        int flag; //0: order 1: query 2: cancel

        string resultMsg;

        int trans_id;

        order_result order_res;

        query_result query_res;

        cancel_result cancel_res;
};


#endif