// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Calibration information for a detector consisting of multiple groups
//===----------------------------------------------------------------------===//
#pragma once
#include <JsonFile.h>
#include <string>
#include <iomanip>
#include <optional>
#include <cstring>


///\brief Raise a runtime error if the provided vector is not sorted by index
///\tparam Obj the type of object in the vector
///\param vec the vector to check
///\param name the name of the vector for the error message
///\throws std::runtime_error if the vector is not sorted by index
///\note The integer index must be stored in each `Obj` as a public member variable named `index`
template<class Obj> void check_sorted_index_is_iota(const std::vector<Obj>& vec, const std::string & name){
  std::vector<int> indexes(vec.size(), 0);
  std::iota(indexes.begin(), indexes.end(), 0);
  auto match = std::mismatch(vec.begin(), vec.end(), indexes.begin(),
                             [](const auto & g, const auto & i){return g.index ==i;});
  if (match.first != vec.end()){
    std::stringstream ss;
    ss << "Incorrect " << name << " group indexing!" << std::endl;
    ss << "[";
    if (match.first != vec.begin()){
      ss << "...,";
    }
    for (auto p = match.first; p != vec.end(); ++p){
      ss << p->index << ",";
    }
    ss.seekp(-1, std::stringstream::cur);
    ss << "] does not match\n[";
    if (match.second != indexes.begin()){
      ss << "...,";
    }
    for (auto p = match.second; p != indexes.end(); ++p){
      ss << *p << ",";
    }
    ss.seekp(-1, std::stringstream::cur);
    ss << "]\n";
    throw std::runtime_error(ss.str());
  }
}

///\brief The Calibration of a single unit, with an index, position range, and correction polynomial
///\note A CalibrationUnit is a single unit of a CalibrationGroup.
///      For an instrument like BIFROST it is a single tube of a 3-unit triplet.
///      For an instrument like LOKI it is a single straw of a 7-straw tube.
class CalibrationUnit {
public:
  ///\param index the unique identifier for this unit
  int index{-1};
  ///\param left the left edge of the position range
  ///\param right the right edge of the position range
  // pos(x) = (x - left) / (right - left)  -- assuming x between left and right
  double left{0}, right{1};
  ///\param c0 the constant term of the correction polynomial, equivalent to zero if not provided
  ///\param c1 the linear term of the correction polynomial, equivalent to zero if not provided
  ///\param c2 the quadratic term of the correction polynomial, equivalent to zero if not provided
  ///\param c3 the cubic term of the correction polynomial, equivalent to zero if not provided
  // linear_pos(pos) = pos - (c0 + c1 pos + c2 pos^2 + c3 pos^3)
  std::optional<double> c0{std::nullopt}, c1{std::nullopt}, c2{std::nullopt}, c3{std::nullopt};

  ///\brief Default constructor
  CalibrationUnit() = default;
  ///\brief Constructor with all required values
  CalibrationUnit(int i, double l, double r): index{i}, left{l}, right{r} {}
  ///\brief Constructor with all required and all optional values
  CalibrationUnit(int i, std::pair<double, double> lr, std::array<double, 4> p)
      : index{i}, left{lr.first}, right{lr.second}, c0{p[0]}, c1{p[1]}, c2{p[2]}, c3{p[3]} {}

  ///\brief The edges are not necessarily in order, so this function returns the minimum
  ///\returns the minimum of left and right
  [[nodiscard]] inline double minEdge() const {return left < right ? left : right;}

  ///\brief The edges are not necessarily in order, so this function returns the maximum
  ///\returns the maximum of left and right
  [[nodiscard]] inline double maxEdge() const {return left < right ? right : left;}

  ///\brief Calculate the correction polynomial value for the specified position
  ///\param x a value in the range (0, 1) inclusive
  ///\returns c0 + c1 * x + c2 * x^2 + c3 * x^3
  ///\note Since all coefficients are optional, any missing values are replaced by 0.
  [[nodiscard]] inline double positionCorrection(double x) const{
    return c0.value_or(0.) + x * (c1.value_or(0.) + x * (c2.value_or(0.) + x * (c3.value_or(0.))));
  }

  ///\brief Determine if this CalibrationUnit covers the provided position
  ///\param x a position within the containing Group
  ///\returns true if x lies within (left, right)
  [[nodiscard]] inline bool contains(double x) const {
    return (std::min(left, right) <= x) && (x <= std::max(left, right));
  }

  ///\brief Find the unit position for the specified global position
  ///\param global_position a value in the range (0, 1) inclusive, like for `contains`
  ///\returns (x - left) / (right - left) clamped to the range (0 ,1)
  ///\note It is allowed for the unit position and global position to have opposite senses
  ///      and no check is performed to ensure the global position 'belongs' to this Unit.
  [[nodiscard]] inline double unitPosition(double global_position) const {
    auto x = (global_position - left) / (right - left);
    return x < 0 ? 0 : x > 1 ? 1 : x;
  }
};


///\brief A CalibrationGroup, with an index and a vector of CalibrationUnits
///\note A CalibrationGroup is a group of CalibrationUnits.
///      For an instrument like BIFROST a group is a triplet, with three PSD tube units.
///      For an instrument like LOKI a group is a tube, with seven straw units.
class CalibrationGroup {
public:
  ///\param index the unique identifier for this group
  int index{-1};
  ///\param elements the vector of CalibrationUnits in this group
  std::vector<CalibrationUnit> elements;
  CalibrationGroup() = default;
  CalibrationGroup(int i, std::vector<CalibrationUnit> && els);
  [[nodiscard]] size_t size() const {return elements.size();}
};


///\brief A Calibration, with a version, date, info, instrument, and vector of CalibrationGroups
///\note A Calibration is a collection of CalibrationGroups, each of which is a collection of CalibrationUnits.
///      BIFROST consists of a single Calibration, with a 45 groups, each of three units.
class Calibration {
public:
  using Groups = std::vector<CalibrationGroup>;

  Calibration() = default;
  Calibration(int group_count, int element_count);

  ///\brief Get the version of the calibration schema
  [[nodiscard]] int version() const {return version_;}

  ///\brief Set the version of the calibration schema
  void setVersion(int v) { version_ = v;}

  ///\brief Get the date and time of the calibration
  [[nodiscard]] std::time_t date() const {return date_;}

  ///\brief Set the date and time of the calibration to the current time
  void setDate() { date_ = std::time({});}

  ///\brief Set the date and time of the calibration
  [[maybe_unused]] void setDate(std::time_t t) { date_ = t;}

  ///\brief Get the date and time of the calibration as a string
  [[nodiscard]] std::string dateString() const;

  ///\brief Set the date and time of the calibration from a string
  void setDate(const std::string & date_str);

  ///\brief Get the description of the calibration
  [[nodiscard]] const std::string & info() const {return info_;}

  ///\brief Set the description of the calibration
  void setInfo(const std::string & i) { info_ = i;}

  ///\brief Get the name of the instrument the calibration applies to
  [[nodiscard]] const std::string & instrument() const {return instrument_;}

  ///\brief Set the name of the instrument the calibration applies to
  void setInstrument(const std::string & i) { instrument_ = i;}

  ///\brief Get the vector of CalibrationGroups
  [[nodiscard]] const Groups & groups() const {return groups_;}

  ///\brief Set the vector of CalibrationGroups
  void setGroups(Groups groups);

  ///\brief Get the number of groups
  [[nodiscard]] size_t groupCount() const {return groups_.size();}

  ///\brief Get the number of elements in each group
  [[nodiscard]] size_t elementCount() const {return groups_.empty() ? 0 : groups_.front().size();}

  ///\brief Get the corrected position for the specified group, unit, and (in-unit) position
  [[nodiscard]] double posCorrection(int group, int unit, double pos) const;

  ///\brief Get the unit number for the specified group and global position
  [[nodiscard]] int getUnitId(int group, double pos) const;

  ///\brief Get the unit position for the specified group, unit, and global position
  [[nodiscard]] double unitPosition(int group, int unit, double global_position) const;

  ///\brief Get a pointer to the specified CalibrationUnit
  ///\note The returned pointer lifetime must not exceed the lifetime of the Calibration object
  ///      to avoid memory access errors. It is used in the GUI to update the calibration values
  ///      when a user enters new values into a table.
  [[nodiscard]] CalibrationUnit * unitPointer(int group, int unit) {
    return &(groups_[group].elements[unit]);
  }

private:
  ///\param version_ the calibration-schema version
  int version_{};
  ///\param date_ the date and time of the calibration
  std::time_t date_{};
  ///\param info_ a description of the calibration
  std::string info_;
  ///\param instrument_ the name of the instrument being calibrated, likely one of the ESS instruments
  std::string instrument_;
  ///\param groups_ the vector of CalibrationGroups
  Groups groups_;
};

///\brief CalibrationGroup JSON serialization
///\param json_out the json object to serialize to
///\param group_in the CalibrationGroup object to serialize
///\note The name of this function is set by the nlohmann::json library
///      https://json.nlohmann.me/api/adl_serializer/to_json/
[[maybe_unused]] void to_json(nlohmann::json &, const CalibrationGroup &);

///\brief CalibrationGroup JSON deserialization
///\param json_in the json object to deserialize from
///\param group_out the CalibrationGroup object to deserialize into
///\note The name of this function is set by the nlohmann::json library
///      https://json.nlohmann.me/api/adl_serializer/from_json/
[[maybe_unused]] void from_json(const nlohmann::json &, CalibrationGroup &);

///\brief Calibration JSON serialization
///\param json_out the json object to serialize to
///\param calibration_in the Calibration object to serialize
///\note The name of this function is set by the nlohmann::json library
///      https://json.nlohmann.me/api/adl_serializer/to_json/
[[maybe_unused]] void to_json(nlohmann::json &, const Calibration &);

///\brief Calibration JSON deserialization
///\param json_in the json object to deserialize from
///\param calibration_out the Calibration object to deserialize into
///\note The name of this function is set by the nlohmann::json library
///      https://json.nlohmann.me/api/adl_serializer/from_json/
[[maybe_unused]] void from_json(const nlohmann::json &, Calibration &);
