
if (WIN32 AND NOT DEFINED MINGW)
    # We are using the full path of the file to find the template directory.
    # This changes __FILE__ to have the full path on Windows.
    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FC")
endif(WIN32 AND NOT DEFINED MINGW)

# Unit test usernames/passwords

if ( "${TEST_USER_1_USERNAME}" STREQUAL "" )
  message(WARNING
    "The following variables must be set in order for unit tests to work correctly:
        TEST_USER_1_USERNAME
        TEST_USER_2_USERNAME
        TEST_USER_1_PASSWORD
        TEST_USER_2_PASSWORD"
    )
    message("Please define these variables in your cache in order to run unit tests.")
endif( "${TEST_USER_1_USERNAME}" STREQUAL "" )

add_definitions(-DTEST_USER_1_USERNAME="${TEST_USER_1_USERNAME}")
add_definitions(-DTEST_USER_1_PASSWORD="${TEST_USER_1_PASSWORD}")
add_definitions(-DTEST_USER_2_USERNAME="${TEST_USER_2_USERNAME}")
add_definitions(-DTEST_USER_2_PASSWORD="${TEST_USER_2_PASSWORD}")

# --- session token server ---------------------------------

set(ST_SERVER_SOURCES
    api_ut_helpers.cpp
    session_token_test_server.cpp
    session_token_test_connection.cpp
    session_token_test_request.cpp
)
set(ST_SERVER_HEADERS
    api_ut_helpers.hpp
    session_token_test_server.hpp
    session_token_test_connection.hpp
    session_token_test_request.hpp
)

add_library(session_token_server STATIC
    ${ST_SERVER_HEADERS}
    ${ST_SERVER_SOURCES}
)

TARGET_LINK_LIBRARIES(session_token_server
    ${MFAPI_APP_CONSTANTS_LIBRARY}
    mf_sdk_utils
    ${OPENSSL_LIBRARIES}
    ${PTHREAD_LIBRARY}
    ${ZLIB_LIBRARIES}
    ${Boost_LIBRARIES}
)

# --- ut_api -----------------------------------------------
add_executable(ut_api
    api_ut_helpers.cpp
    ut_api.cpp)

target_link_libraries(ut_api
    mf_api_sdk
    ut_expect_server
)

add_test(ut_api
    ut_api "${CMAKE_CURRENT_SOURCE_DIR}/test_sources"
)

# --- ut_session_maintainer --------------------------
add_executable(ut_session_maintainer
    ut_session_maintainer.cpp)

target_link_libraries(ut_session_maintainer
    mf_api_sdk
    session_token_server
    ut_expect_server
)

add_test(ut_session_maintainer
    ut_session_maintainer "${CMAKE_CURRENT_SOURCE_DIR}/test_sources"
)

# --- ut_multithread ---------------------------------------
add_executable(ut_multithread
    ut_multithread.cpp)

target_link_libraries(ut_multithread
    mf_api_sdk
)

add_test(ut_multithread
    ut_multithread "${CMAKE_CURRENT_SOURCE_DIR}/test_sources"
)

# --- live -------------------------------------------------
set(UT_LIVE_SOURCES
    ut_live.cpp
)

set(UT_LIVE_HEADERS
    ut_live.hpp
)

add_library(ut_live STATIC
    ${UT_LIVE_HEADERS}
    ${UT_LIVE_SOURCES}
)

TARGET_LINK_LIBRARIES(ut_live
    mf_api_sdk
    session_token_server
    ut_expect_server
)

add_executable(ut_live_general ut_live_general.cpp)
target_link_libraries(ut_live_general
  ut_live
  mf_api_sdk
  mf_uploader_sdk
  ${Boost_LIBRARIES}
  )

add_executable(ut_live_general_wait ut_live_general.cpp)
set_target_properties(ut_live_general_wait
  PROPERTIES
    COMPILE_DEFINITIONS "MF_UNIT_TEST_LONG_WAIT"
  )
target_link_libraries(ut_live_general_wait
  ut_live
  mf_api_sdk
  mf_uploader_sdk
  ${Boost_LIBRARIES}
  )

add_executable(ut_live_billing ut_live_billing.cpp)
target_link_libraries(ut_live_billing ut_live)

add_executable(ut_live_device ut_live_device.cpp)
target_link_libraries(ut_live_device ut_live)

add_executable(ut_live_user ut_live_user.cpp)
target_link_libraries(ut_live_user ut_live)

add_executable(ut_live_notifications ut_live_notifications.cpp)
target_link_libraries(ut_live_notifications ut_live)

add_executable(ut_live_system ut_live_system.cpp)
target_link_libraries(ut_live_system ut_live)

add_executable(ut_live_folder ut_live_folder.cpp)
target_link_libraries(ut_live_folder ut_live)

# Don't run these automatically.
#add_test(ut_live
#    ut_live "${CMAKE_CURRENT_SOURCE_DIR}/test_sources"
#)

# Copy expected script and data files
file(COPY check_expected.sh DESTINATION .)
file(COPY ut_live_general-expected-failures.txt DESTINATION .)

# --- ut_clone_cloud_tree ----------------------------------
add_executable(ut_clone_cloud_tree
    ut_clone_cloud_tree.cpp)

target_link_libraries(ut_clone_cloud_tree
    mf_api_sdk
    ${Boost_LIBRARIES}
)

# --- ut_poll_changes --------------------------------------
add_executable(ut_poll_changes
    ut_poll_changes.cpp)

target_link_libraries(ut_poll_changes
    mf_api_sdk
    ${Boost_LIBRARIES}
)

# --- ut_get_info_folder --------------------------------------
add_executable(ut_get_info_folder
    ut_get_info_folder.cpp)

target_link_libraries(ut_get_info_folder
    mf_api_sdk
    ${Boost_LIBRARIES}
)

# --- ut_get_info_file --------------------------------------
add_executable(ut_get_info_file
    ut_get_info_file.cpp)

target_link_libraries(ut_get_info_file
    mf_api_sdk
    ${Boost_LIBRARIES}
)

# --- ut_get_changes_device --------------------------------------
add_executable(ut_get_changes_device
    ut_get_changes_device.cpp)

target_link_libraries(ut_get_changes_device
    mf_api_sdk
    ${Boost_LIBRARIES}
)

# --- ut_get_foreign_changes_device --------------------------------------
add_executable(ut_get_foreign_changes_device
    ut_get_foreign_changes_device.cpp)

target_link_libraries(ut_get_foreign_changes_device
    mf_api_sdk
    ${Boost_LIBRARIES}
)

# --- ut_poll_foreign_changes --------------------------------------
add_executable(ut_poll_foreign_changes
    ut_poll_foreign_changes.cpp)

target_link_libraries(ut_poll_foreign_changes
    mf_api_sdk
    ${Boost_LIBRARIES}
)

# --- ut_move_file --------------------------------------
add_executable(ut_move_file
    ut_move_file.cpp)

target_link_libraries(ut_move_file
    mf_api_sdk
    ${Boost_LIBRARIES}
)

# --- ut_move_folder --------------------------------------
add_executable(ut_move_folder
    ut_move_folder.cpp)

target_link_libraries(ut_move_folder
    mf_api_sdk
    ${Boost_LIBRARIES}
)

# --- ut_delete_file --------------------------------------
add_executable(ut_delete_file
    ut_delete_file.cpp)

target_link_libraries(ut_delete_file
    mf_api_sdk
    ${Boost_LIBRARIES}
)

# --- ut_delete_folder --------------------------------------
add_executable(ut_delete_folder
    ut_delete_folder.cpp)

target_link_libraries(ut_delete_folder
    mf_api_sdk
    ${Boost_LIBRARIES}
)

# --- ut_create_folder --------------------------------------
add_executable(ut_create_folder
    ut_create_folder.cpp)

target_link_libraries(ut_create_folder
    mf_api_sdk
    ${Boost_LIBRARIES}
)

# --- ut_get_foreign_resources_device ------------------------
add_executable(ut_get_foreign_resources_device
    ut_get_foreign_resources_device.cpp)

target_link_libraries(ut_get_foreign_resources_device
    mf_api_sdk
    ${Boost_LIBRARIES}
)

# --- ut_rename_file ------------------------
add_executable(ut_rename_file
    ut_rename_file.cpp)

target_link_libraries(ut_rename_file
    mf_api_sdk
    ${Boost_LIBRARIES}
)

# --- ut_rename_folder ------------------------
add_executable(ut_rename_folder
    ut_rename_folder.cpp)

target_link_libraries(ut_rename_folder
    mf_api_sdk
    ${Boost_LIBRARIES}
)
