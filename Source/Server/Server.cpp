#include "Server.h"

#include "Session.h"

Server::Server(boost::asio::io_context& io_context, short port)
    : _acceptor{ io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port) }
{
    accept();
}

void Server::accept()
{
    _acceptor.async_accept(
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket)
        {
            if (!ec)
            {
                std::make_shared<Session>(std::move(socket))->start();
            }
            accept();
        });
}
