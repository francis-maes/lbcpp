INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
)

GENERATE_INTROSPECTION_CODE(LuaLibrary.xml LuaLibrary.cpp)

SET(LBCPP_LUA_CPP_SOURCES
  precompiled.h
  precompiled.cpp
  Node.h
  Expression.h
  Statement.h
  Node.cpp
  Visitor.h
  Rewriter.h
  Scope.h
  ScopeVisitor.h
  Scope.cpp
  PrettyPrinterVisitor.h
  DerivableRewriter.h
  LuaInterpreter.h
  LuaSandBox.h
  LuaLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/LuaLibrary.cpp
)

FILE(GLOB SUBLUA_MAIN_SOURCES lib/*.lua)
FILE(GLOB SUBLUA_LANGUAGE_SOURCES lib/Language/*.lua)

SET(LBCPP_LUA_SOURCES
  ${LBCPP_LUA_CPP_SOURCES}
  ${SUBLUA_MAIN_SOURCES}
  ${SUBLUA_LANGUAGE_SOURCES}
)

SOURCE_GROUP(Cpp FILES ${LBCPP_LUA_CPP_SOURCES})
SOURCE_GROUP(Main FILES ${SUBLUA_MAIN_SOURCES})
SOURCE_GROUP(Language FILES ${SUBLUA_LANGUAGE_SOURCES})

GENERATE_LBCPP_PRECOMPILED(precompiled LBCPP_LUA_SOURCES)

ADD_LIBRARY(lbcpp-lua${NAME_FILE_SUFFIX} SHARED ${LBCPP_LUA_SOURCES})
TARGET_LINK_LIBRARIES(lbcpp-lua${NAME_FILE_SUFFIX} lbcpp-core${NAME_FILE_SUFFIX})

