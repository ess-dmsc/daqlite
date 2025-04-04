// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
//
/// \file
//
/// \brief GUI table interface for Calibration information for the BIFROST detector
//===----------------------------------------------------------------------===//
#include <QTableWidget>
#include <QStringList>
#include <QLineEdit>

#include "AppWindow.h"
#include "./ui_AppWindow.h"

void MainWindow::setup_calibration(){
  setup_calibration_info();
  setup_calibration_table();
  connect(ui->actionSave_Calibration, &QAction::triggered, this, &MainWindow::save_calibration);
  connect(ui->actionLoad_Calibration, &QAction::triggered, this, &MainWindow::load_calibration);
}

void MainWindow::setup_calibration_info(){
  auto layout = ui->calibrationInfoLayout->layout();
  QString message{calibration.info().c_str()};
  ui->calibrationInfo->setToolTip("Info");
  layout->addWidget(ui->calibrationInfo);
  connect(ui->calibrationInfo, &QLineEdit::textChanged, this, [&](const QString & text){
    calibration.setInfo(text.toStdString());
  });

  std::string date_time_format{"yyyy.MM.ddThh:mm:ss"};
  auto c_date = calibration.date();
  auto then = QDateTime::fromSecsSinceEpoch(c_date);
  ui->calibrationTime->setDisplayFormat(date_time_format.c_str());
  ui->calibrationTime->setDateTime(then);
  ui->calibrationTime->setReadOnly(true);
}

void MainWindow::update_calibration_info(){
  ui->calibrationInfo->setText(calibration.info().c_str());
  ui->calibrationTime->setDateTime(QDateTime::fromSecsSinceEpoch(calibration.date()));
}

void MainWindow::setup_calibration_table(){
  auto layout = ui->calibrationVerticalLayout->layout();

  std::vector<float> arcs{2.7, 3.2, 3.8, 4.4, 5.0};

  auto rows = configuration.Instrument.groups * configuration.Instrument.units_per_group;
  calibration_table_columns = {
      {"arc/meV", [arcs](int arc, int, int, CalibrationUnit *){
        return new FloatTableItem(arcs[arc], false);
      }},
      {"triplet", [](int, int triplet, int, CalibrationUnit *){
        return new IntTableItem(triplet, false); //Item(tr("%1").arg(triplet));
      }},
      {"tube", [](int, int, int tube, CalibrationUnit *){
        return new IntTableItem(tube, false); // Item(tr("%1").arg(tube));
      }},
      {"x left", [](int, int, int, CalibrationUnit * unit){
        return new CalibrationUnitLeftItem(unit);
      }},
      {"x right", [](int, int, int, CalibrationUnit * unit){
        return new CalibrationUnitRightItem(unit); // Item(tr("%1").arg(unit.right));
      }},
      {"c0", [](int, int, int, CalibrationUnit * unit){
        return new CalibrationUnitC0Item(unit);
      }},
      {"c1", [](int, int, int, CalibrationUnit * unit){
        return new CalibrationUnitC1Item(unit);
      }},
      {"c2", [](int, int, int, CalibrationUnit * unit){
        return new CalibrationUnitC2Item(unit);
      }},
      {"c3", [](int, int, int, CalibrationUnit * unit){
        return new CalibrationUnitC3Item(unit);
      }},
  };
  QStringList headers;
  for (const auto & [label, _]: calibration_table_columns){
    headers << label.c_str();
  }
  auto new_calibration_table = new QTableWidget(rows, headers.size(), this);
  if (calibration_table) {
    // TODO verify this doesn't cause a segfault
    layout->replaceWidget(calibration_table, new_calibration_table);
    delete calibration_table;
    calibration_table = new_calibration_table;
  } else {
    calibration_table = new_calibration_table;
    layout->addWidget(calibration_table);
  }
  calibration_table->setHorizontalHeaderLabels(headers);

  setup_calibration_table_items();
}

void MainWindow::setup_calibration_table_items(){
  auto rows = calibration_table->rowCount();
  auto columns = calibration_table->columnCount();
  calibration_table_items.reserve(rows * columns);
  // It's not clear if this item management would be handled by setItem internally:
  for (auto & item: calibration_table_items) delete item;
  calibration_table_items.clear();
  for (int group=0; group<configuration.Instrument.groups; ++group){
    for (int unit=0; unit<configuration.Instrument.units_per_group; ++unit){
      auto arc = group / 9;
      auto triplet = group % 9;
      auto row = group * configuration.Instrument.units_per_group + unit;
      for (int column=0; column < columns; ++column){
        // this item handles updating the editable unit fields without a separate callback
        auto item = calibration_table_columns[column].second(arc, triplet, unit, calibration.unitPointer(group, unit));
        calibration_table->setItem(row, column, item);
        calibration_table_items.push_back(item);
      }
    }
  }
}

void MainWindow::save_calibration() {
  using namespace std::filesystem;
  auto q_filename = QFileDialog::getSaveFileName(this, tr("Save Configuration"), "", tr("JSON files (*.json *.JSON)"));
  auto p = path(q_filename.toStdString());
  if (!p.has_filename()) return;
  auto ext = std::string(p.extension());
  std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){return std::tolower(c);});
  if (ext != ".json"){
    p.replace_extension(path(".json"));
  }
  calibration.setDate();
  nlohmann::json j = calibration;
  to_json_file(j, std::string(p));
}

void MainWindow::load_calibration() {
  using namespace std::filesystem;
  auto title = tr("Load Configuration");
  auto files = tr("JSON files (*.json *.JSON)");
  auto p = path(QFileDialog::getOpenFileName(this, title, "", files).toStdString());
  if (!p.has_filename()) return;
  nlohmann::json j;
  try {
    j = from_json_file(std::string(p));
    calibration = j;
    setup_calibration_table_items();
    update_calibration_info();
  } catch (const std::runtime_error & ex) {
    QMessageBox msgBox;
    auto txt = fmt::format("Loading calibration from {} failed", std::string(p));
    msgBox.setText(txt.c_str());
    msgBox.setDetailedText(ex.what());
    msgBox.exec();
  }

}