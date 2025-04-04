// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
//
/// \file
//
/// \brief GUI main window for the Fylgje application
//===----------------------------------------------------------------------===//
#include <iostream>
#include <sstream>
#include <fmt/core.h>
#include "AppWindow.h"
#include "./ui_AppWindow.h"

MainWindow::MainWindow(const Configuration & Config, const Calibration & calibration, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), configuration(Config), calibration(calibration)
{
  ui->setupUi(this);
  initialize();

  maxBox = {ui->intMax00, ui->intMax01, ui->intMax02,
            ui->intMax10, ui->intMax11, ui->intMax12,
            ui->intMax20, ui->intMax21, ui->intMax22};

  connect(ui->arcRadio1, &QRadioButton::clicked, this, &MainWindow::set_arc_1);
  connect(ui->arcRadio2, &QRadioButton::clicked, this, &MainWindow::set_arc_2);
  connect(ui->arcRadio3, &QRadioButton::clicked, this, &MainWindow::set_arc_3);
  connect(ui->arcRadio4, &QRadioButton::clicked, this, &MainWindow::set_arc_4);
  connect(ui->arcRadio5, &QRadioButton::clicked, this, &MainWindow::set_arc_5);

  connect(ui->tripRadio1, &QRadioButton::clicked, this, &MainWindow::set_triplet_1);
  connect(ui->tripRadio2, &QRadioButton::clicked, this, &MainWindow::set_triplet_2);
  connect(ui->tripRadio3, &QRadioButton::clicked, this, &MainWindow::set_triplet_3);
  connect(ui->tripRadio4, &QRadioButton::clicked, this, &MainWindow::set_triplet_4);
  connect(ui->tripRadio5, &QRadioButton::clicked, this, &MainWindow::set_triplet_5);
  connect(ui->tripRadio6, &QRadioButton::clicked, this, &MainWindow::set_triplet_6);
  connect(ui->tripRadio7, &QRadioButton::clicked, this, &MainWindow::set_triplet_7);
  connect(ui->tripRadio8, &QRadioButton::clicked, this, &MainWindow::set_triplet_8);
  connect(ui->tripRadio9, &QRadioButton::clicked, this, &MainWindow::set_triplet_9);

  connect(ui->int_AB_Radio, &QRadioButton::clicked, this, &MainWindow::set_int_AB);
  connect(ui->int__t_Radio, &QRadioButton::clicked, this, &MainWindow::set_int_t);
  connect(ui->int_xP_Radio, &QRadioButton::clicked, this, &MainWindow::set_int_xP);
  connect(ui->int_xt_Radio, &QRadioButton::clicked, this, &MainWindow::set_int_xt);
  connect(ui->int__A_Radio, &QRadioButton::clicked, this, &MainWindow::set_int_a);
  connect(ui->int_Pt_Radio, &QRadioButton::clicked, this, &MainWindow::set_int_Pt);
  connect(ui->int__P_Radio, &QRadioButton::clicked, this, &MainWindow::set_int_p);
  connect(ui->int__x_Radio, &QRadioButton::clicked, this, &MainWindow::set_int_x);
  connect(ui->int__B_Radio, &QRadioButton::clicked, this, &MainWindow::set_int_b);

  connect(ui->timeLive, &QRadioButton::clicked, this, &MainWindow::set_time_live);
  connect(ui->timeHistorical, &QRadioButton::clicked, this, &MainWindow::set_time_historical);
  connect(ui->timeFixed, &QRadioButton::clicked, this, &MainWindow::set_time_fixed);

  _fixed_arc = 0;
  ui->arcRadio1->setChecked(true);
  _fixed_triplet = 0;
  ui->tripletBox->setChecked(false);
  ui->tripRadio1->setChecked(true);
  _fixed_type = int_t::ab;
  ui->intTypeBox->setChecked(true);
  ui->int_AB_Radio->setChecked(true);

  connect(ui->resetButton, &QPushButton::pressed, this, &MainWindow::reset);
  connect(ui->scaleButton, &QPushButton::pressed, this, &MainWindow::plot);
  connect(ui->autoscaleButton, &QPushButton::toggled, this, &MainWindow::autoscale_toggled);
  connect(ui->intTypeBox, &QGroupBox::toggled, this, &MainWindow::update_intensity_limits);
  connect(ui->tripletBox, &QGroupBox::toggled, this, &MainWindow::update_intensity_limits);
  connect(ui->pauseButton, &QPushButton::toggled, this, &MainWindow::pause_toggled);

  connect(ui->arcCycleCheck, &QCheckBox::clicked, this, &MainWindow::cycle);
  connect(ui->tripletCycleCheck, &QCheckBox::clicked, this, &MainWindow::cycle);
  connect(ui->typeCycleCheck, &QCheckBox::clicked, this, &MainWindow::cycle);

  connect(ui->filterShowAll, &QCheckBox::clicked, this, &MainWindow::set_filter_all);
  connect(ui->filterShowIncluded, &QCheckBox::clicked, this, &MainWindow::set_filter_included);
  connect(ui->filterShowExcluded, &QCheckBox::clicked, this, &MainWindow::set_filter_excluded);

  setup_add_bin_boxes();
  setup_time_limits();
  setup_intensity_limits();
  setup_gradient_list();
  setup_calibration();
  setup_data();
  setup();
}

void MainWindow::setup_add_bin_boxes() {
    auto layout = ui->binsFrame->layout();
    std::tuple<QString, int_t, void (MainWindow::*)(int), void (MainWindow::*)(int)> groups[]{
        {"A", int_t::a, &MainWindow::set_bins_a_1d, &MainWindow::set_bins_a_2d},
        {"B", int_t::b, &MainWindow::set_bins_b_1d, &MainWindow::set_bins_b_2d},
        {"x", int_t::x, &MainWindow::set_bins_x_1d, &MainWindow::set_bins_x_2d},
        {"p", int_t::p, &MainWindow::set_bins_p_1d, &MainWindow::set_bins_p_2d},
        {"t", int_t::t, &MainWindow::set_bins_t_1d, &MainWindow::set_bins_t_2d},
    };
    auto make_label = [](const QString & name){
        auto label = new QLabel();
        label->setText(name);
        label->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignVCenter);
        return label;
    };
    for (const auto& [name, t, call1, call2]: groups){
        layout->addWidget(make_label(name));
        auto k1 = std::make_pair(t, 1);
        auto k2 = std::make_pair(t, 2);

        bin_boxes[k1] = new TwoSpinBox(4, 1024);
        layout->addWidget(bin_boxes.at(k1));
        connect(bin_boxes.at(k1), QOverload<int>::of(&TwoSpinBox::valueChanged), this, call1);

        bin_boxes[k2] = new TwoSpinBox(4, 512);
        layout->addWidget(bin_boxes.at(k2));
        connect(bin_boxes.at(k2), QOverload<int>::of(&TwoSpinBox::valueChanged), this, call2);
    }
}

void MainWindow::setup_time_limits(){
  std::string date_time_format{"yyyy.MM.ddThh:mm:ss"};
  auto now = QDateTime::currentDateTimeUtc();
  for (auto & dt: {ui->timeBeginning, ui->timeEnding}){
    dt->setDisplayFormat(date_time_format.c_str());
    dt->setDateTime(now);
    dt->setMinimumDateTime(now.addDays(-28));
  }
  connect(ui->timeBeginning, &QDateTimeEdit::dateTimeChanged, this, &MainWindow::set_time_early);
  connect(ui->timeEnding, &QDateTimeEdit::dateTimeChanged, this, &MainWindow::set_time_late);
}


void MainWindow::set_time_live(){
  time_status = Time::Live;
  auto now = QDateTime::currentDateTimeUtc();
  for (auto & dt: {ui->timeBeginning, ui->timeEnding}){
    dt->setDateTime(now);
    dt->setEnabled(false);
  }
  consumer->Consumer->consumeForever();
  consumer->Consumer->consumeFrom(now.toMSecsSinceEpoch());
}
void MainWindow::set_time_historical(){
  time_status = Time::Historical;
  for (auto & dt: {ui->timeBeginning, ui->timeEnding}){
    dt->setEnabled(true);
  }
}
void MainWindow::set_time_fixed(){
  time_status = Time::Fixed;
  for (auto & dt: {ui->timeBeginning, ui->timeEnding}){
    dt->setEnabled(false);
  }
  consumer->Consumer->consumeUntil(ui->timeEnding->dateTime().toMSecsSinceEpoch());
  consumer->Consumer->consumeFrom(ui->timeBeginning->dateTime().toMSecsSinceEpoch());
}

void MainWindow::set_time_early(const QDateTime & time){
  if (time_status == Time::Historical){
    reset();
    consumer->consume_from(time.toMSecsSinceEpoch());
  }
}

void MainWindow::set_time_late(const QDateTime & time){
  if (time_status == Time::Historical){
    consumer->consume_until(time.toMSecsSinceEpoch());
  }
}


void MainWindow::setup_intensity_limits() {
  auto signal = QOverload<int>::of(&QSpinBox::valueChanged);
  auto slot = &MainWindow::get_intensity_limits;
//  for (auto sender: minBox) connect(sender, signal, this, slot);
  for (auto sender: maxBox) connect(sender, signal, this, slot);
}

void MainWindow::setup_gradient_list(){
  ui->colormapComboBox->clear();
  std::array<std::string,6> validNames {"gray", "hot", "cold", "night", "candy", "thermal"};
  for (const auto& name: validNames){
    ui->colormapComboBox->addItem(QString(name.c_str()));
  }
  // set the gradient name based on the configuration
  auto setting = configuration.Plot.ColorGradient;
  bool validName{false};
  for (const auto & name: validNames) if (name == setting){
    ui->colormapComboBox->setCurrentText(QString(setting.c_str()));
    validName = true;
    break;
  }
  if (!validName){
    std::cout << fmt::format("Configuration provided color map name {} is not known", setting);
  }
  // set inverted checkbox from the configuration
  ui->colormapInvertedCheck->clicked(configuration.Plot.InvertGradient);
  // set the scaling to log, according to the configuration
  ui->scaleButton->clicked(configuration.Plot.LogScale);
}

void MainWindow::initialize(){
  auto tubes = configuration.Instrument.units_per_group;
  auto pixelation = configuration.Instrument.pixels_per_unit;
  data = new ::bifrost::data::Manager(5, 9, tubes, pixelation, calibration);
  plots = new PlotManager(ui->plotGrid, 3, 3);
  max.resize(data->key_count());
  std::fill(max.begin(), max.end(), 0);
}

void MainWindow::setup(){
    setup_consumer();
//
    /// Update timer
    auto *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::timer_callback_window_update);
    timer->start(1000);
}

void MainWindow::timer_callback_window_update() {
  plot();
  if (time_status == Time::Live) ui->timeEnding->setDateTime(QDateTime::currentDateTimeUtc());
}

void MainWindow::setup_consumer(){
    delete consumer;
    consumer = new WorkerThread(data, configuration);
    // caengraph.WThread = consumer;
    consumer->start();
}

MainWindow::~MainWindow(){
  delete ui;
}

void MainWindow::cycle() {
  if (is_paused()) return;
  // entrypoint
  auto type_index = type_order_index(_fixed_type);
  using fylgje::Cycles;
  auto arc = ui->arcCycleCheck->isChecked();
  auto triplet = ui->tripletCycleCheck->isChecked();
  auto type = ui->typeCycleCheck->isChecked();
  auto index = 4 * type + 2 * triplet + arc;
  switch (index){
    case 1: cycle_one = Cycles<1>({5}, {_fixed_arc}); break;
    case 2: cycle_one = Cycles<1>({9}, {_fixed_triplet}); break;
    case 4: cycle_one = Cycles<1>({9}, {type_index}); break;
    case 3: cycle_two = Cycles<2>({5, 9}, {_fixed_arc, _fixed_triplet}); break;
    case 5: cycle_two = Cycles<2>({5, 9}, {_fixed_arc, type_index}); break;
    case 6: cycle_two = Cycles<2>({9, 9}, {_fixed_triplet, type_index}); break;
    case 7: cycle_three = Cycles<3>({5, 9, 9}, {_fixed_arc, _fixed_triplet, type_index}); break;
    default: break;
  }
  if (ui->arcCycleCheck->isChecked() || ui->tripletCycleCheck->isChecked() || ui->typeCycleCheck->isChecked()) {
    cycle_any();
  }
}

void MainWindow::cycle_any(){
  if (is_paused()) return;
  auto arc = ui->arcCycleCheck->isChecked();
  auto triplet = ui->tripletCycleCheck->isChecked();
  auto type = ui->typeCycleCheck->isChecked();
  auto index = 4 * type + 2 * triplet + arc;
  switch (index){
    case 1: cycle_arc(); break;
    case 2: cycle_triplet(); break;
    case 3: cycle_arc_triplet(); break;
    case 4: cycle_type(); break;
    case 5: cycle_arc_type(); break;
    case 6: cycle_triplet_type(); break;
    case 7: cycle_arc_triplet_type(); break;
    default: std::cout << "Error cycling with "
    << (arc ? "arc" : "")
    << (triplet ? "triplet" : "")
    << (index ? "index" : "")
    << ((index | triplet | arc) ? "" : "nothing")
    << " checked" << std::endl;
  }
  set_arc_radio(_fixed_arc);
  set_triplet_radio(_fixed_triplet);
  set_type_radio(_fixed_type);
  if (ui->arcCycleCheck->isChecked() || ui->tripletCycleCheck->isChecked() || ui->typeCycleCheck->isChecked()){
    QTimer::singleShot(static_cast<int>(ui->arcPeriodSpin->value() * 1000), this, &MainWindow::cycle_any);
  }
}

void MainWindow::set_arc_radio(int arc){
  QRadioButton* radios[]{ui->arcRadio1, ui->arcRadio2, ui->arcRadio3, ui->arcRadio4, ui->arcRadio5};
  radios[arc]->setChecked(true);
}

void MainWindow::set_triplet_radio(int triplet){
  QRadioButton* radios[]{ui->tripRadio1, ui->tripRadio2, ui->tripRadio3,
                         ui->tripRadio4, ui->tripRadio5, ui->tripRadio6,
                         ui->tripRadio7, ui->tripRadio8, ui->tripRadio9};
  radios[triplet]->setChecked(true);
}

void MainWindow::set_type_radio(MainWindow::int_t type) {
  QRadioButton* radios[]{ui->int__x_Radio, ui->int__A_Radio, ui->int__P_Radio,
                         ui->int_xP_Radio, ui->int_AB_Radio, ui->int__B_Radio,
                         ui->int_xt_Radio, ui->int_Pt_Radio, ui->int__t_Radio};
  radios[type_order_index(type)]->setChecked(true);
}

void MainWindow::cycle_arc() {
  cycle_one->next();
  set_arc(cycle_one->at(0));
}

void MainWindow::cycle_triplet() {
  cycle_one->next();
  set_triplet(cycle_one->at(0));
}

void MainWindow::cycle_type(){
  cycle_one->next();
  set_int(type_order[cycle_one->at(0)]);
}

void MainWindow::cycle_arc_triplet(){
  cycle_two->next();
  set_arc(cycle_two->at(0), false);
  set_triplet(cycle_two->at(1));
}

void MainWindow::cycle_arc_type(){
  cycle_two->next();
  set_arc(cycle_two->at(0), false);
  set_int(type_order[cycle_two->at(1)]);
}

void MainWindow::cycle_triplet_type(){
  cycle_two->next();
  set_triplet(cycle_two->at(0), false);
  set_int(type_order[cycle_two->at(1)]);
}

void MainWindow::cycle_arc_triplet_type(){
  cycle_three->next();
  set_arc(cycle_three->at(0), false);
  set_triplet(cycle_three->at(1), false);
  set_int(type_order[cycle_three->at(2)]);
}

void MainWindow::set_arc(int n, bool plot_now){
  _fixed_arc = n;
  update_intensity_limits();
  if (plot_now) plot();
}

void MainWindow::set_triplet(int n, bool plot_now){
  _fixed_triplet = n;
  update_intensity_limits();
  if (plot_now) plot();
}

void MainWindow::set_int(int_t t, bool plot_now){
  _fixed_type = t;
  update_intensity_limits();
  if (plot_now) plot();
}

void MainWindow::plot(){
  if (is_paused()) return;
  set_intensity_limits();
  _triplet_fixed = ui->tripletBox->isChecked();
  _type_fixed = ui->intTypeBox->isChecked();
  if (_triplet_fixed && _type_fixed) {
    plot_single(_fixed_arc, _fixed_triplet, _fixed_type);
  } else if (_triplet_fixed) {
    plot_one_triplet(_fixed_arc, _fixed_triplet);
  } else if (_type_fixed) {
    plot_one_type(_fixed_arc, _fixed_type);
  }
  //plots->scale(minima, maxima, !ui->scaleButton->isChecked());
}

void MainWindow::plot_single(int arc, int triplet, int_t t){
    PlotManager::Dim d{PlotManager::Dim::none};
    if (int_t::a == t || int_t::b == t || int_t::x == t || int_t::p == t || int_t::t == t){
        d = PlotManager::Dim::one;
    }
    if (int_t::ab == t || int_t::xt == t || int_t::pt == t || int_t::xp == t){
        d = PlotManager::Dim::two;
    }
    plots->make_single(d, t);
    auto key = data->key(arc, triplet, t);
    auto is_log = ui->scaleButton->isChecked();
    auto gradient = ui->colormapComboBox->currentText().toStdString();
    auto is_inverted = ui->colormapInvertedCheck->isChecked();
    auto intensity = 1.0 * max.at(key);
    if (PlotManager::Dim::one == d){
      using ::bifrost::data::Filter;
      std::optional<std::vector<double>> all{std::nullopt}, included{std::nullopt}, excluded{std::nullopt};
      if (ui->filter1Everything->isChecked()) all = data->data_1D(arc, triplet, t, Filter::none);
      if (ui->filter1Included->isChecked()) included = data->data_1D(arc, triplet, t, Filter::positive);
      if (ui->filter1Excluded->isChecked()) excluded = data->data_1D(arc, triplet, t, Filter::negative);
      plots->plot_all_included_excluded(0, 0, data->axis(t), all, included, excluded, 0.0, intensity, is_log);
    }
    if (PlotManager::Dim::two == d){
        plots->plot(0, 0, data->data_2D(arc, triplet, t, plot_filter), 0.0, intensity, is_log, gradient, is_inverted, {}, {}, {});
    }
}

void MainWindow::plot_one_type(int arc, int_t t){
  PlotManager::Dim d{PlotManager::Dim::none};
  if (int_t::a == t || int_t::b == t || int_t::x == t || int_t::p == t || int_t::t == t){
      d = PlotManager::Dim::one;
  } else if (int_t::ab == t || int_t::xt == t || int_t::pt == t || int_t::xp == t){
      d = PlotManager::Dim::two;
  }
  plots->make_all_same(d, t);
  auto is_log = ui->scaleButton->isChecked();
  using ::bifrost::data::Filter;
  std::optional<std::vector<double>> all{std::nullopt}, included{std::nullopt}, excluded{std::nullopt};
  if (PlotManager::Dim::one == d){
    for (int i=0; i<3; ++i) {
      for (int j=0; j<3; ++j) {
        auto key = data->key(arc, i*3+j, t);
        auto intensity = 1.0 * max.at(key);
        if (ui->filter1Everything->isChecked()) all = data->data_1D(arc, i*3+j, t, Filter::none);
        if (ui->filter1Included->isChecked()) included = data->data_1D(arc, i*3+j, t, Filter::positive);
        if (ui->filter1Excluded->isChecked()) excluded = data->data_1D(arc, i*3+j, t, Filter::negative);
        plots->plot_all_included_excluded(i, j, data->axis(t), all, included, excluded, 0.0, intensity, is_log);
      }
    }
  }
  if (PlotManager::Dim::two == d){
    auto gradient = ui->colormapComboBox->currentText().toStdString();
    auto is_inverted = ui->colormapInvertedCheck->isChecked();
      for (int i=0; i<3; ++i) {
          for (int j=0; j<3; ++j) {
            auto key = data->key(arc, i*3+j, t);
            plots->plot(i, j, data->data_2D(arc, i*3+j, t, plot_filter), 0.0, 1.0*max[key], is_log, gradient, is_inverted, {}, {}, {});
          }
      }
  }
}

void MainWindow::plot_one_triplet(int arc, int triplet){
  using ::bifrost::data::Filter;
  plots->make_multi(type_order);
  int i[]{0,0,0,1,1,1,2,2,2};
  int j[]{0,1,2,0,1,2,0,1,2};
  auto is_log = ui->scaleButton->isChecked();
  std::optional<std::vector<double>> all{std::nullopt}, included{std::nullopt}, excluded{std::nullopt};
  for (int t: {0, 1, 2, 5, 8}){
    auto key = data->key(arc, triplet, type_order[t]);
    auto intensity = 1.0 * max[key];
    if (ui->filter1Everything->isChecked()) all = data->data_1D(arc, triplet, type_order[t], Filter::none);
    if (ui->filter1Included->isChecked()) included = data->data_1D(arc, triplet, type_order[t], Filter::positive);
    if (ui->filter1Excluded->isChecked()) excluded = data->data_1D(arc, triplet, type_order[t], Filter::negative);
    plots->plot_all_included_excluded(i[t], j[t], data->axis(type_order[t]), all, included, excluded, 0.0, intensity, is_log);
  }
  auto gradient = ui->colormapComboBox->currentText().toStdString();
  auto is_inverted = ui->colormapInvertedCheck->isChecked();
  for (int t: {3, 4, 6, 7}){
    auto key = data->key(arc, triplet, type_order[t]);
    plots->plot(i[t], j[t], data->data_2D(arc, triplet, type_order[t], plot_filter), 0.0, 1.0*max[key], is_log, gradient, is_inverted, {}, {}, {});
  }
}

void MainWindow::reset(){
  data->clear();
}

void MainWindow::pause_toggled(bool checked)
{
  std::cout << "Intensity limits" << std::endl;
  for (key_t key=0; key < data->key_count(); ++key){
    auto value = max[key];
    std::stringstream stype;
    stype << data->key_type(key);
    fmt::print("{:1d} {:1d} {:6s} ({}, {})\n", data->key_arc(key), data->key_triplet(key), stype.str(), 0, value);
  }
  std::cout << "pause button is now " << (checked ? "on" : "off") << std::endl;
  if (!checked) cycle();
}

bool MainWindow::is_paused(){
  return ui->pauseButton->isChecked();
}

MainWindow::PlotType MainWindow::selected_plot_type(){
  if (ui->tripletBox->isChecked() && !ui->intTypeBox->isChecked()) return PlotType::Types;
  if (ui->intTypeBox->isChecked() && !ui->tripletBox->isChecked()) return PlotType::Triplets;
  if (ui->intTypeBox->isChecked() && ui->tripletBox->isChecked()) return PlotType::Singular;
  return PlotType::Unknown;
}


void MainWindow::autoscale_toggled(bool check)
{
  for (auto x: maxBox) x->setReadOnly(check);
  if (check) set_intensity_limits();
}

void MainWindow::set_intensity_limits_triplets(){
  for (int triplet=0; triplet<9; ++triplet){
    auto key = data->key(_fixed_arc, triplet, _fixed_type);
    maxBox[triplet]->setValue(max.at(key));
  }
}

void MainWindow::get_intensity_limits_triplets(){
  for (int triplet=0; triplet<9; ++triplet){
    auto key = data->key(_fixed_arc, triplet, _fixed_type);
    max[key] = maxBox[triplet]->value();
  }
}

void MainWindow::auto_intensity_limits_triplets(){
  for (int triplet=0; triplet<9; ++triplet){
    auto key = data->key(_fixed_arc, triplet, _fixed_type);
    max[key] = static_cast<int>(data->max(key, plot_filter));
  }
}

void MainWindow::set_intensity_limits_types(){
  int_t types[]{int_t::x, int_t::a, int_t::p, int_t::xp, int_t::ab, int_t::b, int_t::xt, int_t::pt, int_t::t};
  for (int type=0; type<9; ++type){
    auto key = data->key(_fixed_arc, _fixed_triplet, types[type]);
    maxBox[type]->setValue(max.at(key));
  }
}

void MainWindow::get_intensity_limits_types(){
  int_t types[]{int_t::x, int_t::a, int_t::p, int_t::xp, int_t::ab, int_t::b, int_t::xt, int_t::pt, int_t::t};
  for (int type=0; type<9; ++type){
    auto key = data->key(_fixed_arc, _fixed_triplet, types[type]);
    max[key] = maxBox[type]->value();
  }
}

void MainWindow::auto_intensity_limits_types(){
  int_t types[]{int_t::x, int_t::a, int_t::p, int_t::xp, int_t::ab, int_t::b, int_t::xt, int_t::pt, int_t::t};
  for (auto & type : types){
    auto key = data->key(_fixed_arc, _fixed_triplet, type);
    max[key] = static_cast<int>(data->max(key, plot_filter));
  }
}

void MainWindow::set_intensity_limits_singular(){
  auto key = data->key(_fixed_arc, _fixed_triplet, _fixed_type);
  maxBox[0]->setValue(max.at(key));
}

void MainWindow::get_intensity_limits_singular(){
  auto key = data->key(_fixed_arc, _fixed_triplet, _fixed_type);
  max[key] = maxBox[0]->value();
}

void MainWindow::auto_intensity_limits_singular(){
  auto key = data->key(_fixed_arc, _fixed_triplet, _fixed_type);
  max[key] = static_cast<int>(data->max(key, plot_filter));
}

void MainWindow::set_intensity_limits() {
  if (ui->autoscaleButton->isChecked()) {
    auto_intensity_limits();
    update_intensity_limits();
  } else {
    get_intensity_limits();
  }
}

void MainWindow::auto_intensity_limits() {
  auto pt = selected_plot_type();
  if (pt == PlotType::Types) auto_intensity_limits_types();
  if (pt == PlotType::Triplets) auto_intensity_limits_triplets();
  if (pt == PlotType::Singular) auto_intensity_limits_singular();
}

void MainWindow::update_intensity_limits() {
  auto pt = selected_plot_type();
  if (pt == PlotType::Types) set_intensity_limits_types();
  if (pt == PlotType::Triplets) set_intensity_limits_triplets();
  if (pt == PlotType::Singular) set_intensity_limits_singular();
}

void MainWindow::get_intensity_limits(){
  auto pt = selected_plot_type();
  if (pt == PlotType::Types) get_intensity_limits_types();
  if (pt == PlotType::Triplets) get_intensity_limits_triplets();
  if (pt == PlotType::Singular) get_intensity_limits_singular();
}
