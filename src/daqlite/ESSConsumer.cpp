// Copyright \(C\) 2020 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file ESSConsumer.cpp
///
//===----------------------------------------------------------------------===//

#include <ESSConsumer.h>

#include <types/PlotType.h>
#include <Configuration.h>
#include <ThreadSafeVector.h>

#include <flatbuffers/flatbuffers.h>
#include <da00_dataarray_generated.h>
#include <ev42_events_generated.h>
#include <ev44_events_generated.h>

#include <algorithm>
#include <assert.h>
#include <cstddef>
#include <cstdint>
#include <fmt/format.h>
#include <memory>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

using std::string;
using std::vector;

ESSConsumer::ESSConsumer(
    Configuration &Config,
    vector<std::pair<string, string>> &KafkaConfig)
    : mConfig(Config)
    , mKafkaConfig(KafkaConfig) {
  auto &geom = mConfig.mGeometry;
  mNumPixels = geom.XDim * geom.YDim * geom.ZDim;
  mMinPixel = geom.Offset + 1;
  mMaxPixel = geom.Offset + mNumPixels;
  assert(mMaxPixel != 0);
  assert(mMinPixel < mMaxPixel);

  mConsumer = subscribeTopic();
  assert(mConsumer != nullptr);

  for (DataType t: {DataType::NONE, DataType::ANY, DataType::TOF, DataType::HISTOGRAM, DataType::HISTOGRAM_TOF, DataType::PIXEL_ID}) {
    mSubscriptionCount[t] = 0;
    mDeliveryCount[t] = 0;
  }
}

RdKafka::KafkaConsumer *ESSConsumer::subscribeTopic() const {
  auto mConf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

  if (!mConf) {
    fmt::print("Unable to create global Conf object\n");
    return nullptr;
  }

  string ErrStr;
  /// \todo figure out good values for these
  /// \todo some may be obsolete
  mConf->set("metadata.broker.list", mConfig.mKafka.Broker, ErrStr);
  mConf->set("message.max.bytes", mConfig.mKafka.MessageMaxBytes, ErrStr);
  mConf->set("fetch.message.max.bytes", mConfig.mKafka.FetchMessagMaxBytes,
             ErrStr);
  mConf->set("replica.fetch.max.bytes", mConfig.mKafka.ReplicaFetchMaxBytes,
             ErrStr);
  string GroupId = randomGroupString(16);
  mConf->set("group.id", GroupId, ErrStr);
  mConf->set("enable.auto.commit", mConfig.mKafka.EnableAutoCommit, ErrStr);
  mConf->set("enable.auto.offset.store", mConfig.mKafka.EnableAutoOffsetStore,
             ErrStr);

  for (auto &Config : mKafkaConfig) {
    mConf->set(Config.first, Config.second, ErrStr);
  }

  auto ret = RdKafka::KafkaConsumer::create(mConf, ErrStr);
  if (!ret) {
    fmt::print("Failed to create consumer: {}\n", ErrStr);
    return nullptr;
  }
  //
  // // Start consumer for topic+partition at start offset
  RdKafka::ErrorCode resp = ret->subscribe({mConfig.mKafka.Topic});
  if (resp != RdKafka::ERR_NO_ERROR) {
    fmt::print("Failed to subscribe consumer to '{}': {}\n",
               mConfig.mKafka.Topic, err2str(resp));
  }

  return ret;
}

uint32_t ESSConsumer::processEV44Data(RdKafka::Message *Msg) {
  auto EvMsg = GetEvent44Message(Msg->payload());
  auto PixelIds = EvMsg->pixel_id();
  auto TOFs = EvMsg->time_of_flight();

  // If source name is set in config, only process messages from that source
  if (!mConfig.mKafka.Source.empty() &&
      EvMsg->source_name()->str() != mConfig.mKafka.Source) {
    return 0;
  }

  if (PixelIds->size() != TOFs->size()) {
    return 0;
  }

  // local temporary histograms to avoid locking during processing
  vector<uint32_t> PixelVector(mNumPixels, 0);
  vector<uint32_t> TofBinVector(mConfig.mTOF.BinSize, 0);

  for (uint i = 0; i < PixelIds->size(); i++) {
    uint32_t Pixel = (*PixelIds)[i];
    uint32_t Tof = (*TOFs)[i] / mConfig.mTOF.Scale; // ns to us

    // accumulate events for 2D TOF
    uint32_t TofBin = std::min(Tof, mConfig.mTOF.MaxValue) *
                      (mConfig.mTOF.BinSize - 1) / mConfig.mTOF.MaxValue;
    mPixelIDs.push_back(Pixel);
    mTOFs.push_back(TofBin);

    if ((Pixel > mMaxPixel) or (Pixel < mMinPixel)) {
      mEventDiscard++;
    } else {
      mEventAccept++;

      Pixel = Pixel - mConfig.mGeometry.Offset;
      PixelVector[Pixel]++;

      Tof = std::min(Tof, mConfig.mTOF.MaxValue);
      TofBinVector[Tof * (mConfig.mTOF.BinSize - 1) / mConfig.mTOF.MaxValue]++;
    }
  }

  // update thread safe histograms storage with new data
  mHistogram.add_values(PixelVector);
  mHistogramTof.add_values(TofBinVector);

  mEventCount += PixelIds->size();
  return PixelIds->size();
}

uint32_t ESSConsumer::processDA00Data(RdKafka::Message *Msg) {
  auto EvMsg = Getda00_DataArray(Msg->payload());
  if (EvMsg->data()->size() == 0) {
    return 0;
  }

  if (!mConfig.mKafka.Source.empty() &&
      EvMsg->source_name()->str() != mConfig.mKafka.Source) {
    return 0;
  }

  const auto TimeBinsVariable = EvMsg->data()->Get(0);
  const auto DataBinsVariable = EvMsg->data()->Get(1);

  auto BinEdges = getDataVector(*TimeBinsVariable);
  auto DataBins = getDataVector(*DataBinsVariable);

  // Bin edges has one plus element to describe last edge compared to the data
  // which has as many elements as bins
  if (BinEdges.size() != DataBins.size() + 1) {
    mEventDiscard++;
    return 0;
  }

  int64_t MaxTime = *std::max_element(BinEdges.begin(), BinEdges.end());

  if (MaxTime / mConfig.mTOF.Scale > mConfig.mTOF.MaxValue) {
    return 0;
  }

  mHistogram.add_values(DataBins);
  mTOFs = BinEdges;

  mConfig.mTOF.BinSize = BinEdges.size() - 1;

  mEventCount++;
  mEventAccept++;
  return mHistogram.size();
}

uint32_t ESSConsumer::processEV42Data(RdKafka::Message *Msg) {
  auto EvMsg = GetEventMessage(Msg->payload());
  auto PixelIds = EvMsg->detector_id();
  auto TOFs = EvMsg->time_of_flight();

  // If source name is set in config, only process messages from that source
  if (!mConfig.mKafka.Source.empty() &&
      EvMsg->source_name()->str() != mConfig.mKafka.Source) {
    return 0;
  }

  if (PixelIds->size() != TOFs->size()) {
    return 0;
  }

  vector<uint32_t> PixelVector(mNumPixels, 0);
  vector<uint32_t> TofBinVector(mConfig.mTOF.BinSize, 0);

  for (uint i = 0; i < PixelIds->size(); i++) {
    uint32_t Pixel = (*PixelIds)[i];
    uint32_t Tof = (*TOFs)[i] / mConfig.mTOF.Scale; // ns to us

    // accumulate events for 2D TOF
    uint32_t TofBin = std::min(Tof, mConfig.mTOF.MaxValue) *
                      (mConfig.mTOF.BinSize - 1) / mConfig.mTOF.MaxValue;
    mPixelIDs.push_back(Pixel);
    mTOFs.push_back(TofBin);

    if ((Pixel > mMaxPixel) or (Pixel < mMinPixel)) {
      mEventDiscard++;
    } else {
      mEventAccept++;
      Pixel = Pixel - mConfig.mGeometry.Offset;
      PixelVector[Pixel]++;
      Tof = std::min(Tof, mConfig.mTOF.MaxValue);
      TofBinVector[Tof * (mConfig.mTOF.BinSize - 1) / mConfig.mTOF.MaxValue]++;
    }
  }

  mHistogram.add_values(PixelVector);
  mHistogramTof.add_values(TofBinVector);

  mEventCount += PixelIds->size();
  return PixelIds->size();
}

bool ESSConsumer::handleMessage(RdKafka::Message *Message) {
  mKafkaStats.MessagesRx++;

  const uint8_t *FlatBuffer = static_cast<const uint8_t *>(Message->payload());

  flatbuffers::Verifier Verifier(FlatBuffer, Message->len());

  switch (Message->err()) {
  case RdKafka::ERR__TIMED_OUT:
    mKafkaStats.MessagesTMO++;
    return false;
    break;

  case RdKafka::ERR_NO_ERROR:
    mKafkaStats.MessagesData++;

    if (VerifyEvent44MessageBuffer(Verifier)) {
      processEV44Data(Message);
    } else if (VerifyEventMessageBuffer(Verifier)) {
      processEV42Data(Message);
    } else if (Verifyda00_DataArrayBuffer(Verifier)) {
      processDA00Data(Message);
    } else {
      mKafkaStats.MessagesUnknown++;
      fmt::print("Unknown message type\n");
      return false;
    }

    return true;
    break;

  case RdKafka::ERR__PARTITION_EOF:
    mKafkaStats.MessagesEOF++;
    return false;
    break;

  case RdKafka::ERR__UNKNOWN_TOPIC:
  case RdKafka::ERR__UNKNOWN_PARTITION:
    mKafkaStats.MessagesUnknown++;
    fmt::print("Consume failed: {}\n", Message->errstr());
    return false;
    break;

  default: // Other errors
    mKafkaStats.MessagesOther++;
    fmt::print("Consume failed: {}", Message->errstr());
    return false;
  }
}

// Copied from daquiri - added seed based on pid
string ESSConsumer::randomGroupString(size_t length) {
  srand(getpid());
  auto randchar = []() -> char {
    const char charset[] = "0123456789"
                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "abcdefghijklmnopqrstuvwxyz";
    const size_t max_index = (sizeof(charset) - 1);
    return charset[rand() % max_index];
  };
  string str(length, 0);
  std::generate_n(str.begin(), length, randchar);
  return str;
}

vector<int64_t>
ESSConsumer::getDataVector(const da00_Variable &Variable) const {
  vector<int64_t> Data;

  auto data = Variable.unit()->str();
  auto shape = Variable.shape()->Get(0);

  switch (Variable.data_type()) {
  case da00_dtype::int32: {
    auto dataPtr = reinterpret_cast<const int32_t *>(Variable.data());

    // skip the first element which is the legth of the data
    dataPtr++;
    Data.assign(dataPtr, dataPtr + shape);
    break;
  }
  case da00_dtype::int64: {
    auto dataPtr = reinterpret_cast<const int64_t *>(Variable.data());

    // skip the first element which is the legth of the data
    dataPtr++;
    Data.assign(dataPtr, dataPtr + shape);
    break;
  }
  case da00_dtype::uint32: {
    auto dataPtr = reinterpret_cast<const uint32_t *>(Variable.data());

    // skip the first element which is the legth of the data
    dataPtr++;
    Data.assign(dataPtr, dataPtr + shape);
    break;
  }
  case da00_dtype::uint64: {
    auto dataPtr = reinterpret_cast<const uint64_t *>(Variable.data());

    // skip the first element which is the legth of the data
    dataPtr++;
    Data.assign(dataPtr, dataPtr + shape);
    break;
  }
  default:
    break;
  }
  return Data;
}

/// \todo is timeout reasonable?
std::unique_ptr<RdKafka::Message> ESSConsumer::consume() {
  std::unique_ptr<RdKafka::Message> msg(mConsumer->consume(1000));
  return msg;
}


/// \brief read out the histogram data and reset it
vector<uint32_t> ESSConsumer::readResetHistogram() {
  vector<uint32_t> ret = mHistogram;

  if (checkDelivery(DataType::HISTOGRAM)) {
    mHistogram.clear();
  }

  return ret;
}

/// \brief read out the TOF histogram data and reset it
vector<uint32_t> ESSConsumer::readResetHistogramTof() {
  vector<uint32_t> ret = mHistogramTof;

  if (checkDelivery(DataType::HISTOGRAM_TOF)) {
    mHistogramTof.clear();
  }

  return ret;
}

/// \brief read out the event pixel IDs and clear the vector
vector<uint32_t> ESSConsumer::readResetPixelIDs() {
  vector<uint32_t> ret = mPixelIDs;

  if (checkDelivery(DataType::PIXEL_ID)) {
    mPixelIDs.clear();
  }


  return ret;
}

/// \brief read out the event TOFs and clear the vector
vector<uint32_t> ESSConsumer::readResetTOFs() {
  vector<uint32_t> ret = mTOFs;

  if (checkDelivery(DataType::TOF)) {
    mTOFs.clear();
  }

  return ret;
}

vector<uint32_t> ESSConsumer::getTofs() const {
  vector<uint32_t> ret = mTOFs;

  return ret;
}

bool ESSConsumer::checkDelivery(DataType Type) {
  mDeliveryCount[Type] += 1;
  if (mDeliveryCount[Type] == mSubscriptionCount[Type]) {
    mDeliveryCount[Type] = 0;

    return true;
  }

  return false;
}

void ESSConsumer::addSubscriber(PlotType Type) {
  mSubscribers += 1;

  // Increment the total number of plots
  mSubscriptionCount[DataType::ANY] += 1;

  // Register data types for the plot type
  switch (Type)
  {
    case PlotType::TOF:
      mSubscriptionCount[DataType::HISTOGRAM_TOF] += 1;
      break;

    case PlotType::TOF2D:
      mSubscriptionCount[DataType::PIXEL_ID] += 1;
      mSubscriptionCount[DataType::TOF] += 1;
      break;

    case PlotType::PIXELS:
      mSubscriptionCount[DataType::HISTOGRAM] += 1;
      break;

    case PlotType::HISTOGRAM:
      mSubscriptionCount[DataType::HISTOGRAM] += 1;
      break;

    default:
      break;
  }

  // Uncomment to print the subscription state
  // for (const auto& dt: DataType::types()) {
  //   fmt::print("ESSConsumer::addSubscriber {} {}\n", Type, mSubscriptionCount[dt]);
  // }
}

void ESSConsumer::gotEventRequest() {
  mEventRequests += 1;

  // Reset if all event requests have been delivered
  if (mEventRequests == mSubscriptionCount[DataType::ANY]) {
      mEventCount = 0;
      mEventAccept = 0;
      mEventDiscard = 0;

      mEventRequests = 0;
  }
}



