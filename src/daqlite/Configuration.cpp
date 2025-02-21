// Copyright (C) 2020 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file Configuration.cpp
///
//===----------------------------------------------------------------------===//

#include <Configuration.h>

#include <nlohmann/json.hpp>

#include <fmt/format.h>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <stdexcept>

using std::string;
using std::vector;

void Configuration::prettyJSON(nlohmann::json &obj, const std::string &header, int indent) {
  fmt::print("{}:\n", header);
  std::cout << obj.dump(indent) << "\n\n" << std::endl;
}

vector<Configuration> Configuration::getConfigurations(const std::string &Path) {
  vector<Configuration> Configurations;

  // Open JSON file for reading
  std::ifstream ifs(Path, std::ofstream::in);
  if (!ifs.good()) {
    throw(std::runtime_error("Unable to create ifstream (bad filename?), exiting ..."));
  }

  // Load JSON file
  nlohmann::json MainJSON;
  ifs >> MainJSON;

  // Uncomment to print JSON object
  // Configuration::prettyJSON(MainJSON, "Main");

  // ---------------------------------------------------------------------------
  // Common options
  //
  // Read Kafka, Geometry and TOF options - but no plots
  nlohmann::json Common;
  for (const auto& key: {"kafka", "geometry", "tof"}) {
    if (MainJSON.contains(key)) {
      Common[key] = MainJSON[key];
    }
  }

  Configuration conf;
  conf.fromJsonObj(Common);
  Configurations.push_back(conf);

  // ---------------------------------------------------------------------------
  // Single plot
  if (MainJSON.contains("plot")) {
    Common["plot"] = MainJSON["plot"];
  }

  // Uncomment to print JSON object
  // Configuration::prettyJSON(Common, "Top plot");
  conf.fromJsonObj(Common);
  Configurations.push_back(conf);

  // ---------------------------------------------------------------------------
  // Multiple plots
  if (MainJSON.contains("plots")) {
    for (const auto& [key, plot] : MainJSON["plots"].items()) {
      Common["plot"] = plot;

      // Uncomment to print JSON object
      // Configuration::prettyJSON(Common, key);
      conf.fromJsonObj(Common);
      Configurations.push_back(conf);
    }
  }

  return Configurations;
}


void Configuration::fromJsonObj(const nlohmann::json &obj) {
  mJsonObj = obj;

  getGeometryConfig();
  getKafkaConfig();
  getPlotConfig();
  getTOFConfig();
  print();
}

void Configuration::fromJsonFile(const std::string &fname) {
  std::ifstream ifs(fname, std::ofstream::in);
  if (!ifs.good()) {
    throw(std::runtime_error("Unable to create ifstream (bad filename?), exiting ..."));
  }

  try {
    ifs >> mJsonObj;
  } catch (...) {
    throw(std::runtime_error("File is not valid JSON"));
  }

  getGeometryConfig();
  getKafkaConfig();
  getPlotConfig();
  getTOFConfig();
  print();
}

void Configuration::getGeometryConfig() {
  /// 'geometry' field is mandatory
  mGeometry.XDim = getVal("geometry", "xdim", mGeometry.XDim, true);
  mGeometry.YDim = getVal("geometry", "ydim", mGeometry.YDim, true);
  mGeometry.ZDim = getVal("geometry", "zdim", mGeometry.ZDim, true);
  mGeometry.Offset = getVal("geometry", "offset", mGeometry.Offset);
}

void Configuration::getKafkaConfig() {
  /// 'broker' and 'topic' must be specified
  using std::operator""s;
  mKafka.Broker = getVal("kafka", "broker", "n/a"s, true);
  mKafka.Topic = getVal("kafka", "topic", "n/a"s, true);
  mKafka.Source = getVal("kafka", "source", ""s, false);
  /// The rest are optional, using default values
  mKafka.MessageMaxBytes =
      getVal("kafka", "message.max.bytes", mKafka.MessageMaxBytes);
  mKafka.FetchMessagMaxBytes =
      getVal("kafka", "fetch.message.max.bytes", mKafka.FetchMessagMaxBytes);
  mKafka.ReplicaFetchMaxBytes =
      getVal("kafka", "replica.fetch.max.bytes", mKafka.ReplicaFetchMaxBytes);
  mKafka.EnableAutoCommit =
      getVal("kafka", "enable.auto.commit", mKafka.EnableAutoCommit);
  mKafka.EnableAutoOffsetStore =
      getVal("kafka", "enable.auto.offset.store", mKafka.EnableAutoOffsetStore);
}

void Configuration::getPlotConfig() {
  // Plot options - all are optional
  std::string plot;
  mPlot.Plot = getVal("plot", "plot_type", mPlot.Plot.asString());

  mPlot.ClearPeriodic = getVal("plot", "clear_periodic", mPlot.ClearPeriodic);
  mPlot.ClearEverySeconds =
      getVal("plot", "clear_interval_seconds", mPlot.ClearEverySeconds);
  mPlot.Interpolate = getVal("plot", "interpolate_pixels", mPlot.Interpolate);
  mPlot.ColorGradient = getVal("plot", "color_gradient", mPlot.ColorGradient);
  mPlot.InvertGradient = getVal("plot", "invert_gradient", mPlot.InvertGradient);
  mPlot.LogScale = getVal("plot", "log_scale", mPlot.LogScale);

  // Window options - all are optional
  mPlot.WindowTitle = getVal("plot", "window_title", mPlot.WindowTitle);
  mPlot.PlotTitle = getVal("plot", "plot_title", mPlot.PlotTitle);
  mPlot.XAxis = getVal("plot", "xaxis", mPlot.XAxis);
  mPlot.Width = getVal("plot", "window_width", mPlot.Width);
  mPlot.Height = getVal("plot", "window_height", mPlot.Height);
}

void Configuration::getTOFConfig() {
  mTOF.Scale = getVal("tof", "scale", mTOF.Scale);
  mTOF.MaxValue = getVal("tof", "max_value", mTOF.MaxValue);
  mTOF.BinSize = getVal("tof", "bin_size", mTOF.BinSize);
  mTOF.AutoScaleX = getVal("tof", "auto_scale_x", mTOF.AutoScaleX);
  mTOF.AutoScaleY = getVal("tof", "auto_scale_y", mTOF.AutoScaleY);
}

void Configuration::print() {
  fmt::print("[Kafka]\n");
  fmt::print("  Broker {}\n", mKafka.Broker);
  fmt::print("  Topic {}\n", mKafka.Topic);
  fmt::print("[Geometry]\n");
  fmt::print("  Dimensions ({}, {}, {})\n", mGeometry.XDim, mGeometry.YDim,
             mGeometry.ZDim);
  fmt::print("  Pixel Offset {}\n", mGeometry.Offset);
  fmt::print("[Plot]\n");
  fmt::print("  WindowTitle {}\n", mPlot.WindowTitle);
  fmt::print("  Plot type {}\n", mPlot.Plot);
  fmt::print("  Clear periodically {}\n", mPlot.ClearPeriodic);
  fmt::print("  Clear interval (s) {}\n", mPlot.ClearEverySeconds);
  fmt::print("  Interpolate image {}\n", mPlot.Interpolate);
  fmt::print("  Color gradient {}\n", mPlot.ColorGradient);
  fmt::print("  Invert gradient {}\n", mPlot.InvertGradient);
  fmt::print("  Log Scale {}\n", mPlot.LogScale);
  fmt::print("  PlotTitle {}\n", mPlot.PlotTitle);
  fmt::print("  X Axis {}\n", mPlot.XAxis);
  fmt::print("[TOF]\n");
  fmt::print("  Scale {}\n", mTOF.Scale);
  fmt::print("  Max value {}\n", mTOF.MaxValue);
  fmt::print("  Bin size {}\n", mTOF.BinSize);
  fmt::print("  Auto scale x {}\n", mTOF.AutoScaleX);
  fmt::print("  Auto scale y {}\n", mTOF.AutoScaleY);
}

//\brief getVal() template is used to effectively achieve
// getInt(), getString() and getBool() functionality through T
template <typename T>
T Configuration::getVal(const std::string &Group, const std::string &Option, T Default,
                        bool Throw) {
  T ConfigVal;

  // Check if the option is present
  if (mJsonObj.contains(Group) && mJsonObj[Group].contains(Option)) {
    ConfigVal = mJsonObj[Group][Option];
  }

  // ... inform, if it is missing
  else {
    fmt::print("Missing [{}][{}] configuration\n", Group, Option);
    if (Throw) {
      throw std::runtime_error("Daqlite config error");
    } else {
      fmt::print("Using default: {}\n", Default);
      return Default;
    }
  }

  return ConfigVal;
}
