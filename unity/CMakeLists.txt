file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS lib/* inc/*)

add_library(unity-lib ${LIB_SOURCES})

target_include_directories(unity-lib PUBLIC inc)

target_link_libraries(unity-lib PRIVATE common-lib)
target_link_libraries(unity-lib PRIVATE platform-lib)
target_link_libraries(unity-lib PRIVATE archive-lib)
