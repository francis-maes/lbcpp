INCLUDE_DIRECTORIES(${LBCpp_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR})

FIND_PACKAGE(OpenGL REQUIRED)
INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})

GENERATE_INTROSPECTION_CODE(ExplorerClasses.xml ExplorerClasses.cpp)

SET(EXPLORER_UTILITIES_SOURCES
  Utilities/SplittedLayout.h
  Utilities/SimpleTreeViewItem.h
  Utilities/VariableSelector.h
  Utilities/PropertyListDisplayComponent.h
  Utilities/ComponentWithPreferedSize.h
  Utilities/FileType.h
)

SET(EXPLORER_COMPONENTS_SOURCES
  Components/common.h
  Components/StringComponent.h
  Components/TableComponent.h
  Components/ObjectGraphComponent.h
  Components/ObjectContainerNameListComponent.h
  Components/ObjectComponents.cpp

  Components/VariableProxyComponent.h
  Components/VariableBrowser.h
  Components/VariableTreeComponent.h
  Components/HexadecimalFileComponent.h
)

SET(EXPLORER_PROCESS_MANAGER_SOURCES
  ProcessManager/ProcessManager.h
  ProcessManager/ProcessManager.cpp
  ProcessManager/RecentProcesses.h
  ProcessManager/RecentProcesses.cpp
  ProcessManager/ProcessManagerComponent.h
  ProcessManager/ProcessComponent.cpp
  ProcessManager/ProcessConsoleSettingsComponent.cpp
  ProcessManager/ProcessManagerComponent.cpp
  ProcessManager/NewProcessDialogWindow.h
)

SET(EXPLORER_PROTEINS_SOURCES
  Proteins/MultiSequenceComponent.h
  Proteins/MultiProtein1DComponent.h
  Proteins/MultiProtein2DComponent.h
  Proteins/ProteinComponent.h
)

SET(EXPLORER_APPLICATION_SOURCES
  ExplorerConfiguration.h
  ExplorerConfiguration.cpp
  ExplorerApplication.cpp
  ExplorerClasses.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ExplorerClasses.cpp
)

SET(LBCPP_EXPLORER_SOURCES
 ${EXPLORER_UTILITIES_SOURCES}
 ${EXPLORER_COMPONENTS_SOURCES}
 ${EXPLORER_APPLICATION_SOURCES}
 ${EXPLORER_PROCESS_MANAGER_SOURCES}
 ${EXPLORER_PROTEINS_SOURCES}
)

SOURCE_GROUP(Utilities FILES ${EXPLORER_UTILITIES_SOURCES})
SOURCE_GROUP(Components FILES ${EXPLORER_COMPONENTS_SOURCES})
SOURCE_GROUP(Explorer FILES ${EXPLORER_APPLICATION_SOURCES})
SOURCE_GROUP(ProcessManager FILES ${EXPLORER_PROCESS_MANAGER_SOURCES})
SOURCE_GROUP(Proteins FILES ${EXPLORER_PROTEINS_SOURCES})

IF(APPLE)
       SET(MACOSX_BUNDLE_INFO_STRING "LBC++ Explorer")
       SET(MACOSX_BUNDLE_ICON_FILE "")
       SET(MACOSX_BUNDLE_GUI_IDENTIFIER "")
       SET(MACOSX_BUNDLE_LONG_VERSION_STRING "")
       SET(MACOSX_BUNDLE_BUNDLE_NAME "")
       SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "")
       SET(MACOSX_BUNDLE_BUNDLE_VERSION "")
       SET(MACOSX_BUNDLE_COPYRIGHT "Francis Maes")

       ADD_EXECUTABLE(lbcpp-explorer MACOSX_BUNDLE
                ${LBCPP_EXPLORER_SOURCES})
ELSE(APPLE)
  ADD_EXECUTABLE(lbcpp-explorer ${LBCPP_EXPLORER_SOURCES})
ENDIF(APPLE)

TARGET_LINK_LIBRARIES(lbcpp-explorer proteins lbcpp-core)
