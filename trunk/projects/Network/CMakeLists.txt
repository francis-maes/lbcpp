INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

GENERATE_INTROSPECTION_CODE(NetworkLibrary.xml NetworkLibrary.cpp)

ADD_LIBRARY(network SHARED
  NetworkCommand.h
  NetworkCommand.cpp
  NetworkContext.h
  NetworkContext.cpp
  ClientServerWorkUnit.h
  ManagerServer.h
  ServerWorkUnit.h
  NetworkLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/NetworkLibrary.cpp
)
TARGET_LINK_LIBRARIES(network lbcpp-core)
