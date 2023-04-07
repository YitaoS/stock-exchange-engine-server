#include <iostream>
#include "parsed_transcation.hpp"
#include "pugixml.hpp"
#include "parser.hpp"

// int main() {
//     // The XML data in string format
//     // const char * xmlData = R"(<create>
//     //     <account id="ACCOUNT_ID" balance="16"/>
//     //     <symbol sym="SYM">
//     //         <account id="ACCOUNT_ID">100</account>
//     //     </symbol>
//     // </create>)";

//     const char * xmlData = R"(
//     <transactions id="123">
//         <order sym="ABC" amount="100.0" limit="10.0"/>
//         <query id="456"/>
//         <cancel id="789"/>
//     </transactions>
// )";

//     vector<parsed_transaction> res = parse_transaction_xml(xmlData);

//     for (int i = 0; i < res.size(); i++) {
//         cout << res[i].flag << endl;
//     }

//     return 0;
// }

int main() {
    const char * xmlData = R"(123
    <create>
        <account id="98765" balance="16"/>
        <symbol sym="SYM">
            <account id="23456">100</account>
            <account id="34567">200</account>
            <account id="45678">300</account>
            <account id="56789">400</account>
        </symbol>
    </create>)";

    std::vector<parsed_create> res = parse_create_xml(xmlData);

    for (const auto& pc : res) {
        std::cout << "Parsed create element:" << std::endl;
        std::cout << "Flag: " << pc.flag << std::endl;
        if (pc.flag == 1) {
            std::cout << "Account ID: " << pc.account_id << std::endl;
            std::cout << "Balance: " << pc.balance << std::endl;
        } else if (pc.flag == 0) {
            std::cout << "Symbol name: " << pc.sym_name << std::endl;
            for (int i = 0; i < pc.account_share_list.size(); i++) {
                cout << "Account ID: " << pc.account_share_list[i].first << endl;
                cout << "Shares: " << pc.account_share_list[i].second << endl;
            }
        }
        std::cout << std::endl;
    }

    return 0;
}