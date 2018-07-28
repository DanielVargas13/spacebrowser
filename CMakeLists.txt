cmake_minimum_required(VERSION 3.0.2)

PROJECT(browser)

SET(CMAKE_BUILD_TYPE Debug)

set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Krzysztof Borowczyk")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

set(CMAKE_CXX_STANDARD 14)

FIND_PACKAGE(Qt5 COMPONENTS Core Widgets Quick WebEngine WebChannel REQUIRED)

ADD_DEFINITIONS(-Wall)

#ADD_DEFINITIONS(-DBOOST_ALL_DYN_LINK=1)
#SET(BOOST_ROOT "~/boost/")
#find_package(Boost 1.55.0 REQUIRED system thread log)
#if (Boost_FOUND)
#  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
#  LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
#endif(Boost_FOUND)
#INCLUDE_DIRECTORIES("~/mysql_connector/include")
#LINK_DIRECTORIES("~/mysql_connector/lib")


INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(./gnupgpp)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

SET(SOURCES
  main.cpp
  db/Tabs.cpp
  db/Sql.cpp
  db/Config.cpp
  db/ScriptBlock.cpp
  db/Passwords.cpp
  db/Keys.cpp
  misc/DebugHelpers.cpp
  ViewHandler.cpp
  ContentFilter.cpp
  BasicDownloader.cpp
  PrintHandler.cpp
  PasswordManager.cpp
  resources.qrc
  )

#SET(HEADERS

#  )

ADD_SUBDIRECTORY(gnupgpp)

ADD_EXECUTABLE(${PROJECT_NAME} ${SOURCES})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} Qt5::Core Qt5::Quick Qt5::Widgets
    Qt5::WebEngine gnupgpp pqxx pthread ${Boost_LIBRARIES}
    )

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

include(CPack)
