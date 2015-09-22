
# --- download_url -----------------------------------------

add_executable(download_url
  download_url.cpp
  )

target_link_libraries(download_url
  mf_downloader_sdk
  ${Boost_LIBRARIES}
  )

# this is a standalone test program and isn't automated.
#add_test(download_url download_url)

# --- ut_downloader ----------------------------------------
add_executable(ut_downloader
  ut_downloader.cpp
  )

target_link_libraries(ut_downloader
  mf_downloader_sdk
  ut_expect_server
  ${Boost_LIBRARIES}
  )

add_test(ut_downloader
  ut_downloader "${CMAKE_CURRENT_SOURCE_DIR}/test_sources"
  )
