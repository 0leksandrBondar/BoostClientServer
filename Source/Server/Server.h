#pragma once

#include <boost/asio.hpp>

class Session;

class Server
{
public:
    Server(boost::asio::io_context& io_context, short port);

    void registerClient(const std::string& name, std::shared_ptr<Session> session);
    void unregisterClient(const std::string& name);
    std::shared_ptr<Session> getClientSession(const std::string& name);

private:
    void accept();

private:
    std::mutex _mutex;
    boost::asio::ip::tcp::acceptor _acceptor;
    std::unordered_map<std::string, std::shared_ptr<Session>> _clients;
};