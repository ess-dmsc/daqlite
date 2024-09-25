// Copyright (C) 2020 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file ESSConsumer.h
///
/// \brief Wrapper class for librdkafka
///
/// Sets up the kafka consumer and handles binning of event pixel ids
//===----------------------------------------------------------------------===//

#pragma once

#include <Configuration.h>
#include <ThreadSafeVector.h>
#include <cstdint>
#include <da00_dataarray_generated.h>
#include <flatbuffers/flatbuffers.h>
#include <librdkafka/rdkafkacpp.h>
#include <mutex>
#include <string>
#include <vector>

/// \class ESSConsumer
/// \brief A class to handle Kafka consumer operations for ESS data.
///
/// The ESSConsumer class is responsible for consuming messages from a Kafka
/// broker, processing the messages, and managing histograms and other data
/// structures related to the consumed messages.
///
/// \details
/// This class provides methods to configure and subscribe to Kafka topics,
/// consume messages, handle errors, and process specific types of data
/// messages. It also includes methods to read out and reset histograms and
/// other data structures.
///
/// \note
/// The class uses librdkafka for Kafka operations and includes thread-safe
/// data structures for histogram storage.
///
/// \example
/// \code
/// Configuration config;
/// std::vector<std::pair<std::string, std::string>> kafkaConfig;
/// ESSConsumer consumer(config, kafkaConfig);
/// auto message = consumer.consume();
/// if (consumer.handleMessage(message.get())) {
///     // Process the message
/// }
/// \endcode
///
/// \see Configuration
/// \see RdKafka::Message
/// \see RdKafka::KafkaConsumer
class ESSConsumer {
public:
  /// \brief Constructor needs the configured Broker and Topic
  ESSConsumer(Configuration &Config,
              std::vector<std::pair<std::string, std::string>> &KafkaConfig);

  /// \brief wrapper function for librdkafka consumer
  std::unique_ptr<RdKafka::Message> consume();

  /// \brief setup librdkafka parameters for Broker and Topic
  RdKafka::KafkaConsumer *subscribeTopic() const;

  /// \brief initial checks for kafka error messages
  /// \return true if message contains data, false otherwise
  bool handleMessage(RdKafka::Message *message);

  /// \brief return a random group id so that simultaneous consume from
  /// multiple applications is possible.
  static std::string randomGroupString(size_t length);

  uint64_t EventCount{0};
  uint64_t EventAccept{0};
  uint64_t EventDiscard{0};

  /// \brief read out the histogram data and reset it
  std::vector<uint32_t> readResetHistogram() {
    std::vector<uint32_t> ret = mHistogram;
    mHistogram.clear();
    return ret;
  }

  /// \brief read out the TOF histogram data and reset it
  std::vector<uint32_t> readResetHistogramTof() {
    std::vector<uint32_t> ret = mHistogramTof;
    mHistogramTof.clear();
    return ret;
  }

  /// \brief read out the event pixel IDs and clear the vector
  std::vector<uint32_t> readResetPixelIDs() {
    std::vector<uint32_t> ret = mPixelIDs;
    mPixelIDs.clear();
    return ret;
  }

  /// \brief read out the event TOFs and clear the vector
  std::vector<uint32_t> readResetTOFs() {
    std::vector<uint32_t> ret = mTOFs;
    mTOFs.clear();
    return ret;
  }

  std::vector<uint32_t> getTofs() const {
    std::vector<uint32_t> ret = mTOFs;
    return ret;
  }

private:
  RdKafka::Conf *mConf;
  RdKafka::Conf *mTConf;
  RdKafka::KafkaConsumer *mConsumer;
  RdKafka::Topic *mTopic;

  // Thread safe histogram data storage
  ThreadSafeVector<uint32_t, int64_t> mHistogram;
  ThreadSafeVector<uint32_t, int64_t> mHistogramTof;
  ThreadSafeVector<uint32_t, int64_t> mPixelIDs;
  ThreadSafeVector<uint32_t, int64_t> mTOFs;

  /// \brief configuration obtained from main()
  Configuration &mConfig;

  /// \brief loadable Kafka-specific configuration
  std::vector<std::pair<std::string, std::string>> &mKafkaConfig;

  /// \brief histograms the event pixelids and ignores TOF
  uint32_t processEV42Data(RdKafka::Message *Msg);

  /// \brief histograms the event pixelids and ignores TOF
  uint32_t processEV44Data(RdKafka::Message *Msg);

  /// \brief histograms the DA00 TOF data bins
  uint32_t processDA00Data(RdKafka::Message *Msg);

  std::vector<int64_t> getDataVector(const da00_Variable &Variable) const;

  /// \brief Some stat counters
  /// \todo use or delete?
  struct Stat {
    uint64_t MessagesRx{0};
    uint64_t MessagesTMO{0};
    uint64_t MessagesData{0};
    uint64_t MessagesEOF{0};
    uint64_t MessagesUnknown{0};
    uint64_t MessagesOther{0};
  } mKafkaStats;

  uint32_t mMinPixel{0}; ///< Offset
  uint32_t mMaxPixel{0}; ///< Number of pixels + offset
};
