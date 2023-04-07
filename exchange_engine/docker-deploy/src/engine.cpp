#include "stock_server.hpp"
#include "db_interact.hpp"
#include <fstream>
#include <iostream>
#include <pqxx/pqxx>
#include <sstream>

std::mutex global_mutex;

int main(int argc, char * argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: ./engine <address> <port> <threads>\n"
              << "Example:\n"
              << "./engine 0.0.0.0 8080 1\n";
    return EXIT_FAILURE;
  }

  //Allocate & initialize a Postgres connection object
  connection * C;

  try {
    //Establish a connection to the database
    //Parameters: database name, user name, user password
    connectDatabase(DB_PARAMETER,&C);
    drop_tables(C);
    create_tables(C, get_tables_create_sql());
    //initialize_tables(C);
  }
  catch (const std::exception & e) {
    cerr << e.what() << std::endl;
    return 1;
  }

  C->disconnect();
  // //TODO: create PLAYER, TEAM, STATE, and COLOR tables in the ACC_BBALL database
  // //      load each table with rows from the provided source txt files

  // exercise(C);

  //////////////////
  
  auto const address = net::ip::make_address(argv[1]);
  auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
  auto const threads = std::max<int>(1, std::atoi(argv[3]));

  // The io_context is required for all I/O
  net::io_context ioc{threads};

  //create a stream for log writing
  // Create and launch a listening port
  std::make_shared<listener>(
      ioc, tcp::endpoint{address, port}, global_mutex)
      ->run();

  // Run the I/O service on the requested number of threads
  std::vector<std::thread> v;
  v.reserve(threads - 1);
  for (auto i = threads - 1; i > 0; --i)
    v.emplace_back([&ioc] { ioc.run(); });
  ioc.run();
  return EXIT_SUCCESS;
}