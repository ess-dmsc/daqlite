// Copyright (C) 2020 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file PixelsPlot.h
///
/// \brief Creates a QCustomPlot based on the configuration parameters
//===----------------------------------------------------------------------===//

#pragma once

#include <AbstractPlot.h>

#include <QPlot/qcustomplot/qcustomplot.h>

#include <stdint.h>
#include <map>
#include <string>
#include <utility>
#include <vector>

// Forward declarations
class Configuration;
class ESSConsumer;
class ESSGeometry;

class PixelsPlot : public AbstractPlot {
  Q_OBJECT
public:
  enum Projection {ProjectionXY, ProjectionXZ, ProjectionYZ};

  /// \brief plot needs the configurable plotting options
  PixelsPlot(Configuration &Config, ESSConsumer&, Projection Proj);

  /// \brief adds histogram data, clears periodically then calls
  /// plotDetectorImage()
  void updateData() override;

  /// \brief Support for different gradients
  QCPColorGradient getColorGradient(const std::string &GradientName);

  /// \brief update plot based on (possibly dynamic) config settings
  void setCustomParameters();

  /// \brief clears histogram data
  void clearDetectorImage() override;

  /// \brief updates the image
  /// \param Force forces updates of histogram data with zero count
  void plotDetectorImage(bool Force) override;

public slots:
  void showPointToolTip(QMouseEvent *event);

private:
  // QCustomPlot variables
  QCPColorScale *mColorScale{nullptr};
  QCPColorMap *mColorMap{nullptr};

  /// \brief configuration obtained from main()
  Configuration &mConfig;

  std::vector<uint32_t> HistogramData;

  /// \brief for calculating x, y, z from pixelid
  ESSGeometry *LogicalGeometry;

  //
  Projection mProjection;

  /// \brief reference time for periodic clearing of histogram
  std::chrono::time_point<std::chrono::high_resolution_clock> t1;
};
