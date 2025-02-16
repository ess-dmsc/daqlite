// Copyright \(C\) 2020 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file MainWindow.cpp
///
//===----------------------------------------------------------------------===//

#include <MainWindow.h>
#include <ui_MainWindow.h>

#include <Custom2DPlot.h>
#include <CustomAMOR2DTOFPlot.h>
#include <CustomTofPlot.h>
#include <HistogramPlot.h>
#include <WorkerThread.h>

#include <QApplication>
#include <QMetaType>
#include <QPushButton>

#include <stdint.h>
#include <string.h>
#include <memory>
#include <stdexcept>
#include <string>

class QWidget;

MainWindow::MainWindow(const Configuration &Config, WorkerThread *Worker, QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , mConfig(Config)
  , mWorker(Worker)
  , mCount(0) {
  ui->setupUi(this);
  setupPlots();

  ui->lblDescriptionText->setText(mConfig.mPlot.PlotTitle.c_str());
  ui->lblEventRateText->setText("0");

  // Connect all windows buttons
  auto signal = &QPushButton::clicked;
  connect(ui->pushButtonQuit,       signal, this, &MainWindow::handleExitButton);
  connect(ui->pushButtonClear,      signal, this, &MainWindow::handleClearButton);
  connect(ui->pushButtonLog,        signal, this, &MainWindow::handleLogButton);
  connect(ui->pushButtonGradient,   signal, this, &MainWindow::handleGradientButton);
  connect(ui->pushButtonInvert,     signal, this, &MainWindow::handleInvertButton);
  connect(ui->pushButtonAutoScaleX, signal, this, &MainWindow::handleAutoScaleXButton);
  connect(ui->pushButtonAutoScaleY, signal, this, &MainWindow::handleAutoScaleYButton);

  updateGradientLabel();
  updateAutoScaleLabels();
  show();
  startKafkaConsumerThread();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::setupPlots() {
  PlotType Type(mConfig.mPlot.Plot);

  if (Type == PlotType::TOF2D) {
    Plots.push_back(std::make_unique<CustomAMOR2DTOFPlot>(
        mConfig, mWorker->getConsumer()));

    // register plot on ui
    ui->gridLayout->addWidget(Plots.back().get(), 0, 0, 1, 1);

  }

  else if (Type == PlotType::TOF) {
    Plots.push_back(std::make_unique<CustomTofPlot>(
        mConfig, mWorker->getConsumer()));

    // register plot on ui
    ui->gridLayout->addWidget(Plots.back().get(), 0, 0, 1, 1);

    // Hide irrelevant buttons for TOF
    ui->pushButtonGradient->setVisible(false);
    ui->pushButtonInvert->setVisible(false);
    ui->lblGradientText->setVisible(false);
    ui->lblGradient->setVisible(false);

  }

  else if (Type == PlotType::HISTOGRAM) {
    Plots.push_back(std::make_unique<HistogramPlot>(
        mConfig, mWorker->getConsumer()));

    ui->gridLayout->addWidget(Plots.back().get(), 0, 0, 1, 1);

    // Hide irrelevant buttons for TOF
    ui->pushButtonGradient->setVisible(false);
    ui->pushButtonInvert->setVisible(false);
    ui->lblGradientText->setVisible(false);
    ui->lblGradient->setVisible(false);

  }

  else if (Type == PlotType::PIXELS) {

    // Always create the XY plot
    Plots.push_back(std::make_unique<Custom2DPlot>(
        mConfig, mWorker->getConsumer(),
        Custom2DPlot::ProjectionXY));
    ui->gridLayout->addWidget(Plots.back().get(), 0, 0, 1, 1);

    // If detector is 3D, also create XZ and YZ
    if (mConfig.mGeometry.ZDim > 1) {
      Plots.push_back(std::make_unique<Custom2DPlot>(
          mConfig, mWorker->getConsumer(),
          Custom2DPlot::ProjectionXZ));
      ui->gridLayout->addWidget(Plots.back().get(), 0, 1, 1, 1);
      Plots.push_back(std::make_unique<Custom2DPlot>(
          mConfig, mWorker->getConsumer(),
          Custom2DPlot::ProjectionYZ));
      ui->gridLayout->addWidget(Plots.back().get(), 0, 2, 1, 1);
    }
  }

  else {
    throw(std::runtime_error("No valid plot type specified"));
  }

  // Autoscale buttons are only relevant for TOF and HISTOGRAM
  if (Plots[0]->getPlotType() == PlotType::TOF || Plots[0]->getPlotType() == PlotType::HISTOGRAM) {
    ui->pushButtonAutoScaleX->setVisible(true);
    ui->lblAutoScaleXText->setVisible(true);
    ui->lblAutoScaleX->setVisible(true);
    ui->pushButtonAutoScaleY->setVisible(true);
    ui->lblAutoScaleYText->setVisible(true);
    ui->lblAutoScaleY->setVisible(true);
  }

  if (Plots[0]->getPlotType() == PlotType::HISTOGRAM) {
    ui->lblBinSizeText->setVisible(true);
    ui->lblBinSize->setVisible(true);
  }
}

void MainWindow::startKafkaConsumerThread() {
  qRegisterMetaType<int>("int&");
  connect(mWorker, &WorkerThread::resultReady, this,
          &MainWindow::handleKafkaData);
}

// SLOT
void MainWindow::handleKafkaData(int ElapsedCountMS) {
  auto &Consumer = mWorker->getConsumer();

  uint64_t EventRate = Consumer.getEventCount() * 1000ULL / ElapsedCountMS;
  uint64_t EventAccept = Consumer.getEventAccept() * 1000ULL / ElapsedCountMS;
  uint64_t EventDiscardRate = Consumer.getEventDiscard() * 1000ULL / ElapsedCountMS;

  ui->lblEventRateText->setText(QString::number(EventRate));
  ui->lblAcceptRateText->setText(QString::number(EventAccept));
  ui->lblDiscardedPixelsText->setText(QString::number(EventDiscardRate));
  ui->lblBinSizeText->setText(QString::number(mConfig.mTOF.BinSize) + " " + QString::number(mCount));

  for (auto &Plot : Plots) {
    Plot->updateData();
  }
  Consumer.gotEventRequest();


  mCount += 1;
}

// SLOT
void MainWindow::handleExitButton() { QApplication::quit(); }

void MainWindow::handleClearButton() {

  for (auto &Plot : Plots) {
    Plot->clearDetectorImage();
  }
}

void MainWindow::updateGradientLabel() {

  for (auto &Plot : Plots) {
    if (Plot->getPlotType() == PlotType::PIXELS) {
      if (mConfig.mPlot.InvertGradient) {
        ui->lblGradientText->setText(
            QString::fromStdString(mConfig.mPlot.ColorGradient + " (I)"));
      } else {
        ui->lblGradientText->setText(
            QString::fromStdString(mConfig.mPlot.ColorGradient));
      }
    }
  }
}

/// \brief Autoscale is only relevant for TOF
void MainWindow::updateAutoScaleLabels() {
  if (Plots[0]->getPlotType() ==  PlotType::TOF || Plots[0]->getPlotType() ==  PlotType::HISTOGRAM) {
    if (mConfig.mTOF.AutoScaleX) {
      ui->lblAutoScaleXText->setText(QString::fromStdString("on"));
    } else {
      ui->lblAutoScaleXText->setText(QString::fromStdString("off"));
    }

    if (mConfig.mTOF.AutoScaleY) {
      ui->lblAutoScaleYText->setText(QString::fromStdString("on"));
    } else {
      ui->lblAutoScaleYText->setText(QString::fromStdString("off"));
    }
  }
}

// toggle the log scale flag
void MainWindow::handleLogButton() {
  mConfig.mPlot.LogScale = not mConfig.mPlot.LogScale;
}

// toggle the invert gradient flag (irrelevant for TOF)
void MainWindow::handleInvertButton() {
  if (Plots[0]->getPlotType() ==  PlotType::PIXELS || Plots[0]->getPlotType() ==  PlotType::TOF2D) {
    mConfig.mPlot.InvertGradient = not mConfig.mPlot.InvertGradient;
    updateGradientLabel();
  }
}

// toggle the auto scale x button
void MainWindow::handleAutoScaleXButton() {
  if (Plots[0]->getPlotType() ==  PlotType::TOF || Plots[0]->getPlotType() ==  PlotType::HISTOGRAM) {
    mConfig.mTOF.AutoScaleX = not mConfig.mTOF.AutoScaleX;
    updateAutoScaleLabels();
  }
}

// toggle the auto scale y button
void MainWindow::handleAutoScaleYButton() {
  if (Plots[0]->getPlotType() ==  PlotType::TOF || Plots[0]->getPlotType() ==  PlotType::HISTOGRAM) {
    mConfig.mTOF.AutoScaleY = not mConfig.mTOF.AutoScaleY;
    updateAutoScaleLabels();
  }
}

void MainWindow::handleGradientButton() {

  for (auto &Plot : Plots) {
    if (Plot->getPlotType() ==  PlotType::PIXELS) {

      Custom2DPlot *Plot2D = dynamic_cast<Custom2DPlot *>(Plot.get());

      /// \todo unnecessary code here could be part of the object since it has
      /// the config at construction
      mConfig.mPlot.ColorGradient =
          Plot2D->getNextColorGradient(mConfig.mPlot.ColorGradient);

    } else if (Plot->getPlotType() != PlotType::TOF2D) {

      CustomAMOR2DTOFPlot *PlotTOF2D =
          dynamic_cast<CustomAMOR2DTOFPlot *>(Plot.get());

      mConfig.mPlot.ColorGradient =
          PlotTOF2D->getNextColorGradient(mConfig.mPlot.ColorGradient);

    } else {
      return;
    }

    Plot->plotDetectorImage(true);
  }
  updateGradientLabel();
}
