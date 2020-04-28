cmake_minimum_required(VERSION 3.16)
project(td_game_server)

set(ECS_SOURCE
    ${SYSTEMS_SOURCE}
    ${CMAKE_CURRENT_SOURCE_DIR}/eventManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ecs.cpp
    PARENT_SCOPE
)
