// Copyright (C) 2020 - 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file daqlite.cpp
///
/// \brief Daquiri Light main application
///
/// Handles command line option(s), instantiates GUI
//===----------------------------------------------------------------------===//

#include "Configuration.h"
#include "MainWindow.h"
#include "WorkerThread.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QPushButton>
#include <QString>

#include <fmt/format.h>

#include <stdio.h>
#include <memory>
#include <string>
#include <vector>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  // Handle all commandline args
  QCommandLineParser CLI;
  CLI.setApplicationDescription("Daquiri light - when you're driving home");
  CLI.addHelpOption();

  QCommandLineOption configurationOption("f", "Configuration file", "unusedDefault");
  CLI.addOption(configurationOption);

  QCommandLineOption kafkaBrokerOption("b", "Kafka broker", "unusedDefault");
  CLI.addOption(kafkaBrokerOption);

  QCommandLineOption kafkaTopicOption("t", "Kafka topic", "unusedDefault");
  CLI.addOption(kafkaTopicOption);

  QCommandLineOption kafkaConfigOption("k", "Kafka configuration file", "unusedDefault");
  CLI.addOption(kafkaConfigOption);

  CLI.process(app);

  // Parent button used to quit all plot widgets
  QPushButton main("&Quit");
  main.connect(&main, &QPushButton::clicked, app.quit);

  // ---------------------------------------------------------------------------
  // Get top configuration
  const std::string FileName = CLI.value(configurationOption).toStdString();
  std::vector<Configuration> confs = Configuration::getConfigurations(FileName);
  Configuration MainConfig = confs.front();

  if (CLI.isSet(kafkaBrokerOption)) {
    MainConfig.mKafka.Broker = CLI.value(kafkaBrokerOption).toStdString();
    fmt::print("<<<< \n WARNING Overriding kafka broker to {} \n>>>>\n", MainConfig.mKafka.Broker);
  }

  if (CLI.isSet(kafkaTopicOption)) {
    MainConfig.mKafka.Topic = CLI.value(kafkaTopicOption).toStdString();
    fmt::print("<<<< \n WARNING Overriding kafka topic to {} \n>>>>\n", MainConfig.mKafka.Topic);
  }

  if (CLI.isSet(kafkaConfigOption)) {
    MainConfig.mKafkaConfigFile = CLI.value(kafkaConfigOption).toStdString();
  }

  // Setup worker thread
  std::shared_ptr<WorkerThread> Worker = std::make_shared<WorkerThread>(MainConfig);


  // Setup a window for each plot
  for (size_t i=1; i < confs.size(); ++i) {
    Configuration Config = confs[i];

    if (CLI.isSet(kafkaBrokerOption)) {
      Config.mKafka.Broker = CLI.value(kafkaBrokerOption).toStdString();
      fmt::print("<<<< \n WARNING Overriding kafka broker to {} \n>>>>\n", MainConfig.mKafka.Broker);
    }

    if (CLI.isSet(kafkaTopicOption)) {
      Config.mKafka.Topic = CLI.value(kafkaTopicOption).toStdString();
      fmt::print("<<<< \n WARNING Overriding kafka topic to {} \n>>>>\n", MainConfig.mKafka.Topic);
    }

    if (CLI.isSet(kafkaConfigOption)) {
      Config.mKafkaConfigFile = CLI.value(kafkaConfigOption).toStdString();
    }

    MainWindow* w = new MainWindow(Config, Worker.get());
    w->setWindowTitle(QString::fromStdString(Config.mPlot.WindowTitle));
    w->setParent(&main, Qt::Window);
    w->show();
  }

  // Start the worker and let the Qt event handler take over
  Worker->start();
  // main.show();
  // main.raise();

  return app.exec();
}
