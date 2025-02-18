# Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file

include(QtLibraryConfig)
find_package(RdKafka REQUIRED)
find_package(qplot REQUIRED)

#

set(ar51consumer_src
  ar51consumer.cpp
  ESSConsumer.cpp
  GraphBase.cpp
  VMM3aGraph.cpp
  CAENGraph.cpp
  CDTGraph.cpp
  WorkerThread.cpp
  MainWindow.cpp
  )

set(ar51consumer_inc
  ESSConsumer.h
  GraphBase.h
  VMM3aGraph.h
  CAENGraph.h
  CDTGraph.h
  WorkerThread.h
  MainWindow.h
  )


add_executable(
  ar51consumer
  ${ar51consumer_src}
  ${ar51consumer_inc}
)

target_link_libraries(
  ar51consumer
  PUBLIC fmt::fmt
  PRIVATE RdKafka::rdkafka++
  PRIVATE RdKafka::rdkafka
  PRIVATE QPlot
  PRIVATE QtColorWidgets
  PRIVATE Qt6::Widgets
)

target_link_libraries(ar51consumer
  PRIVATE $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>)
target_link_libraries(ar51consumer
  PRIVATE $<$<AND:$<CXX_COMPILER_ID:AppleClang>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,11.0>>:c++fs>)
