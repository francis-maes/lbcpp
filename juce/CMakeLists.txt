#FIND_PACKAGE(OpenGL REQUIRED)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

# Only build core library if network and UI disabled
IF(LBCPP_USER_INTERFACE)
  # NOP
ELSE(LBCPP_USER_INTERFACE)
  IF(LBCPP_NETWORKING)
    # NOP
  ELSE(LBCPP_NETWORKING)
    ADD_DEFINITIONS(-DJUCE_ONLY_BUILD_CORE_LIBRARY=1)
  ENDIF(LBCPP_NETWORKING)
ENDIF(LBCPP_USER_INTERFACE) 


SET(JUCE_ADDITIONAL_SOURCES
  ConsoleProcess.h
  ConsoleProcess.cpp
  Win32ConsoleProcess.h
  PosixConsoleProcess.h
)

IF(APPLE)
  SET(JUCE_SOURCES
    juce_amalgamated.h
    juce_amalgamated.mm
    ${JUCE_ADDITIONAL_SOURCES}
  )
ELSE(APPLE)
  SET(JUCE_SOURCES
    juce_amalgamated.h
    juce_amalgamated.cpp
    ${JUCE_ADDITIONAL_SOURCES}
  )
ENDIF(APPLE)

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
      ADD_DEFINITIONS(-DLINUX)
      INCLUDE_DIRECTORIES(/usr/include/freetype2)
      SET(JUCE_PLATFORM_SPECIFIC_LIBRARIES pthread freetype dl) # asound Xinerma)

      # Include and link with X11 if network or UI enabled      
      IF(LBCPP_USER_INTERFACE)
        FIND_PACKAGE(X11 REQUIRED)
        INCLUDE_DIRECTORIES(${X11_INCLUDE_DIR})
        SET(JUCE_PLATFORM_SPECIFIC_LIBRARIES ${JUCE_PLATFORM_SPECIFIC_LIBRARIES} ${X11_LIBRARIES})
      ELSE(LBCPP_USE_INTERFACE)
        IF(LBCPP_NETWORKING)
          FIND_PACKAGE(X11 REQUIRED)
          INCLUDE_DIRECTORIES(${X11_INCLUDE_DIR})
          SET(JUCE_PLATFORM_SPECIFIC_LIBRARIES ${JUCE_PLATFORM_SPECIFIC_LIBRARIES} ${X11_LIBRARIES})
        ELSE(LBCPP_NETWORKING)
          # Don't use X11
        ENDIF(LBCPP_NETWORKING)
      ENDIF(LBCPP_USER_INTERFACE)
    ENDIF(APPLE)
  ENDIF(UNIX)
ENDIF(WIN32)

ADD_LIBRARY(juce SHARED ${JUCE_SOURCES})
TARGET_LINK_LIBRARIES(juce ${JUCE_PLATFORM_SPECIFIC_LIBRARIES})# ${OPENGL_LIBRARIES})

### Juce Bin Build

ADD_EXECUTABLE(jucebinbuild jucebinbuild.cpp)
SET_TARGET_PROPERTIES(jucebinbuild PROPERTIES PROJECT_LABEL "TOOL JuceBinBuild")
TARGET_LINK_LIBRARIES(jucebinbuild juce)
