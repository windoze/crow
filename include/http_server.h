#pragma once

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio.hpp>
#include <cstdint>
#include <atomic>
#include <future>
#include <vector>

#include <memory>

#include "http_connection.h"
#include "datetime.h"
#include "logging.h"
#include "dumb_timer_queue.h"

namespace crow
{
    using namespace boost;
    using tcp = asio::ip::tcp;
    
    template <typename Handler, typename ... Middlewares>
    class Server
    {
        typedef tcp::socket Socket;
    public:
        Server(Handler* handler, uint16_t port)
        : acceptor_(fibio::asio::get_io_service(), tcp::endpoint(asio::ip::address(), port))
        , handler_(handler)
        , port_(port)
        {}
        void start() {
            svc_.reset(new fibio::fiber(&Server::do_accept, this));
        }
        void stop() {
            acceptor_.close();
        }
        void join() {
            if(svc_) {
                svc_->join();
                svc_.reset();
            }
        }
    private:
        void do_accept() {
            boost::system::error_code ec;
            while (!ec) {
                auto p = new Connection<Socket, Handler, Middlewares...>(handler_, server_name_, middlewares_);
                acceptor_.async_accept(p->socket(), fibio::asio::yield[ec]);
                if (!ec) {
                    fibio::fiber(&Connection<Socket, Handler, Middlewares...>::start, p).detach();
                }
            }
        }
        
        tcp::acceptor acceptor_;
        Handler* handler_;
        std::string server_name_ = "FiberizedCrow/0.1";
        uint16_t port_;
        std::tuple<Middlewares...> middlewares_;
        std::unique_ptr<fibio::fiber> svc_;
    };
}
