cmake_minimum_required(VERSION 3.9)
project(bvd_agent_based_model)

get_filename_component(PARENTDIR ${CMAKE_BINARY_DIR} DIRECTORY)
set(EXECUTABLE_OUTPUT_PATH ${PARENTDIR}/build)
message(STATUS "Cmake EXECUTABLE OUTPUT DIR is ${EXECUTABLE_OUTPUT_PATH}")

#set(CMAKE_VERBOSE_MAKEFILE TRUE)

find_package(GSL REQUIRED)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

#set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_BUILD_TYPE RELEASE)

set(CMAKE_C_FLAGS_DEBUG "-Wall -g -pedantic -fopenmp -D_SUPPRESS_OUTPUT_ -D_DEBUG_ -D_FARM_INITIALIZER_DEBUG_ -D_OUTPUT_DEBUG_ -D_FARM_MANAGER_DEBUG_ -D_INITIALIZER_DEBUG_ -D_HERD_DEBUG_ -D_MARKET_DEBUG_")
set(CMAKE_C_FLAGS_RELEASE "-Wall -g -pedantic -fopenmp -D_FARM_INITIALIZER_DEBUG_ -D_SUPPRESS_MINOR_OUTPUT_")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}" )
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}" )

add_executable(bvd_agent_based_model
        include/AdvancedOutput.h
        include/BVD_Random_Number_Generator.h
        include/BVDContainmentStrategy.h
        include/BVDOptions.h
        include/BVDSettings.h
        include/CommandlineOptions.h
        include/Cow.h
        include/CowWellFarm.h
        include/CowWellFarmManager.h
        include/CSV_Reader.h
        include/CSVFileHandler.h
        include/Events.h
        include/Farm.h
        include/FarmManager.h
        include/FileHandler.h
        include/HDF5Handler.h
        include/Herd.h
        include/Initializer.h
        include/Market.h
        include/Model_Constants.h
        include/Output.h
        include/OutputTable.h
        include/Simple_One_Herd_Farm.h
        include/SimpleFarmManager.h
        include/Slaughterhouse.h
        include/SlaughterHouseManager.h
        include/Small_One_Herd_Farm.h
        include/SmallFarmManager.h
        include/SQLiteHandler.h
        include/System.h
        include/TableBasedOutput.h
        include/TradeFilter.h
        include/Utilities.h
        projectImports/Catch/catch.hpp
        projectImports/FakeIt/fakeit.hpp
        projectImports/inih/cpp/INIReader.cpp
        projectImports/inih/cpp/INIReader.h
        projectImports/inih/ini.c
        projectImports/inih/ini.h
        src/AdvancedOutput.cpp
        src/Breeding_module_parameter_gnu.cpp
        src/BVD_Random_Number_Generator.cpp
        src/BVDContainmentStrategy.cpp
        src/BVDOptions.cpp
        src/BVDSettings.cpp
        src/CommandlineOptions.cpp
        src/Cow.cpp
        src/CowWellFarm.cpp
        src/CowWellFarmManager.cpp
        src/CSVFileHandler.cpp
        src/Events.cpp
        src/Farm.cpp
        src/FarmManager.cpp
        src/FileHandler.cpp
        src/HDF5Handler.cpp
        src/Herd.cpp
        src/Initializer.cpp
        src/Market.cpp
        src/Output.cpp
        src/OutputTable.cpp
        src/programm.cpp
        src/Simple_One_Herd_Farm.cpp
        src/SimpleFarmManager.cpp
        src/Slaughterhouse.cpp
        src/SlaughterHouseManager.cpp
        src/Small_One_Herd_Farm.cpp
        src/SmallFarmManager.cpp
        src/SQLiteHandler.cpp
        src/System.cpp
        src/TableBasedOutput.cpp
        src/TradeFilter.cpp
        src/Utilities.cpp
        config.h)

#tests/test_cow.cpp
#tests/test_csv_reader.cpp
#tests/test_event.cpp
#tests/test_herd.cpp
#tests/test_initializer.cpp
#tests/test_market.cpp
#tests/test_output.cpp
#tests/test_system.cpp
#tests/tests.cpp

include_directories(include
        /usr/include/hdf5/serial
        projectImports
        projectImports/inih
        projectImports/inih/cpp
        .)

# This needs to account for platform-specific names of libraries, like HDF5
find_program(UNAME uname)
execute_process(COMMAND ${UNAME} -s
        OUTPUT_VARIABLE OS_IDENTIFICATION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
find_program(LSB_RELEASE lsb_release)
execute_process(COMMAND ${LSB_RELEASE} -is
        OUTPUT_VARIABLE LSB_RELEASE_ID_SHORT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
message(STATUS "System identified as ${OS_IDENTIFICATION}, version ${LSB_RELEASE_ID_SHORT}")


# TODO check the GSL package for Mac, I'm not sure it uses the custom path for homebrew/macports

if(OS_IDENTIFICATION STREQUAL "Darwin")
    find_library(HDF5 "hdf5" PATHS /opt/local/lib)
    find_library(HDF5_HL "hdf5_hl" PATHS /opt/local/lib)
    if(NOT HDF5 OR NOT HDF5_HL)
        message(FATAL_ERROR "hdf5 library not found")
    endif()

    find_library(M "m" PATHS /opt/local/lib)

    find_library(SQLITE3 "sqlite3" PATHS /opt/local/lib)
    if(NOT SQLITE3)
        message(FATAL_ERROR "sqlite3 library not found")
    endif()
else()
    if(LSB_RELEASE_ID_SHORT STREQUAL "Ubuntu" OR LSB_RELEASE_ID_SHORT STREQUAL "Debian")
    find_library(HDF5 "hdf5_serial")
    find_library(HDF5_HL "hdf5_serial_hl")
    else()
    find_library(HDF5 "hdf5")
    find_library(HDF5_HL "hdf5_hl")
    endif()
    if(NOT HDF5 OR NOT HDF5_HL)
    message(FATAL_ERROR "hdf5 library not found")
    endif()

    find_library(M "m")

    find_library(SQLITE3 "sqlite3")
    if(NOT SQLITE3)
    message(FATAL_ERROR "sqlite3 library not found")
    endif()
endif()

target_link_libraries(bvd_agent_based_model GSL::gsl GSL::gslcblas ${M} ${SQLITE3} ${HDF5} ${HDF5_HL})