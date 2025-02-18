// Copyright (C) 2020 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file CustomTofPlot.h
///
/// \brief Creates a QCustomPlot based on the configuration parameters
//===----------------------------------------------------------------------===//

#pragma once

#include <AbstractPlot.h>

#include <stdint.h>
#include <vector>

// Forward declarations
class Configuration;
class ESSConsumer;
class ESSGeometry;
class QCPGraph;
class QMouseEvent;
class QObject;



class CustomTofPlot : public AbstractPlot {
  Q_OBJECT
public:

  /// \brief plot needs the configurable plotting options
  CustomTofPlot(Configuration &Config, ESSConsumer &Consumer);

  /// \brief adds histogram data, clears periodically then calls
  /// plotDetectorImage()
  void updateData() override;

  /// \brief update plot based on (possibly dynamic) config settings
  void setCustomParameters();

  ///
  void clearDetectorImage() override;

public slots:
  void showPointToolTip(QMouseEvent *event);

private:
  /// \brief updates the image
  /// \param Force forces updates of histogram data with zero count
  void plotDetectorImage(bool Force) override;

  // QCustomPlot variables
  QCPGraph *mGraph{nullptr};

  /// \brief configuration obtained from main()
  Configuration &mConfig;

  std::vector<uint32_t> HistogramTofData;

  /// \brief for calculating x, y, z from pixelid
  ESSGeometry *LogicalGeometry;

  /// \brief reference time for periodic clearing of histogram
  std::chrono::time_point<std::chrono::high_resolution_clock> t1;
};
