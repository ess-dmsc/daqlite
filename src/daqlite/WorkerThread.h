// Copyright (C) 2020 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file WorkerThread.h
///
/// \brief main consumer loop for Daquiri Light (daqlite)
/// The worker thread continuously calls ESSConsumer::consume() and
/// ESSConsumer::handleMessage() to histogram the pixelids. Once every second
/// the histogram is copied and the plotting thread (qt main thread?) is
/// notified.
//===----------------------------------------------------------------------===//

#pragma once

#include <ESSConsumer.h>
#include <KafkaConfig.h>
#include <QMutex>
#include <QThread>
#include <QMutex>
#include <iostream>

class WorkerThread : public QThread {
  Q_OBJECT

public:
  WorkerThread(Configuration &Config) : mConfig(Config) {
    KafkaConfig KafkaCfg(Config.KafkaConfigFile);
    Consumer = new ESSConsumer(Config, KafkaCfg.CfgParms);
  };

  ~WorkerThread(){};

  /// \brief thread main loop
  void run() override;

  /// \brief Getter for the consumer
  ESSConsumer *consumer() { return Consumer; }

signals:
  /// \brief this signal is 'emitted' when there is new data to be plotted
  /// this is done periodically (approximately once every second)
  void resultReady(int &val);

private:
  /// \brief configuration obtained from main()
  Configuration &mConfig;

  /// \brief Kafka consumer
  ESSConsumer *Consumer;
};
