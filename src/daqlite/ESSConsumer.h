// Copyright (C) 2020 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file ESSConsumer.h
///
/// \brief Wrapper class for librdkafka
///
/// Sets up the kafka consumer and handles binning of event pixel ids
//===----------------------------------------------------------------------===//

#pragma once

#include <ThreadSafeVector.h>
#include <types/DataType.h>

#include <librdkafka/rdkafkacpp.h>

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// Forward declarations
class Configuration;
class PlotType;
struct da00_Variable;

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

  /// \brief return a random group id so that simultaneous consume from  fmt::print("ESSConsumer::readResetTOFs: Clearing = {} {} {}\n\n", mSubscriptionCount[DataType::TOF], mDeliveryCount[DataType::TOF]);

  /// multiple applications is possible.
  static std::string randomGroupString(size_t length);

  size_t getHistogramSize() const { return mHistogram.size(); }
  size_t getHistogramTofSize() const { return mHistogramTof.size(); }
  size_t getPixelIDsSize() const { return mPixelIDs.size(); }
  size_t getTOFsSize() const { return mTOFs.size(); }

  uint64_t getEventCount() const { return mEventCount; };
  uint64_t getEventAccept() const { return mEventAccept; };
  uint64_t getEventDiscard() const { return mEventDiscard; };

  /// \brief read out the histogram data and reset it
  std::vector<uint32_t> readResetHistogram();

  /// \brief read out the TOF histogram data and reset it
  std::vector<uint32_t> readResetHistogramTof();

  /// \brief read out the event pixel IDs and reset it
  std::vector<uint32_t> readResetPixelIDs();

  /// \brief read out the event TOFs and reset it
  std::vector<uint32_t> readResetTOFs();

  /// \brief read out the event TOFs (no reset)
  std::vector<uint32_t> getTofs() const;

  /// \brief Add a new plot subscribing for data
  ///
  /// \param Type  The plot type
  void addSubscriber(PlotType Type);

  /// Call this after pulling events data. Cleared all subscriptions have been delivered
  void gotEventRequest();

private:
  RdKafka::Conf *mConf;
  RdKafka::Conf *mTConf;
  RdKafka::KafkaConsumer *mConsumer;
  RdKafka::Topic *mTopic;

  uint64_t mEventCount{0};
  uint64_t mEventAccept{0};
  uint64_t mEventDiscard{0};

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

  uint32_t mNumPixels{0}; ///< Number of pixels
  uint32_t mMinPixel{0};  ///< Offset
  uint32_t mMaxPixel{0};  ///< Number of pixels + offset

  /// \brief  Check if all deliveries have been made for a given data type
  /// \param  Type  Check for this data type
  /// \return true if all deliveries are done
  bool checkDelivery(DataType Type);

  /// \brief Number of plots subscribing to ESSConsumer data (is incremented
  ///        when calling addSubscriber)
  size_t mSubscribers{0};

  /// \brief  Count the current number of request for event stats
  size_t mEventRequests{0};

  /// \brief The number of subscribers for each data type
  std::map<DataType, size_t> mSubscriptionCount;

  /// \brief The number of deliveries made so far for different data types
  std::map<DataType, size_t> mDeliveryCount;
};
