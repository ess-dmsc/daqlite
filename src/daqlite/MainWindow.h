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
#include <QTextEdit>

#include <stddef.h>
#include <memory>
#include <vector>

// Forward declarations
class AbstractPlot;
class HelpWindow;
class QLineEdit;
class QObject;
class QToolButton;
class QWidget;
class WorkerThread;

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  /// \brief Constructor
  ///
  /// \param Config  All plot and Kafka configuration options
  /// \param Worker  Common worker thread shared by a all plot windows
  /// \param parent  Parent widget
  MainWindow(const Configuration &Config, WorkerThread *Worker, QWidget *parent = nullptr);

  /// \brief Destructor
  ~MainWindow();

  /// \brief create the plot widgets
  void setupPlots();

  /// \brief spin up a thread for consuming topic
  void startKafkaConsumerThread();

  /// \brief initialize gradient combo box
  void initGradientComboBox();

  /// \brief update gradient combo box
  void updateGradientComboBox();

  /// \brief Generate a gradient icon for the gradient combo box
  /// \param Key  The name of the gradient (see Gradients.h for the list)
  /// \return the generated icon
  QIcon makeIcon(std::string Key);

  /// \brief Detect if the plot window is closed
  /// \param event Close event
  void closeEvent(QCloseEvent *event) override;

public slots:
  void handleExitButton();
  void handleClearButton();
  void handleLogButton();
  void handleInvertButton();
  void handleAutoScaleXButton();
  void handleAutoScaleYButton();
  void handleKafkaData(int ElapsedCountNS);
  void handleGradientComboBox(int index);

  /// Display the help window
  void showHelp();

private:
  Ui::MainWindow *ui;

  std::vector<std::unique_ptr<AbstractPlot>> Plots;

  /// \brief Configuration obtained from ctor
  Configuration mConfig;

  /// \brief
  static HelpWindow *Helper;

  // Pointer to worker thread
  WorkerThread *mWorker;

  /// \brief Number of updates data deliveries so far
  size_t mCount;

  /// \brief The size of the gradient icons
  QSize mGradientIconSize;

  /// \brief List of gradient names
  std::vector<std::string> mGradients;
};


