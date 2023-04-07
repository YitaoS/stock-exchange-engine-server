#ifndef PROXY_SERVER
#define PROXY_SERVER
#include "session.hpp"

namespace beast = boost::beast;  // from <boost/beast.hpp>
namespace net = boost::asio;     // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>

class listener : public std::enable_shared_from_this<listener> {
  net::io_context & ioc_;
  tcp::acceptor acceptor_;
  std::mutex & my_mutex;
  int num_thread;

  void fail(beast::error_code ec, char const * what) {
    std::cerr << what << ": " << ec.message() << "\n";
  }

 public:
  listener(net::io_context & ioc,
           tcp::endpoint endpoint,
           std::mutex & my_mutex);

  // Start accepting incoming connections
  void run() { do_accept(); }

 private:
  void do_accept();

  void on_accept(beast::error_code ec, tcp::socket socket);
};
#endif  //PROXY_SERVER