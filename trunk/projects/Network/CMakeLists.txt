INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

GENERATE_INTROSPECTION_CODE(NetworkClasses.xml NetworkClasses.cpp)

ADD_LIBRARY(network SHARED
  Command.h
  Command.cpp
  ClientServerWorkUnit.h
  NetworkClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/NetworkClasses.cpp
)
TARGET_LINK_LIBRARIES(network lbcpp-core)

ADD_EXECUTABLE(network-MainClientWorkUnit MainClientWorkUnit.cpp)
TARGET_LINK_LIBRARIES(network-MainClientWorkUnit lbcpp-core)

