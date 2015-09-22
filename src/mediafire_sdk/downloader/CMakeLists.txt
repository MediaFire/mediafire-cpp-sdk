cmake_minimum_required(VERSION 2.8)

project (mf_downloader_sdk CXX)

set(MF_DOWNLOADER_HEADERS
  acceptor_interface.hpp
  callbacks.hpp
  download.hpp
  download_config.hpp
  download_interface.hpp
  download_status.hpp
  error.hpp

  detail/download_container.hpp
  detail/file_writer.hpp
  detail/header_utils.hpp
  detail/memory_writer.hpp
  detail/no_target_writer.hpp
  detail/request_response_proxy.hpp
  detail/unknown_name_file_writer.hpp

  error/conditions/generic.hpp

  reader/md5_reader.hpp
  reader/reader_interface.hpp
  reader/sha256_reader.hpp
  )

set(MF_DOWNLOADER_SOURCES
  download.cpp

  detail/download_container.cpp
  detail/file_writer.cpp
  detail/header_utils.cpp
  detail/memory_writer.cpp
  detail/no_target_writer.cpp
  detail/unknown_name_file_writer.cpp

  error/conditions/generic.cpp

  reader/md5_reader.cpp
  reader/sha256_reader.cpp
  )

foreach ( file ${MF_DOWNLOADER_HEADERS} )
  get_filename_component( dir ${file} DIRECTORY )
  install( FILES ${file} DESTINATION include/mediafire_sdk/downloader/${dir} )
endforeach()

add_definitions(
  -DBOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
  -DBOOST_MPL_LIMIT_VECTOR_SIZE=40 # Set max MSM transitions
  -DBOOST_MPL_LIMIT_MAP_SIZE=40 # Set max MSM transitions
  -DFUSION_MAX_VECTOR_SIZE=20 # Set max MSM states
  )

add_library(mf_downloader_sdk STATIC
  ${MF_DOWNLOADER_HEADERS}
  ${MF_DOWNLOADER_SOURCES}
  )
install(TARGETS mf_downloader_sdk DESTINATION lib)

TARGET_LINK_LIBRARIES(mf_downloader_sdk
  ${MFAPI_APP_CONSTANTS_LIBRARY}
  mf_http_sdk
  )

add_subdirectory(unit_tests)
