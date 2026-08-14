cmake_minimum_required(VERSION 3.25)

if(WIN32)
    add_subdirectory(SMBReader)
endif()

if(UNIX)
    add_subdirectory(LinuxTest)
endif()