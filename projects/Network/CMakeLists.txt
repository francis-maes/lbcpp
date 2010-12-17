INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

GENERATE_INTROSPECTION_CODE(NetworkLibrary.xml NetworkLibrary.cpp)

ADD_LIBRARY(network SHARED
  NetworkCommand.h
  NetworkCommand.cpp
  ClientServerWorkUnit.h
  ManagerServer.h
  ServerWorkUnit.h
  NetworkLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/NetworkLibrary.cpp
)
TARGET_LINK_LIBRARIES(network lbcpp-core)

ADD_EXECUTABLE(network-MainManagerServer MainManagerServer)
TARGET_LINK_LIBRARIES(network-MainManagerServer lbcpp-core)

ADD_EXECUTABLE(network-MainServerWorkUnit MainServerWorkUnit)
TARGET_LINK_LIBRARIES(network-MainServerWorkUnit lbcpp-core)
