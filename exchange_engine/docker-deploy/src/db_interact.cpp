#include "db_interact.hpp"
#include "engine_exception.hpp"
using namespace std;
void connectDatabase(string db_parameter,connection ** C){
    *C = new connection(db_parameter);
    if ((*C)->is_open()) {
      cout << "Opened database successfully: " << (*C)->dbname() << endl;
    }
    else {
      cout << "Can't open database" << endl;
      throw engine_exception("Error when opening database","");
    }
    return;
}

void drop_tables(connection * C) {
  pqxx::work txn{*C};

  //Drop tables
  txn.exec("DROP TABLE IF EXISTS EXECUTED_TRANS");
  txn.exec("DROP TABLE IF EXISTS CANCELED_TRANS");
  txn.exec("DROP TABLE IF EXISTS TRANSACTIONS");
  txn.exec("DROP TABLE IF EXISTS POSITIONS");
  txn.exec("DROP TABLE IF EXISTS ACCOUNTS");
  txn.exec("DROP TABLE IF EXISTS SYMBOLS");
  txn.commit();
  std::cout << "Tables dropped successfully" << std::endl;
}

std::vector<const char *> get_tables_create_sql() {
  const char * sql_create_table_symbols = "CREATE TABLE SYMBOLS ( "
                                          "SYMBOL_ID SERIAL PRIMARY KEY NOT NULL,"
                                          "SYMBOL_NAME VARCHAR(256) UNIQUE NOT NULL"
                                          ");";

  const char * sql_create_table_accounts =
      "CREATE TABLE IF NOT EXISTS ACCOUNTS ("
      "ACCOUNT_ID BIGINT PRIMARY KEY NOT NULL,"
      "BALANCE DOUBLE PRECISION NOT NULL"
      ");";

  const char * sql_create_table_positions = "CREATE TABLE IF NOT EXISTS POSITIONS ("
                                            "POSITION_ID SERIAL PRIMARY KEY NOT NULL,"
                                            "SHARES DOUBLE PRECISION NOT NULL,"
                                            "ACCOUNT_ID BIGINT NOT NULL,"
                                            "SYMBOL_NAME VARCHAR(256) NOT NULL,"
                                            "FOREIGN KEY (ACCOUNT_ID) REFERENCES ACCOUNTS(ACCOUNT_ID) ON DELETE SET NULL ON UPDATE CASCADE,"
                                            "FOREIGN KEY (SYMBOL_NAME) REFERENCES SYMBOLS(SYMBOL_NAME) ON DELETE SET NULL ON UPDATE CASCADE,"
                                            "CONSTRAINT account_symbol_constraint UNIQUE (ACCOUNT_ID, SYMBOL_NAME)"
                                            ");";

  const char * sql_create_table_transactions = "CREATE TABLE IF NOT EXISTS TRANSACTIONS ("
                                         "TRANS_ID SERIAL PRIMARY KEY NOT NULL,"
                                         "STATUS VARCHAR(256) NOT NULL,"
                                         "ACCOUNT_ID BIGINT NOT NULL,"
                                         "SYMBOL_NAME VARCHAR(256) NOT NULL,"
                                         "TIME BIGINT NOT NULL,"
                                         "LIMIT_PRICE DOUBLE PRECISION NOT NULL,"
                                         "SHARES DOUBLE PRECISION NOT NULL,"
                                         "FOREIGN KEY (ACCOUNT_ID) REFERENCES ACCOUNTS(ACCOUNT_ID) ON DELETE SET NULL ON UPDATE CASCADE,"
                                         "FOREIGN KEY (SYMBOL_NAME) REFERENCES SYMBOLS(SYMBOL_NAME) ON DELETE SET NULL ON UPDATE CASCADE"
                                         ");";
  
  const char * sql_create_table_cancel = "CREATE TABLE IF NOT EXISTS CANCELED_TRANS ("
                                         "TRANS_ID INT NOT NULL,"
                                         "TIME BIGINT NOT NULL,"
                                         "SHARES DOUBLE PRECISION NOT NULL,"
                                         "FOREIGN KEY (TRANS_ID) REFERENCES TRANSACTIONS(TRANS_ID) ON DELETE SET NULL ON UPDATE CASCADE"
                                         ");";
  
  const char * sql_create_table_execute = "CREATE TABLE IF NOT EXISTS EXECUTED_TRANS ("
                                          "TRANS_ID INT NOT NULL,"
                                          "TIME BIGINT NOT NULL,"
                                          "PRICE DOUBLE PRECISION NOT NULL,"
                                          "SHARES DOUBLE PRECISION NOT NULL,"
                                          "FOREIGN KEY (TRANS_ID) REFERENCES TRANSACTIONS(TRANS_ID) ON DELETE SET NULL ON UPDATE CASCADE"
                                          ");";
  std::vector<const char *> res;
  res.push_back(sql_create_table_symbols);
  res.push_back(sql_create_table_accounts);
  res.push_back(sql_create_table_positions);
  res.push_back(sql_create_table_transactions);
  res.push_back(sql_create_table_cancel);
  res.push_back(sql_create_table_execute);
  return res;
}

void create_tables(connection * C, std::vector<const char *> tables_create_sql) {
  pqxx::work txn_create_table{*C};
  for (const char * table_create_sql : tables_create_sql) {
    txn_create_table.exec(table_create_sql);
  }
  txn_create_table.commit();

  std::cout << "Tables created successfully" << std::endl;
}

void executeQuery(string query, connection *C){
    work trans(*C);
    trans.exec(query);
    trans.commit();
}
