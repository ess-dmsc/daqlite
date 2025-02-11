// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief using nlohmann json parser to read configurations from file
//===----------------------------------------------------------------------===//

#include <KafkaConfig.h>

#include <JsonFile.h>

#include <nlohmann/json.hpp>

#include <cstdio>
#include <map>
#include <stdexcept>

using std::string;

KafkaConfig::KafkaConfig(const string &KafkaConfigFile) {
  if (KafkaConfigFile == "") {
    return;
  }
  printf("KAFKA CONFIG FROM FILE");
  nlohmann::json root = from_json_file(KafkaConfigFile);

  try {
    nlohmann::json KafkaParms = root["KafkaParms"];

    for (const auto &Parm : KafkaParms) {
      std::map<string, string> MyMap = Parm;
      for (auto it = MyMap.begin(); it != MyMap.end(); it++) {
        std::pair<string, string> CfgPair{it->first, it->second};
        CfgParms.push_back(CfgPair);
      }
    }

  } catch (...) {
    printf("Kafka JSON config - error: Invalid Json file: %s",
        KafkaConfigFile.c_str());
    throw std::runtime_error("Invalid Json file for Kafka config");
  }
}
