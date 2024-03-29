// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file ar52consumer.cpp
///
/// \brief ar52 schema consumer - proof of concept
///
/// Identifies types of modes
//===----------------------------------------------------------------------===//

#include <ESSConsumer.h>
#include <MainWindow.h>
#include <QApplication>
#include <QCommandLineParser>
#include <WorkerThread.h>
#include <stdio.h>


struct {
  QCommandLineOption KafkaBrokerOption{"b", "Kafka broker", "unusedDefault"};
  QCommandLineOption KafkaTopicOption{"t", "Kafka topic", "unusedDefault"};
  QCommandLineOption ReadoutType{"r", "readout", "unusedDefault"};
} Options;


int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  QCommandLineParser CLI;
  CLI.setApplicationDescription("Area51 - where lost channels are found");
  CLI.addHelpOption();


  CLI.addOption(Options.KafkaBrokerOption);
  CLI.addOption(Options.KafkaTopicOption);
  CLI.addOption(Options.ReadoutType);

  CLI.process(app);

  MainWindow win {
    CLI.value(Options.KafkaBrokerOption).toStdString(),
    CLI.value(Options.KafkaTopicOption).toStdString(),
    CLI.value(Options.ReadoutType).toStdString()
  };
  win.resize(1400, 600);
  return app.exec();
}
