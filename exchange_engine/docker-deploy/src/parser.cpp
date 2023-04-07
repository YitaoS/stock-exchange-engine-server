#include <iostream>
#include <string>
#include <fstream>
#include <pqxx/pqxx>
#include <ctime>

#include "command.hpp"
#include "parser.hpp"
using namespace std;
using namespace pugi;

command parse(const char * xmlData){

    pugi::xml_document doc;
    command ans;
    if (!doc.load_string(xmlData)) {
        std::cerr << "Failed to parse XML data." << std::endl;
        ans.flag = 2;
        return ans;
    }
    pugi::xml_node root = doc.document_element();
    if(string(root.name())=="create"){
        ans.flag = 0;
        ans.creates = parse_create_xml(xmlData);
    }else{
        ans.flag = 1;
        ans.transactions = parse_transaction_xml(xmlData);
    }
    return ans;
};

std::vector<parsed_create> parse_create_xml(const char * xmlData) {
    std::vector<parsed_create> result;

    pugi::xml_document doc;
    if (!doc.load_string(xmlData)) {
        std::cerr << "Failed to parse XML data." << std::endl;
        return result;
    }

    pugi::xml_node createNode = doc.document_element();

    for (pugi::xml_node childNode : createNode.children()) {
        parsed_create pc;

        if (std::string(childNode.name()) == "account") {
            pc.flag = 1;
            pc.account_id = childNode.attribute("id").as_llong();
            pc.balance = childNode.attribute("balance").as_double();
        } else if (std::string(childNode.name()) == "symbol") {
            pc.flag = 0;
            pc.sym_name = childNode.attribute("sym").as_string();
            for (pugi::xml_node grandchildNode : childNode.children()) {
                // pc.account_id = grandchildNode.attribute("id").as_llong();
                // pc.shares = grandchildNode.text().as_double();
                pc.account_share_list.push_back(make_pair(grandchildNode.attribute("id").as_llong(), grandchildNode.text().as_double()));
            }
        } else {
            continue;
        }

        result.push_back(pc);
    }

    return result;
}

vector<parsed_transaction> parse_transaction_xml(const char* xmlData) {
    vector<parsed_transaction> result;

    pugi::xml_document doc;
    if (!doc.load_string(xmlData)) {
        cerr << "Failed to parse XML data." << endl;
        return result;
    }

    pugi::xml_node transactionsNode = doc.document_element();

    // Get the account ID from the 'id' attribute of the 'transactions' node
    long account_id = transactionsNode.attribute("id").as_llong();

    // Loop over the child nodes of the 'transactions' node
    for (pugi::xml_node childNode : transactionsNode.children()) {
        parsed_transaction pt;

        // Set the account ID for this transaction
        pt.account_id = account_id;

        // Determine the transaction type based on the tag name
        string tagName = childNode.name();
        if (tagName == "order") {
            pt.flag = 0;
            pt.sym_name = childNode.attribute("sym").as_string();
            pt.amount = childNode.attribute("amount").as_double();
            pt.limit = childNode.attribute("limit").as_double();
        } else if (tagName == "query") {
            pt.flag = 1;
            pt.trans_id = childNode.attribute("id").as_int();
        } else if (tagName == "cancel") {
            pt.flag = 2;
            pt.trans_id = childNode.attribute("id").as_int();
        } else {
            continue; // Ignore nodes with unrecognized tags
        }

        result.push_back(pt);
    }

    return result;
}
