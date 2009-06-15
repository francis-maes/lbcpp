INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

SET(JUCE_SOURCES
  juce_amalgamated.h
  juce_amalgamated.cpp
  juce_amalgamated.m
)

ADD_LIBRARY(juce ${JUCE_SOURCES})
