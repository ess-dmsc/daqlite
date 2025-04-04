// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file ESSConsumer.h
///
/// \brief Wrapper class for librdkafka
///
/// Sets up the kafka consumer and handles binning of event pixel ids
//===----------------------------------------------------------------------===//

#pragma once

#include "Configuration.h"
#include "ar51_readout_data_generated.h"
#include <librdkafka/rdkafkacpp.h>
#include "DataManager.h"

class ESSConsumer {
public:
  using data_t = ::bifrost::data::Manager;
  enum Status {Continue, Update, Halt};
  enum Start {Beginning, End, Time};

  // Data format for the common ESS readout header
  struct PacketHeaderV0 {
    uint8_t Padding0;
    uint8_t Version;
    uint32_t CookieAndType;
    uint16_t TotalLength;
    uint8_t OutputQueue;
    uint8_t TimeSource;
    uint32_t PulseHigh;
    uint32_t PulseLow;
    uint32_t PrevPulseHigh;
    uint32_t PrevPulseLow;
    uint32_t SeqNum;
  } __attribute__((packed));

  struct CAENReadout {
    uint8_t Fiber;
    uint8_t FEN;
    uint16_t Length;
    uint32_t HighTime;
    uint32_t LowTime;
    uint8_t Flags_OM;
    uint8_t Group;
    uint16_t Unused;
    int16_t A;
    int16_t B;
    int16_t C;
    int16_t D;
  } __attribute__((packed));


  /// \brief Constructor needs the configured Broker and Topic
  ESSConsumer(data_t * data, Configuration & configuration,
              std::vector<std::pair<std::string, std::string>> &KafkaConfig);

  /// \brief wrapper function for librdkafka consumer
  RdKafka::Message *consume();

  /// \brief setup librdkafka parameters for Broker and Topic
  [[nodiscard]] RdKafka::KafkaConsumer *subscribeTopic() const;

  /// \brief initial checks for kafka error messages
  /// \return true if message contains data, false otherwise
  Status handleMessage(RdKafka::Message *Msg);

  /// \brief print out some information
  uint32_t processAR51Data(RdKafka::Message *Msg);

  ///
  uint32_t parseCAENData(uint8_t * Readout, int Size, uint32_t pulse_high, uint32_t pulse_low, uint32_t prev_high, uint32_t prev_low);

  static std::string randomGroupString(size_t length);

  [[maybe_unused]] void consumeAll();
  void consumeForever();
  void consumeFrom(int64_t ms_since_utc_epoch);
  void consumeUntil(int64_t ms_since_utc_epoch);

private:
  Configuration & configuration;

  RdKafka::KafkaConsumer *mConsumer;
  int32_t my_partition{0};
  int64_t earliest_timestamp{-1}, latest_timestamp{-1};

  data_t * histograms;

  /// \brief loadable Kafka-specific configuration
  std::vector<std::pair<std::string, std::string>> &mKafkaConfig;

  void setConsumerOffset(Start start, int64_t ms_since_utc_epoch);
  void setTopicPartitionOffset(std::vector<RdKafka::TopicPartition*>& tps, Start start, int64_t ms_since_utc_epoch);
};
