#ifndef __PARSER_H__
#define __PARSER_H__

#include <iostream>
#include <string>
#include <fstream>
#include <pqxx/pqxx>
#include <ctime>
#include <string>

#include "pugixml.hpp"
#include "parsed_create.hpp"
#include "parsed_transcation.hpp"
#include "command.hpp"
using namespace std;
using namespace pugi;

command parse(const char *xmlData);

std::vector<parsed_create> parse_create_xml(const char * xmlData);

std::vector<parsed_transaction> parse_transaction_xml(const char* xmlData);

#endif