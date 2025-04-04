// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Interface between DataManager and QCustomPlot for plotting
//===----------------------------------------------------------------------===//

#pragma once
#include <map>
#include <QGridLayout>
#include <QPlot/qcustomplot/qcustomplot.h>
#include <fmt/format.h>
#include "DataManager.h"
#include <iostream>

/// \brief Create a QCPColorGradient from a colormap name string
QCPColorGradient named_colormap(std::string_view name, bool invert);

class PlotManager{
public:
  using type_t = ::bifrost::data::Type;
  using layout_t = QGridLayout;
  enum class Dim {none, one, two};

  PlotManager(layout_t * l, int n1, int n2): layout(l), n1(n1), n2(n2) {
      dims[0] = Dim::none;
  }

  /// \brief (Re)set the plot layout to a single 1-D or 2-D plot
  void make_single(Dim d, type_t t);

  /// \brief (Re)set the plot layout to 9 identical 1-D or 2-D plots
  void make_all_same(Dim d, type_t t);

  /// \brief (Re)set the plot layout to 9 different 1-D and 2-D plots
  void make_multi(std::array<type_t, 9> ts);

  /// \brief Plot a 1D histogram convenience function converting to QVectors
  void plot(int i, int j, const std::vector<double> & x, const std::vector<double> & y, double min, double max, bool is_log);

  /// \brief Plot a 1D histogram
  void plot(int i, int j, const QVector<double> * x, const QVector<double> * y, double min, double max, bool is_log);

  /// \brief Plot multiple 1D histograms on the same plot
  void plot_all_included_excluded(int i, int j, const std::vector<double> & std_x,
                                  const std::optional<std::vector<double>> & all,
                                  const std::optional<std::vector<double>> & included,
                                  const std::optional<std::vector<double>> & excluded,
                                  double min, double max, bool is_log);


  /// \brief Plot a 2D histogram
  void plot(int i, int j, QCPColorMapData * data, double min, double max, bool is_log,
            std::string_view gradient, bool is_inverted,
            const std::optional<std::vector<std::pair<double, double>>> & left,
            const std::optional<std::vector<std::pair<double, double>>> & center,
            const std::optional<std::vector<std::pair<double, double>>> & right);


private:
  [[nodiscard]] inline int key(int i, int j, ::bifrost::data::Filter filter = ::bifrost::data::Filter::none) const {
      using ::bifrost::data::Filter;
      auto index = static_cast<int>(filter);
      return i + n1 * j + n1 * n2 * index;
    }
    void make_plot(int i, int j, bool flip, type_t t);
    void make_1D(int i, int j, bool flip, type_t t);
    void make_2D(int i, int j, bool flip, type_t t);

private:
  layout_t * layout{};
  int n1;
  int n2;

  std::map<int, QCustomPlot *> plots;
  std::map<int, QCPColorMap *> images;
  std::map<int, QCPGraph *> lines;
  std::map<int, QCPCurve *> polygons;
  std::map<int, Dim> dims;
  std::map<int, type_t> types;
  std::map<int, bool> flipped;

  void clear(){
    qDeleteAll(layout->children());
    plots.clear();
    images.clear();
    lines.clear();
    dims.clear();
    types.clear();
    flipped.clear();
  }

  void remove(int i, int j){
    auto item = layout->itemAtPosition(i, j);
    if (item){
      layout->removeItem(item);
      delete item->widget();
      delete item;
    }
    auto k = key(i, j);
    if (plots.count(k)) plots.erase(k);
    if (images.count(k)) images.erase(k);
    if (lines.count(k)) lines.erase(k);
    if (dims.count(k)) dims.erase(k);
    if (types.count(k)) types.erase(k);
    if (flipped.count(k)) flipped.erase(k);
  }

  void empty_layout(){
    for (int i=0; i<layout->rowCount(); ++i){
      for (int j=0; j<layout->columnCount(); ++j){
        remove(i, j);
      }
    }
  }
};