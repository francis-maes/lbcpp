PROJECT(LBCpp)
cmake_minimum_required(VERSION 2.4)

ADD_SUBDIRECTORY(translator)

###
### CR-compilator CMake support
###
# Where is CR-algo compiler?
GET_TARGET_PROPERTY(LBCPP_TRANSLATOR_COMMAND lbcpp-translator LOCATION)

# add_cralgorithm macro
MACRO(ADD_CRALGORITHM OUTPUTFILE INPUTFILE)
 # MESSAGE("${LBCPP_TRANSLATOR_COMMAND} ${INPUTFILE} ${OUTPUTFILE}")
  ADD_CUSTOM_COMMAND(OUTPUT ${OUTPUTFILE}
                  COMMAND ${LBCPP_TRANSLATOR_COMMAND} ${INPUTFILE} ${OUTPUTFILE}
                  MAIN_DEPENDENCY ${INPUTFILE}
                  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                  DEPENDS ${LBCPP_TRANSLATOR_COMMAND} ${INPUTFILE}
 		  COMMENT "CR-algorithm ${INPUTFILE}"
  )
ENDMACRO(ADD_CRALGORITHM)

# add_cralgorithm_sources
MACRO(ADD_CRALGORITHM_SOURCES) #SOURCES
  SET(CRALGORITHMS_OUTPUT_FILES "")
  set_source_files_properties(${ARGN} PROPERTIES HEADER_FILE_ONLY TRUE)
  foreach(SOURCE ${ARGN})
     SET(inputfile ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE})
     SET(outputfile ${CMAKE_CURRENT_BINARY_DIR}/GeneratedCode/${SOURCE})
     STRING(REPLACE ".cr++" ".cpp" outputfile ${outputfile})
     STRING(REPLACE ".lcpp" ".cpp" outputfile ${outputfile})
   #  MESSAGE("output: " ${outputfile})
     ADD_CRALGORITHM(${outputfile} ${inputfile})
     SET(CRALGORITHMS_OUTPUT_FILES ${CRALGORITHMS_OUTPUT_FILES} "${outputfile}")
  endforeach(SOURCE)
ENDMACRO(ADD_CRALGORITHM_SOURCES)

# add_cralgorithm_headers
MACRO(ADD_CRALGORITHM_HEADERS) #SOURCES
  SET(CRALGORITHMS_OUTPUT_FILES "")
  foreach(SOURCE ${ARGN})
     SET(inputfile ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE})
     SET(outputfile ${CMAKE_CURRENT_BINARY_DIR}/GeneratedCode/${SOURCE})
     STRING(REPLACE ".cr++" ".h" outputfile ${outputfile})
   #  MESSAGE("output: " ${outputfile})
     ADD_CRALGORITHM(${outputfile} ${inputfile})
     SET(CRALGORITHMS_OUTPUT_FILES ${CRALGORITHMS_OUTPUT_FILES} "${outputfile}")
  endforeach(SOURCE)
ENDMACRO(ADD_CRALGORITHM_HEADERS)

# add_crexecutable macro
MACRO(ADD_CREXECUTABLE NAME) #SOURCES
  ADD_CRALGORITHM_SOURCES(${ARGN})
  SOURCE_GROUP(GeneratedCode FILES ${CRALGORITHMS_OUTPUT_FILES})
  ADD_EXECUTABLE(${NAME} ${CRALGORITHMS_OUTPUT_FILES} ${ARGN})
  TARGET_LINK_LIBRARIES(${NAME} lbcpp-core)
ENDMACRO(ADD_CREXECUTABLE)

# add_crtest macro
MACRO(ADD_CRTEST SOURCEFILE)
  STRING(REPLACE ".cr++" "" targetname ${SOURCEFILE})
  SET(targetname "test-${targetname}")
  ADD_CREXECUTABLE(${targetname} ${SOURCEFILE})
  ADD_TEST(${targetname} ${targetname})
ENDMACRO(ADD_CRTEST)

ADD_SUBDIRECTORY(src)

OPTION(LBCPP_TESTS "Include tests into the project" OFF)

IF(LBCPP_TESTS)
  ADD_SUBDIRECTORY(lbcpp-lib)
  ENABLE_TESTING()
ENDIF(LBCPP_TESTS)
ADD_SUBDIRECTORY(test)

OPTION(LBCPP_EXAMPLES "Include examples into the project" ON)
IF (LBCPP_EXAMPLES)
  ADD_SUBDIRECTORY(examples)
ENDIF(LBCPP_EXAMPLES)

OPTION(LBCPP_EXPLORER "Include explorer into the project" ON)
IF (LBCPP_EXPLORER)
  ADD_SUBDIRECTORY(explorer)
ENDIF(LBCPP_EXPLORER)

