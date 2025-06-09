file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS bin/*)

add_executable(playground-app ${APP_SOURCES})

target_link_libraries(playground-app PRIVATE unity-lib)
target_link_libraries(playground-app PRIVATE platform-lib)
target_link_libraries(playground-app PRIVATE common-lib)
target_link_libraries(playground-app PRIVATE archive-lib)
