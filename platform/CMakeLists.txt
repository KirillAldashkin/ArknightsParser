file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS lib/* inc/*)

add_library(platform-lib ${LIB_SOURCES})

target_link_libraries(platform-lib PRIVATE common-lib)

target_include_directories(platform-lib PUBLIC inc)
if (WIN32)
  target_compile_definitions(platform-lib PUBLIC OS_WINDOWS)
elseif (UNIX)
  target_compile_definitions(platform-lib PUBLIC OS_UNIX)
else()
  message(SEND_ERROR "Unknown target OS")
endif()

if (CMAKE_CXX_BYTE_ORDER STREQUAL BIG_ENDIAN)
  target_compile_definitions(platform-lib PUBLIC BYTE_ORDER_BIG)
elseif (CMAKE_CXX_BYTE_ORDER STREQUAL LITTLE_ENDIAN)
  target_compile_definitions(platform-lib PUBLIC BYTE_ORDER_LITTLE)
else()
  message(SEND_ERROR "Unknown byte order")
endif()