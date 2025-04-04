// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
//
/// \file
//
/// \brief GUI interface for saving stored data to HDF5 files
//===----------------------------------------------------------------------===//
#include "AppWindow.h"
#include "./ui_AppWindow.h"

void MainWindow::setup_data(){
  connect(ui->actionSaveHDF5, &QAction::triggered, this, &MainWindow::save_data);
}

void MainWindow::save_data() {
  using namespace std::filesystem;
  auto q_filename = QFileDialog::getSaveFileName(this, tr("Save Data"), "", tr("HDF5 files (*.h5 *.H5 *.hdf5 *.HDF5)"));
  auto p = path(q_filename.toStdString());
  if (!p.has_filename()) return;
  auto ext = std::string(p.extension());
  std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){return std::tolower(c);});
  if (ext != ".h5" || ext != ".hdf5"){
    p.replace_extension(path(".h5"));
  }
  try{
    data->save_to(p);
  } catch (const std::runtime_error & ex){
    QMessageBox msgBox;
    auto txt = fmt::format("Saving data to {} failed", std::string(p));
    msgBox.setText(txt.c_str());
    msgBox.setDetailedText(ex.what());
    msgBox.exec();
  }
}