include(InstallConfig)
include(BuildInfo)

find_package(date REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(h5cpp 0.4.0 REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(daqlite)
add_subdirectory(schemasniffer)
add_subdirectory(ar51consumer)
