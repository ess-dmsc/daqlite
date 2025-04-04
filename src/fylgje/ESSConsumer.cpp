// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file ESSConsumer.cpp
///
//===----------------------------------------------------------------------===//

#include "ESSConsumer.h"
#include <algorithm>
#include <fmt/format.h>
#include <iostream>
#include <unistd.h>
#include <utility>
#include <vector>

/**
 * @brief Convert packet header and data times to seconds since reference time
 * @param pulse_hi Latest header reference time integer seconds since epoch
 * @param pulse_lo Latest header reference time 88.053 MHz ticks since pulse_hi
 * @param prev_hi Previous header reference time integer seconds since epoch
 * @param prev_lo Previous header reference time 88.053 MHz ticks since prev_hi
 * @param high Event reference time integer seconds since epoch
 * @param low Event reference time 88.053 MHz ticks since high
 * @return A positive double representing the time in seconds since _a_ reference time
 */
static double frame_time(uint32_t pulse_hi, uint32_t pulse_lo, uint32_t prev_hi, uint32_t prev_lo, uint32_t high, uint32_t low){
  auto converter = [high,low](uint32_t h, uint32_t l) {
    // low is allowed to be less than l, in which case direct subtraction would yield a large positive integer
    // if the cast to int is not done before subtraction.
    const int ticks = 88'052'500;
    auto diff = static_cast<int>(low)-static_cast<int>(l);
    return static_cast<double>(high-h) + static_cast<double>(diff) / ticks;
  };
  double time{0.};
  if (high > pulse_hi || (high == pulse_hi && low > pulse_lo)){
    time =  converter(pulse_hi, pulse_lo);
  } else if (high > prev_hi || (high == prev_hi && low > prev_lo)){
    time = converter(prev_hi, prev_lo);
  }
  return time;
}

ESSConsumer::ESSConsumer(data_t * data, Configuration & config,
                         std::vector<std::pair<std::string, std::string>> &KafkaConfig) :
  configuration(config),
  histograms(data),
  mKafkaConfig(KafkaConfig)
  {
  mConsumer = subscribeTopic();
  assert(mConsumer != nullptr);
  // if ... something is set in the gui, then seek the consumer offset before consuming
    setConsumerOffset(End, -1);
}

RdKafka::KafkaConsumer *ESSConsumer::subscribeTopic() const {
  auto mConf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

  if (!mConf) {
    fmt::print("Unable to create global Conf object\n");
    return nullptr;
  }

  std::string ErrStr;
  /// \todo figure out good values for these
  /// \todo some may be obsolete
  mConf->set("metadata.broker.list", configuration.Kafka.Broker, ErrStr);
  mConf->set("message.max.bytes", configuration.Kafka.MessageMaxBytes, ErrStr);
  mConf->set("fetch.message.max.bytes", configuration.Kafka.FetchMessageMaxBytes, ErrStr);
  mConf->set("replica.fetch.max.bytes", configuration.Kafka.ReplicaFetchMaxBytes, ErrStr);
  mConf->set("group.id", randomGroupString(16u), ErrStr);
  mConf->set("enable.auto.commit", configuration.Kafka.EnableAutoCommit, ErrStr);
  mConf->set("enable.auto.offset.store", configuration.Kafka.EnableAutoOffsetStore, ErrStr);

  for (auto &Config : mKafkaConfig) {
    mConf->set(Config.first, Config.second, ErrStr);
  }

  auto ret = RdKafka::KafkaConsumer::create(mConf, ErrStr);
  if (!ret) {
    fmt::print("Failed to create consumer: {}\n", ErrStr);
    return nullptr;
  }
  //
//  // // Start consumer for topic+partition at start offset
//  std::cout << "Subscribe to topic " << configuration.Kafka.Topic << "\n";
//  RdKafka::ErrorCode resp = ret->subscribe({configuration.Kafka.Topic});
//  if (resp != RdKafka::ERR_NO_ERROR) {
//    fmt::print("Failed to subscribe consumer to '{}': {}\n", configuration.Kafka.Topic, err2str(resp));
//  }

  return ret;
}

void ESSConsumer::consumeFrom(int64_t ms_since_utc_epoch){
  earliest_timestamp = ms_since_utc_epoch < 0 ? 0 : ms_since_utc_epoch;
  std::vector<RdKafka::TopicPartition*> tps;
  mConsumer->assignment(tps);
  setTopicPartitionOffset(tps, Time, ms_since_utc_epoch);
  mConsumer->seek(*tps.front(), 1);
}

void ESSConsumer::consumeUntil(int64_t ms_since_utc_epoch){
  auto duration = std::chrono::system_clock::now().time_since_epoch();
  auto milliseconds= std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
  latest_timestamp = ms_since_utc_epoch < 0 ? -1 : ms_since_utc_epoch;
  if (ms_since_utc_epoch < milliseconds){
    // consume only in the past; do we _need_ to seek backwards?
    std::vector<RdKafka::TopicPartition*> tps;
    mConsumer->assignment(tps);
    setTopicPartitionOffset(tps, earliest_timestamp < 0 ? Beginning : Time, earliest_timestamp);
    mConsumer->seek(*tps.front(), 1);
  }
}

void ESSConsumer::consumeAll(){
  std::vector<RdKafka::TopicPartition*> tps;
  mConsumer->assignment(tps);
  setTopicPartitionOffset(tps, Beginning, 0);
  mConsumer->seek(*tps.front(), 1);
}

void ESSConsumer::consumeForever(){
  latest_timestamp = -1;
}

void ESSConsumer::setConsumerOffset(Start start, int64_t ms_since_utc_epoch) {
  // set the consumer starting point, using the partition's known offsets ...
  RdKafka::Topic * only_rkt{nullptr};
  RdKafka::Metadata * metadataptr;

  auto resp = mConsumer->metadata(true, only_rkt, &metadataptr, 1000);
  if (resp != RdKafka::ERR_NO_ERROR){
    fmt::print("Failed retrieving metadata: {}\n", err2str(resp));
  }
  if (metadataptr == nullptr){
    fmt::print("metadataptr still NULL\n");
  } else {
    auto topic_metadata = metadataptr->topics();
    fmt::print("Got metadata about on {} topics\n", topic_metadata->size());
    for (const auto & topic_meta: *topic_metadata){
      fmt::print(" {} has {} partitions [", topic_meta->topic(), topic_meta->partitions()->size());
      const auto & partitions = topic_meta->partitions();
      for (const auto & partition: *partitions){
        fmt::print(" {},", partition->id());
      }
      fmt::print("]\n");
      if (topic_meta->topic() == configuration.Kafka.Topic){
        my_partition =  topic_meta->partitions()->front()->id();
      }
    }
  }

  std::vector<std::string> subscriptions;
  resp = mConsumer->subscription(subscriptions);
  if (resp != RdKafka::ERR_NO_ERROR){
    fmt::print("Failed to retrieve subscribed topics: {}\n", err2str(resp));
  } else {
    fmt::print("Subscribed to [");
    for (const auto & sub: subscriptions) fmt::print("{}, ", sub);
    fmt::print("]\n");
  }

  std::vector<RdKafka::TopicPartition*> tps;
  tps.push_back(RdKafka::TopicPartition::create(configuration.Kafka.Topic, my_partition));
  setTopicPartitionOffset(tps, start, ms_since_utc_epoch);
  mConsumer->assign(tps); // since consumption hasn't started, we seek by assigning the (topic, partition, offset)
}

void ESSConsumer::setTopicPartitionOffset(std::vector<RdKafka::TopicPartition*>& tps, Start start, int64_t ms_since_utc_epoch){
  int64_t low{0}, high{0};
  auto resp = mConsumer->get_watermark_offsets(configuration.Kafka.Topic, my_partition, &low, &high);
  if (resp != RdKafka::ERR_NO_ERROR) {
    fmt::print("Failed remembering watermark offsets for {} (partition {}): {}\n", configuration.Kafka.Topic, my_partition, err2str(resp));
  }
  if (low == high) {
    resp = mConsumer->query_watermark_offsets(configuration.Kafka.Topic, my_partition, &low, &high, 1000);
    if (resp != RdKafka::ERR_NO_ERROR) {
      fmt::print("Failed retrieving watermark offsets for {} (partition {}): {}\n", configuration.Kafka.Topic, my_partition, err2str(resp));
    }
  }
  fmt::print("Valid offsets for {} (partition: {}) are in range ({}, {})\n", configuration.Kafka.Topic, my_partition, low, high);

  tps.front()->set_offset(start == ESSConsumer::Start::Beginning ? low : start == ESSConsumer::Start::End ? high : ms_since_utc_epoch);
  if (start == ESSConsumer::Start::Time){
    // now handle converting a time to an offset
    resp = mConsumer->offsetsForTimes(tps, 1000);
    if (resp != RdKafka::ERR_NO_ERROR){
      fmt::print("Failed retrieving soonest offset after {} for {} (partition {}):  {}\n",
                 ms_since_utc_epoch, configuration.Kafka.Topic, my_partition, err2str(resp));
    }
  }
}


/// \brief Example parser for CAEN Data
uint32_t ESSConsumer::parseCAENData(uint8_t * Readout, int Size, uint32_t hi, uint32_t lo, uint32_t p_hi, uint32_t p_lo) {
  uint32_t processed{0};
  int BytesLeft = Size;
  while (BytesLeft >= static_cast<int>(sizeof(CAENReadout))) {
    auto * crd = (CAENReadout *)Readout;
    if (crd->FEN != 0){
      printf("FEN %u, Length %u, HighTime %u, LowTime %u, Flags %u, Group %u\n",
             crd->FEN, crd->Length, crd->HighTime, crd->LowTime, crd->Flags_OM, crd->Group);
    } else {
      auto time = frame_time(hi, lo, p_hi, p_lo, crd->HighTime, crd->LowTime);
      histograms->add(crd->Fiber, crd->Group, crd->A, crd->B, time);
    }
    BytesLeft -= sizeof(CAENReadout);
    Readout += sizeof(CAENReadout);
    ++processed;
  }
  return processed;
}

/// Main processing function for AR51 data
uint32_t ESSConsumer::processAR51Data(RdKafka::Message *Msg) {

  // First check header
  const auto & RawReadoutMsg = GetRawReadoutMessage(Msg->payload());
  auto MsgSize = static_cast<int>(RawReadoutMsg->raw_data()->size());

  auto * Header = (struct PacketHeaderV0 *)RawReadoutMsg->raw_data()->Data();

  if ((Header->CookieAndType & 0xffffff) != 0x535345) {
    printf("Non-ESS readout (cookie 0x%08x)\n", Header->CookieAndType);
    return 0;
  }

  if (Header->TotalLength !=  MsgSize) {
    printf("Readout size mismatch\n");
    return 0;
  }

  if (MsgSize == sizeof(struct PacketHeaderV0)) {
    return 0;
  }

  auto Type = Header->CookieAndType >> 28;
  auto pulse_hi = Header->PulseHigh;
  auto pulse_lo = Header->PulseLow;
  auto prev_hi = Header->PrevPulseHigh;
  auto prev_lo = Header->PrevPulseLow;

  uint8_t * DataPtr = (uint8_t * )Header + 30;
  if (Header->Version == 1) {
    DataPtr += 2;
  }
  //TODO Is this correct for Version 1 headers too?
  auto DataLength = Header->TotalLength - sizeof(struct PacketHeaderV0);

  // Dispatch technology specific
  if (3 == Type){
      return parseCAENData(DataPtr, static_cast<int>(DataLength), pulse_hi, pulse_lo, prev_hi, prev_lo);
  } else {
      fmt::print("Unregistered readout Type {}\n", Type);
  }
  return 0;
}


///\brief Main entry for kafka message processing
ESSConsumer::Status ESSConsumer::handleMessage(RdKafka::Message *Message) {
  switch (Message->err()) {
  case RdKafka::ERR__TIMED_OUT:
    return Continue;

  case RdKafka::ERR_NO_ERROR: {
      uint32_t count{0};
      if (latest_timestamp < 0 || Message->timestamp().timestamp < latest_timestamp) {
        if (RawReadoutMessageBufferHasIdentifier(Message->payload())) {
          count = processAR51Data(Message);
        } else {
          printf("Not a ar51 Kafka message!\n");
        }
      }
      return count ? Update : Continue;
  }
  default:
    fmt::print("Consume failed: {}", Message->errstr());
    return Halt;
  }
}

// Copied from daqlite - modified to not reinstantiate charset 'length' times
std::string ESSConsumer::randomGroupString(size_t length) {
  srand(getpid());
  const char charset[] = "0123456789"
                         "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                         "abcdefghijklmnopqrstuvwxyz";
  const size_t max_index = (sizeof(charset) - 1);
  auto randchar = [&charset]() -> char {
    return charset[rand() % max_index];
  };
  std::string str(length, 0);
  std::generate_n(str.begin(), length, randchar);
  return str;
}

/// \todo is timeout reasonable?
RdKafka::Message *ESSConsumer::consume() { return mConsumer->consume(1000); }
