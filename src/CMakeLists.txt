include(InstallConfig)
include(BuildInfo)

find_package(fmt REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(daqlite)
add_subdirectory(schemasniffer)
add_subdirectory(ar51consumer)
add_subdirectory(fylgje)
