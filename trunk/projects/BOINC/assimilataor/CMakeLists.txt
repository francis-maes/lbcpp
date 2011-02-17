IF(UNIX)
  IF(NOT APPLE)
    IF(CMAKE_SIZEOF_VOID_P MATCHES 4)   
      INCLUDE_DIRECTORIES(
        ${MYSQL_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/db
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/sched
       
        # Not used
        # ${OpenSSL_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../include/api ${CMAKE_CURRENT_SOURCE_DIR}/../include/client ${CMAKE_CURRENT_SOURCE_DIR}/../include/tools
      )

      SET(LIBS
        ${MYSQL_LIBRARIES}
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/libsched.a
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/libboinc.a

        # Not used
        # ${OpenSSL_LIBRARIES} ${CMAKE_CURRENT_SOURCE_DIR}/../lib/libboinc_crypt.a pthread crypto
      ) 
      
      ADD_EXECUTABLE(sample_assimilator
        sample_assimilator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../validator/validate_util.cpp
      )

      TARGET_LINK_LIBRARIES(sample_assimilator ${LIBS})

    ENDIF(CMAKE_SIZEOF_VOID_P MATCHES 4)
  ENDIF(NOT APPLE)
ENDIF(UNIX)      
