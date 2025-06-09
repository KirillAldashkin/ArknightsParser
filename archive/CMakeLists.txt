file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS lib/* inc/*)

add_library(archive-lib ${LIB_SOURCES})

target_compile_definitions(archive-lib PRIVATE 
  Z7_LZMA_PROB32)

target_include_directories(archive-lib PUBLIC inc)
