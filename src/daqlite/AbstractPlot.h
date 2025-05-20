// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file AbstractPlot.h
///
/// \brief
//===----------------------------------------------------------------------===//

#pragma once

#include <ESSConsumer.h>
#include <types/PlotType.h>

#include <QPlot/QPlot.h>

#include <optional>

class AbstractPlot : public QCustomPlot {
  Q_OBJECT

public:
  PlotType getPlotType() { return mPlotType; }

  virtual void clearDetectorImage() = 0;

  virtual void updateData() = 0;

  virtual void plotDetectorImage(bool Force) = 0;

protected:
  // AbstractPlot is abstract and can ONLY be instantiated from a derived class
  AbstractPlot(PlotType Type, ESSConsumer &Consumer)
    : mPlotType(Type)
    , mConsumer(Consumer) {
    mConsumer.addSubscriber(mPlotType);
  };

  /// \brief Type plot type - Pixel, Histogram, etc.
  PlotType mPlotType;

  /// \brief Consumer thread used to deliver data to the plot
  ESSConsumer &mConsumer;

  /// \brief Store default axis ranges.
  void showEvent(QShowEvent *) override;

  /// \brief Handle canvas reset
  /// \param event Key event
  void keyPressEvent(QKeyEvent *event) override;

  /// \brief Draw a zoom rectangle over the base plot
  /// \param event Paint event
  void paintEvent(QPaintEvent *event) override;

  /// \brief Register first zoom rectangle corner
  /// \param event Mouse event
  void mousePressEvent(QMouseEvent *event) override;

  /// \brief Register second zoom rectangle corner
  /// \param event Mouse event
  void mouseMoveEvent(QMouseEvent *event) override;

  /// \brief Reset zoom rectangle
  /// \param event Mouse event
  void mouseReleaseEvent(QMouseEvent *event) override;

  /// Zoom rectangle vars
  bool mZoomRectActive{false};

  /// \brief First zoom rectangle corner
  std::optional<QPointF> mPoint0;

  /// \brief Second zoom rectangle corner
  std::optional<QPointF> mPoint1;

  /// \brief Default range for X-axis
  std::optional<QCPRange> mXRange;

  /// \brief Default range for Y-axis
  std::optional<QCPRange> mYRange;
};