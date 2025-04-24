cmake_minimum_required(VERSION 3.16...3.29)

project(Client)

set(Source
        Main.cpp
        Client.h
        Client.cpp
)

add_executable(${PROJECT_NAME} ${Source})

target_link_libraries(${PROJECT_NAME} PUBLIC
        Boost::asio
        spdlog::spdlog
        nlohmann_json::nlohmann_json
)


