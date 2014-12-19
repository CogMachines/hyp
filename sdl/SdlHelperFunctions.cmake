






























































































































































function(xmt_add_library_explicit PROJ_NAME)
  if (WIN32)



  endif()
  add_library(${PROJ_NAME} ${ARGN} ${INCS} ${TEMPLATE_IMPLS})
  xmt_msvc_links(${PROJ_NAME})
endfunction(xmt_add_library_explicit)

function(xmt_add_executable_explicit PROJ_NAME)
  if (WIN32)



  endif()
  add_executable(${PROJ_NAME} ${ARGN} ${INCS} ${TEMPLATE_IMPLS})
  xmt_msvc_links(${PROJ_NAME})
endfunction(xmt_add_executable_explicit)
































































































































  add_custom_target(${TARGET_NAME} ALL COMMAND ${MAVEN_EXECUTABLE} -Dmaven.project.build.directory=${work_dir} -fae -B -q -f pom.xml -Dmaven.test.skip=true clean compile package




function(xmt_maven_project_with_profile TARGET_NAME BINARY_DIR PROFILE_NAME)
  set(work_dir ${BINARY_DIR}/target)
  message(STATUS "Run mvn package: \"${MAVEN_EXECUTABLE}\" wth profile:\"${PROFILE_NAME}\"")

  add_custom_target(${TARGET_NAME} ALL COMMAND ${MAVEN_EXECUTABLE} -P${PROFILE_NAME} -Dmaven.project.build.directory=${work_dir} -fae -B -q -f pom.xml -Dmaven.test.skip=true clean compile package
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})





    -Dmaven.project.build.directory=${work_dir} -Dtest_Dir=${CMAKE_CURRENT_SOURCE_DIR}/src/test/unit









function(xmt_maven_project_with_profile_test TARGET_NAME BINARY_DIR PROFILE_NAME)
  xmt_maven_project_with_profile(${TARGET_NAME} ${BINARY_DIR} ${PROFILE_NAME})
  xmt_maven_test(${TARGET_NAME} ${BINARY_DIR})
endfunction(xmt_maven_project_with_profile_test)








