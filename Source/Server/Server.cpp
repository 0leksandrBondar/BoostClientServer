#include "Server.h"
#include <spdlog/spdlog.h>

#include "Session.h"

Server::Server(boost::asio::io_context& io_context, short port)
    : _acceptor{ io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port) }
{
    accept();
}

void Server::registerClient(const std::string& name, std::shared_ptr<Session> session)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _clients[name] = session;
    spdlog::info("Registered client: {}", name);
}

void Server::unregisterClient(const std::string& name)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _clients.erase(name);
    spdlog::info("Unregistered client: {}", name);
}

std::shared_ptr<Session> Server::getClientSession(const std::string& name)
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _clients.find(name);
    return it != _clients.end() ? it->second : nullptr;
}

void Server::accept()
{
    _acceptor.async_accept(
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket)
        {
            if (!ec)
            {
                std::make_shared<Session>(std::move(socket), *this)->start();
            }
            accept();
        });
}
