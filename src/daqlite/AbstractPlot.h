// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file AbstractPlot.h
///
/// \brief 
//===----------------------------------------------------------------------===//

#pragma once

#include "ESSConsumer.h"
#include <QPlot/QPlot.h>

enum PlotType {
    TOF2D,
    TOF,
    PIXEL,
    HISTOGRAM
};

class AbstractPlot : public QCustomPlot {
    Q_OBJECT

    PlotType mPlotType;

protected:
    ESSConsumer *mConsumer{nullptr};


public:
    void registerConsumer(ESSConsumer &Consumer) {
        mConsumer = &Consumer;
    }

    virtual void clearDetectorImage() = 0;

    virtual void updateData() = 0;

    virtual void plotDetectorImage(bool Force) = 0;

    PlotType getPlotType() {
        return mPlotType;
    }
};