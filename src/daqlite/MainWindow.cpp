// Copyright (C) 2020 - 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file MainWindow.cpp
///
//===----------------------------------------------------------------------===//

#include "AbstractPlot.h"
#include "CustomAMOR2DTOFPlot.h"
#include "CustomTofPlot.h"
#include "ui_MainWindow.h"
#include <MainWindow.h>
#include <memory>
#include <string.h>

void MainWindow::setupPlots() {
  if (strcmp(mConfig.Plot.PlotType.c_str(), "tof2d") == 0) {
    Plots.push_back(std::make_unique<CustomAMOR2DTOFPlot>(mConfig));

    // register plot on ui
    registerPlot(Plots.back().get());

  } else if (strcmp(mConfig.Plot.PlotType.c_str(), "tof") == 0) {
    Plots.push_back(std::make_unique<CustomTofPlot>(mConfig));

    // register plot on ui
    registerPlot(Plots.back().get());

    // Hide irrelevant buttons for TOF
    ui->pushButtonGradient->setVisible(false);
    ui->pushButtonInvert->setVisible(false);
    ui->lblGradientText->setVisible(false);
    ui->lblGradient->setVisible(false);

  } else if (strcmp(mConfig.Plot.PlotType.c_str(), "pixels") == 0) {

    // Always create the XY plot
    Plots.push_back(
        std::make_unique<Custom2DPlot>(mConfig, Custom2DPlot::ProjectionXY));

    registerPlot(Plots.back().get());

    // If detector is 3D, also create XZ and YZ
    if (mConfig.Geometry.ZDim > 1) {
      Plots.push_back(
          std::make_unique<Custom2DPlot>(mConfig, Custom2DPlot::ProjectionXZ));
      registerPlot(Plots.back().get());
      Plots.push_back(
          std::make_unique<Custom2DPlot>(mConfig, Custom2DPlot::ProjectionYZ));
      registerPlot(Plots.back().get());
    }
  } else {
    throw(std::runtime_error("No valid plot type specified"));
  }

  // Autoscale buttons are only relevant for TOF
  if (Plots[0]->getPlotType() == TOF) {
    ui->pushButtonAutoScaleX->setVisible(false);
    ui->lblAutoScaleXText->setVisible(false);
    ui->lblAutoScaleX->setVisible(false);
    ui->pushButtonAutoScaleY->setVisible(false);
    ui->lblAutoScaleYText->setVisible(false);
    ui->lblAutoScaleY->setVisible(false);
  }
}

void MainWindow::registerPlot(AbstractPlot *Plot) const {
  if (Plot != nullptr) {
    ui->gridLayout->addWidget(Plot, 0, 0, 1, 1);
  }
}

MainWindow::MainWindow(Configuration &Config, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), mConfig(Config) {

  ui->setupUi(this);

  setupPlots();

  ui->lblDescriptionText->setText(mConfig.Plot.PlotTitle.c_str());
  ui->lblEventRateText->setText("0");

  connect(ui->pushButtonQuit, SIGNAL(clicked()), this,
          SLOT(handleExitButton()));
  connect(ui->pushButtonClear, SIGNAL(clicked()), this,
          SLOT(handleClearButton()));
  connect(ui->pushButtonLog, SIGNAL(clicked()), this, SLOT(handleLogButton()));
  connect(ui->pushButtonGradient, SIGNAL(clicked()), this,
          SLOT(handleGradientButton()));
  connect(ui->pushButtonInvert, SIGNAL(clicked()), this,
          SLOT(handleInvertButton()));
  connect(ui->pushButtonAutoScaleX, SIGNAL(clicked()), this,
          SLOT(handleAutoScaleXButton()));
  connect(ui->pushButtonAutoScaleY, SIGNAL(clicked()), this,
          SLOT(handleAutoScaleYButton()));

  updateGradientLabel();
  updateAutoScaleLabels();
  show();
  startKafkaConsumerThread();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::startKafkaConsumerThread() {
  KafkaConsumerThread = new WorkerThread(mConfig);
  qRegisterMetaType<int>("int&");
  connect(KafkaConsumerThread, &WorkerThread::resultReady, this,
          &MainWindow::handleKafkaData);
  KafkaConsumerThread->start();
}

// SLOT
void MainWindow::handleKafkaData(int ElapsedCountMS) {
  auto Consumer = KafkaConsumerThread->consumer();

  uint64_t EventRate = Consumer->EventCount * 1000ULL / ElapsedCountMS;
  uint64_t EventAccept = Consumer->EventAccept * 1000ULL / ElapsedCountMS;
  uint64_t EventDiscardRate = Consumer->EventDiscard * 1000ULL / ElapsedCountMS;

  ui->lblEventRateText->setText(QString::number(EventRate));
  ui->lblAcceptRateText->setText(QString::number(EventAccept));
  ui->lblDiscardedPixelsText->setText(QString::number(EventDiscardRate));


  for (auto &Plot : Plots) {
    Plot->updateData();
    Plot->plotDetectorImage(false);
  }

  Consumer->EventCount = 0;
  Consumer->EventAccept = 0;
  Consumer->EventDiscard = 0;
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
    if (Plot->getPlotType() != TOF) {
      if (mConfig.Plot.InvertGradient) {
        ui->lblGradientText->setText(
            QString::fromStdString(mConfig.Plot.ColorGradient + " (I)"));
      } else {
        ui->lblGradientText->setText(
            QString::fromStdString(mConfig.Plot.ColorGradient));
      }
    }
  }
}

/// \brief Autoscale is only relevant for TOF
void MainWindow::updateAutoScaleLabels() {
  if (Plots[0]->getPlotType() != TOF) {
    if (mConfig.TOF.AutoScaleX) {
      ui->lblAutoScaleXText->setText(QString::fromStdString("on"));
    } else {
      ui->lblAutoScaleXText->setText(QString::fromStdString("off"));
    }

    if (mConfig.TOF.AutoScaleY) {
      ui->lblAutoScaleYText->setText(QString::fromStdString("on"));
    } else {
      ui->lblAutoScaleYText->setText(QString::fromStdString("off"));
    }
  }
}

// toggle the log scale flag
void MainWindow::handleLogButton() {
  mConfig.Plot.LogScale = not mConfig.Plot.LogScale;
}

// toggle the invert gradient flag (irrelevant for TOF)
void MainWindow::handleInvertButton() {
  if (Plots[0]->getPlotType() != TOF) {
    mConfig.Plot.InvertGradient = not mConfig.Plot.InvertGradient;
    updateGradientLabel();
  }
}

// toggle the auto scale x button
void MainWindow::handleAutoScaleXButton() {
  if (Plots[0]->getPlotType() == TOF) {
    mConfig.TOF.AutoScaleX = not mConfig.TOF.AutoScaleX;
    updateAutoScaleLabels();
  }
}

// toggle the auto scale y button
void MainWindow::handleAutoScaleYButton() {
  if (Plots[0]->getPlotType() == TOF) {
    mConfig.TOF.AutoScaleY = not mConfig.TOF.AutoScaleY;
    updateAutoScaleLabels();
  }
}

void MainWindow::handleGradientButton() {

  for (auto &Plot : Plots) {
    if (Plot->getPlotType() == PIXEL) {

      Custom2DPlot *Plot2D = dynamic_cast<Custom2DPlot *>(Plot.get());

      /// \todo unnecessary code here could be part of the object since it has
      /// the config at construction
      mConfig.Plot.ColorGradient =
          Plot2D->getNextColorGradient(mConfig.Plot.ColorGradient);

    } else if (Plot->getPlotType() != TOF2D) {

      CustomAMOR2DTOFPlot *PlotTOF2D =
          dynamic_cast<CustomAMOR2DTOFPlot *>(Plot.get());

      mConfig.Plot.ColorGradient =
          PlotTOF2D->getNextColorGradient(mConfig.Plot.ColorGradient);

    } else {
      return;
    }

    Plot->plotDetectorImage(true);
  }
  updateGradientLabel();
}
