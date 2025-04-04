// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
//
/// \file
//
/// \brief Implementation of plot interface for Fylgje
//===----------------------------------------------------------------------===//
#include "PlotManager.h"

void PlotManager::make_single(Dim d, type_t t){
  if (layout->rowCount() != 1 || layout->columnCount() != 1 || d != dims[0]){
    empty_layout();
    // why is this _always_ 3x3?
    // std::cout << layout->columnCount() << " " << layout->rowCount() << std::endl;
    if (Dim::one == d) return make_1D(0, 0, false, t);
    if (Dim::two == d) return make_2D(0, 0, false, t);
    // std::cout << layout->columnCount() << " " << layout->rowCount() << std::endl;
  }
}

void PlotManager::make_all_same(Dim d, type_t t){
  empty_layout();
  for (int i=0; i<3; ++i) {
    for (int j=0; j<3; ++j) {
      // if the plot exists but is the wrong type, remove it.
      if (layout->itemAtPosition(i, j) && d !=dims[key(i, j)]){
        remove(i, j);
      }
      // if the plot doesn't exist create it
      if (!layout->itemAtPosition(i, j)) {
        (Dim::one == d) ? make_1D(i, j, false, t) : make_2D(i, j, false, t);
      }
    }
  }
}

void PlotManager::make_multi(std::array<type_t, 9> ts){
  /*
   * 1 1 1
   * 2 2 -
   * 2 2 -
   */
  empty_layout();
  for (int i=0; i<3; ++i) {
    for (int j=0; j<3; ++j) {
      Dim t{i == 0 || j > 1 ? Dim::one : Dim::two};
      // if the plot exists but is the wrong type, remove it.
      if (layout->itemAtPosition(i, j) && t !=dims[key(i, j)]){
        remove(i, j);
      }
      // if the plot doesn't exist create it
      if (!layout->itemAtPosition(i, j)) {
        (Dim::one == t) ? make_1D(i, j, (i>0) & (j>1), ts[i*3 + j]) : make_2D(i, j, false, ts[i*3 + j]);
      }
    }
  }
}

void PlotManager::plot(int i, int j, const std::vector<double> & x, const std::vector<double> & y, double min, double max, bool is_log){
  QVector<double> q_x(x.begin(), x.end());
  QVector<double> q_y(y.begin(), y.end());
  plot(i, j, &q_x, &q_y, min, max, is_log);
}

void PlotManager::plot(int i, int j, const QVector<double> * x, const QVector<double> * y, double min, double max, bool is_log){
  // 1-D data plotting
  auto k = key(i, j);
  if (!dims.count(k) || dims.at(k) != Dim::one) return;
  auto g = lines.at(k);
  g->setData(*x, *y);
  auto p = plots.at(k);
  QCPAxis * independent{flipped[k] ? p->yAxis : p->xAxis};
  independent->setRange(x->front(), x->back());
  // apply scaling
  QCPAxis * ax{flipped[k] ? p->xAxis : p->yAxis};
  ax->setScaleType(is_log ? QCPAxis::stLogarithmic : QCPAxis::stLinear);
  ax->setRange(min - (max - min) / 40, max + (max - min) / 20);
}

void PlotManager::plot_all_included_excluded(int i, int j, const std::vector<double> & std_x,
                                const std::optional<std::vector<double>> & all,
                                const std::optional<std::vector<double>> & included,
                                const std::optional<std::vector<double>> & excluded,
                                double min, double max, bool is_log) {
  using ::bifrost::data::Filter;
  // 1-D data plotting
  auto k = key(i, j);
  if (!dims.count(k) || dims.at(k) != Dim::one) return;

  QVector<double> x(std_x.begin(), std_x.end());
  if (all.has_value()) {
    lines.at(key(i, j, Filter::none))->setData(x, QVector<double>(all.value().begin(), all.value().end()));
  }
  if (included.has_value()) {
    lines.at(key(i, j, Filter::positive))->setData(x, QVector<double>(included.value().begin(), included.value().end()));
  }
  if (excluded.has_value()) {
    lines.at(key(i, j, Filter::negative))->setData(x, QVector<double>(excluded.value().begin(), excluded.value().end()));
  }

  auto p = plots.at(k);
  QCPAxis * independent{flipped[k] ? p->yAxis : p->xAxis};
  independent->setRange(x.front(), x.back());
  // apply scaling
  QCPAxis * ax{flipped[k] ? p->xAxis : p->yAxis};
  ax->setScaleType(is_log ? QCPAxis::stLogarithmic : QCPAxis::stLinear);
  ax->setRange(min - (max - min) / 40, max + (max - min) / 20);
}

void PlotManager::plot(int i, int j, QCPColorMapData * data, double min, double max, bool is_log,
          std::string_view gradient, bool is_inverted,
          const std::optional<std::vector<std::pair<double, double>>> & left,
          const std::optional<std::vector<std::pair<double, double>>> & center,
          const std::optional<std::vector<std::pair<double, double>>> & right){
  auto k = key(i, j);
  if (!dims.count(k) || dims.at(k) != Dim::two) return;
  if (!images.count(k)) return;
  auto im = images.at(k);
  // Why does setData _require_ a mutable pointer?
  im->setData(data);
  auto p = plots.at(k);
  p->xAxis->setRange(0, ::bifrost::data::BIN2D);
  p->yAxis->setRange(0, ::bifrost::data::BIN2D);
  im->setGradient(named_colormap(gradient, is_inverted));
  im->setDataScaleType(is_log ? QCPAxis::stLogarithmic : QCPAxis::stLinear);
  im->setDataRange(QCPRange(min, max));

  auto poly = [](const std::vector<std::pair<double, double>> & v){
    std::vector<double> x, y;
    x.reserve(v.size()+1);
    y.reserve(v.size()+1);
    for (const auto & [a, b]: v){
      x.push_back(a);
      y.push_back(b);
    }
    x.push_back(v.front().first);
    y.push_back(v.front().second);
    QVector<double> q_x(x.begin(), x.end());
    QVector<double> q_y(y.begin(), y.end());
    return std::make_pair(q_x, q_y);
  };
  using ::bifrost::data::Filter;
  if (left.has_value()){
    auto [x, y] = poly(left.value());
    polygons[key(i, j, Filter::negative)]->addData(x, y);
  }
  if (center.has_value()){
    auto [x, y] = poly(center.value());
    polygons[key(i, j, Filter::none)]->addData(x, y);
  }
  if (right.has_value()){
    auto [x, y] = poly(right.value());
    polygons[key(i, j, Filter::positive)]->addData(x, y);
  }
}


void PlotManager::make_plot(int i, int j, bool flip, type_t t){
  auto item = layout->itemAtPosition(i, j);
  if (!item){
    auto p = new QCustomPlot();
    p->axisRect()->setAutoMargins(QCP::msNone);
    p->xAxis->setTicks(false);
    p->yAxis->setTicks(false);
    p->xAxis->setTickPen(QPen(Qt::NoPen));
    layout->addWidget(p, i, j);
    plots[key(i, j)] = p;
    types[key(i, j)] = t;
    flipped[key(i, j)] = flip;
  }
}

void PlotManager::make_1D(int i, int j, bool flip, type_t t){
  using ::bifrost::data::Filter;
  auto k = key(i, j);
  dims[k] = Dim::one;
  make_plot(i, j, flip, t);

  plots[k]->yAxis->setTicks(true);
  plots[k]->xAxis->setTicks(true);
  plots[k]->yAxis->setTickLabels(true);
  plots[k]->xAxis->setTickLabels(true);
  plots[k]->axisRect()->setupFullAxesBox();

  std::vector<std::pair<Filter, QColor>> filter_color{
      {{Filter::none, Qt::black}, {Filter::positive, Qt::darkGreen}, {Filter::negative, Qt::darkRed}}
  };
  for (const auto & [filter, color]: filter_color){
    auto lk = key(i, j, filter);
    lines[lk] = new QCPGraph(flip ? plots[k]->yAxis : plots[k]->xAxis, flip ? plots[k]->xAxis : plots[k]->yAxis);
    lines[lk]->setLineStyle(QCPGraph::LineStyle::lsStepCenter);
    lines[lk]->setPen(QPen(color));
  }
  //plots[k]->setInteractions(QCP::iRangeDrag| QCP::iRangeZoom | QCP::iSelectPlottables);
}

void PlotManager::make_2D(int i, int j, bool flip, type_t t){
  using ::bifrost::data::Filter;
  dims[key(i, j)] = Dim::two;
  make_plot(i, j, flip, t);
  auto p = plots[key(i, j)];
  p->xAxis->setRange(0, n2);
  p->yAxis->setRange(0, n2);
  p->axisRect()->setupFullAxesBox();

  auto m = new QCPColorMap(flip ? p->yAxis : p->xAxis, flip ? p->xAxis : p->yAxis);
  m->data()->setSize(n2, n2);
  m->data()->setRange(QCPRange(0, n2-1), QCPRange(0, n2-1));
  m->setTightBoundary(false);
  m->setInterpolate(false);

  auto s = new QCPColorScale(p);
  m->setColorScale(s);
  m->setGradient(QCPColorGradient::gpGrayscale);
  m->rescaleDataRange();
  images[key(i, j)] = m;

  // abuse the Filter enum to also specify tubes
  std::vector<std::pair<Filter, QColor>> filter_color{
      {{Filter::none, Qt::green}, {Filter::positive, Qt::yellow}, {Filter::negative, Qt::magenta}}
  };
  for (const auto & [filter, color]: filter_color){
    auto pk = key(i, j, filter);
    polygons[pk] = new QCPCurve(flip ? p->yAxis : p->xAxis, flip ? p->xAxis : p->yAxis);
    polygons[pk]->setLineStyle(QCPCurve::LineStyle::lsLine);
    polygons[pk]->setPen(QPen(color));
  }
}


QCPColorGradient named_colormap(std::string_view name, bool invert){
  auto grad = QCPColorGradient();
  auto preset = QCPColorGradient::gpGrayscale;
  if (name == "gray" || name == "grey"){
    preset=QCPColorGradient::gpGrayscale;
  } else if (name == "hot"){
    preset=QCPColorGradient::gpHot;
  } else if (name == "cold"){
    preset=QCPColorGradient::gpCold;
  } else if (name == "night"){
    preset=QCPColorGradient::gpNight;
  } else if (name == "candy") {
    preset=QCPColorGradient::gpCandy;
  } else if (name == "geography") {
    preset=QCPColorGradient::gpGeography;
  } else if (name == "thermal") {
    preset=QCPColorGradient::gpThermal;
  }
  grad.loadPreset(preset);
  if (invert) grad=grad.inverted();
  return grad;
}