cmake_minimum_required(VERSION 3.16...3.29)

project(Server)

set(Source
        Main.cpp
        Server.h
        Server.cpp
        Session.cpp
        Session.h
)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")

add_executable(${PROJECT_NAME} ${Source})

target_link_libraries(${PROJECT_NAME} PUBLIC
        Boost::asio
        spdlog::spdlog
        nlohmann_json::nlohmann_json
)



