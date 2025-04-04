// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file Configuration.h
///
/// \brief Fylgje configuration parameters
///
/// Provides some defaults values and allow loading from json file
//===----------------------------------------------------------------------===//

#pragma once

#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

class Configuration {
public:
  /// \brief constructor using default values
  /// Default are likely to be unsuitable and this should probably
  /// always be followed by a call to fromJsonFile()
  Configuration(){};

  /// \brief loads configuration from json file
  void fromJsonFile(std::string fname);

  // get the Instrument information from the config JSON
  void getInstrumentConfig();

  // get the Kafka related config options
  void getKafkaConfig();

  // get the Geometry related config options
  void getGeometryConfig();

  // get the Plot related config options
  void getPlotConfig();

  // get the TOF related config options
  void getTOFConfig();

  /// \brief prints the settings
  void print();

  /// \brief return value of type T from the json object, possibly default,
  // and optionally throws if value is not found
  template <typename T>
  T getVal(std::string Group, std::string Option, T Default,
           bool Throw = false);

  // Configurable options
  struct Instrument {
    std::string Name{"bifrost"};
    int groups{45};
    int units_per_group{3};
    int pixels_per_unit{100};
  };

  struct Kafka {
    std::string Topic{"bifrost_detector_samples"};
    std::string Broker{"172.17.5.38:9092"};
    std::string MessageMaxBytes{"10000000"};
    std::string FetchMessageMaxBytes{"10000000"};
    std::string ReplicaFetchMaxBytes{"10000000"};
    std::string EnableAutoCommit{"false"};
    std::string EnableAutoOffsetStore{"false"};
  };

  struct Plot {
    bool ClearPeriodic{false};
    double ClearEverySeconds{5};
    std::string ColorGradient{"thermal"};
    bool InvertGradient{false};
    bool LogScale{false};
    std::string WindowTitle{"BIFROST fylgje"};
    int Width{1300};
    int Height{900};
  };

  struct Instrument Instrument;
  struct Kafka Kafka;
  struct Plot Plot;

  std::string KafkaConfigFile{""};
  std::vector<std::pair<std::string, std::string>> KafkaConfig;

  nlohmann::json JsonObj;
};
