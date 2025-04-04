include(QtLibraryConfig)
find_package(RdKafka REQUIRED)
find_package(qplot REQUIRED)
find_package(h5cpp REQUIRED)

set(fylgje_src
  AppMain.cpp
  AppWindow.cpp
  AppOverview.cpp
  AppCalibration.cpp
  AppData.cpp
  Calibration.cpp
  Configuration.cpp
  DataManager.cpp
  ESSConsumer.cpp
  KafkaConfig.cpp
  PlotManager.cpp
  TableItemTypes.cpp
  WorkerThread.cpp
)

set(fylgje_inc
  AppWindow.h
  Calibration.h
  Configuration.h
  Cycles.h
  DataManager.h
  ESSConsumer.h
  JsonFile.h
  KafkaConfig.h
  PlotManager.h
  TwoSpinBox.h
  WorkerThread.h
)

set(fylgje_ui
  AppWindow.ui
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
  PRIVATE Qt6::Widgets
  PRIVATE h5cpp::h5cpp
)

target_link_libraries(fylgje
  PRIVATE $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>)
target_link_libraries(fylgje
  PRIVATE $<$<AND:$<CXX_COMPILER_ID:AppleClang>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,11.0>>:c++fs>)
