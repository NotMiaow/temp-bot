cmake_minimum_required(VERSION 3.7)
project(mia_bot)

add_library(MIA_SOURCE
    ${ECS_SOURCE}
    miaBot.cpp
)
