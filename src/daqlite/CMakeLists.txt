include(QtLibraryConfig)
find_package(RdKafka REQUIRED)
find_package(qplot REQUIRED)

set(daqlite_src
  AbstractPlot.cpp
  AMOR2DTofPlot.cpp
  Configuration.cpp
  daqlite.cpp
  ESSConsumer.cpp
  HelpWindow.cpp
  HistogramPlot.cpp
  KafkaConfig.cpp
  MainWindow.cpp
  PixelsPlot.cpp
  TofPlot.cpp
  WorkerThread.cpp
  )

set(daqlite_inc
  AbstractPlot.h
  AMOR2DTofPlot.h
  Configuration.h
  ESSConsumer.h
  HelpWindow.h
  HistogramPlot.h
  KafkaConfig.h
  MainWindow.h
  PixelsPlot.h
  ThreadSafeVector.h
  TofPlot.h
  WorkerThread.h

  # Types
  types/PlotType.h
  types/CommonTypes.h
  )

set(daqlite_ui
  MainWindow.ui
  )

set(CMAKE_AUTOUIC ON)

add_executable(
  daqlite
  ${daqlite_src}
  ${daqlite_inc}
  ${daqlite_ui}
)

target_link_libraries(
  daqlite
  PUBLIC fmt::fmt
  PRIVATE RdKafka::rdkafka++
  PRIVATE RdKafka::rdkafka
  PRIVATE QPlot
  PRIVATE Qt6::Widgets
  PRIVATE Qt6::Core5Compat
)

target_link_libraries(daqlite
  PRIVATE $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>)
target_link_libraries(daqlite
  PRIVATE $<$<AND:$<CXX_COMPILER_ID:AppleClang>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,11.0>>:c++fs>)
