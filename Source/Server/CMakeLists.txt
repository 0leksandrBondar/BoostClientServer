cmake_minimum_required(VERSION 3.16...3.29)

project(Server)

set(Source
        Main.cpp
)

add_executable(${PROJECT_NAME} ${Source})

target_link_libraries(${PROJECT_NAME} PUBLIC
        Boost::asio
)



