#pragma once

#include <string>
#include <functional>
#include <memory>
#include <future>
#include <cstdint>
#include <type_traits>
#include <thread>

#include "settings.h"
#include "logging.h" 
#include "http_server.h"
#include "utility.h"
#include "routing.h"
#include "middleware_context.h"
#include "http_request.h"

#define CROW_ROUTE(app, url) app.route<crow::black_magic::get_parameter_tag(url)>(url)

namespace crow
{
    template <typename ... Middlewares>
    class Crow
    {
    public:
        using self_t = Crow;
        using server_t = Server<Crow, Middlewares...>;
        
        Crow()
        {}

        void handle(const request& req, response& res)
        {
            router_.handle(req, res);
        }

        template <uint64_t Tag>
        auto route(std::string&& rule)
            -> typename std::result_of<decltype(&Router::new_rule_tagged<Tag>)(Router, std::string&&)>::type
        {
            return router_.new_rule_tagged<Tag>(std::move(rule));
        }
        
        template

        self_t& port(std::uint16_t port)
        {
            port_ = port;
            return *this;
        }

        void validate()
        {
            router_.validate();
        }

        self_t& start()
        {
            validate();
            server_.reset(new server_t(this, port_));
            server_->start();
            return *this;
        }
        
        void stop() {
            if(server_) server_->stop();
        }
        
        void join() {
            if(server_) {
                server_->join();
                server_.reset();
            }
        }
        
        void debug_print()
        {
            router_.debug_print();
        }

        // middleware
        using context_t = detail::context<Middlewares...>;
        template <typename T>
        typename T::context& get_context(const request& req)
        {
            static_assert(black_magic::contains<T, Middlewares...>::value, "App doesn't have the specified middleware type.");
            auto& ctx = *reinterpret_cast<context_t*>(req.middleware_context);
            return ctx.template get<T>();
        }

    private:
        uint16_t port_ = 80;

        Router router_;
        std::unique_ptr<server_t> server_;
    };
    template <typename ... Middlewares>
    using App = Crow<Middlewares...>;
    using SimpleApp = Crow<>;
};

