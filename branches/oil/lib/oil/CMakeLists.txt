INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${LBCpp_SOURCE_DIR}/shark/include
  ${CMAKE_CURRENT_SOURCE_DIR}
)

GENERATE_INTROSPECTION_CODE(LBCppLibrary.xml LBCppLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Core/CoreLibrary.xml CoreLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Core/Loader/CoreLoaderLibrary.xml CoreLoaderLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Data/Stream/StreamLibrary.xml StreamLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Data/DataLibrary.xml DataLibrary.cpp)

GENERATE_INTROSPECTION_CODE(Execution/ExecutionLibrary.xml ExecutionLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Execution/Callback/ExecutionCallbackLibrary.xml ExecutionCallbackLibrary.cpp)
GENERATE_INTROSPECTION_CODE(Execution/Context/ExecutionContextLibrary.xml ExecutionContextLibrary.cpp)

IF(LBCPP_USER_INTERFACE)
  GENERATE_INTROSPECTION_CODE(UserInterface/UserInterfaceLibrary.xml UserInterfaceLibrary.cpp)
  GENERATE_INTROSPECTION_CODE(UserInterface/Component/UserInterfaceComponentLibrary.xml UserInterfaceComponentLibrary.cpp)
  GENERATE_INTROSPECTION_CODE(UserInterface/TreeView/UserInterfaceTreeViewLibrary.xml UserInterfaceTreeViewLibrary.cpp)
  GENERATE_INTROSPECTION_CODE(UserInterface/Plot/UserInterfacePlotLibrary.xml UserInterfacePlotLibrary.cpp)
ENDIF(LBCPP_USER_INTERFACE)

#### Core ####

SET(LBCPP_CORE_SOURCES
  ../include/lbcpp/Core/Utilities.h  
  Core/Utilities.cpp
  ../include/lbcpp/Core/ReferenceCountedObject.h
  ../include/lbcpp/Core/predeclarations.h
  ../include/lbcpp/Core/Object.h
  Core/Object.cpp
  ../include/lbcpp/Core/Boolean.h
  Core/Boolean.cpp
  ../include/lbcpp/Core/Integer.h
  Core/Integer.cpp
  ../include/lbcpp/Core/Double.h
  Core/Double.cpp
  ../include/lbcpp/Core/String.h
  Core/String.cpp
  ../include/lbcpp/Core/ClassManager.h
  Core/ClassManager.cpp
  ../include/lbcpp/Core/Signature.h
  ../include/lbcpp/Core/Class.h
  Core/Class.cpp
  ../include/lbcpp/Core/Enumeration.h
  Core/Enumeration.cpp
  ../include/lbcpp/Core/DefaultClass.h
  Core/DefaultClass.cpp
  ../include/lbcpp/Core/TemplateClass.h
  Core/TemplateClass.cpp
  ../include/lbcpp/Core/Library.h
  Core/Library.cpp
  ../include/lbcpp/Core/Pair.h
  ../include/lbcpp/Core/Vector.h
  Core/Vector.cpp
  ../include/lbcpp/Core/Loader.h
  Core/Loader.cpp
  ../include/lbcpp/Core/XmlSerialisation.h
  Core/XmlSerialisation.cpp
  ../include/lbcpp/Core/Table.h
  Core/Table.cpp
  ../include/lbcpp/Core/NativeToObject.h
  ../include/lbcpp/Core/ObjectToNative.h
  Core/CoreLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/CoreLibrary.cpp
)

SET(LBCPP_CORE_LOADER_SOURCES
  Core/Loader/LbcppLoader.h
  Core/Loader/RawTextLoader.h
  Core/Loader/XmlLoader.h
  Core/Loader/DirectoryLoader.h
  Core/Loader/CoreLoaderLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/CoreLoaderLibrary.cpp
)


#### Execution ####

SET(LBCPP_EXECUTION_SOURCES
  ../include/lbcpp/Execution/predeclarations.h
  ../include/lbcpp/Execution/WorkUnit.h
  Execution/WorkUnit.cpp
  ../include/lbcpp/Execution/TestUnit.h
  ../include/lbcpp/Execution/ExecutionStack.h
  ../include/lbcpp/Execution/ExecutionCallback.h
  Execution/ExecutionCallback.cpp
  ../include/lbcpp/Execution/ExecutionContext.h
  Execution/ExecutionContext.cpp
  ../include/lbcpp/Execution/ExecutionContextCallback.h
  ../include/lbcpp/Execution/ExecutionTrace.h
  Execution/ExecutionTrace.cpp
  ../include/lbcpp/Execution/Notification.h
  Execution/Notification.cpp
  Execution/ExecutionLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ExecutionLibrary.cpp
)

SET(LBCPP_EXECUTION_CALLBACK_SOURCES
  Execution/Callback/ExecutionNotifications.h
  Execution/Callback/ConsoleExecutionCallback.h
  Execution/Callback/MakeTraceExecutionCallback.h
  Execution/Callback/MakeAndAutoSaveTraceExecutionCallback.h
  Execution/Callback/ExecutionCallbackLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ExecutionCallbackLibrary.cpp
)

SET(LBCPP_EXECUTION_CONTEXT_SOURCES
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
	../include/lbcpp/UserInterface/Plot.h
    ../include/lbcpp/UserInterface/PieChart.h
    ../include/lbcpp/UserInterface/Histogram.h
    ../include/lbcpp/UserInterface/ObjectComponent.h
    UserInterface/ObjectComponent.cpp
    ../include/lbcpp/UserInterface/UserInterfaceManager.h
    UserInterface/UserInterfaceManager.cpp
    ${LBCPP_USER_INTEFACE_GENERATED_FILES}
	UserInterface/UserInterfaceLibrary.xml
    ${CMAKE_CURRENT_BINARY_DIR}/UserInterfaceLibrary.cpp
  )

  SET(LBCPP_USER_INTERFACE_COMPONENT_SOURCES
	UserInterface/Component/TableComponent.h
	UserInterface/Component/TableComponent.cpp
    UserInterface/Component/ConfusionMatrixComponent.h
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
	UserInterface/Plot/PlotConfigurationComponent.h
	UserInterface/Plot/PlotContentComponent.h
    UserInterface/Plot/PlotComponent.h
	UserInterface/Plot/UserInterfacePlotLibrary.xml
    ${CMAKE_CURRENT_BINARY_DIR}/UserInterfacePlotLibrary.cpp
  )

ENDIF(LBCPP_USER_INTERFACE)

##### MAIN ####

SET(LBCPP_MAIN_SOURCES
  ../include/lbcpp/common.h
  precompiled.h
  precompiled.cpp
  ../include/lbcpp/lbcpp.h
  ../include/lbcpp/library.h
  ../include/lbcpp/Core.h
  library.cpp
  LBCppLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/LBCppLibrary.cpp
)

SET(LBCPP_SOURCES
  ${LBCPP_MAIN_SOURCES}

  # Core
  ${LBCPP_CORE_SOURCES}
	${LBCPP_CORE_LOADER_SOURCES}

  # Execution
  ${LBCPP_EXECUTION_SOURCES}
    ${LBCPP_EXECUTION_CALLBACK_SOURCES}
    ${LBCPP_EXECUTION_CONTEXT_SOURCES}
  
  # User Interface
  ${LBCPP_USER_INTERFACE_SOURCES}
    ${LBCPP_USER_INTERFACE_COMPONENT_SOURCES}
	${LBCPP_USER_INTERFACE_TREE_VIEW_SOURCES}
    ${LBCPP_USER_INTERFACE_PLOT_SOURCES}
    ${LBCPP_USER_INTERFACE_DATA_FILES}
)

GENERATE_LBCPP_PRECOMPILED(precompiled LBCPP_SOURCES)

## Main
SOURCE_GROUP(Main FILES ${LBCPP_MAIN_SOURCES})

## Core
SOURCE_GROUP(Core FILES ${LBCPP_CORE_SOURCES})
SOURCE_SUB_GROUP(Core Loader FILES ${LBCPP_CORE_LOADER_SOURCES})

## Execution
SOURCE_GROUP(Execution FILES ${LBCPP_EXECUTION_SOURCES})
SOURCE_SUB_GROUP(Execution Callback FILES ${LBCPP_EXECUTION_CALLBACK_SOURCES})
SOURCE_SUB_GROUP(Execution Context FILES ${LBCPP_EXECUTION_CONTEXT_SOURCES})

## UserInterface
SOURCE_GROUP(UserInterface FILES ${LBCPP_USER_INTERFACE_SOURCES})
SOURCE_SUB_GROUP(UserInterface Component FILES ${LBCPP_USER_INTERFACE_COMPONENT_SOURCES})
SOURCE_SUB_GROUP(UserInterface TreeView FILES ${LBCPP_USER_INTERFACE_TREE_VIEW_SOURCES})
SOURCE_SUB_GROUP(UserInterface Plot FILES ${LBCPP_USER_INTERFACE_PLOT_SOURCES})
SOURCE_SUB_GROUP(UserInterface Data FILES ${LBCPP_USER_INTERFACE_DATA_FILES})

IF (UNIX)
  ADD_LIBRARY(lbcpp-core SHARED ${LBCPP_SOURCES})
ELSE (UNIX)
  ADD_LIBRARY(lbcpp-core ${LBCPP_SOURCES})
ENDIF (UNIX)
TARGET_LINK_LIBRARIES(lbcpp-core juce lua shark)
