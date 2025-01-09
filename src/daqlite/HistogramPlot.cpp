// Copyright (C) 2020 - 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file HistogramPlot.cpp
///
//===----------------------------------------------------------------------===//

#include <HistogramPlot.h>
#include <QPlot/qcustomplot/qcustomplot.h>
#include <WorkerThread.h>
#include <algorithm>
#include <assert.h>
#include <cstddef>
#include <cstdint>
#include <fmt/format.h>
#include <string>
#include <vector>

HistogramPlot::HistogramPlot(Configuration &Config, ESSConsumer &Consumer)
    : AbstractPlot(HISTOGRAM, Consumer), mConfig(Config) {

  // Register callback functions for events
  connect(this, SIGNAL(mouseMove(QMouseEvent *)), this,
          SLOT(showPointToolTip(QMouseEvent *)));
  setAttribute(Qt::WA_AlwaysShowToolTips);

  auto &geom = mConfig.Geometry;

  LogicalGeometry = new ESSGeometry(geom.XDim, geom.YDim, geom.ZDim, 1);

  HistogramYAxisValues.resize(mConfig.TOF.BinSize);

  // this will also allow rescaling the color scale by dragging/zooming
  setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

  axisRect()->setupFullAxesBox(true);

  // set up the QCPColorMap:
  yAxis->setRangeReversed(false);
  yAxis->setSubTicks(true);
  xAxis->setSubTicks(false);
  xAxis->setTickLabelRotation(90);

  mGraph = new QCPGraph(xAxis, yAxis);
  // mGraph->setLineStyle(QCPGraph::lsNone);
  mGraph->setBrush(QBrush(QColor(0, 0, 255, 20)));
  mGraph->setLineStyle(QCPGraph::lsStepCenter);
  mGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));

  // we want the color map to have nx * ny data points

  if (mConfig.Plot.XAxis.empty()) {
    xAxis->setLabel("TOF (us)");
  } else {
    xAxis->setLabel(mConfig.Plot.XAxis.c_str());
  }

  yAxis->setLabel("Value Sum");
  xAxis->setRange(0, 50000);

  setCustomParameters();

  t1 = std::chrono::high_resolution_clock::now();
}

void HistogramPlot::setCustomParameters() {
  if (mConfig.Plot.LogScale) {
    yAxis->setScaleType(QCPAxis::stLogarithmic);
  } else {
    yAxis->setScaleType(QCPAxis::stLinear);
  }
}

void HistogramPlot::plotDetectorImage(bool Force) {
  setCustomParameters();
  mGraph->data()->clear();

  for (unsigned int i = 0; i < HistogramYAxisValues.size(); i++) {
    // calculate the middle x value of the bin to place the data point
    auto binWidth = HistogramXAxisValues[i + 1] - HistogramXAxisValues[i];
    auto middleXValue = HistogramXAxisValues[i] + binWidth / 2.0;

    double ScaledXValue = middleXValue / mConfig.TOF.Scale;

    mGraph->addData(ScaledXValue, HistogramYAxisValues[i]);
  }

  // yAxis->rescale();
  if (mConfig.TOF.AutoScaleX && !HistogramXAxisValues.empty()) {
    double MaxX = *std::max_element(HistogramXAxisValues.begin(),
                                    HistogramXAxisValues.end());

    double MinX = *std::min_element(HistogramXAxisValues.begin(),
                                    HistogramXAxisValues.end());

    xAxis->setRange(MinX / mConfig.TOF.Scale, MaxX / mConfig.TOF.Scale * 1.05);
  }
  if (mConfig.TOF.AutoScaleY && !HistogramYAxisValues.empty()) {
    auto MaxY = *std::max_element(HistogramYAxisValues.begin(),
                                  HistogramYAxisValues.end());
    yAxis->setRange(0, MaxY * 1.05);
  }

  replot();
}

void HistogramPlot::updateData() {
  // printf("addData (TOF) Histogram size %lu\n", Histogram.size());
  auto t2 = std::chrono::high_resolution_clock::now();
  std::chrono::duration<int64_t, std::nano> elapsed = t2 - t1;

  // continure the the update only if we have data available from the consumer
  if (mConsumer.getHistogramSize() == 0 or mConsumer.getTOFsSize() == 0) {
    return;
  }

  std::vector<uint32_t> YAxisValues = mConsumer.readResetHistogram();
  auto TofValues = mConsumer.getTofs();

  HistogramXAxisValues = TofValues;

  if (YAxisValues.size() != HistogramXAxisValues.size() - 1) {
    fmt::print("HistogramPlot::updateData() - Y axis values in not fit for x "
               "axis values. Skip processing!\n");
    return;
  }

  // Periodically clear the histogram data sets
  //
  int64_t nsBetweenClear = 1000000000LL * mConfig.Plot.ClearEverySeconds;
  if (mConfig.Plot.ClearPeriodic and (elapsed.count() >= nsBetweenClear)) {
    std::fill(HistogramYAxisValues.begin(), HistogramYAxisValues.end(), 0);
    std::fill(HistogramXAxisValues.begin(), HistogramXAxisValues.end(), 0);
    t1 = std::chrono::high_resolution_clock::now();
  }

  if (HistogramYAxisValues.size() < YAxisValues.size()) {
    HistogramYAxisValues.resize(YAxisValues.size());
  }

  for (unsigned int i = 0; i < YAxisValues.size(); i++) {
    HistogramYAxisValues[i] += YAxisValues[i];
  }

  plotDetectorImage(false);
  return;
}

void HistogramPlot::clearDetectorImage() {
  std::fill(HistogramYAxisValues.begin(), HistogramYAxisValues.end(), 0);
  plotDetectorImage(true);
}

// MouseOver, display coordinate and data in tooltip
void HistogramPlot::showPointToolTip(QMouseEvent *event) {
  int x = this->xAxis->pixelToCoord(event->pos().x());

  // Calculate x coord width of the graphical representation of the column
  int xCoordStep = int(mConfig.TOF.MaxValue / mConfig.TOF.BinSize);

  // Get the index in data store for the x coordinate
  int xCoordDataIndex = int((x - xCoordStep / 2) / xCoordStep);

  // Get coulmn middle TOF value for the x coordinate
  int xCoordTofValue = int((x + xCoordStep / 2) / xCoordStep) * xCoordStep;

  // Get the count value from the data store
  double count = mGraph->data()->at(xCoordDataIndex)->mainValue();

  setToolTip(QString("Tof: %1 Value: %2").arg(xCoordTofValue).arg(count));
}