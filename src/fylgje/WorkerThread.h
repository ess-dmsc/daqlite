// Copyright (C) 2023-2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file WorkerThread.h
///
/// \brief main consumer loop for ar51consumer
//===----------------------------------------------------------------------===//

#pragma once

#include "ESSConsumer.h"
#include "KafkaConfig.h"
#include "Configuration.h"
#include <QThread>
#include <utility>

class WorkerThread : public QThread {
  Q_OBJECT

public:
  using data_t = ESSConsumer::data_t;
  WorkerThread(data_t * data, Configuration & Config):
  configuration(Config) {
    KafkaConfig kcfg(Config.KafkaConfigFile);
    Consumer = new ESSConsumer(data, configuration, kcfg.CfgParms);
  };

  /// \brief thread main loop
  void run() override;

  /// \brief Kafka consumer
  ESSConsumer *Consumer;

  void consume_from(int64_t ms_since_utc_epoch);
  void consume_until(int64_t ms_since_utc_epoch);

private:
  Configuration &configuration;

signals:
  /// \brief this signal is 'emitted' when there is new data
  void resultReady();

};
