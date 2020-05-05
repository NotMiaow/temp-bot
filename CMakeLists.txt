cmake_minimum_required(VERSION 3.7)
project(mia_bot)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost 1.72.0 REQUIRED system)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

INCLUDE_DIRECTORIES(
        ${Boost_INCLUDE_DIRS}
        ${discordpp_SOURCE_DIR}
        ${discordpp-rest-beast_SOURCE_DIR}
        ${discordpp-websocket-beast_SOURCE_DIR}
        ${discordpp-plugin-overload_SOURCE_DIR}
        ${discordpp-plugin-responder_SOURCE_DIR}
        ${json_SOURCE_DIR}
        ${OPENSSL_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/lib
        ${PROJECT_SOURCE_DIR}/_lib/basic_lib
        ${PROJECT_SOURCE_DIR}/_lib/queue
        ${PROJECT_SOURCE_DIR}/_lib/component_array
        ${PROJECT_SOURCE_DIR}/_headers/ecs/components
        ${PROJECT_SOURCE_DIR}/_headers/ecs/systems
        ${PROJECT_SOURCE_DIR}/_headers/ecs
        ${PROJECT_SOURCE_DIR}/_headers/events
        ${PROJECT_SOURCE_DIR}/_headers
)

add_subdirectory(lib/discordpp)          # discordpp
add_subdirectory(lib/rest-beast)         # discordpp-rest-beast
add_subdirectory(lib/websocket-beast)    # discordpp-websocket-beast
add_subdirectory(lib/plugin-overload)    # discordpp-plugin-overload
add_subdirectory(lib/plugin-responder)   # discordpp-plugin-new
add_subdirectory(_source/ecs/systems)
add_subdirectory(_source/ecs)
add_subdirectory(_source)

#multithreading
set(THREADS_PREFER_PTHREAD_FLAG ON)
#add executable
add_executable(${PROJECT_NAME} main.cc)

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
        ${Boost_LIBRARIES}
        discordpp
        discordpp-rest-beast
        discordpp-websocket-beast
        Threads::Threads
        crypto
        ssl
        MIA_SOURCE
)
