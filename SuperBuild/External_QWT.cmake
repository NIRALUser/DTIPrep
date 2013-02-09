ExternalProject_add(QWT
  SVN_REPOSITORY http://svn.code.sf.net/p/qwt/code/branches/qwt-6.0
#SVN_REPOSITORY http://svn.code.sf.net/p/qwt/code/trunk/qwt
#SVN_REVISION -r "1672"  ## Sourceforge code moved.
  SOURCE_DIR QWT
  BINARY_DIR QWT-build
  ${cmakeversion_external_update} "${cmakeversion_external_update_value}"
  PATCH_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt.qwt <SOURCE_DIR>/CMakeLists.txt
  CMAKE_ARGS ${COMMON_EXTERNAL_PROJECT_ARGS}
  -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}
  INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}
)

set(QWT_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/lib/libqwt.a)

set(QWT_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include)

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_BINARY_DIR} )
