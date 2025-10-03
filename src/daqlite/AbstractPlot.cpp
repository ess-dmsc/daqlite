// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file AbstractPlot.cpp
///
//===----------------------------------------------------------------------===//

#include <AbstractPlot.h>
#include <Configuration.h>
#include <ESSConsumer.h>

#include <fmt/format.h>

AbstractPlot::AbstractPlot(PlotType Type, ESSConsumer &Consumer, Configuration &Config)
    : mConsumer(Consumer)
    , mConfig(Config)
    , mPlotType(Type)
    , mZoomRectActive(false) {
    mConsumer.addSubscriber(mPlotType);
    mConsumer.addSource(mConfig.mPlot.Source);
  };

void AbstractPlot::paintEvent(QPaintEvent *event) {
  // ---------------------------------------------------------------------------
  // If activated, draw a zoom rectangle on top of the base plot

  // Draw the base plot
  QCustomPlot::paintEvent(event);

  // Sanity checks
  if (!mZoomRectActive) {
    return;
  }

  if (!mPoint0 || !mPoint1) {
    return;
  }

  // Set up parameters for zoom rect
  QPainter painter(this);
  painter.save();

  const QColor lineColor(QColorConstants::DarkGray);
  const QColor fillColor(QColorConstants::Black);
  const double opacity = 0.2;

  const QPointF &p0 = mPoint0.value();
  const QPointF &p1 = mPoint1.value();

  // ---------------------------------------------------------------------------
  // Draw zoom rect outline

  // Set the pen color
  QPen pen;
  pen.setColor(lineColor);
  pen.setWidthF(1.0);
  painter.setPen(pen);

  // Set no brush
  QBrush brush;
  brush.setStyle(Qt::NoBrush);
  painter.setBrush(brush);

  // Draw rubber band
  QRectF rect(p0, p1);
  rect = rect.normalized();
  rect.adjust(-1, -1, 1, 1);
  painter.drawRect(rect);

  // ---------------------------------------------------------------------------
  // Fill the zoom rect

  // Turn off the pen
  pen.setStyle(Qt::NoPen);
  painter.setPen(pen);

  // Set brush style and color
  brush.setStyle(Qt::SolidPattern);
  brush.setColor(fillColor);
  painter.setBrush(brush);

  painter.setOpacity(opacity);
  rect.adjust(1, 1, 0, 0);
  painter.drawRect(rect);
  painter.restore();
}


void AbstractPlot::showEvent(QShowEvent *) {
  // Store initial plot ranges, if these are unset
  if (!mXRange) {
    mXRange = xAxis->range();
  }

  if (!mYRange) {
    mYRange = yAxis->range();
  }
}

void AbstractPlot::keyPressEvent(QKeyEvent *event) {
  // Canvas resets
  const bool ctrlOn = event->modifiers() == Qt::ControlModifier;
  if (ctrlOn) {
    const auto key = event->key();

    switch (key) {
    // Reset canvas range
    case Qt::Key_R:
        if (mXRange && mYRange) {
          xAxis->setRange(mXRange.value());
          yAxis->setRange(mYRange.value());
        }
        break;

    // Store new canvas range
    case Qt::Key_S:
        mXRange = xAxis->range();
        mYRange = yAxis->range();
        break;

      default:
        break;
    }

  // Otherwise pass key event to base class
  } else {
    QCustomPlot::keyPressEvent(event);
  }
}

void AbstractPlot::mousePressEvent(QMouseEvent *event) {
  // Initiate a zoom rectangle if
  //
  //   - Left mouse and Ctrl keyboard modifier is pressed
  const bool leftMouse = event->button() == Qt::LeftButton;
  const bool ctrlOn = event->modifiers() == Qt::ControlModifier;
  if (leftMouse && ctrlOn) {
    mZoomRectActive = true;
    mPoint0 = event->position().toPoint();
    // fmt::print("mousePressEvent: {} {}\n", mPoint0->x(), mPoint0->y());
  }

  // ... otherwise, we let the base class handle the event
  else {
    QCustomPlot::mousePressEvent(event);
  }
}

void AbstractPlot::mouseMoveEvent(QMouseEvent *event) {
  // Call base class if zoom rect is NOT active
  if (!mZoomRectActive) {
    QCustomPlot::mouseMoveEvent(event);

    return;
  }

  // Record new rect position and request canvas update
  mPoint1 = event->position().toPoint();
  update();
}

void AbstractPlot::mouseReleaseEvent(QMouseEvent *event) {
  // Call base class if zoom rect is NOT active
  if (!mZoomRectActive || !mPoint0 || !mPoint1) {
    QCustomPlot::mouseReleaseEvent(event);

    return;
  }

  // Convert zoom pixels to physical space
  const double x0 = xAxis->pixelToCoord(mPoint0->x());
  const double x1 = xAxis->pixelToCoord(mPoint1->x());

  const double y0 = yAxis->pixelToCoord(mPoint0->y());
  const double y1 = yAxis->pixelToCoord(mPoint1->y());

  // Set new axis ranges
  xAxis->setRange(x0, x1);
  yAxis->setRange(y0, y1);

  // Reset zoom vars and request canvas update
  mZoomRectActive = false;
  mPoint0 = std::nullopt;
  mPoint1 = std::nullopt;

  update();
}
