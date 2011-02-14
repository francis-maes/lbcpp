INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
)

ADD_EXECUTABLE(unzipper
  Main.cpp
)

TARGET_LINK_LIBRARIES(unzipper
  lbcpp-core
)

