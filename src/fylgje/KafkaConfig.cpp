// Copyright (C) 2022-2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <KafkaConfig.h>
#include <JsonFile.h>
#include <condition_variable>
#include <iostream>
#include <cstdio>

KafkaConfig::KafkaConfig(const std::string & KafkaConfigFile) {
  if (KafkaConfigFile.empty()) {
    return;
  }
  printf("KAFKA CONFIG FROM FILE");
  nlohmann::json root = from_json_file(KafkaConfigFile);

  try {
    nlohmann::json KafkaParms = root["KafkaParms"];

    for (const auto &Parm : KafkaParms) {
      std::map<std::string, std::string> MyMap = Parm;
      for (auto it = MyMap.begin(); it != MyMap.end(); it++) {
        std::pair<std::string, std::string> CfgPair{it->first, it->second};
        CfgParms.push_back(CfgPair);
      }
    }

  } catch (...) {
    printf("Kafka JSON config - error: Invalid Json file: %s",
        KafkaConfigFile.c_str());
    throw std::runtime_error("Invalid Json file for Kafka config");
  }
}
