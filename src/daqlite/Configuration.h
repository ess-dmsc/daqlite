// Copyright (C) 2020 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file Configuration.h
///
/// \brief Daquiri light configuration parameters
///
/// Provides some defauls values and allow loading from json file
//===----------------------------------------------------------------------===//

#pragma once

#include <types/PlotType.h>

#include <nlohmann/json.hpp>

#include <string>
#include <utility>
#include <vector>

class Configuration {
public:
  /// \brief constructor using default values
  /// Default are likely to be unsuitable and this should probably
  /// always be followed by a call to fromJsonFile()
  Configuration(){}

  /// \brief loads configuration from JSON file
  void fromJsonObj(const nlohmann::json &obj);

  /// \brief loads configuration from JSON file
  void fromJsonFile(const std::string &path);

  /// \brief loads configuration from JSON file
  static std::vector<Configuration> getConfigurations(const std::string &path);

  static void prettyJSON(nlohmann::json &obj, const std::string &header="", int indent=4);

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
  T getVal(const std::string &Group, const std::string &Option, T Default,
           bool Throw = false);

  // Configurable options
  struct TOFOptions {
    unsigned int Scale{1000};     // ns -> us
    unsigned int MaxValue{25000}; // us
    unsigned int BinSize{512};    // bins
    bool AutoScaleX{true};
    bool AutoScaleY{true};
  };

  struct GeometryOptions {
    int XDim{1};
    int YDim{1};
    int ZDim{1};
    int Offset{0};
  };

  struct KafkaOptions {
    std::string Topic{"nmx_detector"};
    std::string Broker{"172.17.5.38:9092"};
    std::string Source{""};
    std::string MessageMaxBytes{"10000000"};
    std::string FetchMessagMaxBytes{"10000000"};
    std::string ReplicaFetchMaxBytes{"10000000"};
    std::string EnableAutoCommit{"false"};
    std::string EnableAutoOffsetStore{"false"};
  };

  struct PlotOptions {
    PlotType Plot{PlotType::PIXELS}; // "tof" and "tof2d" are also possible
    bool ClearPeriodic{false};
    uint32_t ClearEverySeconds{5};
    bool Interpolate{false};
    std::string ColorGradient{"hot"};
    bool InvertGradient{false};
    bool LogScale{false};
    std::string WindowTitle{"Daquiri Lite - Daqlite"};
    std::string PlotTitle{""};
    std::string XAxis{""};

    int Width{600};             // Default window width
    int Height{400};            // Default window height
    bool defaultGeometry{true}; // True if window geometries are default
  };

  struct TOFOptions mTOF;
  struct GeometryOptions mGeometry;
  struct KafkaOptions mKafka;
  struct PlotOptions mPlot;

  std::string mKafkaConfigFile{""};
  std::vector<std::pair<std::string, std::string>> mKafkaConfig;

  nlohmann::json mJsonObj;
};
