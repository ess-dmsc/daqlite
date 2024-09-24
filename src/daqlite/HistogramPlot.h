// Copyright (C) 2020 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file HistogramPlot.h
///
/// \brief Creates a hiostogram plot based on QCustomPlot and
///  on the configuration parameters
//===----------------------------------------------------------------------===//

#pragma once

#include <AbstractPlot.h>
#include <ESSConsumer.h>
#include <Configuration.h>
#include <QPlot/QPlot.h>
#include <chrono>
#include <logical_geometry/ESSGeometry.h>
#include <qvector.h>
#include <vector>

class HistogramPlot : public AbstractPlot {
  Q_OBJECT
public:
  /// \brief plot needs the configurable plotting options
  HistogramPlot(Configuration &Config, ESSConsumer &Consumer);

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

  std::vector<uint32_t> HistogramYAxisValues;
  std::vector<uint32_t> HistogramXAxisValues;

  /// \brief for calculating x, y, z from pixelid
  ESSGeometry *LogicalGeometry;

  /// \brief reference time for periodic clearing of histogram
  std::chrono::time_point<std::chrono::high_resolution_clock> t1;
};
