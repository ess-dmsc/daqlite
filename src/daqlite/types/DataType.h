// Copyright (C) 2020 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file DataType.h
///
/// Used for characterizing data types used by daqlite plots
//===----------------------------------------------------------------------===//

#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector> 

class DataType {
public:

  // Enum definition
  enum Types {
    NONE = 0x01,
    ANY = 0x02,
    TOF = 0x03,
    HISTOGRAM = 0x04,
    HISTOGRAM_TOF = 0x05,
    PIXEL_ID = 0x06
  };

  // Max and min enum values
  static constexpr int MIN = Types::NONE;
  static constexpr int MAX = Types::PIXEL_ID;

  // Construct from string
  DataType(const std::string &type) {

    // Convert to lower case, so both "value" and "VALUE" will work
    std::string lower = type;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){ return std::tolower(c); });

    if (lower == "none") {
      mDataType = Types::NONE;
    }

    else if (lower == "any") {
      mDataType = Types::ANY;
    }

    else if (lower == "tof") {
      mDataType = Types::TOF;
    }

    else if (lower == "histogram") {
      mDataType = Types::HISTOGRAM;
    }

    else if (lower == "histogram_tof") {
      mDataType = Types::HISTOGRAM_TOF;
    }

    else if (lower == "pixel_id") {
      mDataType = Types::PIXEL_ID;
    }

    else {
      throw std::invalid_argument("Invalid DataType string: " + type);
    }
  }

  // Construct from integer
  DataType(const int type) {
    if (type >= MIN && type <= MAX) {
      mDataType = static_cast<Types>(type);
    }

    else {
      throw std::invalid_argument("Invalid PlotType integer: " +
                                  std::to_string(type));
    }
  }

  std::string asString() const {
    std::string result = "";

    switch (mDataType) {
      case Types::NONE:
        result = "NONE";
        break;

      case Types::ANY:
        result = "ANY";
        break;

      case Types::TOF:
        result = "TOF";
        break;

      case Types::HISTOGRAM:
        result = "HISTOGRAM";
        break;

      case Types::HISTOGRAM_TOF:
        result = "HISTOGRAM_TOF";
        break;

      case Types::PIXEL_ID:
        result = "PIXEL_ID";
        break;

      default:
        break;
    }

    return result;
  }

  static std::vector<int> types() {
    return std::vector<int>{
      Types::NONE,
      Types::ANY,
      Types::TOF,
      Types::HISTOGRAM,
      Types::HISTOGRAM_TOF,
      Types::PIXEL_ID
    };
  }

  operator int() const { return static_cast<int>(mDataType); }

  bool operator==(const DataType &other) const {
    return mDataType == other.mDataType;
  }

  bool operator!=(const DataType &other) const {
    return mDataType != other.mDataType;
  }

  bool operator==(const Types &other) const {
    return mDataType == other;
  }

  bool operator!=(const Types &other) const {
    return mDataType != other;
  }

private:
  Types mDataType;
};
