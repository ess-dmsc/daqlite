// Copyright (C) 2020 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file daqlite.cpp
///
/// \brief Daquiri Light main application
///
/// Handles command line option(s), instantiates GUI
//===----------------------------------------------------------------------===//

#include <Configuration.h>
#include <MainWindow.h>
#include <WorkerThread.h>

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

namespace {
  /// \brief Extract specified command line options and add them to the plot configuration

  /// \param CLI     Command line options
  /// \param Config  Plot configuration
  void setKafkaOptions(const QCommandLineParser &CLI, Configuration &Config) {
    for (const QString &option: CLI.optionNames()) {
      if (option == "b") {
        Config.mKafka.Broker = CLI.value(option).toStdString();
        fmt::print("<<<< \n WARNING Overriding kafka broker to {} \n>>>>\n", Config.mKafka.Broker);
      }

      else if (option == "t") {
        Config.mKafka.Topic = CLI.value(option).toStdString();
        fmt::print("<<<< \n WARNING Overriding kafka topic to {} \n>>>>\n", Config.mKafka.Topic);
      }

      else if (option == "k") {
        Config.mKafkaConfigFile = CLI.value(option).toStdString();
        fmt::print("<<<< \n WARNING Overriding path to kafka config file to {} \n>>>>\n", Config.mKafkaConfigFile);
      }
    }
  }
}

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  // Handle all command line args
  QCommandLineParser CLI;
  CLI.setApplicationDescription("Daquiri light - when you're driving home");
  CLI.addHelpOption();

  // Add specified options
  std::vector<std::tuple<QString, QString, QString>> Options = {
    {"f", "Configuration file",       "unusedDefault"},
    {"b", "Kafka broker",             "unusedDefault"},
    {"t", "Kafka topic",              "unusedDefault"},
    {"k", "Kafka configuration file", "unusedDefault"},
  };
  for (const auto& [key, info, unused]: Options) {
    QCommandLineOption option(key, info, unused);
    CLI.addOption(option);
  }
  CLI.process(app);

  // Parent button used to quit all plot widgets
  QPushButton main("&Quit");
  main.connect(&main, &QPushButton::clicked, app.quit);

  // ---------------------------------------------------------------------------
  // Get top configuration
  const std::string FileName = CLI.value("f").toStdString();
  std::vector<Configuration> confs = Configuration::getConfigurations(FileName);
  Configuration MainConfig = confs.front();
  setKafkaOptions(CLI, MainConfig);

  // Setup worker thread
  std::shared_ptr<WorkerThread> Worker = std::make_shared<WorkerThread>(MainConfig);

  // Setup a window for each plot
  for (size_t i=0; i < confs.size(); ++i) {
    Configuration Config = confs[i];
    setKafkaOptions(CLI, MainConfig);

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
