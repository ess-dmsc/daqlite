// Copyright (C) 2022-2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file CustomAMOR2DTOFPlot.h
///
/// \brief Creates (maybe) a QCustomPlot based on the configuration parameters
//===----------------------------------------------------------------------===//

#pragma once

#include <AbstractPlot.h>

#include <QPlot/qcustomplot/qcustomplot.h>

#include <stdint.h>
#include <map>
#include <string>
#include <utility>

// Forward declarations
class Configuration;
class ESSConsumer;
class ESSGeometry;

class CustomAMOR2DTOFPlot : public AbstractPlot {
  Q_OBJECT
public:
  /// \brief plot needs the configurable plotting options
  CustomAMOR2DTOFPlot(Configuration &Config, ESSConsumer &Consumer);

  /// \brief adds histogram data, clears periodically then calls
  /// plotDetectorImage()
  void updateData() override;

  /// \brief Support for different gradients
  QCPColorGradient getColorGradient(const std::string &GradientName);

  /// \brief update plot based on (possibly dynamic) config settings
  void setCustomParameters();

  /// \brief rotate through gradient names
  std::string getNextColorGradient(const std::string &GradientName);

  /// \brief clears histogram data (overridden from AbstractPlot)
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

  /// \brief allocated according to config in constructor
  #define TOF2DX 512
  #define TOF2DY 512
  uint32_t HistogramData2D[TOF2DX + 1][TOF2DY + 1];

  /// \brief for calculating x, y, z from pixelid
  ESSGeometry *LogicalGeometry;

  // See colors here
  // https://www.qcustomplot.com/documentation/classQCPColorGradient.html
  std::map<std::string, QCPColorGradient> mGradients{
      {"hot", QCPColorGradient::gpHot},
      {"grayscale", QCPColorGradient::gpGrayscale},
      {"cold", QCPColorGradient::gpCold},
      {"night", QCPColorGradient::gpNight},
      {"candy", QCPColorGradient::gpCandy},
      {"geography", QCPColorGradient::gpGeography},
      {"ion", QCPColorGradient::gpIon},
      {"thermal", QCPColorGradient::gpThermal},
      {"polar", QCPColorGradient::gpPolar},
      {"spectrum", QCPColorGradient::gpSpectrum},
      {"jet", QCPColorGradient::gpJet},
      {"hues", QCPColorGradient::gpHues}};

  /// \brief reference time for periodic clearing of histogram
  std::chrono::time_point<std::chrono::high_resolution_clock> t1;
};
