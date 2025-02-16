// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file AbstractPlot.h
///
/// \brief
//===----------------------------------------------------------------------===//

#pragma once

#include "CommonTypes.h"
#include "ESSConsumer.h"

#include <QPlot/QPlot.h>

class AbstractPlot : public QCustomPlot {
  Q_OBJECT


protected:
  AbstractPlot(PlotType Type, ESSConsumer &Consumer)
  : mPlotType(Type)
  , mConsumer(Consumer) {
  mConsumer.addSubscriber(mPlotType);
};

  PlotType mPlotType;

  ESSConsumer &mConsumer;

public:
  PlotType getPlotType() { return mPlotType; }

  virtual void clearDetectorImage() = 0;

  virtual void updateData() = 0;

  virtual void plotDetectorImage(bool Force) = 0;
};