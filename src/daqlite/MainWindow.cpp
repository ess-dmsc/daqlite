// Copyright (C) 2020 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file MainWindow.cpp
///
//===----------------------------------------------------------------------===//

#include <MainWindow.h>
#include <ui_MainWindow.h>

#include <AMOR2DTofPlot.h>
#include <HelpWindow.h>
#include <HistogramPlot.h>
#include <PixelsPlot.h>
#include <TofPlot.h>
#include <WorkerThread.h>

#include <types/Gradients.h>

#include <fmt/core.h>

#include <QApplication>
#include <QTextEdit>
#include <QMetaType>
#include <QPushButton>
#include <QPixmap>
#include <QImage>
#include <QToolButton>

#include <stdint.h>
#include <string.h>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>

class QWidget;

// Initialize helper to nullptr
HelpWindow *MainWindow::Helper = nullptr;

MainWindow::MainWindow(const Configuration &Config, WorkerThread *Worker, QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , mConfig(Config)
  , mWorker(Worker)
  , mCount(0)
  , mGradientIconSize(QSize(128, 24)) {
  ui->setupUi(this);
  setupPlots();

  ui->lblDescriptionText->setText(mConfig.mPlot.PlotTitle.c_str());
  ui->lblEventRateText->setText("0");

  // Connect all windows buttons
  auto signal = &QPushButton::clicked;
  connect(ui->pushButtonQuit,     signal, this, &MainWindow::handleExitButton);
  connect(ui->pushButtonClear,    signal, this, &MainWindow::handleClearButton);
  connect(ui->checkBoxLog,        signal, this, &MainWindow::handleLogButton);
  connect(ui->checkBoxInvert,     signal, this, &MainWindow::handleInvertButton);
  connect(ui->checkBoxAutoScaleX, signal, this, &MainWindow::handleAutoScaleXButton);
  connect(ui->checkBoxAutoScaleY, signal, this, &MainWindow::handleAutoScaleYButton);
  connect(ui->checkBoxAutoScaleY, signal, this, &MainWindow::handleAutoScaleYButton);
  connect(ui->helpButton,         signal, this, &MainWindow::showHelp);

  ui->checkBoxLog->setCheckState(mConfig.mPlot.LogScale ? Qt::Checked : Qt::Unchecked);
  ui->checkBoxInvert->setCheckState(mConfig.mPlot.InvertGradient ? Qt::Checked : Qt::Unchecked);
  ui->checkBoxAutoScaleX->setCheckState(mConfig.mTOF.AutoScaleX ? Qt::Checked : Qt::Unchecked);
  ui->checkBoxAutoScaleY->setCheckState(mConfig.mTOF.AutoScaleY ? Qt::Checked : Qt::Unchecked);

  initGradientComboBox();

  // ---------------------------------------------------------------------------
  // If window sizes have not been explicitly specified, we resize and fit plot
  // to the size of the current screen
  //
  // - Pixel and Tof2D plots are square
  // - Other plots are long and narrow
  adjustSize();
  int &h = mConfig.mPlot.Height;
  int &w = mConfig.mPlot.Width;
  if (mConfig.mPlot.defaultGeometry) {
    // Adjust size and get minimum required size
    double size = std::max(minimumWidth(), minimumHeight());

    // Get screen geometry
    auto const geom = QApplication::primaryScreen()->geometry();

    // Resize square plots
    if (mConfig.mPlot.Plot == PlotType::PIXELS || mConfig.mPlot.Plot == PlotType::TOF2D) {
      size = std::max(size, 0.4 * geom.height());
      w = 1.1 * size;
      h = size;
    }

    // ... and the rest
    else {
      size = std::max(size, 0.4 * geom.width());
      w = size;
      h = 0.4 * size;
    }
  }
  resize(w, h);

  show();
  startKafkaConsumerThread();
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::setupPlots() {
  PlotType Type(mConfig.mPlot.Plot);

  if (Type == PlotType::TOF2D) {
    Plots.push_back(std::make_unique<AMOR2DTofPlot>(
        mConfig, mWorker->getConsumer()));

    // register plot on ui
    ui->gridLayout->addWidget(Plots.back().get(), 0, 0, 1, 1);

  }

  else if (Type == PlotType::TOF) {
    Plots.push_back(std::make_unique<TofPlot>(
        mConfig, mWorker->getConsumer()));

    // register plot on ui
    ui->gridLayout->addWidget(Plots.back().get(), 0, 0, 1, 1);

    // Hide irrelevant buttons for TOF
    ui->comboGradient->setVisible(false);
    ui->checkBoxInvert->setVisible(false);
    ui->gradientLine->setVisible(false);
  }

  else if (Type == PlotType::HISTOGRAM) {
    Plots.push_back(std::make_unique<HistogramPlot>(
        mConfig, mWorker->getConsumer()));

    ui->gridLayout->addWidget(Plots.back().get(), 0, 0, 1, 1);

    // Hide irrelevant buttons for TOF
    ui->comboGradient->setVisible(false);
    ui->checkBoxInvert->setVisible(false);
    ui->gradientLine->setVisible(false);
  }

  else if (Type == PlotType::PIXELS) {

    // Always create the XY plot
    Plots.push_back(std::make_unique<PixelsPlot>(
        mConfig, mWorker->getConsumer(),
        PixelsPlot::ProjectionXY));
    ui->gridLayout->addWidget(Plots.back().get(), 0, 0, 1, 1);

    // If detector is 3D, also create XZ and YZ
    if (mConfig.mGeometry.ZDim > 1) {
      Plots.push_back(std::make_unique<PixelsPlot>(
          mConfig, mWorker->getConsumer(),
          PixelsPlot::ProjectionXZ));
      ui->gridLayout->addWidget(Plots.back().get(), 0, 1, 1, 1);
      Plots.push_back(std::make_unique<PixelsPlot>(
          mConfig, mWorker->getConsumer(),
          PixelsPlot::ProjectionYZ));
      ui->gridLayout->addWidget(Plots.back().get(), 0, 2, 1, 1);
    }
  }

  else {
    throw(std::runtime_error("No valid plot type specified"));
  }

  // Autoscale buttons are only relevant for TOF and HISTOGRAM
  const auto PlotType = Plots[0]->getPlotType();
  const bool ScaleOn = PlotType == PlotType::TOF || PlotType == PlotType::HISTOGRAM;
  ui->labelAutoScale->setVisible(ScaleOn);
  ui->checkBoxAutoScaleX->setVisible(ScaleOn);
  ui->checkBoxAutoScaleY->setVisible(ScaleOn);

  ui->lblBinSizeText->setVisible(PlotType == PlotType::HISTOGRAM);
  ui->lblBinSize->setVisible(PlotType == PlotType::HISTOGRAM);
}

void MainWindow::startKafkaConsumerThread() {
  qRegisterMetaType<int>("int&");
  connect(mWorker, &WorkerThread::resultReady, this,
          &MainWindow::handleKafkaData);
}

void MainWindow::handleKafkaData(int ElapsedCountMS) {
  auto &Consumer = mWorker->getConsumer();

  uint64_t EventRate = Consumer.getEventCount() * 1000ULL / ElapsedCountMS;
  uint64_t EventAccept = Consumer.getEventAccept() * 1000ULL / ElapsedCountMS;
  uint64_t EventDiscardRate = Consumer.getEventDiscard() * 1000ULL / ElapsedCountMS;

  ui->lblEventRateText->setText(QString::number(EventRate));
  ui->lblAcceptRateText->setText(QString::number(EventAccept));
  ui->lblDiscardedPixelsText->setText(QString::number(EventDiscardRate));
  ui->lblBinSizeText->setText(QString("%1 %2").arg(mConfig.mTOF.BinSize).arg(mCount));

  for (auto &Plot : Plots) {
    Plot->updateData();
  }
  Consumer.gotEventRequest();

  mCount += 1;
}

void MainWindow::handleExitButton() {
  QApplication::quit();
}

void MainWindow::handleClearButton() {
  for (auto &Plot : Plots) {
    Plot->clearDetectorImage();
  }
}

void MainWindow::initGradientComboBox() {
  mGradients.clear();
  ui->comboGradient->setIconSize(mGradientIconSize);
  ui->comboGradient->clear();

  // Initialize vars
  int currentIndex = -1;

  // Loop through all gradient and add them to the combo
  for (auto &[name, gradient]: GRADIENTS) {
    // Check and store gradient name
    if (name == mConfig.mPlot.ColorGradient) {
      currentIndex = mGradients.size();
    }
    mGradients.push_back(name);

    // Generate and add gradient icon for the combo box
    const QIcon icon = makeIcon(name);
    ui->comboGradient->addItem(icon, "");
  }

  // Finalize
  ui->pushButtonClear->adjustSize();
  ui->comboGradient->setFixedHeight(ui->pushButtonClear->height());
  ui->comboGradient->setCurrentIndex(currentIndex);
  ui->comboGradient->setToolTip(QString::fromStdString(mGradients[currentIndex]));
  connect(ui->comboGradient, &QComboBox::currentIndexChanged, this, &MainWindow::handleGradientComboBox);
}

void MainWindow::updateGradientComboBox() {
  for (size_t index=0; index < mGradients.size(); ++index) {
    const QIcon icon = makeIcon(mGradients[index]);
    ui->comboGradient->setItemIcon(index, icon);
  }
}

// toggle the log scale flag
void MainWindow::handleLogButton() {
  mConfig.mPlot.LogScale = not mConfig.mPlot.LogScale;
}

// toggle the invert gradient flag (irrelevant for TOF)
void MainWindow::handleInvertButton() {
  const auto PlotType = Plots[0]->getPlotType();
  if (PlotType == PlotType::PIXELS || PlotType == PlotType::TOF2D) {
    mConfig.mPlot.InvertGradient = not mConfig.mPlot.InvertGradient;
    updateGradientComboBox();
  }
}

// toggle the auto scale x button
void MainWindow::handleAutoScaleXButton() {
  const auto PlotType = Plots[0]->getPlotType();
  if (PlotType == PlotType::TOF || PlotType == PlotType::HISTOGRAM) {
    mConfig.mTOF.AutoScaleX = not mConfig.mTOF.AutoScaleX;
  }
}

// toggle the auto scale y button
void MainWindow::handleAutoScaleYButton() {
  const auto PlotType = Plots[0]->getPlotType();
  if (PlotType == PlotType::TOF || PlotType == PlotType::HISTOGRAM) {
    mConfig.mTOF.AutoScaleY = not mConfig.mTOF.AutoScaleY;
  }
}

void MainWindow::handleGradientComboBox(int index) {
  for (auto &Plot : Plots) {
    const auto PlotType = Plot->getPlotType();
    if (PlotType == PlotType::PIXELS || PlotType == PlotType::TOF2D) {
      mConfig.mPlot.ColorGradient = mGradients[index];
    } else {
      return;
    }
    ui->comboGradient->setToolTip(QString::fromStdString(mGradients[index]));

    Plot->plotDetectorImage(true);
  }
}

QIcon MainWindow::makeIcon(std::string key) {
  const size_t width = mGradientIconSize.width();
  const auto range = QCPRange(0, width - 1);
  QImage image(width, 1, QImage::Format_RGB32);
  for (size_t i=0; i<width; ++i) {
    const QColor color(GRADIENTS[key].color(mConfig.mPlot.InvertGradient ? width - i : i, range));
    image.setPixelColor(i, 0, color);
  }
  image = image.scaled(mGradientIconSize);

  return QIcon(QPixmap::fromImage(image));
}

void MainWindow::closeEvent(QCloseEvent *) {
  // If a windows is closed, we inform consumer to cancel the subscription
  // for the plot
  for (const auto &Plot: Plots) {
    mWorker->getConsumer().addSubscriber(Plot->getPlotType(), false);
  }

  // Close daqlite, if no subscribers are left
  if (mWorker->getConsumer().subscriptionCount() == 0) {
    QApplication::quit();
  }
}

void MainWindow::showHelp() {
  if (Helper == nullptr) {
    Helper = new HelpWindow(this);
    Helper->setParent(this, Qt::Window);
    Helper->setWindowFlags( Helper->windowFlags() | Qt::FramelessWindowHint);
  }

  if (Helper->isHidden()) {
    Helper->show();
  }

  Helper->move(QCursor::pos());
  Helper->placeHelp(QCursor::pos());
  Helper->raise();
}
