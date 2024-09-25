// Copyright (C) 2020 - 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file MainWindow.h
///
/// Main (and only) window for daqlite
//===----------------------------------------------------------------------===//

#pragma once

#include <AbstractPlot.h>
#include <Configuration.h>
#include <QMainWindow>
#include <WorkerThread.h>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

  // typedef std::variant<std::unique_ptr<Custom2DPlot>,
  //                      std::unique_ptr<CustomAMOR2DTOFPlot>,
  //                      std::unique_ptr<CustomTofPlot>>
  //     PlotVariants;

public:
  MainWindow(Configuration &Config, QWidget *parent = nullptr);
  ~MainWindow();

  /// \brief create the plot widgets
  void setupPlots();

  /// \brief spin up a thread for consuming topic
  void startKafkaConsumerThread();

  /// \brief update GUI label text
  void updateGradientLabel();

  /// \brief update GUI label text
  void updateAutoScaleLabels();

public slots:
  void handleExitButton();
  void handleClearButton();
  void handleLogButton();
  void handleGradientButton();
  void handleInvertButton();
  void handleAutoScaleXButton();
  void handleAutoScaleYButton();
  void handleKafkaData(int ElapsedCountNS);

private:
  Ui::MainWindow *ui;

  std::vector<std::unique_ptr<AbstractPlot>> Plots;

  // Q3DScatter scatter;

  /// \brief configuration obtained from main()
  Configuration &mConfig;

  /// \brief
  std::unique_ptr<WorkerThread> KafkaConsumerThread;
};
