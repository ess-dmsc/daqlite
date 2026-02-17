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
#include <set>
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
  /// \brief  Threaded vector
  using TSVector = ThreadSafeVector<uint32_t, int64_t>;

  /// \brief  Type used for having one threaded vector per flat buffer source
  using TSVectorMap = std::map<std::string, TSVector>;

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

  /// \brief return a random group id so that simultaneous consumption from
  /// multiple applications is possible.
  static std::string randomGroupString(size_t length);

  uint64_t getEventCount() const { return mEventCount; };
  uint64_t getEventAccept() const { return mEventAccept; };
  uint64_t getEventDiscard() const { return mEventDiscard; };

  /// \brief Add a new plot subscribing for data
  ///
  /// \param Type  The plot type
  void addSubscriber(PlotType Type, bool add = true);

  /// \return The current number of data subscriptions
  size_t subscriptionCount() const;

  /// \brief Call this after pulling events data in order to clear all delivered
  /// subscriptions
  void gotEventRequest();

  /// \brief Read out data for a given data type, optionally from a specific
  /// source
  ///
  /// \param dataType  Type of the data (HISTOGRAM, HISTOGRAM_TOF,
  ///                  PIXEL_ID, or TOF)
  /// \param source    Flat buffer source name. If empty (""), combines data
  ///                  from all sources element-wise (adds values at the same
  ///                  index across all sources)
  /// \param reset     If true and all deliveries are complete, clears
  ///                  the internal data container(s)
  /// \return          Vector containing the requested data. Returns empty
  ///                  vector if source not found or dataType is invalid
  std::vector<uint32_t> readData(DataType dataType,
                                 const std::string &source = "",
                                 bool reset = true);

  /// \brief Get the data container size for a specific source and data type
  /// \param dataType  Type of the data
  /// \param source    Flat buffer source name (empty string returns 0)
  /// \return          Size of the data container for the specified source, or 0
  ///                  if not found
  size_t getDataSize(DataType dataType, const std::string &source = "") const;

  /// \brief Get the number of bins for the TOF data container
  /// \param source    Flat buffer source name
  /// \return          Number of bins (TOF data size - 1), or 0 if source not
  ///                  found or empty
  size_t getBinSize(const std::string &source = "") const;

  /// \brief Register a flat buffer source for processing
  /// \param source  The flat buffer source name to register. Empty strings are
  ///                ignored.
  /// \note Only messages from registered sources will be processed
  ///       when sources are defined
  void addSource(const std::string &source);

private:
  /// \brief Get a pointer to the data container map for a given data type
  /// \param dataType  Type of the data (HISTOGRAM, HISTOGRAM_TOF, PIXEL_ID, or
  ///                  TOF)
  /// \return          Pointer to the TSVectorMap containing data for all
  ///                  sources, or nullptr if dataType is invalid
  const TSVectorMap *getData(DataType dataType) const;

  /// \brief Check if a flat buffer source has been registered for processing
  /// \param source  The flat buffer source
  bool hasSource(const std::string &source) const;

  RdKafka::Conf *mConf;
  RdKafka::Conf *mTConf;
  RdKafka::KafkaConsumer *mConsumer;
  RdKafka::Topic *mTopic;

  uint64_t mEventCount{0};
  uint64_t mEventAccept{0};
  uint64_t mEventDiscard{0};

  // Thread safe data storage -  use a map to handle unique data for each flat
  // buffer source
  TSVectorMap mHistograms;
  TSVectorMap mHistogramTOFs;
  TSVectorMap mPixelIDs;
  TSVectorMap mTOFs;

  /// \brief configuration obtained from main()
  Configuration &mConfig;

  /// \brief all registered flat buffer sources
  std::set<std::string> mSources;

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

  /// \brief  Reset data if requested and all deliveries have been made
  /// \param  dataMap   The data map to reset
  /// \param  dataType  The data type being delivered
  /// \param  source    Source name. If empty, resets all sources; otherwise
  ///                   resets specific source
  /// \param  reset     Whether to reset data
  ///
  /// \note Only resets data when checkDelivery confirms all subscribers have
  /// received data
  inline void resetDataIfNeeded(TSVectorMap *dataMap, DataType dataType,
                                const std::string &source, bool reset) {
    if (reset && checkDelivery(dataType)) {
      if (!source.empty()) {
        auto iter = dataMap->find(source);
        if (iter != dataMap->end()) {
          iter->second.clear();
        }
      } else {
        for (auto &[key, data] : *dataMap) {
          data.clear();
        }
      }
    }
  }

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
