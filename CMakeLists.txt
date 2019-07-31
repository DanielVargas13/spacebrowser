cmake_minimum_required(VERSION 3.0.2)

PROJECT(browser)

SET(CMAKE_BUILD_TYPE Debug)

set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Krzysztof Borowczyk")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

FIND_PACKAGE(Qt5 COMPONENTS Core Quick QuickControls2 Sql Test WebEngine WebChannel Widgets REQUIRED)

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
  db/Tabs2.cpp
  db/DbClient.cpp
  db/DbGroup.cpp
  db/Config2.cpp
  db/ScriptBlock2.cpp
  db/Passwords2.cpp
  db/Keys2.cpp
  db/Backend.cpp
  misc/DebugHelpers.cpp
  Tab.cpp
  ViewHandler.cpp
  ContentFilter.cpp
  BasicDownloader.cpp
  PrintHandler.cpp
  PasswordManager.cpp
  TabModel.cpp
  TabView.cpp
  TreeToListProxyModel.cpp
  TreeModel2.cpp
  resources.qrc
  )

#SET(HEADERS

#  )

ADD_SUBDIRECTORY(gnupgpp)

ADD_EXECUTABLE(${PROJECT_NAME} ${SOURCES})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} Qt5::Core Qt5::Quick Qt5::QuickControls2
  Qt5::Sql Qt5::WebEngine Qt5::Widgets gnupgpp pthread
  )

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

include(CPack)

# Tests TEST_BUILD
LINK_DIRECTORIES(gtest/lib)
LINK_DIRECTORIES(gtest/lib64)

ADD_EXECUTABLE(ViewHandler_test
  test/ViewHandler_test.cpp
  test/ViewHandler_test_mock.h
  ViewHandler.cpp
  TreeToListProxyModel.cpp
  TreeModel2.cpp
  Tab.cpp)
TARGET_INCLUDE_DIRECTORIES(ViewHandler_test PUBLIC gtest/include)
#TARGET_LINK_DIRECTORIES(ViewHandler_test PUBLIC gtest/lib)
TARGET_COMPILE_DEFINITIONS(ViewHandler_test PUBLIC TEST_BUILD)
TARGET_LINK_LIBRARIES(ViewHandler_test gtest gmock Qt5::Core Qt5::Quick Qt5::Test pthread)

enable_testing()
ADD_TEST(NAME ViewHandler_test COMMAND ViewHandler_test)
