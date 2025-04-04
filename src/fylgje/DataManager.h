// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Hold and update buffers for BIFROST-fylgje
//===----------------------------------------------------------------------===//
#pragma once
#include <map>
#include <vector>
#include <QVector>
#include <QPlot/qcustomplot/qcustomplot.h>
#include <fmt/format.h>
#include <h5cpp/hdf5.hpp>
#include "Calibration.h"

namespace bifrost {
  /// \brief Convert from group number to arc number
  /// The group number is the index of a triplet within a fiber-ring which is in the range (0, 15]
  /// The arc number corresponds to the triplet energy, which has five discrete values
  inline int arc(int group) {
    return group / 3;
  }

  /// \brief Convert from fiber-ring number to module number
  /// The fiber-ring number is the index of a fiber-ring within a module which is in the range (0, 5)
  /// Each two successive fiber numbers correspond to a single ring, or module.
  inline int module(int fiber) {
    return fiber / 2;
  }

  /// \brief Convert from fiber-ring and group number to triplet number
  /// The triplet number indexes all triplets of a single energy from smallest to largest scattering angle.
  /// Module 0 holds triplets (0,1,2); module 1 holds triplets (3,4,5); and module 2 holds triplets (6,7,8).
  /// Within a module the group number steps through triplet types (short, medium, long) in order, and
  /// triplet energies [2.7, 3.2, 3.8, 4.4, 5.0] meV in order.
  inline int triplet(int fiber, int group) {
    int type = group % 3;
    return module(fiber) * 3 + type;
  }
}

namespace bifrost::data {
  /// \brief Specify whether the calibration should be used to filter data and how
  enum class Filter {none, positive, negative};

  ///\brief Identify the bin number for either A or B
  ///\param x the value to bin
  ///\param shift the number of bits to shift the value to the right (powers of two to divide by)
  ///\param bins the maximum number of bins which the axis is divided into
  int hist_a_or_b(int x, int shift, int bins);

  ///\brief Identify the bin number for A+B
  ///\param x the value to bin
  ///\param shift one less than the number of bits to shift the value to the right (powers of two to divide by)
  ///\param bins the maximum number of bins which the axis is divided into
  int hist_p(int x, int shift, int bins);

  ///\brief Identify the bin number for time
  ///\returns the bin number for the time value modulus the ESS pulse period of 1/14 seconds
  int hist_t(double x, int bins);

  ///\brief Identify the bin number for the ratio of A-B to A+B
  int hist_x(int a, int b, int bins);

  ///\brief Helper to calculate the bin edge values for a histogram
  template<class T, class R>
  void iota_step(T begin, T end, R step, R first=R(0)){
    *begin = first;
    for (auto p=begin; p != end; ++p){
      auto n = p+1;
      if (n != end) *n = *p + step;
    }
  }

  ///\brief Identifier for different histograms which the DataManager holds
  enum class Type {unknown=-1, x=0, a=1, p=2, xp=3, ab=4, b=5, xt=6, pt=7, t=8, pixel=9};

  ///\brief The 1-D histogram types which the DataManager holds
  constexpr Type TYPE1D[]{Type::a, Type::b, Type::x, Type::p, Type::t};

  ///\brief The 2-D histogram types which the DataManager holds
  constexpr Type TYPE2D[]{Type::xp, Type::ab, Type::xt, Type::pt};

  ///\brief All histograms held by the DataManager, ordered for plotting
  constexpr Type TYPEND[]{Type::a, Type::b, Type::x, Type::p, Type::t, Type::xp, Type::ab, Type::xt, Type::pt};

  ///\brief The number of histogram types held by the DataManager
  constexpr size_t TYPECOUNT{9}; // just those used for indexing map_t

  ///\brief Return the name of the dependent dataset for a given histogram type
  std::string type_dataset_name(Type type);

  ///\brief Return the name(s) of the independent dataset(s) for a given histogram type
  std::vector<std::string> axes_names(Type type);

  using key_t = int64_t;

  // This replaces an earlier attempt to use std::map for the data container
  // doing so caused problems with indexing/reindexing intensity limits in the window
  template<class T> class map_t : public std::vector<T> {
  public:
    [[nodiscard]] size_t count(key_t key) const {
      if (key < 0){
        fmt::print("Negative key? {}\n", key);
        return 0;
      }
      if (static_cast<size_t>(key) >= this->size()){
        fmt::print("Too big key since {} > {}\n", key, this->size());
        return 0;
      }
      return 1;
    }
  };

  ///\brief Identify if a specified histogram type is 1-D
  bool is_1D(Type t);
  ///\brief Identify if a specified histogram type is 2-D
  [[maybe_unused]] bool is_2D(Type t);

  ///\brief Default divisor for 15-bit data to bin into 1-D histograms
  constexpr int SHIFT1D = 5;
  ///\brief Default number of bins for 1-D histograms
  constexpr int BIN1D = (1 << 15) >> SHIFT1D;
  ///\brief Default divisor for 15-bit data to bin into 2-D histograms
  constexpr int SHIFT2D = 6;
  ///\brief Default number of bins for 2-D histograms
  constexpr int BIN2D = (1 << 15) >> SHIFT2D;

  ///\brief Manager for holding and updating data for BIFROST-fylgje
  class Manager{
  public:
    using AX = std::vector<double>;
    using D1 = std::vector<double>;
    using D2 = QCPColorMapData;
    using data_t = std::vector<int>;
  private:
    ///\param everything histograms for all data received
    ///\param included histograms for data which passes the calibration filter
    ///\param excluded histograms for data which fails the calibration filter
    map_t<data_t> everything, included, excluded;
    ///\param pixel_data data points to store post-EFU-calculation results
    data_t pixel_data;

    ///\param bins_1d The number of display bins for each 1-D histogram axis type
    std::map<Type, int> bins_1d {{Type::a, BIN1D}, {Type::b, BIN1D}, {Type::p, BIN1D}, {Type::x, BIN1D}, {Type::t, BIN1D}};

    ///\param bins_2d The number of display bins for each 2-D histogram axis type
    std::map<Type, int> bins_2d {{Type::a, BIN2D}, {Type::b, BIN2D}, {Type::x, BIN2D}, {Type::p, BIN2D}, {Type::t, BIN2D}};

    ///\param arcs The number of arcs in the BIFROST detector (5)
    int arcs;
    ///\param triplets The number of triplets in the BIFROST detector per arc (9)
    int triplets;
    ///\param tubes_per_triplet The number of tubes in each triplet (3)
    int tubes_per_triplet;
    ///\param pixels_per_tube The number of pixels in each tube (100)
    int pixels_per_tube;
    ///\param pixels_per_arc The number of pixels in each (triplet-)arc (2700)
    int pixels_per_arc;
    ///\param pixels_per_tube_arc The number of pixels in each tube-arc (900)
    int pixels_per_tube_arc;
    ///\param total_pixels The total number of pixels in the detector (13500)
    int total_pixels;

    ///\param calibration The calibration object to use for filtering data
    Calibration & calibration;

  public:
    Manager(int arcs, int triplets, int tubes, int pixels, Calibration & calib)
    : arcs(arcs), triplets(triplets),
      tubes_per_triplet{tubes}, pixels_per_tube{pixels}, calibration(calib)
      {
      // setup data objects ...
      for (auto data : {&everything, &included, &excluded}) {
        data->resize(key_count());
        for (int a = 0; a < arcs; ++a) {
          for (int t = 0; t < triplets; ++t) {
            for (auto k: TYPE1D) data->at(key(a, t, k)).resize(BIN1D, 0);
            for (auto k: TYPE2D) data->at(key(a, t, k)).resize(BIN2D * BIN2D, 0);
          }
        }
      }
      pixels_per_tube_arc = triplets * pixels_per_tube;
      pixels_per_arc = tubes_per_triplet * pixels_per_tube_arc;
      total_pixels = pixels_per_arc * arcs;
      pixel_data.resize(total_pixels, 0);
    }
    ~Manager() = default;

    ///\brief Calculate the group number from the arc and triplet numbers
    [[nodiscard]] int group(int arc, int triplet) const;

    ///\brief Replicate the EFU calculations to identify a unique pixel number
    ///\returns 0 if no valid pixel
    [[nodiscard]] int pixel(int arc, int triplet, int a, int b) const;

    ///\brief Determine if the charge division would give a pixel number
    [[nodiscard]] bool includes(int arc, int triplet, int a, int b) const;

    ///\brief Reset all histogram data to zeros
    void clear(){
      for (auto data: {&everything, &included, &excluded}) {
        for (auto &d: *data) std::fill(d.begin(), d.end(), 0);
      }
    }

    ///\brief Add a new data point to the histograms
    bool add(int arc, int triplet, int a, int b, double time);

    [[nodiscard]] double max(Filter) const;
    [[nodiscard]] double max(int arc, Filter) const;
    [[nodiscard]] double max(int arc, int triplet, Filter) const;
    [[nodiscard]] double max(int arc, Type t, Filter) const;
    [[nodiscard]] double max(int arc, int triplet, Type t, Filter) const;
    [[nodiscard]] double max(key_t k, Filter) const;

    [[nodiscard]] double min(Filter) const;
    [[nodiscard]] double min(int arc, Filter) const;
    [[nodiscard]] double min(int arc, int triplet, Filter) const;
    [[nodiscard]] double min(int arc, Type t, Filter) const;
    [[nodiscard]] double min(int arc, int triplet, Type t, Filter) const;
    [[nodiscard]] double min(key_t k, Filter) const;

    [[nodiscard]] D1 data_1D(int arc, int triplet, Type t, Filter) const;
    [[nodiscard]] D1 data_1D(key_t k, Filter) const;
    [[nodiscard]] D2 * data_2D(int arc, int triplet, Type t, Filter) const;
    [[nodiscard]] D2 * data_2D(key_t k, Filter) const;

    ///\brief Return the axis values for a given histogram type
    ///\param t the histogram type
    ///\param bins the number of bins to use for the axis, the current display bin count is used if not provided
    [[nodiscard]] AX axis(Type t, std::optional<int> bins = std::nullopt) const {
      if (!bins.has_value()) {
        if (is_1D(t)) {
          if (bins_1d.count(t)) bins = bins_1d.at(t);
        } else {
          if (bins_2d.count(t)) bins = bins_2d.at(t);
        }
      }
      AX x(bins.value());
      double range{1<<15}, start{0};
      if (Type::x == t){
        range = 2;
        start = -1;
      } else if (Type::p == t){
        range *= 2;
      } else if (Type::t == t){
        range = 1.0 / 14.0;
        start = 1.0 / 14.0 / 2.0 / (bins.value() + 1);
      }
      iota_step(x.begin(), x.end(), range / (bins.value() + 1), start);
      return x;
    }

    ///\brief Set the number of display bins to use for a given 1-D histogram axis type
    void set_bins_1d(Type t, int m){
      if (m > 0 && m <= BIN1D) bins_1d[t] = m;
    }

    ///\brief Set the number of display bins to use for a given 2-D histogram axis type
    void set_bins_2d(Type t, int m){
      if (m > 0 && m <= BIN2D) bins_2d[t] = m;
    }

    ///\brief Identify the data key for a given arc, triplet, and histogram type
    [[nodiscard]] key_t key(int arc, int triplet, Type t) const {
      if (arc < 0 || arc >= arcs){
        fmt::print("arc must be in (0, {}), given {}", arcs-1, arc);
        throw std::runtime_error(fmt::format("arc must be in (0, {}), given {}", arcs-1, arc));
      }
      if (triplet < 0 || triplet >= triplets) {
        fmt::print("triplet must be in (0, {}), given {}", triplets-1, triplet);
        throw std::runtime_error(fmt::format("triplet must be in (0, {}), given {}", triplets-1, triplet));
      }
      return static_cast<key_t>(t) * (arcs * triplets) + triplet * arcs + arc;
    }

    ///\brief Identify the type of histogram that a data key corresponds to
    [[nodiscard]] Type key_type(key_t k) const {
      return static_cast<Type>(k / (arcs * triplets));
    }
    ///\brief Identify the triplet number that a data key corresponds to
    [[nodiscard]] int key_triplet(key_t k) const {
      return static_cast<int>((k % (arcs * triplets)) / arcs);
    }
    ///\brief Identify the arc number that a data key corresponds to
    [[nodiscard]] int key_arc(key_t k) const {
      return static_cast<int>(k % arcs);
    }
    ///\brief Identify the number of data keys held by the DataManager
    [[nodiscard]] key_t key_count() const {
      return TYPECOUNT * arcs * triplets;
    }

  private:
    bool add_1D(int arc, int triplet, int a, int b, double time, bool allowed);
    bool add_2D(int arc, int triplet, int a, int b, double time, bool allowed);

    [[nodiscard]] int max_1D(key_t t, Filter) const;
    [[nodiscard]] int max_2D(key_t t, Filter) const;
    [[nodiscard]] int min_1D(key_t t, Filter) const;
    [[nodiscard]] int min_2D(key_t t, Filter) const;

    [[nodiscard]] std::pair<int, int> bins_2D(Type t) const{
      Type x{Type::unknown}, y;
      if (Type::ab == t){
        x = Type::a;
        y = Type::b;
      } else if (Type::pt == t){
        x = Type::p;
        y = Type::t;
      } else if (Type::xt == t){
        x = Type::x;
        y = Type::t;
      } else if (Type::xp == t){
        x = Type::x;
        y = Type::p;
      }
      if (x == Type::unknown) return std::make_pair(-1, -1);
      if (!bins_2d.count(x) || !bins_2d.count(y)) return std::make_pair(0, 0);
      return std::make_pair(bins_2d.at(x), bins_2d.at(y));
    }

  public:
    std::vector<unsigned long long> type_dimensions(Type type) const;

    void save_to(std::filesystem::path file, std::optional<std::string> group = std::nullopt) const;
    void save_to(hdf5::file::File file, std::optional<std::string> group = std::nullopt) const;
    void save_to(hdf5::node::Group group) const;
  };

}

///\brief Output stream operator for the bifrost::data::Type enum for nicer debugging and output
std::ostream & operator<<(std::ostream & os, ::bifrost::data::Type type);