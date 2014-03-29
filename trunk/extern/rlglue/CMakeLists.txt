INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

SET(RLGLUE_SOURCES
  RL_network.c
  RLStruct_util.c
  TaskSpec_Parser.c
)

IF (WIN32)
  SET(RLGLUE_LIBRARIES
    ws2_32.lib
    rpcrt4.lib
  )
ENDIF (WIN32)
  
    

ADD_LIBRARY(rlglue ${RLGLUE_SOURCES})
TARGET_LINK_LIBRARIES(rlglue ${RLGLUE_LIBRARIES})
