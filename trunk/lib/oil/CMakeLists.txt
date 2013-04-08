INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${LBCpp_SOURCE_DIR}/shark/include
  ${CMAKE_CURRENT_SOURCE_DIR}
)

GENERATE_INTROSPECTION_CODE(LBCppLibrary.xml LBCppLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Core/CoreLibrary.xml CoreLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Core/Loader/CoreLoaderLibrary.xml CoreLoaderLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Execution/ExecutionLibrary.xml ExecutionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Execution/Callback/ExecutionCallbackLibrary.xml ExecutionCallbackLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Execution/Context/ExecutionContextLibrary.xml ExecutionContextLibrary.cpp)

IF(LBCPP_USER_INTERFACE)
  GENERATE_INTROSPECTION_CODE(UserInterface/UserInterfaceLibrary.xml UserInterfaceLibrary.cpp)
  GENERATE_INTROSPECTION_CODE(UserInterface/Component/UserInterfaceComponentLibrary.xml UserInterfaceComponentLibrary.cpp)
  GENERATE_INTROSPECTION_CODE(UserInterface/TreeView/UserInterfaceTreeViewLibrary.xml UserInterfaceTreeViewLibrary.cpp)
  GENERATE_INTROSPECTION_CODE(UserInterface/Plot/UserInterfacePlotLibrary.xml UserInterfacePlotLibrary.cpp)
ENDIF(LBCPP_USER_INTERFACE)

GENERATE_INTROSPECTION_CODE(Lua/LuaLibrary.xml LuaLibrary.cpp)

SET(OIL_INCLUDES ../../include/oil)

#### Core ####

SET(OIL_CORE_SOURCES
  ${OIL_INCLUDES}/Core/Utilities.h  
  Core/Utilities.cpp
  ${OIL_INCLUDES}/Core/ReferenceCountedObject.h
  ${OIL_INCLUDES}/Core/predeclarations.h
  ${OIL_INCLUDES}/Core/Object.h
  Core/Object.cpp
  ${OIL_INCLUDES}/Core/Boolean.h
  Core/Boolean.cpp
  ${OIL_INCLUDES}/Core/Integer.h
  Core/Integer.cpp
  ${OIL_INCLUDES}/Core/Double.h
  Core/Double.cpp
  ${OIL_INCLUDES}/Core/String.h
  Core/String.cpp
  ${OIL_INCLUDES}/Core/ClassManager.h
  Core/ClassManager.cpp
  ${OIL_INCLUDES}/Core/Signature.h
  ${OIL_INCLUDES}/Core/Class.h
  Core/Class.cpp
  ${OIL_INCLUDES}/Core/Enumeration.h
  Core/Enumeration.cpp
  ${OIL_INCLUDES}/Core/DefaultClass.h
  Core/DefaultClass.cpp
  ${OIL_INCLUDES}/Core/TemplateClass.h
  Core/TemplateClass.cpp
  ${OIL_INCLUDES}/Core/Library.h
  Core/Library.cpp
  ${OIL_INCLUDES}/Core/Pair.h
  ${OIL_INCLUDES}/Core/Vector.h
  Core/Vector.cpp
  ${OIL_INCLUDES}/Core/Table.h
  Core/Table.cpp
  ${OIL_INCLUDES}/Core/Loader.h
  Core/Loader.cpp
  ${OIL_INCLUDES}/Core/RandomGenerator.h
  Core/RandomGenerator.cpp  
  ${OIL_INCLUDES}/Core/XmlSerialisation.h
  Core/XmlSerialisation.cpp
  ${OIL_INCLUDES}/Core/NativeToObject.h
  ${OIL_INCLUDES}/Core/ObjectToNative.h
  Core/CoreLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/CoreLibrary.cpp
)

SET(OIL_CORE_LOADER_SOURCES
  Core/Loader/LbcppLoader.h
  Core/Loader/RawTextLoader.h
  Core/Loader/XmlLoader.h
  Core/Loader/DirectoryLoader.h
  Core/Loader/CoreLoaderLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/CoreLoaderLibrary.cpp
)

#### Execution ####

SET(OIL_EXECUTION_SOURCES
  ${OIL_INCLUDES}/Execution/predeclarations.h
  ${OIL_INCLUDES}/Execution/WorkUnit.h
  Execution/WorkUnit.cpp
  ${OIL_INCLUDES}/Execution/TestUnit.h
  ${OIL_INCLUDES}/Execution/ExecutionStack.h
  ${OIL_INCLUDES}/Execution/ExecutionCallback.h
  Execution/ExecutionCallback.cpp
  ${OIL_INCLUDES}/Execution/ExecutionContext.h
  Execution/ExecutionContext.cpp
  ${OIL_INCLUDES}/Execution/ExecutionContextCallback.h
  ${OIL_INCLUDES}/Execution/ExecutionTrace.h
  Execution/ExecutionTrace.cpp
  ${OIL_INCLUDES}/Execution/Notification.h
  Execution/Notification.cpp
  Execution/ExecutionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ExecutionLibrary.cpp
)

SET(OIL_EXECUTION_CALLBACK_SOURCES
  Execution/Callback/ExecutionNotifications.h
  Execution/Callback/ConsoleExecutionCallback.h
  Execution/Callback/MakeTraceExecutionCallback.h
  Execution/Callback/MakeAndAutoSaveTraceExecutionCallback.h
  Execution/Callback/ExecutionCallbackLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ExecutionCallbackLibrary.cpp
)

SET(OIL_EXECUTION_CONTEXT_SOURCES
  Execution/Context/SingleThreadedExecutionContext.h
  Execution/Context/SubExecutionContext.h
  Execution/Context/MultiThreadedExecutionContext.h
  Execution/Context/ExecutionContextLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ExecutionContextLibrary.cpp
)


#### UserInterface ####
IF(LBCPP_USER_INTERFACE)

  # Res input data.
  FILE(GLOB LBCPP_USER_INTERFACE_DATA_FILES
    UserInterface/Data/*.png
  )

  # Where is jucebinbuild?
  GET_TARGET_PROPERTY(JUCE_BIN_BUILD_COMMAND jucebinbuild LOCATION)

  # The generated files.
  SET(LBCPP_USER_INTERFACE_DATA_CPP ${CMAKE_CURRENT_BINARY_DIR}/UserInterfaceData.cpp)
  SET(LBCPP_USER_INTERFACE_DATA_H ${CMAKE_CURRENT_BINARY_DIR}/UserInterfaceData.h)
  
  # Main commmand for resources generation.
  ADD_CUSTOM_COMMAND(
    OUTPUT ${LBCPP_USER_INTERFACE_DATA_CPP}
    COMMAND ${JUCE_BIN_BUILD_COMMAND} ${CMAKE_CURRENT_SOURCE_DIR}/UserInterface/Data ${CMAKE_CURRENT_BINARY_DIR} UserInterfaceData precompiled.h
    DEPENDS ${LBCPP_USER_INTERFACE_DATA_FILES} ${JUCE_BIN_BUILD_COMMAND} 
    COMMENT "User Interface Data"
  )
  # Dummy custom command for second generated file.
  ADD_CUSTOM_COMMAND(
    OUTPUT ${LBCPP_USER_INTERFACE_DATA_H}
    COMMAND echo
    DEPENDS  ${LBCPP_USER_INTERFACE_DATA_FILES} ${JUCE_BIN_BUILD_COMMAND}
  )

  SET(LBCPP_USER_INTEFACE_GENERATED_FILES
    ${LBCPP_USER_INTERFACE_DATA_H}
    ${LBCPP_USER_INTERFACE_DATA_CPP}
  )

  #IDE_ONLY_SOURCES(LBCPP_USER_INTEFACE_GENERATED_FILES ${LBCPP_USER_INTEFACE_GENERATED_FILES})

  SET(LBCPP_USER_INTERFACE_SOURCES
	${OIL_INCLUDES}/UserInterface/Plot.h
    ${OIL_INCLUDES}/UserInterface/PieChart.h
    ${OIL_INCLUDES}/UserInterface/Histogram.h
    ${OIL_INCLUDES}/UserInterface/ObjectComponent.h
    UserInterface/ObjectComponent.cpp
    ${OIL_INCLUDES}/UserInterface/UserInterfaceManager.h
    UserInterface/UserInterfaceManager.cpp
    ${LBCPP_USER_INTEFACE_GENERATED_FILES}
	UserInterface/UserInterfaceLibrary.xml
    ${CMAKE_CURRENT_BINARY_DIR}/UserInterfaceLibrary.cpp
  )

  SET(LBCPP_USER_INTERFACE_COMPONENT_SOURCES
	UserInterface/Component/TableComponent.h
	UserInterface/Component/TableComponent.cpp
    UserInterface/Component/PieChartComponent.h
    UserInterface/Component/HistogramComponent.h
    UserInterface/Component/UserInterfaceComponentLibrary.xml
    ${CMAKE_CURRENT_BINARY_DIR}/UserInterfaceComponentLibrary.cpp
  )

  SET(LBCPP_USER_INTERFACE_TREE_VIEW_SOURCES
	UserInterface/TreeView/GenericTreeView.h
	UserInterface/TreeView/GenericTreeView.cpp
	UserInterface/TreeView/ObjectTreeView.h
	UserInterface/TreeView/FileTreeView.h
	UserInterface/TreeView/XmlTreeView.h
	UserInterface/TreeView/ResultsTreeView.h
    UserInterface/TreeView/ExecutionTraceTreeView.h
    UserInterface/TreeView/ExecutionTraceTreeView.cpp
	UserInterface/TreeView/UserInterfaceTreeViewLibrary.xml
    ${CMAKE_CURRENT_BINARY_DIR}/UserInterfaceTreeViewLibrary.cpp
  )

  SET(LBCPP_USER_INTERFACE_PLOT_SOURCES
	UserInterface/Plot/Plot.cpp
    UserInterface/Plot/TwoDimensionalPlotDrawable.h
    UserInterface/Plot/TwoDimensionalPlotDrawable.cpp
	UserInterface/Plot/ContourPlotDrawable.h
	UserInterface/Plot/ContourPlotDrawable.cpp
	UserInterface/Plot/PlotConfigurationComponent.h
	UserInterface/Plot/PlotContentComponent.h
    UserInterface/Plot/PlotComponent.h
	UserInterface/Plot/UserInterfacePlotLibrary.xml
    ${CMAKE_CURRENT_BINARY_DIR}/UserInterfacePlotLibrary.cpp
  )

ENDIF(LBCPP_USER_INTERFACE)

SET(LBCPP_LUA_SOURCES
  ${OIL_INCLUDES}/Lua/Lua.h
  Lua/Lua.cpp
  Lua/LuaLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/LuaLibrary.cpp
)

##### MAIN ####

SET(OIL_MAIN_SOURCES
  ${OIL_INCLUDES}/common.h
  precompiled.h
  precompiled.cpp
  ${OIL_INCLUDES}/lbcpp.h
  ${OIL_INCLUDES}/library.h
  ${OIL_INCLUDES}/Core.h
  library.cpp
  LBCppLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/LBCppLibrary.cpp
)

SET(LBCPP_SOURCES
  ${OIL_MAIN_SOURCES}

  # Core
  ${OIL_CORE_SOURCES}
	${OIL_CORE_LOADER_SOURCES}

  # Execution
  ${OIL_EXECUTION_SOURCES}
    ${OIL_EXECUTION_CALLBACK_SOURCES}
    ${OIL_EXECUTION_CONTEXT_SOURCES}
  
  # Lua
  ${LBCPP_LUA_SOURCES}
    
  # User Interface
  ${LBCPP_USER_INTERFACE_SOURCES}
    ${LBCPP_USER_INTERFACE_COMPONENT_SOURCES}
	${LBCPP_USER_INTERFACE_TREE_VIEW_SOURCES}
    ${LBCPP_USER_INTERFACE_PLOT_SOURCES}
    ${LBCPP_USER_INTERFACE_DATA_FILES}
)

GENERATE_LBCPP_PRECOMPILED(precompiled LBCPP_SOURCES)

## Main
SOURCE_GROUP(Main FILES $OIL_MAIN_SOURCES})

## Core
SOURCE_GROUP(Core FILES ${OIL_CORE_SOURCES})
SOURCE_SUB_GROUP(Core Loader FILES ${OIL_CORE_LOADER_SOURCES})

## Execution
SOURCE_GROUP(Execution FILES ${OIL_EXECUTION_SOURCES})
SOURCE_SUB_GROUP(Execution Callback FILES ${OIL_EXECUTION_CALLBACK_SOURCES})
SOURCE_SUB_GROUP(Execution Context FILES ${OIL_EXECUTION_CONTEXT_SOURCES})

## Lua
SOURCE_GROUP(Lua FILES ${LBCPP_LUA_SOURCES})
	  
## UserInterface
SOURCE_GROUP(UserInterface FILES ${LBCPP_USER_INTERFACE_SOURCES})
SOURCE_SUB_GROUP(UserInterface Component FILES ${LBCPP_USER_INTERFACE_COMPONENT_SOURCES})
SOURCE_SUB_GROUP(UserInterface TreeView FILES ${LBCPP_USER_INTERFACE_TREE_VIEW_SOURCES})
SOURCE_SUB_GROUP(UserInterface Plot FILES ${LBCPP_USER_INTERFACE_PLOT_SOURCES})
SOURCE_SUB_GROUP(UserInterface Data FILES ${LBCPP_USER_INTERFACE_DATA_FILES})

IF (UNIX)
  ADD_LIBRARY(oil SHARED ${LBCPP_SOURCES})
ELSE (UNIX)
  ADD_LIBRARY(oil ${LBCPP_SOURCES})
ENDIF (UNIX)
TARGET_LINK_LIBRARIES(oil juce lua shark)
