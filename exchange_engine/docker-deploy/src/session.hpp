#ifndef SESSION
#define SESSION
#include <boost/asio.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/bind/bind.hpp>
#include <boost/config.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <unistd.h>
#include <utility>
#include <vector>

#include "db_interact.hpp"
#include "parser.hpp"
#include "handle_command.hpp"
#include "pugixml.hpp"

using tcp = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>

class session : public std::enable_shared_from_this<session> {
  int num_thread;
  tcp::socket socket_;
  enum { max_length = 65536 };
  char data_[max_length];
  char resp_[max_length];
  std::mutex & mutex;
  string response;

 public:
  // Take ownership of the stream
  session(tcp::socket && socket,
          std::mutex & mutex,int num_thread) :
      socket_(std::move(socket)),
      mutex(mutex),
      num_thread(num_thread){}

  void run();

 private:
  void do_read()
  {
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
          if (!ec)
          {
            data_[length] = '\0';
            //handle_order();
            // cout << "-------------------------" << endl;
            // cout << "thread_num: " << num_thread << endl;
            // cout << data_ << endl;
            // cout << "-------------------------" << endl;
            command cmd = parse(data_);

            connection * C;

            try {
              //Establish a connection to the database
              //Parameters: database name, user name, user password
              connectDatabase(DB_PARAMETER,&C);
              //initialize_tables(C);
            }
            catch (const std::exception & e) {
              cerr << e.what() << std::endl;
            }
            // if (res.flag == 0) {
            //   for (const auto& pc : res.creates) {
            //       std::cout << "Parsed create element:" << std::endl;
            //       std::cout << "Flag: " << pc.flag << std::endl;
            //       if (pc.flag == 1) {
            //           std::cout << "Account ID: " << pc.account_id << std::endl;
            //           std::cout << "Balance: " << pc.balance << std::endl;
            //       } else if (pc.flag == 0) {
            //           std::cout << "Symbol name: " << pc.sym_name << std::endl;
            //           for (int i = 0; i < pc.account_share_list.size(); i++) {
            //               cout << "Account ID: " << pc.account_share_list[i].first << endl;
            //               cout << "Shares: " << pc.account_share_list[i].second << endl;
            //           }
            //       }
            //       std::cout << std::endl;
            //   }
            // }
            // else if (res.flag == 1) {
            //   for (const auto& transaction : res.transactions) {
            //     cout << "Parsed transaction:" << endl;
            //     cout << "  Flag: " << transaction.flag << endl;
            //     cout << "  Account ID: " << transaction.account_id << endl;
            //     if (transaction.flag == 0) {
            //         cout << "  Symbol: " << transaction.sym_name << endl;
            //         cout << "  Amount: " << transaction.amount << endl;
            //         cout << "  Limit: " << transaction.limit << endl;
            //     } else if (transaction.flag == 1 || transaction.flag == 2) {
            //         cout << "  Trans ID: " << transaction.trans_id << endl;
            //     }
            //   }
            // }

            response = handle_command(cmd, C);
            // cout << "******************************" << endl;
            // cout << response << endl;
            // cout << "******************************" << endl;
            
            C->disconnect();

            do_write(response.length());
                        
          }else{
            socket_.close();
          }
        });
  }

  // void handle_order(){
    


  //   send_order();
  // }

  void do_write(std::size_t length)
  {
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(response, length),
        [this, self](boost::system::error_code ec, std::size_t /*length*/)
        {
          socket_.close();
        });
  }
};
#endif  //SESSION