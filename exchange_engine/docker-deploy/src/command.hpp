#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <iostream>
#include <string>
#include <fstream>
#include <pqxx/pqxx>
#include <ctime>
#include <vector>

#include "pugixml.hpp"
#include "parsed_create.hpp"
#include "parsed_transcation.hpp"

using namespace std;
using namespace pugi;

class command{
    public:
    int flag; //0:create 1: transaction 2: invalid
    vector<parsed_transaction> transactions;
    vector<parsed_create> creates;
};



#endif