// Copyright (C) 2020 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file MainWindow.h
///
/// Main (and only) window for daqlite
//===----------------------------------------------------------------------===//

#pragma once

#include <Configuration.h>
#include <QMainWindow>

#include <stddef.h>
#include <memory>
#include <vector>

// Forward declarations
class AbstractPlot;
class QObject;
class QWidget;
class WorkerThread;

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(const Configuration &Config, WorkerThread *Worker, QWidget *parent = nullptr);
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

  /// \brief Configuration obtained from ctor
  Configuration mConfig;

  // Pointer to worker thread
  WorkerThread *mWorker;

  /// \brief Number of updates data deliveries so far
  size_t mCount;
};
