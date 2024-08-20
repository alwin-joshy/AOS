#
# Copyright 2024, UNSW Sydney
#
# This software may be distributed and modified according to the terms of
# the GNU General Public License version 2. Note that NO WARRANTY is provided.
# See "LICENSE_GPLv2.txt" for details.

cmake_minimum_required(VERSION 3.7.2)

project(libdebugger C)

add_library(debugger EXCLUDE_FROM_ALL src/debugger.c)
target_include_directories(debugger PUBLIC include)
target_link_libraries(debugger muslc sel4 picotcp utils sos_Config)


