// Copyright (C) 2020 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file daqlite.cpp
///
/// \brief Daquiri Light main application
///
/// Handles command line option(s), instantiates GUI
//===----------------------------------------------------------------------===//

#include <Configuration.h>
#include <Custom2DPlot.h>
#include <MainWindow.h>
#include <WorkerThread.h>

#include <QApplication>
#include <QCommandLineParser>
#include <QPlot/QPlot.h>
#include <fmt/format.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  QCommandLineParser CLI;
  CLI.setApplicationDescription("Daquiri light - when you're driving home");
  CLI.addHelpOption();

  QCommandLineOption fileOption("f", "Configuration file", "unusedDefault");
  CLI.addOption(fileOption);

  QCommandLineOption kafkaBrokerOption("b", "Kafka broker", "unusedDefault");
  CLI.addOption(kafkaBrokerOption);

  QCommandLineOption kafkaTopicOption("t", "Kafka topic", "unusedDefault");
  CLI.addOption(kafkaTopicOption);

  QCommandLineOption kafkaConfigOption("k", "Kafka configuration file", "unusedDefault");
  CLI.addOption(kafkaConfigOption);

  CLI.process(app);

  Configuration Config;
  if (CLI.isSet(fileOption)) {
    std::string FileName = CLI.value(fileOption).toStdString();
    Config.fromJsonFile(FileName);
  }

  if (CLI.isSet(kafkaBrokerOption)) {
    std::string KafkaBroker = CLI.value(kafkaBrokerOption).toStdString();
    Config.Kafka.Broker = KafkaBroker;
    printf("<<<< \n WARNING Override kafka broker to %s \n>>>>\n", Config.Kafka.Broker.c_str());
  }

  if (CLI.isSet(kafkaTopicOption)) {
    std::string KafkaTopic = CLI.value(kafkaTopicOption).toStdString();
    Config.Kafka.Topic = KafkaTopic;
    printf("<<<< \n WARNING Override kafka topic to %s \n>>>>\n", Config.Kafka.Topic.c_str());
  }

  if (CLI.isSet(kafkaConfigOption)) {
    std::string FileName = CLI.value(kafkaConfigOption).toStdString();
    Config.KafkaConfigFile = FileName;
  }

  MainWindow w(Config);
  w.setWindowTitle(QString::fromStdString(Config.Plot.WindowTitle));
  w.resize(Config.Plot.Width, Config.Plot.Height);

  return app.exec();
}
