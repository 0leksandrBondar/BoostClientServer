#include "Server.h"
#include "spdlog/spdlog.h"

#include <boost/asio.hpp>

int main()
{
    try
    {
        boost::asio::io_context io_context;
        Server server(io_context, 12345);

        std::vector<std::thread> threads;
        int thread_count = std::thread::hardware_concurrency();
        for (int i = 0; i < thread_count; ++i)
        {
            threads.emplace_back([&io_context]() { io_context.run(); });
        }

        for (auto& t : threads)
        {
            t.join();
        }
    }
    catch (const std::exception& e)
    {
        spdlog::error("Server crashed: {}", e.what());
    }

    return 0;
}
