#
# Test executable
#


add_test(NAME DTIPrepTest COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:DTIPrep> --help )

if( UNIX )
  set( prog_name Replace_basename_by_itksys_GetFilenameName )
  add_executable(${prog_name}Test ${prog_name}.cxx)
  target_link_libraries(${prog_name}Test ${ITK_LIBRARIES})
  add_test(NAME ${prog_name}Test COMMAND $<TARGET_FILE:${prog_name}Test> )

  set( prog_name Replace_basename_by_itksys_GetFilenamePath )
  add_executable(${prog_name}Test ${prog_name}.cxx)
  target_link_libraries(${prog_name}Test ${ITK_LIBRARIES})
  add_test(NAME ${prog_name}Test COMMAND $<TARGET_FILE:${prog_name}Test> )
endif()
