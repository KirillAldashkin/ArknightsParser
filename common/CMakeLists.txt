file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS lib/* inc/*)

add_library(common-lib ${LIB_SOURCES})

target_include_directories(common-lib PUBLIC inc)
