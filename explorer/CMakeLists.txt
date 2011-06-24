INCLUDE_DIRECTORIES(${LBCpp_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR})

#FIND_PACKAGE(OpenGL REQUIRED)
#INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})

GENERATE_INTROSPECTION_CODE(ExplorerLibrary.xml ExplorerLibrary.cpp)
GENERATE_INTROSPECTION_CODE(WorkUnitManager/WorkUnitManagerLibrary.xml WorkUnitManagerLibrary.cpp)

SET(EXPLORER_UTILITIES_SOURCES
  Utilities/PropertyListDisplayComponent.h
  Utilities/FileType.h
)

SET(EXPLORER_COMPONENTS_SOURCES
  Components/common.h
  Components/StringComponent.h
  Components/ContainerSelectorComponent.h
  Components/VariableProxyComponent.h
  Components/VariableBrowser.h
  Components/VariableBrowser.cpp
  Components/HexadecimalFileComponent.h
  Components/LuaCodeEditorComponent.h
  Components/VariableComponents.cpp
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

SET(EXPLORER_WORK_UNIT_MANAGER_SOURCES
  WorkUnitManager/NewWorkUnitDialogWindow.h
  WorkUnitManager/NewWorkUnitDialogWindow.cpp
  WorkUnitManager/WorkUnitManagerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/WorkUnitManagerLibrary.cpp
)

SET(EXPLORER_APPLICATION_SOURCES
  ExplorerConfiguration.h
  ExplorerConfiguration.cpp
  ExplorerProject.h
  ExplorerProject.cpp
  ExplorerApplication.cpp
  ExplorerLibrary.xml
  ${CMAKE_CURRENT_BINARY_DIR}/ExplorerLibrary.cpp
)

SET(LBCPP_EXPLORER_SOURCES
  precompiled.h
  precompiled.cpp
  ${EXPLORER_UTILITIES_SOURCES}
  ${EXPLORER_COMPONENTS_SOURCES}
  ${EXPLORER_APPLICATION_SOURCES}
  ${EXPLORER_WORK_UNIT_MANAGER_SOURCES}
  ${EXPLORER_PROCESS_MANAGER_SOURCES}
)

SOURCE_GROUP(Utilities FILES ${EXPLORER_UTILITIES_SOURCES})
SOURCE_GROUP(Components FILES ${EXPLORER_COMPONENTS_SOURCES})
SOURCE_GROUP(Explorer FILES ${EXPLORER_APPLICATION_SOURCES})
SOURCE_GROUP(WorkUnitManager FILES ${EXPLORER_WORK_UNIT_MANAGER_SOURCES})
SOURCE_GROUP(ProcessManager FILES ${EXPLORER_PROCESS_MANAGER_SOURCES})

IF(APPLE)
       SET(MACOSX_BUNDLE_INFO_STRING "LBC++ Explorer")
       SET(MACOSX_BUNDLE_ICON_FILE "")
       SET(MACOSX_BUNDLE_GUI_IDENTIFIER "")
       SET(MACOSX_BUNDLE_LONG_VERSION_STRING "")
       SET(MACOSX_BUNDLE_BUNDLE_NAME "")
       SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "")
       SET(MACOSX_BUNDLE_BUNDLE_VERSION "")
       SET(MACOSX_BUNDLE_COPYRIGHT "Francis Maes")

       ADD_EXECUTABLE(lbcpp-explorer${NAME_FILE_SUFFIX} MACOSX_BUNDLE
                ${LBCPP_EXPLORER_SOURCES})
ELSE(APPLE)
  ADD_EXECUTABLE(lbcpp-explorer${NAME_FILE_SUFFIX} ${LBCPP_EXPLORER_SOURCES})
ENDIF(APPLE)

GENERATE_LBCPP_PRECOMPILED(precompiled LBCPP_EXPLORER_SOURCES)

TARGET_LINK_LIBRARIES(lbcpp-explorer${NAME_FILE_SUFFIX} lbcpp-core${NAME_FILE_SUFFIX})
