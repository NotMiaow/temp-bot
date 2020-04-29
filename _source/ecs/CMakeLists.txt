cmake_minimum_required(VERSION 3.7)
project(mia_bot)

set(ECS_SOURCE
    ${SYSTEMS_SOURCE}
    ${CMAKE_CURRENT_SOURCE_DIR}/eventManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ecs.cpp
    PARENT_SCOPE
)
