#include "Configuration.h"
#include "Calibration.h"
#include "AppWindow.h"

#include <map>
#include <QApplication>

#include <fmt/format.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCommandLineParser CLI;
    CLI.setApplicationDescription("fylgje - your mythical instrument follower");
    std::map<std::string, QCommandLineOption> cliOptions {
        {"file", QCommandLineOption("f", "JSON configuration <file>.", "file")},
        {"broker", QCommandLineOption("b", "Kafka <broker> url.", "broker"),},
        {"topic", QCommandLineOption("t", "Kafka <topic>.", "kafka"),},
        {"config", QCommandLineOption("k", "Kafka <configuration> file.", "configuration"),},
        {"info", QCommandLineOption({"i", "info"}, "Information about fylgje.")},
        {"calibration", QCommandLineOption({"c", "calibration"}, "Detector calibration JSON file", "calibration.json")}
    };
    CLI.addHelpOption();
    for (const auto & [name, opt]: cliOptions) {
      CLI.addOption(opt);
    }
    CLI.process(app);

    if (CLI.isSet(cliOptions.at("info"))) {
      std::cout << "Wikipedia fylgje: https://en.wikipedia.org/wiki/Fylgja" << std::endl;
      return 0;
    }

    Configuration Config;
    if (CLI.isSet(cliOptions.at("file"))) {
      if (auto fileName = CLI.value(cliOptions.at("file")).toStdString(); !fileName.empty()) {
        Config.fromJsonFile(fileName);
      }
    }
    if (CLI.isSet(cliOptions.at("broker"))) {
      if (auto broker = CLI.value(cliOptions.at("broker")).toStdString(); !broker.empty()) {
        Config.Kafka.Broker = broker;
        std::cout << fmt::format("<<<<\n WARNING Override kafka broker to {} \n>>>>\n", Config.Kafka.Broker);
      }
    }
    if (CLI.isSet(cliOptions.at("topic"))) {
      if (auto topic = CLI.value(cliOptions.at("topic")).toStdString(); !topic.empty()) {
        Config.Kafka.Topic = topic;
        std::cout << fmt::format("<<<<\n WARNING Override kafka topic to {} \n>>>>\n", Config.Kafka.Topic);
      }
    }
    if (CLI.isSet(cliOptions.at("config"))) {
      if (auto config = CLI.value(cliOptions.at("config")).toStdString(); !config.empty()){
        Config.KafkaConfigFile = config;
      }
    }

    Calibration calibration{};
    if (CLI.isSet(cliOptions.at("calibration"))) {
      if (auto calib = CLI.value(cliOptions.at("calibration")).toStdString(); !calib.empty()) {
        calibration = from_json_file(calib);
      }
    }

    MainWindow w(Config, calibration);
    w.setWindowTitle(QString::fromStdString(Config.Plot.WindowTitle));
    w.resize(Config.Plot.Width, Config.Plot.Height);
    w.show();
    return app.exec();
}
