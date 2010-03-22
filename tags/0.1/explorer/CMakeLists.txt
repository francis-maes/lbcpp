INCLUDE_DIRECTORIES(${LBCpp_SOURCE_DIR}/include)

FIND_PACKAGE(OpenGL REQUIRED)
INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})

IF(APPLE)
  SET(JUCE_SOURCES
    Juce/juce_amalgamated.h
    Juce/juce_amalgamated.mm
  )
ELSE(APPLE)
  SET(JUCE_SOURCES
    Juce/juce_amalgamated.h
    Juce/juce_amalgamated.cpp
  )
ENDIF(APPLE)

SET(EXPLORER_UTILITIES_SOURCES
  Utilities/SplittedLayout.h
)

SET(EXPLORER_COMPONENTS_SOURCES
  Components/common.h
  Components/StringComponent.h
  Components/TableComponent.h
  Components/ObjectGraphComponent.h
  Components/ObjectGraphAndContentComponent.h
)

SET(EXPLORER_APPLICATION_SOURCES
  ExplorerApplication.cpp
)

SET(LBCPP_EXPLORER_SOURCES
 ${JUCE_SOURCES}
 ${EXPLORER_UTILITIES_SOURCES}
 ${EXPLORER_COMPONENTS_SOURCES}
 ${EXPLORER_APPLICATION_SOURCES}
)

SOURCE_GROUP(Juce FILES ${JUCE_SOURCES})
SOURCE_GROUP(Utilities FILES ${EXPLORER_UTILITIES_SOURCES})
SOURCE_GROUP(Components FILES ${EXPLORER_COMPONENTS_SOURCES})
SOURCE_GROUP(Explorer FILES ${EXPLORER_APPLICATION_SOURCES})

IF   (WIN32)
  SET(JUCE_PLATFORM_SPECIFIC_LIBRARIES
    kernel32.lib
    user32.lib
    shell32.lib
    gdi32.lib
    vfw32.lib
    comdlg32.lib
    winmm.lib
    wininet.lib
    rpcrt4.lib
    ole32.lib
    advapi32.lib
    ws2_32.lib
    OpenGL32.Lib
    GlU32.Lib
  )
ELSE (WIN32)
  IF   (UNIX)
    IF   (APPLE)
      SET(JUCE_PLATFORM_SPECIFIC_LIBRARIES "-framework Carbon -framework Cocoa -framework CoreFoundation -framework CoreAudio -framework CoreMidi -framework IOKit -framework AGL -lobjc")
    ELSE (APPLE)
      FIND_PACKAGE(X11 REQUIRED)
      INCLUDE_DIRECTORIES(${X11_INCLUDE_DIR})
      INCLUDE_DIRECTORIES(/usr/include/freetype2)
      ADD_DEFINITIONS(-DLINUX)
      SET(JUCE_PLATFORM_SPECIFIC_LIBRARIES ${X11_LIBRARIES} freetype pthread) # asound Xinerama)
    ENDIF(APPLE)
  ENDIF(UNIX)
ENDIF(WIN32)

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

TARGET_LINK_LIBRARIES(lbcpp-explorer ${JUCE_PLATFORM_SPECIFIC_LIBRARIES} ${OPENGL_LIBRARIES} lbcpp-core)
