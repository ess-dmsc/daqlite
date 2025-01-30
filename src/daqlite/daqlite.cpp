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

#include <memory>
#include <fmt/format.h>
#include <stdio.h>


void prettyJson(nlohmann::json &obj, const std::string &header, int indent=4) {
  fmt::print("{}:\n", header);
  std::cout << obj.dump(indent) << "\n\n" << std::endl;
}

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



  // Parent button used to quit all plot widgets
  QWidget mainWidget;
  QPushButton button("&Quit");
  QVBoxLayout layout;
  mainWidget.setLayout(&layout);
  layout.addWidget(&button);

  button.connect(&button, &QPushButton::clicked, app.quit);
  mainWidget.show();

  // ---------------------------------------------------------------------------
  // Parse main config file
  const std::string FileName = CLI.value(fileOption).toStdString();
  std::ifstream ifs(FileName, std::ofstream::in);
  if (!ifs.good()) {
    throw(std::runtime_error("Unable to create ifstream (bad filename?), exiting ..."));
  }

  nlohmann::json MainJsonObj;
  ifs >> MainJsonObj;
  std::cout << MainJsonObj << "\n\n";

  // Generate mother config
  Configuration MainConfig;
  MainConfig.fromJsonObj(MainJsonObj);

  if (CLI.isSet(kafkaBrokerOption)) {
    std::string KafkaBroker = CLI.value(kafkaBrokerOption).toStdString();
    MainConfig.Kafka.Broker = KafkaBroker;
    printf("<<<< \n WARNING Override kafka broker to %s \n>>>>\n", MainConfig.Kafka.Broker.c_str());
  }

  if (CLI.isSet(kafkaTopicOption)) {
    std::string KafkaTopic = CLI.value(kafkaTopicOption).toStdString();
    MainConfig.Kafka.Topic = KafkaTopic;
    printf("<<<< \n WARNING Override kafka topic to %s \n>>>>\n", MainConfig.Kafka.Topic.c_str());
  }

  std::shared_ptr<WorkerThread> MainWorker = std::make_shared<WorkerThread>(MainConfig);
//  WorkerThread MainWorker(MainConfig);


  // Loop over all plots
  nlohmann::json plots = MainJsonObj["plots"];
  prettyJson(plots, "All Plots");

  // Generate json for each plot and set up a CustomPlot
  int count = 0;
  for (const auto& plot : plots.items()) {
    nlohmann::json PlotObj = MainJsonObj;

    const auto iter = PlotObj.find("plots");
    if (iter != PlotObj.cend()) {
      PlotObj.erase(iter);
    }
    PlotObj["plot"] = plot.value();

    prettyJson(PlotObj, "Plot " + std::to_string(count));

  
    Configuration Config;
    Config.fromJsonObj(PlotObj);

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

    MainWorker.get();
    MainWindow* w = new MainWindow(Config, MainWorker.get());
    w->setWindowTitle(QString::fromStdString(Config.Plot.WindowTitle + " " + std::to_string(count)));
    w->setParent(&mainWidget, Qt::Window);
    w->show();

    count += 1;
  }

  MainWorker->start();
  mainWidget.raise();

  return app.exec();
}
