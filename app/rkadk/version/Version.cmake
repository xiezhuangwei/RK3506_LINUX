# app/rkadk/version/Version.cmake

message(STATUS "Using fixed version with legacy macro names")

set(VERSION_INFO "git-6cdc879 Fri Nov 22 11:39:55 2024 +0800")
string(TIMESTAMP RKADK_BUILD_TIME "%Y-%m-%d %H:%M:%S")
set(BUILD_INFO "built ${RKADK_BUILD_TIME}")

configure_file(
    "${PROJECT_SOURCE_DIR}/version/version.in"
    "${PROJECT_SOURCE_DIR}/include/version.h"
)