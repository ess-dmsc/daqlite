// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
//
/// \file
//
/// \brief GUI main window header for the Fylgje application
//===----------------------------------------------------------------------===//
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>
#include <QPlot/qcustomplot/qcustomplot.h>
#include <optional>

#include "WorkerThread.h"
#include "PlotManager.h"
#include "DataManager.h"
#include "TwoSpinBox.h"
#include "TableItemTypes.h"
#include "Cycles.h"
#include "Configuration.h"
#include "Calibration.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    using int_t = ::bifrost::data::Type;
    using intensity_map_t = ::bifrost::data::map_t<int>;
    enum class Time {Fixed, Historical, Live};
    enum class PlotType {Unknown, Types, Triplets, Singular};
public:
    MainWindow(const Configuration & Config, const Calibration & calibration, QWidget *parent = nullptr);
    ~MainWindow();

    void set_arc_1(){set_arc(0);}
    void set_arc_2(){set_arc(1);}
    void set_arc_3(){set_arc(2);}
    void set_arc_4(){set_arc(3);}
    void set_arc_5(){set_arc(4);}

    void set_triplet_1(){set_triplet(0);}
    void set_triplet_2(){set_triplet(1);}
    void set_triplet_3(){set_triplet(2);}
    void set_triplet_4(){set_triplet(3);}
    void set_triplet_5(){set_triplet(4);}
    void set_triplet_6(){set_triplet(5);}
    void set_triplet_7(){set_triplet(6);}
    void set_triplet_8(){set_triplet(7);}
    void set_triplet_9(){set_triplet(8);}

    void set_int_x(){set_int(int_t::x);}
    void set_int_a(){set_int(int_t::a);}
    void set_int_b(){set_int(int_t::b);}
    void set_int_p(){set_int(int_t::p);}
    void set_int_t(){set_int(int_t::t);}
    void set_int_xP(){set_int(int_t::xp);}
    void set_int_AB(){set_int(int_t::ab);}
    void set_int_xt(){set_int(int_t::xt);}
    void set_int_Pt(){set_int(int_t::pt);}

    void set_bins_a_1d(int m){data->set_bins_1d(int_t::a, m); set_intensity_limits();}
    void set_bins_b_1d(int m){data->set_bins_1d(int_t::b, m); set_intensity_limits();}
    void set_bins_p_1d(int m){data->set_bins_1d(int_t::p, m); set_intensity_limits();}
    void set_bins_x_1d(int m){data->set_bins_1d(int_t::x, m); set_intensity_limits();}
    void set_bins_t_1d(int m){data->set_bins_1d(int_t::t, m); set_intensity_limits();}
    void set_bins_a_2d(int m){data->set_bins_2d(int_t::a, m); set_intensity_limits();}
    void set_bins_b_2d(int m){data->set_bins_2d(int_t::b, m); set_intensity_limits();}
    void set_bins_p_2d(int m){data->set_bins_2d(int_t::p, m); set_intensity_limits();}
    void set_bins_x_2d(int m){data->set_bins_2d(int_t::x, m); set_intensity_limits();}
    void set_bins_t_2d(int m){data->set_bins_2d(int_t::t, m); set_intensity_limits();}

    void set_time_live();
    void set_time_historical();
    void set_time_fixed();
    void set_time_early(const QDateTime &);
    void set_time_late(const QDateTime &);

    void set_filter_all(){plot_filter = ::bifrost::data::Filter::none; plot();}
    void set_filter_included(){plot_filter = ::bifrost::data::Filter::positive; plot();}
    void set_filter_excluded(){plot_filter = ::bifrost::data::Filter::negative; plot();}

    // gateway, uses private flags to determine which plot type is called
    void plot();

    void timer_callback_window_update();

private:
  void set_arc(int n, bool plot_now=true);
  void set_triplet(int n, bool plot_now=true);
  void set_int(int_t t, bool plot_now=true);

  PlotType selected_plot_type();
  // ensure there is only 1 full-size canvas, draw the fully specified plot
  void plot_single(int arc, int triplet, int_t int_type);
  // plot all triplets for the specified intensity plot type
  void plot_one_type(int arc, int_t int_type);
  // plot all intensity plots for the specified triplet
  void plot_one_triplet(int arc, int triplet);

  void initialize();
  void setup();
  void setup_add_bin_boxes();
  void setup_time_limits();
  void setup_intensity_limits();
  void setup_gradient_list();
  void setup_consumer();
  void setup_calibration();
  void setup_calibration_table();
  void setup_calibration_table_items();
  void setup_calibration_info();
  void update_calibration_info();

  void set_intensity_limits();
  void get_intensity_limits();
  void auto_intensity_limits();
  void set_intensity_limits_triplets();
  void get_intensity_limits_triplets();
  void auto_intensity_limits_triplets();
  void set_intensity_limits_types();
  void get_intensity_limits_types();
  void auto_intensity_limits_types();
  void set_intensity_limits_singular();
  void get_intensity_limits_singular();
  void auto_intensity_limits_singular();
  void update_intensity_limits();

  void set_arc_radio(int arc);
  void set_triplet_radio(int triplet);
  void set_type_radio(int_t type);

  void cycle();
  void cycle_any();
  void cycle_arc();
  void cycle_triplet();
  void cycle_type();
  void cycle_arc_triplet();
  void cycle_arc_type();
  void cycle_triplet_type();
  void cycle_arc_triplet_type();

  void reset();

  void autoscale_toggled(bool checked);
  void pause_toggled(bool checked);
  bool is_paused();


  void save_calibration();
  void load_calibration();

  void setup_data();
  void save_data();


private:
    Ui::MainWindow *ui;
    bool _triplet_fixed{false};
    bool _type_fixed{false};
    int _fixed_arc{0};
    int _fixed_triplet{0};
    int_t _fixed_type{int_t::ab}; // should match the default in AppWindow.ui

    std::map<std::pair<int_t, int>, TwoSpinBox *> bin_boxes{};
    intensity_map_t max;

    std::vector<QSpinBox*> maxBox;

    std::string broker;
    std::string topic;

    ::bifrost::data::Manager * data;
    ::bifrost::data::Filter plot_filter{::bifrost::data::Filter::none};
    PlotManager * plots;
    WorkerThread * consumer{};

    /// \brief configuration obtained from main()
    Configuration configuration;

    /// \brief calibration obtained from main()
    Calibration calibration;

    Time time_status{Time::Live};

    std::optional<fylgje::Cycles<1>> cycle_one;
    std::optional<fylgje::Cycles<2>> cycle_two;
    std::optional<fylgje::Cycles<3>> cycle_three;

    std::array<int_t, 9> type_order{int_t::x, int_t::a, int_t::p, int_t::xp, int_t::ab, int_t::b, int_t::xt, int_t::pt, int_t::t};
    int type_order_index(int_t t){
      auto b = type_order.begin();
      auto e = type_order.end();
      auto i = std::distance(b, std::find(b, e, t));
      if (i < 0 || i >= 9){
        throw std::out_of_range("The index is out of range");
      }
      return static_cast<int>(i);
    }

    QTableWidget * calibration_table{nullptr};
    std::vector<QTableWidgetItem *> calibration_table_items;
    std::vector<std::pair<std::string, std::function<QTableWidgetItem*(int,int,int,CalibrationUnit *)>>> calibration_table_columns;
};
#endif // MAINWINDOW_H
