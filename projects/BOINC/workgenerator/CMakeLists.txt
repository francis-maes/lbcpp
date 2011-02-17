IF(UNIX)
  IF(NOT APPLE)
    IF(CMAKE_SIZEOF_VOID_P MATCHES 4)
      
      INCLUDE_DIRECTORIES(
        ${MYSQL_INCLUDE_DIR}
        ${OPENSSL_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/db
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/tools
        ${CMAKE_CURRENT_SOURCE_DIR}/../include/sched
        
        # Not used
        # ${CMAKE_CURRENT_SOURCE_DIR}/../include/api ${CMAKE_CURRENT_SOURCE_DIR}/../include/client
      )
      
      SET(SSL_LIB /usr/lib/libssl.a)
      SET(LIBS
        ${MYSQL_LIBRARIES}
        ${OPENSSL_LIBRARIES}
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/libsched.a
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/libboinc_crypt.a
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/libboinc.a
        crypto        
      )
      
      ADD_EXECUTABLE(sample_work_generator
        sample_work_generator.cpp
      )
      
      TARGET_LINK_LIBRARIES(sample_work_generator ${LIBS})
      
    ENDIF(CMAKE_SIZEOF_VOID_P MATCHES 4)
  ENDIF(NOT APPLE)
ENDIF(UNIX)
