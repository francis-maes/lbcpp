macro(dobuildvm action output params)
  GET_TARGET_PROPERTY(BUILDVM_COMMAND buildvm LOCATION)
  
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${output}
    COMMAND ${BUILDVM_COMMAND}
    ARGS -m ${action} -o ${CMAKE_CURRENT_BINARY_DIR}/${output} ${params}
    DEPENDS buildvm ${params}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "buildvm ${action} ${output} ${params}"
  )
  add_custom_target(buildvm_${action} ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${output})
endmacro(dobuildvm action output params)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

file(GLOB buildvm_files buildvm*.c)
add_executable(buildvm ${buildvm_files})

set(all_lib lib_base.c lib_math.c lib_bit.c lib_string.c lib_table.c lib_io.c lib_os.c lib_package.c lib_debug.c lib_jit.c lib_ffi.c)


if(WIN32)
  set(LJVM_BUILDED_FILE lj_vm.obj)
  set(LJVM_OBJECT_FILE lj_vm.obj)
  dobuildvm(peobj ${LJVM_BUILDED_FILE} "")
else(WIN32)
  set(LJVM_BUILDED_FILE lj_vm.s)
  set(LJVM_OBJECT_FILE lj_vm.o)
  if(APPLE)
    dobuildvm(machasm ${LJVM_BUILDED_FILE} "")
  else(APPLE)
    dobuildvm(elfasm ${LJVM_BUILDED_FILE} "")
  endif(APPLE)
endif(WIN32)

dobuildvm(bcdef lj_bcdef.h "${all_lib}")
dobuildvm(ffdef lj_ffdef.h "${all_lib}")
dobuildvm(libdef lj_libdef.h "${all_lib}")
dobuildvm(recdef lj_recdef.h "${all_lib}")
dobuildvm(vmdef vmdef.lua "${all_lib}") # francis: replace ../lib/vmdef.lua by vmdef.lua
dobuildvm(folddef lj_folddef.h lj_opt_fold.c)

if(WIN32)
  add_definitions(-D_FILE_OFFSET_BITS=64)
else(WIN32)
  if (APPLE)
    add_definitions(-U_FORTIFY_SOURCE -D_FILE_OFFSET_BITS=64)
  else(APPLE)
    add_definitions(-U_FORTIFY_SOURCE -fno-stack-protector -D_FILE_OFFSET_BITS=64)
  endif(APPLE)
endif(WIN32)

file(GLOB lua_files lj_*.c lib_*.c lj_*.h lib_*.h)

set(ASM_FLAGS "")

#if(${ENFORCE_32_BIT} EQUAL "1")
#  set(ASM_FLAGS ${ASM_FLAGS} -m32)
#endif(${ENFORCE_32_BIT} EQUAL "1")

#if(${ENFORCE_64_BIT} EQUAL "1")
#  set(ASM_FLAGS ${ASM_FLAGS} -m64)
#endif(${ENFORCE_64_BIT} EQUAL "1")

IF(WIN32)

ELSE(WIN32)
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lj_vm.o
    COMMAND gcc
    ARGS -c ${ASM_FLAGS} -o ${CMAKE_CURRENT_BINARY_DIR}/lj_vm.o ${CMAKE_CURRENT_BINARY_DIR}/lj_vm.s
    DEPENDS buildvm ${CMAKE_CURRENT_BINARY_DIR}/lj_vm.s
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "compiling lj_vm.s"
  )

ENDIF(WIN32)


file(GLOB luajit_generated_files ${CMAKE_CURRENT_BINARY_DIR}/lj_*.h)

add_library(lua ${lua_files} ${luajit_generated_files} ${CMAKE_CURRENT_BINARY_DIR}/${LJVM_OBJECT_FILE})
#set_target_properties(lua PROPERTIES LINKER_LANGUAGE C)

SOURCE_GROUP(Generated FILES ${luajit_generated_files})


add_dependencies(lua buildvm_asm buildvm_ffdef buildvm_libdef buildvm_recdef buildvm_vmdef buildvm_folddef)

#add_executable(luaexe luajit.c)
#target_link_libraries(luaexe m dl lua)
#set_target_properties(luaexe PROPERTIES LINKER_LANGUAGE "C" OUTPUT_NAME "lua")

set(compile_flags "")
set(link_flags "")
set(definitions "")

if(UNIX)
  #set(compile_flags ${compile_flags} "-x c++")
  if(APPLE)
    set(LIBS ${LIBS} readline)
    set(definitions "${definitions} -DLUA_USE_MACOSX -DLUA_USE_DLOPEN -DLUA_USE_READLINE")
    set(link_flags ${link_flags} "-undefined dynamic_lookup")
  else(APPLE)
    set(definitions ${definitions} -DLUA_USE_LINUX)
    set(LIBS ${LIBS} lua m dl)
    set(link_flags ${link_flags} "-Wl,-E")
  endif(APPLE)
else(UNIX)
  if(WIN32)
    #set(definitions "${definitions} -DLUA_BUILD_AS_DLL")
    set(definitions "${definitions} -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS")
    #set(compile_flags ${compile_flags} "/TP")
  endif(WIN32)
endif(UNIX)

if(NOT ${definitions} EQUAL "")
  add_definitions(${definitions})
endif(NOT ${definitions} EQUAL "")

if(NOT ${compile_flags} EQUAL "")
  set_target_properties(lua PROPERTIES COMPILE_FLAGS ${compile_flags})
endif(NOT ${compile_flags} EQUAL "")

if(NOT ${link_flags} EQUAL "")
  set_target_properties(lua PROPERTIES LINK_FLAGS ${link_flags})
endif(NOT ${link_flags} EQUAL "")

#target_link_libraries(luaexe ${LIBS})

