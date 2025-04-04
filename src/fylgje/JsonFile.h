// Copyright (C) 2019-2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief reads and writes JSON files to/from nlohmann types
///
/// See https://nlohmann.github.io/json/doxygen/index.html
//===----------------------------------------------------------------------===//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <nlohmann/json.hpp>
#pragma GCC diagnostic pop
#include <fmt/format.h>
#include <fstream>

inline nlohmann::json from_json_file(const std::string &file_name) {
  nlohmann::json json_out;
  std::ifstream ifs(file_name, std::ofstream::in);
  if (ifs.fail()) {
    throw std::runtime_error(
        fmt::format("file permission error or missing json file {}", file_name));
  }
  if (ifs.good())
    ifs >> json_out;

  return json_out;
}

inline void to_json_file(const nlohmann::json &json_in, const std::string &file_name) {
  std::ofstream(file_name, std::ofstream::trunc) << json_in.dump(1);
}
