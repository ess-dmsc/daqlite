include(QtLibraryConfig)
find_package(RdKafka REQUIRED)
find_package(qplot REQUIRED)

set(fylgje_src
  fylgje_main.cpp
  fylgje_window.cpp
  WorkerThread.cpp
  KafkaConfig.cpp
  ESSConsumer.cpp
  data_manager.cpp
  Configuration.cpp
  )

set(fylgje_inc
  Configuration.h
  ESSConsumer.h
  KafkaConfig.h
  fylgje_window.h
  plot_manager.h
  two_spin_box.h
  data_manager.h
  WorkerThread.h
  JsonFile.h
  cycles.h
  )

set(fylgje_ui
  fylgje_window.ui
  )

set(CMAKE_AUTOUIC ON)

add_executable(
  fylgje
  ${fylgje_src}
  ${fylgje_inc}
  ${fylgje_ui}
)

target_link_libraries(
  fylgje
  PUBLIC fmt::fmt
  PRIVATE RdKafka::rdkafka++
  PRIVATE RdKafka::rdkafka
  PRIVATE QPlot
  PRIVATE QtColorWidgets
  PRIVATE Qt6::Widgets
)

target_link_libraries(fylgje
  PRIVATE $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>)
target_link_libraries(fylgje
  PRIVATE $<$<AND:$<CXX_COMPILER_ID:AppleClang>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,11.0>>:c++fs>)
