// Copyright (C) 2020 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file PixelsPlot.cpp
///
//===----------------------------------------------------------------------===//

#include <AMOR2DTofPlot.h>

#include <AbstractPlot.h>
#include <Configuration.h>
#include <ESSConsumer.h>

#include <types/PlotType.h>
#include <types/Gradients.h>

#include <logical_geometry/ESSGeometry.h>

#include <QEvent>

#include <fmt/format.h>
#include <string.h>
#include <stdexcept>
#include <string>
#include <vector>

using std::vector;

AMOR2DTofPlot::AMOR2DTofPlot(Configuration &Config,
                             ESSConsumer &Consumer)
    : AbstractPlot(PlotType::TOF2D, Consumer, Config) {
  if ((not(mConfig.mGeometry.YDim <= TOF2DY) or
       (not(mConfig.mTOF.BinSize <= TOF2DX)))) {
    throw(std::runtime_error("2D TOF histogram size mismatch"));
  }
  memset(HistogramData2D, 0, sizeof(HistogramData2D));

  connect(this, &QCustomPlot::mouseMove, this, &AMOR2DTofPlot::showPointToolTip);
  setAttribute(Qt::WA_AlwaysShowToolTips);

  auto &geom = mConfig.mGeometry;
  LogicalGeometry = new ESSGeometry(geom.XDim, geom.YDim, geom.ZDim, 1);
  // HistogramData.resize(LogicalGeometry->max_pixel() + 1);

  // this will also allow rescaling the color scale by dragging/zooming
  setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

  axisRect()->setupFullAxesBox(true);

  // set up the QCPColorMap:
  yAxis->setRangeReversed(true);
  yAxis->setSubTicks(true);
  xAxis->setSubTicks(false);
  xAxis->setTickLabelRotation(90);

  mColorMap = new QCPColorMap(xAxis, yAxis);

  // we want the color map to have nx * ny data points
  xAxis->setLabel("TOF");
  yAxis->setLabel("Y");
  mColorMap->data()->setSize(mConfig.mTOF.BinSize, geom.YDim);
  mColorMap->data()->setRange(QCPRange(0, mConfig.mTOF.MaxValue),
                              QCPRange(0, mConfig.mGeometry.YDim)); //

  // add a color scale:
  mColorScale = new QCPColorScale(this);

  // add it to the right of the main axis rect
  plotLayout()->addElement(0, 1, mColorScale);

  // scale shall be vertical bar with tick/axis labels
  // right (actually atRight is already the default)
  mColorScale->setType(QCPAxis::atRight);

  // associate the color map with the color scale
  mColorMap->setColorScale(mColorScale);
  mColorMap->setInterpolate(mConfig.mPlot.Interpolate);
  mColorMap->setTightBoundary(false);
  mColorScale->axis()->setLabel("Counts");

  setCustomParameters();

  // make sure the axis rect and color scale synchronize their bottom and top
  // margins (so they line up):
  QCPMarginGroup *marginGroup = new QCPMarginGroup(this);
  axisRect()->setMarginGroup(QCP::msBottom | QCP::msTop, marginGroup);
  mColorScale->setMarginGroup(QCP::msBottom | QCP::msTop, marginGroup);

  // rescale the key (x) and value (y) axes so the whole color map is visible:
  rescaleAxes();

  t1 = std::chrono::high_resolution_clock::now();
}

void AMOR2DTofPlot::setCustomParameters() {
  // set the color gradient of the color map to one of the presets:
  QCPColorGradient Gradient(getColorGradient(mConfig.mPlot.ColorGradient));
  if (mConfig.mPlot.InvertGradient) {
    Gradient = Gradient.inverted();
  }

  mColorMap->setGradient(Gradient);

  if (mConfig.mPlot.LogScale) {
    mColorMap->setDataScaleType(QCPAxis::stLogarithmic);
  } else {
    mColorMap->setDataScaleType(QCPAxis::stLinear);
  }
}

// Try the user supplied gradient name, then fall back to 'hot' and
// provide a list of options
QCPColorGradient
AMOR2DTofPlot::getColorGradient(const std::string &GradientName) {
  if (const auto search = GRADIENTS.find(GradientName); search != GRADIENTS.end()) {
    return search->second;
  } else {
    fmt::print("Gradient {} not found, using 'hot' instead.\n", GradientName);
    fmt::print("Supported gradients are: ");
    for (auto &Gradient : GRADIENTS) {
      fmt::print("{} ", Gradient.first);
    }
    fmt::print("\n");

    return GRADIENTS["hot"];
  }
}

void AMOR2DTofPlot::clearDetectorImage() {
  memset(HistogramData2D, 0, sizeof(HistogramData2D));
  plotDetectorImage(true);
}

void AMOR2DTofPlot::plotDetectorImage(bool Force) {
  setCustomParameters();

  for (int y = 0; y < mConfig.mGeometry.YDim; y++) {
    for (unsigned int x = 0; x < mConfig.mTOF.BinSize; x++) {
      if ((HistogramData2D[x][y] == 0) and (not Force)) {
        continue;
      }
      // printf("debug x %u, y %u, z %u\n", x, y, HistogramData2D[x][y]);
      mColorMap->data()->setCell(x, y, HistogramData2D[x][y]);
    }
  }

  // rescale the data dimension (color) such that all data points lie in the
  // span visualized by the color gradient:
  mColorMap->rescaleDataRange(true);

  replot();
}

void AMOR2DTofPlot::updateData() {
  // Get newest histogram data from Consumer
  vector<uint32_t> PixelIDs = mConsumer.readData(DataType::PIXEL_ID);
  vector<uint32_t> TOFs = mConsumer.readData(DataType::TOF);

  // Accumulate counts, PixelId 0 does not exist
  if (PixelIDs.size() == 0) {
    return;
  }

  for (uint i = 0; i < PixelIDs.size(); i++) {
    if (PixelIDs[i] == 0) {
      continue;
    }
    int tof = TOFs[i];
    int yvals = (PixelIDs[i] - 1) / mConfig.mGeometry.XDim;
    HistogramData2D[tof][yvals]++;
  }
  plotDetectorImage(false);

  return;
}

// MouseOver
void AMOR2DTofPlot::showPointToolTip(QMouseEvent *event) {
  int x = this->xAxis->pixelToCoord(event->pos().x());
  int y = this->yAxis->pixelToCoord(event->pos().y());

  setToolTip(QString("%1 , %2").arg(x).arg(y));
}
