INCLUDE_DIRECTORIES(
  ${LBCpp_SOURCE_DIR}/include
  ${LBCpp_SOURCE_DIR}/extern/shark/include
)

MACRO(OPTIONAL_PROJECT NAME DIRECTORY)
  OPTION(LBCPP_PROJECT_${NAME} "Include project ${DIRECTORY}")
  IF(LBCPP_PROJECT_${NAME})
    ADD_SUBDIRECTORY(${DIRECTORY})

  ENDIF(LBCPP_PROJECT_${NAME})
ENDMACRO(OPTIONAL_PROJECT)

OPTIONAL_PROJECT(EXAMPLES Examples)
OPTIONAL_PROJECT(MOO MOO)
OPTIONAL_PROJECT(COLO Colo)
OPTIONAL_PROJECT(MCGP MCGP)
OPTIONAL_PROJECT(MORPION Morpion)
OPTIONAL_PROJECT(SEQUENTIAL_DECISION SequentialDecision)
