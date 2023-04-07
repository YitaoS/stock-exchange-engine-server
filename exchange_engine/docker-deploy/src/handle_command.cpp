#include "handle_command.hpp"
#include "command.hpp"
#include "create_result.hpp"
#include "parsed_create.hpp"
#include "parsed_transcation.hpp"
#include "trans_result.hpp"
#include <pqxx/connection.hxx>
#include <pqxx/nontransaction.hxx>
#include <sstream>
#include <string>
using namespace pqxx;

string handle_command(const command & cmd, connection * C){
    string ans;
    if (cmd.flag == 0){
        ans = handle_create(cmd.creates,C);
    }else if (cmd.flag == 1){
        ans = handle_transaction(cmd.transactions,C);
    }//2
    nontransaction N(*C);
    show_results(C, N);

    return ans;
}

string handle_create(const vector<parsed_create> & creates, connection * C){
    vector<create_result> crs;
    for (auto item: creates) {
        create_result cr;
        if (item.flag == 0) {
            cr = create_symbol(item,C);
        }else{
            cr = create_account(item,C);
        }
        crs.push_back(cr);
    }
    return parse_create_results(crs);
}

create_result create_account(const parsed_create & account, connection * C){
    create_result ans;
    ans.flag = 1;
    ans.account = account.account_id;
    ans.balance = account.balance;

    if (ans.balance < 0) {
        ans.resultMsg = "Account balance cannot be negative!";
        return ans;
    }

    stringstream query;
    query << "INSERT INTO ACCOUNTS (ACCOUNT_ID, BALANCE) "
          << "VALUES (" << to_string(account.account_id) << ", "
          << to_string(account.balance) << ");";
    
    nontransaction N(*C);
    
    try {
        result R(N.exec(query.str()));
        // cout<<"1"<<endl;
        ans.resultMsg = "Success";
        
        // Process the result here
        // cout << "success" << endl;
        
    }
    catch (unique_violation& e) {
        // Handle the unique violation exception here
        // cerr << "Insert failed: " << e.what() << endl;
        ans.resultMsg = "Account already exists!";
    }

    return ans;
}

void insert_symbol(connection * C, stringstream & insert_query, stringstream & select_query, nontransaction &N){
    bool symbol_exists = false;
    do{
        // cout<<"2"<<endl;
        try {
            result R(N.exec(insert_query.str()));
            symbol_exists = true;
            // Process the result here
            // cout << "success" << endl;
            
        }
        catch (unique_violation& e) {
            // Handle the unique violation exception here
            // cerr << "Insert failed: " << e.what() << endl;

            // nontransaction N_select(*C);
            result R_select(N.exec(select_query.str()));
            if (!R_select.empty()) {
                symbol_exists = true;
            }
        }


    }while(!symbol_exists);
}

create_result create_symbol(const parsed_create & symbol, connection * C){
    nontransaction N(*C);
    create_result ans;
    ans.flag = 0;
    ans.symbol = symbol.sym_name;
    stringstream insert_query;
    insert_query << "INSERT INTO SYMBOLS (SYMBOL_NAME) "
                 << "VALUES (\'" << symbol.sym_name << "\');";
    stringstream select_query;
    select_query << "SELECT * FROM SYMBOLS WHERE SYMBOL_NAME = \'"
                 << symbol.sym_name << "\';";
        
    insert_symbol(C,insert_query,select_query, N);

    for (auto sym: symbol.account_share_list) {
        ans.account_share_list.push_back(make_pair(sym.first, sym.second));
        if (sym.second < 0) {
            ans.result_list.push_back("Shares cannot be negative!");
        }
        else {
            long account_id = sym.first;
            stringstream query;
            query << "SELECT * FROM ACCOUNTS WHERE ACCOUNT_ID = " << to_string(account_id) << ";";
            
            // nontransaction N(*C);
            // cout<<"3"<<endl;
            result R(N.exec(query.str()));
            
            if (R.empty()) {
                ans.result_list.push_back("Account_ID does not exist!");
                continue;
            }
            else {
                query.str("");
                query << "SELECT * FROM POSITIONS WHERE ACCOUNT_ID = " << to_string(account_id)
                      << " AND SYMBOL_NAME = " << N.quote(symbol.sym_name) << " FOR UPDATE;";
                result R2(N.exec(query.str()));
                // cout<<"4"<<endl;
                if (R2.empty()) {
                    // cout<<"5"<<endl;
                    insert_query.str("");
                    select_query.str("");
                    insert_query << "INSERT INTO POSITIONS (SHARES, ACCOUNT_ID, SYMBOL_NAME) "
                                 << "VALUES (" << sym.second << ", " << sym.first << "," << N.quote(symbol.sym_name)
                                 << ");";
                    select_query << "SELECT * FROM POSITIONS WHERE ACCOUNT_ID = " << to_string(account_id)
                                 << " AND SYMBOL_NAME = " << N.quote(symbol.sym_name) << ";";
                    // cout<<insert_query.str()<<endl<<select_query.str()<<endl;

                    insert_symbol(C, insert_query, select_query,N);
                    // cout<<"5"<<endl;
                }
                else {
                    // cout<<"6"<<endl;
                    query.str("");
                    query << "UPDATE POSITIONS SET SHARES = SHARES + " << to_string(sym.second)
                          << " WHERE ACCOUNT_ID = " << to_string(account_id)
                          << " AND SYMBOL_NAME = " << N.quote(symbol.sym_name) << ";";
                    
                    
                    result R3(N.exec(query.str()));
                    // cout<<"6"<<endl;
                }
                N.exec("COMMIT;");
                ans.result_list.push_back("Success");
            }
        }
    }
    // show_results(C, N);
    return ans;
}

// string handle_create(const vector<parsed_create> & creates, connection * C){
//     vector<create_result> crs;
//     for (auto item: creates) {
//         create_result cr;
//         if (item.flag == 0) {
//             cr = create_symbol(item,C);
//         }else{
//             cr = create_account(item,C);
//         }
//         crs.push_back(cr);
//     }
//     return parse_create_results(crs);
// }
string handle_transaction(const vector<parsed_transaction> & trans, connection * C){
    vector<trans_result> trs;
    for (auto item: trans) {
        trans_result tr;
        if (item.flag == 0) {
            tr = trans_order(item, C);
        }
        else if (item.flag == 1) {
            tr = trans_query(item, C);
        }
        else {
            tr = trans_cancel(item, C);
        }
        trs.push_back(tr);
    }

    return parse_trans_results(trs);
}


// class trans_result{
//     public:
//         int flag; //0: order 1: query 2: cancel

//         string resultMsg;

//         int trans_id;

//         order_result order_res;

//         query_result query_res;

//         cancel_result cancel_res;
// };

trans_result trans_order(const parsed_transaction & order, connection *C){
    trans_result ans;
    nontransaction N(*C);
    stringstream query;
    ans.flag = 0;
    ans.order_res.amount = order.amount;
    ans.order_res.limit_price = order.limit;
    ans.order_res.sym_name = order.sym_name;
    double remaining_shares = order.amount;
    
    query << "SELECT * FROM ACCOUNTS WHERE ACCOUNT_ID = " << to_string(order.account_id) << " FOR UPDATE;";
    result R1(N.exec(query.str()));
    if (R1.empty()) {
        ans.resultMsg = "Account does not exist!";
        N.exec("COMMIT;");
        return ans;
    }
    
    query.str("");
    query << "SELECT * FROM SYMBOLS WHERE SYMBOL_NAME = " << N.quote(order.sym_name) << ";";
    result R2(N.exec(query.str()));
    if (R2.empty()) {
        ans.resultMsg = "Symbol does not exist!";
        N.exec("COMMIT;");
        return ans;
    }

    query.str("");
    query << "SELECT * FROM POSITIONS WHERE ACCOUNT_ID = " << to_string(order.account_id) 
          << " AND SYMBOL_NAME = " << N.quote(order.sym_name) << ";";
    result R3(N.exec(query.str()));
    if (R3.empty()) {
        query.str("");
        query << "INSERT INTO POSITIONS (SHARES, ACCOUNT_ID, SYMBOL_NAME) VALUES(" 
              << to_string(0) << ", " << to_string(order.account_id) << ", " << N.quote(order.sym_name)<< ");";
        N.exec(query.str());
    }

    if (order.amount > 0){
        //buy
        // balance should be greater than amount * limit
        double balance = R1[0]["BALANCE"].as<double>();
        if (balance < order.amount * order.limit) {
            ans.resultMsg = "Insufficient balance!";
            N.exec("COMMIT;");
            return ans;
        }

        query.str("");
        query << "UPDATE ACCOUNTS SET BALANCE = BALANCE - " << to_string(order.amount * order.limit) 
              << " WHERE ACCOUNT_ID = " << to_string(order.account_id) << ";";
        N.exec(query.str());

        time_t now = time(0);
        long current_time = static_cast<long>(now);
        query.str("");
        query << "INSERT INTO TRANSACTIONS (STATUS, ACCOUNT_ID, SYMBOL_NAME, TIME, LIMIT_PRICE, SHARES) "
                              << "VALUES (" << N.quote("open") << ", " <<to_string(order.account_id)<<", "
                              << N.quote(order.sym_name)<<", "<< to_string(current_time)
                              << ", " << to_string(order.limit) <<", "<< to_string(order.amount)<< ") RETURNING TRANS_ID;";
        result R_get_trans_id (N.exec(query.str()));
        
        int self_trans_id = R_get_trans_id[0][0].as<int>();
        ans.trans_id = self_trans_id;
        //add lock for newly inserted transaction not to be matched with upcoming transactions
        query.str("");
        query << "SELECT * FROM TRANSACTIONS WHERE TRANS_ID = " << to_string(self_trans_id) << " FOR UPDATE;";
        N.exec(query.str());
        // cout << "Generated Trans_ID is: " << R_get_trans_id[0][0].as<int>() << endl;
        // show_results(C, N);

        // query.str("");
        // query << "SELECT * FROM TRANSACTIONS "
        //search the min price trans which is open and smaller than limit price and name is sym_name
        // query.str("");
        // query << "SELECT * FROM TRANSACTIONS WHERE SHARES < 0 AND STATUS = 'open' AND SYMBOL_NAME = " << N.quote(order.sym_name)
        //       << " ORDER BY LIMIT_PRICE, TRANS_ID LIMIT 1 FOR UPDATE;";
        bool executable = true;
        do{
            query.str("");
            query << "SELECT * FROM TRANSACTIONS WHERE SHARES < 0 AND STATUS = 'open' AND SYMBOL_NAME = " << N.quote(order.sym_name)
                  << " ORDER BY LIMIT_PRICE, TRANS_ID LIMIT 1 FOR UPDATE;";
            result R(N.exec(query.str()));

            if (R.empty()) {
                executable = false;
            }
            else{

                double min_price = R[0]["LIMIT_PRICE"].as<double>();
                int trans_id = R[0]["TRANS_ID"].as<int>();
                double target_shares = R[0]["SHARES"].as<double>();
                long seller_account_id = R[0]["ACCOUNT_ID"].as<long>();

                if (min_price <= ans.order_res.limit_price) {
                    // to be executed
                    double executed_shares;

                    // add lock for seller position
                    query.str("");
                    query << "SELECT * FROM POSITIONS WHERE ACCOUNT_ID = " << trans_id << " AND SYMBOL_NAME = "
                    << N.quote(order.sym_name) << " FOR UPDATE;";
                    N.exec(query.str());

                    if(remaining_shares <= abs(target_shares)){
                        executed_shares = remaining_shares;
                        executable = false;

                        //update seller's order in transactions
                        query.str("");
                        query << "UPDATE TRANSACTIONS SET SHARES = " << to_string(target_shares + executed_shares) 
                              << " WHERE TRANS_ID = " << to_string(trans_id) << ";";
                        N.exec(query.str());

                        //insert seller's order in exectued_trans
                        query.str("");
                        now = time(0);
                        current_time = static_cast<long>(now);

                        query << "INSERT INTO EXECUTED_TRANS (TRANS_ID, TIME, PRICE, SHARES) VALUES ("
                              << to_string(trans_id) << ", " << to_string(current_time) << ", "
                              << to_string(min_price) << ", " << to_string(-executed_shares) << ");";
                        N.exec(query.str());

                        //update self data in transactions
                        query.str("");
                        query << "UPDATE TRANSACTIONS SET STATUS = " << N.quote("executed") << ", SHARES = "
                              << to_string(0) << " WHERE TRANS_ID = " << to_string(self_trans_id) << ";";
                        N.exec(query);

                        //add self data in executed_trans
                        query.str("");
                        now = time(0);
                        current_time = static_cast<long>(now);
                        query << "INSERT INTO EXECUTED_TRANS (TRANS_ID, TIME, PRICE, SHARES) VALUES (" 
                              << to_string(self_trans_id) << ", " << to_string(current_time) << ", "
                              << to_string(min_price) << ", " << to_string(remaining_shares) << ");";
                        N.exec(query.str());

                        //update self positions
                        query.str("");
                        query << "UPDATE POSITIONS SET SHARES = SHARES + " << to_string(executed_shares) 
                              << " WHERE ACCOUNT_ID = " << to_string(order.account_id) << " AND SYMBOL_NAME = " 
                              << N.quote(order.sym_name) << ";";
                        N.exec(query.str());

                        //update self account and do refund
                        query.str("");
                        query << "UPDATE ACCOUNTS SET BALANCE = BALANCE + " 
                              << to_string(executed_shares * (ans.order_res.limit_price - min_price)) 
                              << " WHERE ACCOUNT_ID = " << to_string(order.account_id) << ";";
                        N.exec(query.str());

                        //update seller's positions
                        // query.str("");
                        // query << "UPDATE POSITIONS SET SHARES = SHARES - " << to_string(executed_shares) 
                        //       << " WHERE ACCOUNT_ID = " << to_string(seller_account_id) << " AND SYMBOL_NAME = "
                        //       << N.quote(order.sym_name) << ";";
                        // N.exec(query.str());

                        //update seller's account
                        query.str("");
                        query << "UPDATE ACCOUNTS SET BALANCE = BALANCE + " << to_string(executed_shares * min_price)
                              << " WHERE ACCOUNT_ID = " << to_string(seller_account_id) << ";";
                        N.exec(query.str()); 
                        
                    }else {
                        executed_shares = abs(target_shares);
                         //update the searched trans: shares ->0 , status -> executed
                        
                        //update seller's transactions
                        query.str("");
                        query << "UPDATE TRANSACTIONS SET SHARES = " << to_string(target_shares + executed_shares) 
                              << ", STATUS = 'executed' WHERE TRANS_ID = " << to_string(trans_id) << ";";
                        N.exec(query.str());
                        
                        //update seller's executed_trans
                        now = time(0);
                        current_time = static_cast<long>(now);

                        query.str("");
                        query << "INSERT INTO EXECUTED_TRANS (TRANS_ID, TIME, PRICE, SHARES)"
                              << "VALUES (" << to_string(trans_id) << ", " << to_string(current_time)
                              << ", " << to_string(min_price) << ", " << to_string(-executed_shares) << ");";

                        N.exec(query.str());

                        //update self transactions
                        query.str("");
                        query << "UPDATE TRANSACTIONS SET SHARES = " << to_string(remaining_shares - executed_shares) 
                              << " WHERE TRANS_ID = " << to_string(self_trans_id) << ";";
                        N.exec(query.str());

                        //update self executed_trans
                        now = time(0);
                        current_time = static_cast<long>(now);

                        query.str("");
                        query << "INSERT INTO EXECUTED_TRANS (TRANS_ID, TIME, PRICE, SHARES)"
                              << " VALUES (" << to_string(self_trans_id) << ", " << to_string(current_time)
                              << ", " << to_string(min_price) << ", " << to_string(executed_shares) << ");";
                        N.exec(query.str());

                        //update self positions
                        query.str("");
                        query << "UPDATE POSITIONS SET SHARES = SHARES + " << to_string(executed_shares) 
                              << " WHERE ACCOUNT_ID = " << to_string(order.account_id) << " AND SYMBOL_NAME = " 
                              << N.quote(order.sym_name) << ";";
                        N.exec(query.str());

                        //update self account and do refund
                        query.str("");
                        query << "UPDATE ACCOUNTS SET BALANCE = BALANCE + " 
                              << to_string(executed_shares * (ans.order_res.limit_price - min_price)) 
                              << " WHERE ACCOUNT_ID = " << to_string(order.account_id) << ";";
                        N.exec(query.str());

                        //update seller's positions
                        // query.str("");
                        // query << "UPDATE POSITIONS SET SHARES = SHARES - " << to_string(executed_shares) 
                        //       << " WHERE ACCOUNT_ID = " << to_string(seller_account_id) << " AND SYMBOL_NAME = "
                        //       << N.quote(order.sym_name) << ";";
                        // N.exec(query.str());

                        //update seller's account
                        query.str("");
                        query << "UPDATE ACCOUNTS SET BALANCE = BALANCE + " << to_string(executed_shares * min_price)
                              << " WHERE ACCOUNT_ID = " << to_string(seller_account_id) << ";";
                        N.exec(query.str()); 
                    }
                    remaining_shares -= executed_shares;

                }
                else {
                    // will not execute, then add this transaction to the database
                    executable = false;
                }
            }
            
        } while(executable);
        // show_results(C, N);

    }
    else{
        //sell
        //need to check enough shares in position
        query.str("");
        query << "SELECT * FROM POSITIONS WHERE ACCOUNT_ID = " << to_string(order.account_id) << ";";
        result R2(N.exec(query.str()));

        double self_shares = R2[0]["SHARES"].as<double>();
        if (self_shares < abs(order.amount)) {
            ans.resultMsg = "Insufficient Shares!";
            N.exec("COMMIT;");
            return ans;
        }

        query.str("");
        query << "UPDATE POSITIONS SET SHARES = SHARES + "
              << to_string(order.amount)
              << " WHERE ACCOUNT_ID = " << to_string(order.account_id)
              << " AND SYMBOL_NAME = " << N.quote(order.sym_name) << ";";
        N.exec(query.str());

        time_t now = time(0);
        long current_time = static_cast<long>(now);
        query.str("");
        query << "INSERT INTO TRANSACTIONS (STATUS, ACCOUNT_ID, SYMBOL_NAME, TIME, LIMIT_PRICE, SHARES) "
                              << "VALUES (" << N.quote("open") << ", " <<to_string(order.account_id)<<", "
                              << N.quote(order.sym_name)<<", "<< to_string(current_time)
                              << ", " << to_string(order.limit) <<", "<< to_string(order.amount)<< ") RETURNING TRANS_ID;";
        result R_get_trans_id (N.exec(query.str()));
        
        int self_trans_id = R_get_trans_id[0][0].as<int>();
        ans.trans_id = self_trans_id;
        //add lock for newly inserted transaction not to be matched with upcoming transactions
        query.str("");
        query << "SELECT * FROM TRANSACTIONS WHERE TRANS_ID = " << to_string(self_trans_id) << " FOR UPDATE;";
        N.exec(query.str());
        // cout << "Generated Trans_ID is: " << R_get_trans_id[0][0].as<int>() << endl;
        // show_results(C, N);

        // query.str("");
        // query << "SELECT * FROM TRANSACTIONS "
        //search the min price trans which is open and smaller than limit price and name is sym_name
        // query.str("");
        // query << "SELECT * FROM TRANSACTIONS WHERE SHARES > 0 AND STATUS = 'open' AND SYMBOL_NAME = " << N.quote(order.sym_name)
        //       << " ORDER BY LIMIT_PRICE DESC, TRANS_ID LIMIT 1 FOR UPDATE;";
        bool executable = true;
        do{
            query.str("");
            query << "SELECT * FROM TRANSACTIONS WHERE SHARES > 0 AND STATUS = 'open' AND SYMBOL_NAME = " << N.quote(order.sym_name)
                  << " ORDER BY LIMIT_PRICE DESC, TRANS_ID LIMIT 1 FOR UPDATE;";
            result R(N.exec(query.str()));

            if (R.empty()) {
                executable = false;
            }
            else{

                double max_price = R[0]["LIMIT_PRICE"].as<double>();
                int trans_id = R[0]["TRANS_ID"].as<int>();
                double target_shares = R[0]["SHARES"].as<double>();
                long buyer_account_id = R[0]["ACCOUNT_ID"].as<long>();

                if (max_price >= ans.order_res.limit_price) {
                    // to be executed
                    double executed_shares;

                    // add lock for seller position
                    query.str("");
                    query << "SELECT * FROM POSITIONS WHERE ACCOUNT_ID = " << self_trans_id << " AND SYMBOL_NAME = "
                    << N.quote(order.sym_name) << " FOR UPDATE;";
                    N.exec(query.str());

                    if(abs(remaining_shares) <= target_shares){
                        executed_shares = abs(remaining_shares);
                        executable = false;

                        //update buyer's order in transactions
                        query.str("");
                        query << "UPDATE TRANSACTIONS SET SHARES = " << to_string(target_shares - executed_shares) 
                              << " WHERE TRANS_ID = " << to_string(trans_id) << ";";
                        N.exec(query.str());

                        //insert buyer's order in exectued_trans
                        query.str("");
                        now = time(0);
                        current_time = static_cast<long>(now);

                        query << "INSERT INTO EXECUTED_TRANS (TRANS_ID, TIME, PRICE, SHARES) VALUES ("
                              << to_string(trans_id) << ", " << to_string(current_time) << ", "
                              << to_string(max_price) << ", " << to_string(executed_shares) << ");";
                        N.exec(query.str());

                        //update self data in transactions
                        query.str("");
                        query << "UPDATE TRANSACTIONS SET STATUS = " << N.quote("executed") << ", SHARES = "
                              << to_string(0) << " WHERE TRANS_ID = " << to_string(self_trans_id) << ";";
                        N.exec(query.str());

                        cout << "-----------\n";
                        //add self data in executed_trans
                        query.str("");
                        now = time(0);
                        current_time = static_cast<long>(now);
                        query << "INSERT INTO EXECUTED_TRANS (TRANS_ID, TIME, PRICE, SHARES) VALUES (" 
                              << to_string(self_trans_id) << ", " << to_string(current_time) << ", "
                              << to_string(max_price) << ", " << to_string(remaining_shares) << ");";
                        N.exec(query.str());
                        //update self positions
                        // query.str("");
                        // query << "UPDATE POSITIONS SET SHARES = SHARES - " << to_string(executed_shares) 
                        //       << "WHERE ACCOUNT_ID = " << to_string(order.account_id) << " AND SYMBOL_NAME = " 
                        //       << N.quote(order.sym_name) << ";";
                        // N.exec(query.str());

                        //update self account
                        query.str("");
                        query << "UPDATE ACCOUNTS SET BALANCE = BALANCE + " << to_string(executed_shares * max_price) 
                              << " WHERE ACCOUNT_ID = " << to_string(order.account_id) << ";";
                        N.exec(query.str());

                        //update buyer's positions
                        query.str("");
                        query << "UPDATE POSITIONS SET SHARES = SHARES + " << to_string(executed_shares) 
                              << " WHERE ACCOUNT_ID = " << to_string(buyer_account_id) << " AND SYMBOL_NAME = "
                              << N.quote(order.sym_name) << ";";
                        N.exec(query.str());

                        //update buyer's account
                        query.str("");
                        query << "UPDATE ACCOUNTS SET BALANCE = BALANCE - " << to_string(executed_shares * max_price)
                              << " WHERE ACCOUNT_ID = " << to_string(buyer_account_id) << ";";
                        N.exec(query.str()); 
                        
                    }else {
                        executed_shares = target_shares;
                         //update the searched trans: shares ->0 , status -> executed
                        
                        //update buyer's transactions
                        query.str("");
                        query << "UPDATE TRANSACTIONS SET SHARES = " << to_string(target_shares - executed_shares) 
                              << ", STATUS = 'executed' WHERE TRANS_ID = " << to_string(trans_id) << ";";
                        N.exec(query.str());
                        
                        //update buyer's executed_trans
                        now = time(0);
                        current_time = static_cast<long>(now);

                        query.str("");
                        query << "INSERT INTO EXECUTED_TRANS (TRANS_ID, TIME, PRICE, SHARES)"
                              << "VALUES (" << to_string(trans_id) << ", " << to_string(current_time)
                              << ", " << to_string(max_price) << ", " << to_string(executed_shares) << ");";

                        N.exec(query.str());

                        //update self transactions
                        query.str("");
                        query << "UPDATE TRANSACTIONS SET SHARES = " << to_string(remaining_shares + executed_shares) 
                              << " WHERE TRANS_ID = " << to_string(self_trans_id) << ";";
                        N.exec(query.str());

                        //update self executed_trans
                        now = time(0);
                        current_time = static_cast<long>(now);

                        query.str("");
                        query << "INSERT INTO EXECUTED_TRANS (TRANS_ID, TIME, PRICE, SHARES)"
                              << "VALUES (" << to_string(self_trans_id) << ", " << to_string(current_time)
                              << ", " << to_string(max_price) << ", " << to_string(-executed_shares) << ");";
                        N.exec(query.str());

                        //update self positions
                        // query.str("");
                        // query << "UPDATE POSITIONS SET SHARES = SHARES - " << to_string(executed_shares) 
                        //       << "WHERE ACCOUNT_ID = " << to_string(order.account_id) << " AND SYMBOL_NAME = " 
                        //       << N.quote(order.sym_name) << ";";
                        // N.exec(query.str());

                        //update self account
                        query.str("");
                        query << "UPDATE ACCOUNTS SET BALANCE = BALANCE + " << to_string(executed_shares * max_price) 
                              << " WHERE ACCOUNT_ID = " << to_string(order.account_id) << ";";
                        N.exec(query.str());

                        //update buyer's positions
                        query.str("");
                        query << "UPDATE POSITIONS SET SHARES = SHARES + " << to_string(executed_shares) 
                              << " WHERE ACCOUNT_ID = " << to_string(buyer_account_id) << " AND SYMBOL_NAME = "
                              << N.quote(order.sym_name) << ";";
                        N.exec(query.str());

                        //update buyer's account
                        query.str("");
                        query << "UPDATE ACCOUNTS SET BALANCE = BALANCE - " << to_string(executed_shares * max_price)
                              << " WHERE ACCOUNT_ID = " << to_string(buyer_account_id) << ";";
                        N.exec(query.str()); 
                    }
                    remaining_shares += executed_shares;

                }
                else {
                    // will not execute, then add this transaction to the database
                    executable = false;
                }
            }
            
        } while(executable);
        // show_results(C, N);

    }
    ans.resultMsg = "Success";
    N.exec("COMMIT;");
    cout << "look at this" << endl;
    // show_results(C, N);
    return ans;
}

// class trans_result{
//     public:
//         int flag; //0: order 1: query 2: cancel

//         string resultMsg;

//         int trans_id;

//         order_result order_res;

//         query_result query_res;

//         cancel_result cancel_res;
// };
// class query_result{
//     public:
//         //query
//         int query_flag; //0: open 1: canceled 2: executed

//         double open_shares;

//         cancel_info cancel_information;

//         vector<executed_info> executed_info_list;

// };
trans_result trans_query(const parsed_transaction & query, connection *C){
    trans_result ans;
    nontransaction N(*C);
    stringstream ss;

    ans.flag = 1;
    ans.trans_id = query.trans_id;

    ss.str("");
    ss << "SELECT * FROM TRANSACTIONS WHERE TRANS_ID = " << to_string(query.trans_id) << ";";
    result R(N.exec(ss.str()));

    if (R.empty()){
        ans.resultMsg = "Account to be queryed does not exist!";
        N.exec("COMMIT;");
        return ans;
    }

    long self_account_id = R[0]["ACCOUNT_ID"].as<long>();
    double self_shares = R[0]["SHARES"].as<double>();

    if (self_account_id != query.account_id) {
        ans.resultMsg = "No access to query other accounts";
        return ans;
    }

    string self_status = R[0]["STATUS"].as<string>();
    if (self_status == "open") {
        ans.query_res.query_flag = 0;
        ans.query_res.open_shares = self_shares;
    }
    else if (self_status == "canceled") {
        ans.query_res.query_flag = 1;
        ss.str("");
        ss << "SELECT * FROM CANCELED_TRANS WHERE TRANS_ID = " << to_string(query.trans_id) << ";";
        result R1(N.exec(ss.str()));

        ans.query_res.cancel_information.shares = R1[0]["SHARES"].as<double>();
        ans.query_res.cancel_information.time = R1[0]["TIME"].as<long>();
    }
    else {
        ans.query_res.query_flag = 2;
    }

    ss.str("");
    ss << "SELECT * FROM EXECUTED_TRANS WHERE TRANS_ID = " << to_string(query.trans_id) << ";";
    result R2(N.exec(ss.str()));

    for (result::iterator i = R2.begin(); i != R2.end(); i++) {
        executed_info tmp;
        tmp.price = i["PRICE"].as<double>();
        tmp.shares = i["SHARES"].as<double>();
        tmp.time = i["TIME"].as<long>();
        ans.query_res.executed_info_list.push_back(tmp);
    }

    ans.resultMsg = "Success";
    return ans;
}

// class cancel_result{
//     public:
//         cancel_info cancel_information;

//         vector<executed_info> executed_info_list;

// };
// class cancel_info{
//     public:
//         double shares;
//         long time;
// };
trans_result trans_cancel(const parsed_transaction & cancel, connection *C){
    trans_result ans;
    nontransaction N(*C);
    stringstream query;

    ans.flag = 2;
    ans.trans_id = cancel.trans_id;
    
    query.str("");
    query << "SELECT * FROM TRANSACTIONS WHERE TRANS_ID = " << to_string(cancel.trans_id) << "FOR UPDATE;";
    result R(N.exec(query.str()));

    if (R.empty()){
        ans.resultMsg = "Account to be canceled does not exist!";
        N.exec("COMMIT;");
        return ans;
    }

    double self_shares = R[0]["SHARES"].as<double>();
    double limit_price = R[0]["LIMIT_PRICE"].as<double>();
    long self_account_id = R[0]["ACCOUNT_ID"].as<long>();
    string self_sym_name = R[0]["SYMBOL_NAME"].as<string>();

    if (self_account_id != cancel.account_id) {
        ans.resultMsg = "No access to other accounts!";
        return ans;
    }
    
    ans.cancel_res.cancel_information.shares = self_shares;

    if (R[0]["STATUS"].as<string>() != "open") {
        ans.resultMsg = "Only open orders can be canceled!";
        N.exec("COMMIT;");
        return ans;
    }
    //update status in transactions to be "canceled"
    query.str("");
    query << "UPDATE TRANSACTIONS SET STATUS = " << N.quote("canceled") << " WHERE TRANS_ID = "
          << to_string(cancel.trans_id) << ";";
    N.exec(query.str());

    //insert into canceled_trans
    time_t now = time(0);
    long current_time = static_cast<long>(now);
    query.str("");
    query << "INSERT INTO CANCELED_TRANS (TRANS_ID, TIME, SHARES) "
          << "VALUES (" << cancel.trans_id << ", " << to_string(current_time) << ", " 
          << to_string(self_shares) << ");";
    N.exec(query.str());

    ans.cancel_res.cancel_information.time = current_time;

    //refund
    if (self_shares > 0) {
        //refund balance to accounts
        query.str("");
        query << "UPDATE ACCOUNTS SET BALANCE = BALANCE + " << to_string(self_shares * limit_price)
              << " WHERE ACCOUNT_ID = " << to_string(self_account_id) << ";";
        N.exec(query.str());
    }
    else {
        //refund shares to positions
        query.str("");
        query << "UPDATE POSITIONS SET SHARES = SHARES - " << to_string(self_shares)
              << " WHERE ACCOUNT_ID = " << to_string(self_account_id)
              << " AND SYMBOL_NAME = " << N.quote(self_sym_name) << ";";
        N.exec(query.str());
    }

    query.str("");
    query << "SELECT * FROM EXECUTED_TRANS WHERE TRANS_ID = " << to_string(cancel.trans_id) << ";";
    result R1(N.exec(query.str()));

    for (result::iterator i = R1.begin(); i != R1.end(); i++) {
        executed_info tmp;
        tmp.price = i["PRICE"].as<double>();
        tmp.shares = i["SHARES"].as<double>();
        tmp.time = i["TIME"].as<long>();
        ans.cancel_res.executed_info_list.push_back(tmp);
    }

    ans.resultMsg = "Success";

    return ans;
}

string parse_create_results(const vector<create_result> & crs){
    stringstream ss;
    ss << "<results>\n";

    for (auto cr: crs) {
        ss << parse_create_result(cr);
    }

    ss << "</results>\n";
    return ss.str();
}

string parse_trans_results(const vector<trans_result> & trs){
    stringstream ss;
    ss << "<results>\n";

    for (auto tr: trs) {
        ss << parse_trans_result(tr);
    }

    ss << "</results>\n";
    
    return ss.str();
}

string parse_create_result(const create_result & cr) {
    stringstream ss;
    // ss << '\t';
    if (cr.flag == 1) {
        ss << '\t';
        if (cr.resultMsg == "Success") {
            ss << "<created id=\"" << cr.account << "\"/>\n";
        }
        else {
            ss << "<error id=\"" << cr.account << "\" >" << cr.resultMsg << "</error>\n";
        }
    }
    else {
        for (int i = 0; i < cr.account_share_list.size(); i++) {
            if (cr.result_list[i] == "Success") {
                ss << '\t' << "<created sym=\"" << cr.symbol << "\" id=\"" << cr.account_share_list[i].first
                   << "\'/>\n";
            }
            else {
                ss << '\t' << "<error sym=\"" << cr.symbol << "\" id=\"" << cr.account_share_list[i].first
                   << "\'>" << cr.result_list[i] << "</error>\n";
            }
        }
    }

    return ss.str();
}

// class trans_result{
//     public:
//         int flag; //0: order 1: query 2: cancel

//         string resultMsg;

//         int trans_id;

//         order_result order_res;

//         query_result query_res;

//         cancel_result cancel_res;
// };
string parse_trans_result(const trans_result & tr){
    stringstream ss;
    //////////////to do
    ss << "\t";
    if (tr.flag == 0) {
        //parse order
        if (tr.resultMsg == "Success") {
            ss << "<opened sym=\"" << tr.order_res.sym_name << "\" amount=\"" << tr.order_res.amount
               << "\" limit=\"" << tr.order_res.limit_price << "\" id=\"" << tr.trans_id << "\"/>\n";
        }
        else {
            ss << "<error sym=\"" << tr.order_res.sym_name << "\" amount=\"" << tr.order_res.amount
               << "\" limit=\"" << tr.order_res.limit_price << "\">" << tr.resultMsg << "</error>\n";
        }
    }
    else if (tr.flag == 1) {
        //parse_query
        if (tr.resultMsg == "Success") {
            ss << "<status id=\"" << tr.trans_id << "\">\n";
            if (tr.query_res.query_flag == 0) {
                ss << "\t\t<open shares=\"" << tr.query_res.open_shares << "\"/>\n";
            }
            else if (tr.query_res.query_flag == 1) {
                ss << "\t\t<canceled shares=\"" << tr.query_res.cancel_information.shares
                   << "\" time=\"" <<tr.query_res.cancel_information.time << "\"/>\n";
            }

            for (auto item: tr.query_res.executed_info_list) {
                ss << "\t\t";
                ss << "<executed shares=\"" << item.shares
                   << "\" price=\"" << item.price
                   << "\" time=\"" << item.time
                   << "\"/>\n";
            }
            ss << "\t</status>\n"; 
        }
        else {
            ss << "<error id=\"" << tr.trans_id << "\">" 
               << tr.resultMsg << "</error>\n";
        }
    }
    else {
        //parse cancel
        if (tr.resultMsg == "Success") {
            ss << "<canceled id=\"" << tr.trans_id << "\">\n";
            ss << "\t\t" << "<canceled shares=\"" << tr.cancel_res.cancel_information.shares
               << "\" time=\"" << tr.cancel_res.cancel_information.time << "\"/>\n";
            for (auto item: tr.cancel_res.executed_info_list) {
                ss << "\t\t";
                ss << "<executed shares=\"" <<item.shares
                   << "\" price=\"" << item.price
                   << "\" time=\"" << item.time
                   << "\"/>\n";
            }
            ss << "\t</canceled>\n";  
        }
        else {
            ss << "<error id=\"" << tr.trans_id << "\">"
               << tr.resultMsg << "</error>\n";
        }
    }
    return ss.str();
};


// string parsed_trans_result(trans_result cr){

// }

void show_results(connection * C, nontransaction &N) {
    cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
    stringstream ss;
    ss << "SELECT * FROM ACCOUNTS;";
    cout << "ACCOUNT_ID " << "BALANCE" << endl;
    result R1(N.exec(ss.str()));
    for (result::iterator i = R1.begin(); i != R1.end(); i++) {
        cout << i[0].as<long>() << " " << i[1].as<double>() << endl;
    }
    cout << endl;

    ss.str("");
    ss << "SELECT * FROM SYMBOLS;";
    cout << "SYMBOL_ID " << "SYMBOL_NAME" << endl;
    result R2(N.exec(ss.str()));
    for (result::iterator i = R2.begin(); i != R2.end(); i++) {
        cout << i[0].as<int>() << " " << i[1].as<string>() << endl;
    }
    cout << endl;

    ss.str("");
    ss << "SELECT * FROM POSITIONS;";
    cout << "POSITION_ID " << "SHARES " << "ACCOUNT_ID " << "SYMBOL_NAME" << endl;
    result R3(N.exec(ss.str()));
    for (result::iterator i = R3.begin(); i != R3.end(); i++) {
        cout << i[0].as<int>() << " " << i[1].as<double>() << " " << i[2].as<long>() << " " << i[3].as<string>() << endl;
    }
    cout << endl;

    ss.str("");
    ss << "SELECT * FROM TRANSACTIONS;";
    cout << "TRANS_ID " << "STATUS " << "ACCOUNT_ID " << "SYMBOL_NAME " << "TIME " << "LIMIT_PRICE " << "SHARES" << endl;
    result R4(N.exec(ss.str()));
    for (result::iterator i = R4.begin(); i != R4.end(); i++) {
        cout << i[0].as<int>() << " " << i[1].as<string>() << " " << i[2].as<long>() << " " << i[3].as<string>() << " "
             << i[4].as<long>() << " " << i[5].as<double>() << " " << i[6].as<double>() << endl;
    }
    cout << endl;

    ss.str("");
    ss << "SELECT * FROM CANCELED_TRANS;";
    cout << "TRANS_ID " << "TIME " << "SHARES" << endl;
    result R5(N.exec(ss.str()));
    for (result::iterator i = R5.begin(); i != R5.end(); i++) {
        cout << i[0].as<int>() << " " << i[1].as<long>() << " " << i[2].as<double>() << endl;
             
    }
    cout << endl;

    ss.str("");
    ss << "SELECT * FROM EXECUTED_TRANS;";
    cout << "TRANS_ID " << "TIME " << "PRICE " << "SHARES" << endl;
    result R6(N.exec(ss.str()));
    for (result::iterator i = R6.begin(); i != R6.end(); i++) {
        cout << i[0].as<int>() << " " << i[1].as<long>() << " " << i[2].as<double>() << " " << i[3].as<double>() << endl;
    }
    cout << endl;

    cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << endl;
}