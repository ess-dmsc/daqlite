// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Read kafka configuration from file
//===----------------------------------------------------------------------===//

#pragma once

#include <string>
#include <utility>
#include <vector>

class KafkaConfig {
public:
  ///\brief Load Kafka configuration from file
  KafkaConfig(const std::string &KafkaConfigFile);

public:
  // Parameters obtained from JSON config file
  std::vector<std::pair<std::string, std::string>> CfgParms;
};
