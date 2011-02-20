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
      SET(JUCE_PLATFORM_SPECIFIC_LIBRARIES "-framework Carbon -framework Cocoa -framework CoreAudio -framework IOKit")
      # Include and link with the others frameworks only if build_core_only is disabled (i.e. Networking and/or UserInterface enabled)      
      IF(LBCPP_USER_INTERFACE)
        SET(JUCE_PLATFORM_SPECIFIC_LIBRARIES ${JUCE_PLATFORM_SPECIFIC_LIBRARIES} "-framework CoreMidi -framework AGL -framework CoreFoundation -lobjc")
      ELSE(LBCPP_USE_INTERFACE)
        IF(LBCPP_NETWORKING)
          SET(JUCE_PLATFORM_SPECIFIC_LIBRARIES ${JUCE_PLATFORM_SPECIFIC_LIBRARIES} "-framework CoreMidi -framework AGL -framework CoreFoundation -lobjc")
        ELSE(LBCPP_NETWORKING)
          # NOP
        ENDIF(LBCPP_NETWORKING)
      ENDIF(LBCPP_USER_INTERFACE)
    ELSE (APPLE)
      ADD_DEFINITIONS(-DLINUX)
      SET(JUCE_PLATFORM_SPECIFIC_LIBRARIES pthread dl) # asound Xinerma)

      # Include and link with X11 if network or UI enabled      
      IF(LBCPP_USER_INTERFACE)
        INCLUDE_DIRECTORIES(/usr/include/freetype2)
        FIND_PACKAGE(X11 REQUIRED)
        INCLUDE_DIRECTORIES(${X11_INCLUDE_DIR})
        SET(JUCE_PLATFORM_SPECIFIC_LIBRARIES ${JUCE_PLATFORM_SPECIFIC_LIBRARIES} freetype ${X11_LIBRARIES})
      ELSE(LBCPP_USE_INTERFACE)
        IF(LBCPP_NETWORKING)
          INCLUDE_DIRECTORIES(/usr/include/freetype2)
          FIND_PACKAGE(X11 REQUIRED)
          INCLUDE_DIRECTORIES(${X11_INCLUDE_DIR})
          SET(JUCE_PLATFORM_SPECIFIC_LIBRARIES ${JUCE_PLATFORM_SPECIFIC_LIBRARIES} freetype ${X11_LIBRARIES})
        ELSE(LBCPP_NETWORKING)
          # Don't use X11
        ENDIF(LBCPP_NETWORKING)
      ENDIF(LBCPP_USER_INTERFACE)
    ENDIF(APPLE)
  ENDIF(UNIX)
ENDIF(WIN32)

ADD_LIBRARY(juce${NAME_FILE_SUFFIX} SHARED ${JUCE_SOURCES})
TARGET_LINK_LIBRARIES(juce${NAME_FILE_SUFFIX} ${JUCE_PLATFORM_SPECIFIC_LIBRARIES})# ${OPENGL_LIBRARIES})

### Juce Bin Build

ADD_EXECUTABLE(jucebinbuild jucebinbuild.cpp)
SET_TARGET_PROPERTIES(jucebinbuild PROPERTIES PROJECT_LABEL "TOOL JuceBinBuild")
TARGET_LINK_LIBRARIES(jucebinbuild juce${NAME_FILE_SUFFIX})
