project(IF_CM_Workers)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

set(EXPORT_HEADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/includes")
set(HEADERS
    ${EXPORT_HEADERS_DIR}/cmwork.h
)

add_library(IF_CM_Workers INTERFACE ${HEADERS})
target_include_directories(IF_CM_Workers INTERFACE ${EXPORT_HEADERS_DIR})