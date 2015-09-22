add_definitions("-DDI_MODULE_NAME=\"utils\"")
add_subdirectory(unit_tests)

if(WIN32)
  set(OS_DIR "win")
elseif(UNIX)
  set(OS_DIR "unix")
else(WIN32)
  message(FATAL_ERROR "Not windows or unix")
endif(WIN32)

set(UTILS_SOURCES
  error/codes/fileio.cpp

  base64.cpp
  fileio.cpp
  io_service_connection.cpp
  md5_hasher.cpp
  sha1_hasher.cpp
  sha256_hasher.cpp
  string.cpp
  thread_id.cpp
  time.cpp
  url_encode.cpp
  ${OS_DIR}/thread_id_impl.cpp
  )

set(UTILS_HEADERS
  base64.hpp
  cancellable_interface.hpp
  container_algorithms.hpp
  error_or.hpp
  fileio.hpp
  forward_declarations/asio.hpp
  io_service_connection-inl.hpp
  io_service_connection.hpp
  io_service_connection_helper.hpp
  md5_hasher.hpp
  mutex.hpp
  pauseable_interface.hpp
  sha1_hasher.hpp
  sha256_hasher.hpp
  string.hpp
  thread_id.hpp
  thread_id_impl.hpp
  time.hpp
  timed_actions.hpp
  timed_events.hpp
  url_encode.hpp
  variant.hpp
  variant_comparison.hpp
  )

foreach( file ${UTILS_HEADERS} )
  get_filename_component( dir ${file} DIRECTORY )
  install(FILES ${file} DESTINATION include/mediafire_sdk/utils/${dir} )
endforeach()

add_library(mf_sdk_utils STATIC
  ${UTILS_HEADERS}
  ${UTILS_SOURCES}
  )
install(TARGETS mf_sdk_utils DESTINATION lib)
