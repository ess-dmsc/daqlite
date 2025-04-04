// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
//
/// \file
//
/// \brief Calibration information for the BIFROST detector
//===----------------------------------------------------------------------===//
#include <iomanip>
#include <sstream>
#include <iostream>
#include "Calibration.h"

///\brief CalibrationGroup creation, checking CalibrationUnit vector is sorted by index and that all indexes are present
CalibrationGroup::CalibrationGroup(int i, std::vector<CalibrationUnit> && els)
: index{i}, elements{std::move(els)} {
  // check that the provided elements have exclusive position ranges, and are sorted + all present
  check_sorted_index_is_iota(els, "unit");
  for (auto ptr = els.begin(); ptr != els.end() && ptr+1 != els.end(); ++ptr){
    auto nxt = ptr+1;
    if (ptr->maxEdge() > nxt->minEdge()) {
      std::stringstream ss;
      ss << fmt::format("Units ({}, {}) ({}, {}) error!", ptr->minEdge(), ptr->maxEdge(), nxt->minEdge(), nxt->maxEdge());
      throw std::runtime_error(ss.str());
    }
  }
}

///\brief Calibration creation, with groupCount groups, each with elementCount elements
Calibration::Calibration(int group_count, int element_count)
    : version_{0}, date_{std::time({})}, info_{"generated"}, instrument_{"generated"} {
  // use the same element ranges for all groups
  double step = 1.0 / element_count;
  std::vector<double> edges(1, 0.);
  edges.reserve(element_count + 1);
  for (int i=0; i<element_count - 1; ++i){
    edges.push_back(edges.back() + step);
  }
  edges.push_back(1.0);
  groups_.reserve(group_count);
  auto make_elements = [&](){
    std::vector<CalibrationUnit> elements;
    elements.reserve(element_count);
    for (int j=0; j<element_count; ++j){
      elements.emplace_back(j, edges[j], edges[j + 1]);
    }
    return elements;
  };
  for (int i=0; i<group_count; ++i){
    groups_.emplace_back(i, make_elements());
  }
}

///\brief Retrieve the Calibration date-time point as a string
[[nodiscard]] std::string Calibration::dateString() const {
  char timeString[std::size("yyyy-mm-ddThh:mm:ssZ")];
  std::strftime(std::data(timeString), std::size(timeString), "%FT%TZ", std::gmtime(&date_));
  return timeString;
}

///\brief Set the Calibration date-time point from a string
void Calibration::setDate(const std::string & date_str) {
  struct std::tm tm{};
  std::istringstream ss(date_str);
  ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
  if (ss.fail()){
    throw std::runtime_error(fmt::format("Failed to parse UTC time from '{}'", date_str));
  }
  date_ = timegm(&tm);
  if (ss.peek() == '.') {
    ss.ignore();
    double fractional;
    ss >> fractional;
    if (ss.fail()) {
      std::cout << fmt::format("Failed to parse fractional seconds from '{}'", date_str) << std::endl;
    } else {
      auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<double>(fractional));
      date_ += std::chrono::duration_cast<std::chrono::seconds>(microseconds).count();
    }
  }
}

void Calibration::setGroups(Groups groups) {
  // ensure consistent group sizes
  auto els = groups.empty() ? 0 : groups.front().size();
  if (std::any_of(groups.begin(), groups.end(), [els](const auto & p){return p.size() != els;})){
    auto message = fmt::format("Inconsistent element sized group(s)! All should match first group, {} elements", els);
    throw std::runtime_error(message);
  }
  // ensure the groups are sorted by their index
  std::sort(groups.begin(), groups.end(),
            [](const auto & a, const auto & b){return a.index < b.index;}
  );
  // and that all indexes are present
  check_sorted_index_is_iota(groups, "group");
  groups_ = std::move(groups);
}


[[nodiscard]] double Calibration::posCorrection(int group, int unit, double pos) const {
  auto corrected = pos - groups_[group].elements[unit].positionCorrection(pos);
  return corrected < 0 ? 0 : corrected > 1 ? 1 : corrected;
}

[[nodiscard]] int Calibration::getUnitId(int group, double pos) const {
  if (group >= static_cast<int>(groups_.size())){
    return -1;
  }
  for (const auto & el: groups_[group].elements){
    if (el.contains(pos)) {
      return el.index;
    }
  }
  return -1;
}

[[nodiscard]] double Calibration::unitPosition(int group, int unit, double global_position) const {
  if (group >= static_cast<int>(groups_.size())){
    return -1;
  }
  if (unit >= static_cast<int>(groups_[group].elements.size())){
    return -1;
  }
  return groups_[group].elements[unit].unitPosition(global_position);
}

/* This pulseHeightOK function is commented out because it was decided to implement the threshold
 * as part of the *configuration* JSON instead of the calibration JSON. This is because the threshold
 * as currently envisaged is a constant for all detectors of a given type, and not a calibration parameter.
 *
 * It is left here as a reminder to re-implement this functionality, using the configuration value,
 * to allow user-interaction when setting its value.
 * */

//[[nodiscard]] int Calibration::pulseHeightOK(int group, int unit, int pulse_height) const {
//  if (group >= static_cast<int>(groups_.size())){
//    return false;
//  }
//  if (unit >= static_cast<int>(groups_[group].elements.size())){
//    return false;
//  }
//  return groups_[group].elements[unit].pulse_height_ok(pulse_height);
//}


///\brief CalibrationGroup JSON serialization
///\param json_out the json object to serialize to
///\param group_in the CalibrationGroup object to serialize
///\note The name of this function is set by the nlohmann::json library
///      https://json.nlohmann.me/api/adl_serializer/to_json/
[[maybe_unused]] void to_json(nlohmann::json & json_out, const CalibrationGroup & group_in){
  auto size = group_in.elements.size();
  std::vector<std::pair<double, double>> division(size);
  std::vector<std::array<double, 4>> polynomial(size);
  for (const auto & el: group_in.elements){
    division[el.index] = {el.left, el.right};
    polynomial[el.index] = {el.c0.value_or(0), el.c1.value_or(0), el.c2.value_or(0), el.c3.value_or(0)};
  }
  json_out = nlohmann::json {
    {"groupindex", group_in.index},
    {"intervals", division},
    {"polynomials", polynomial}
  };
}


///\brief CalibrationGroup JSON deserialization
///\param json_in the json object to deserialize from
///\param group_out the CalibrationGroup object to deserialize into
///\note The name of this function is set by the nlohmann::json library
///      https://json.nlohmann.me/api/adl_serializer/from_json/
[[maybe_unused]] void from_json(const nlohmann::json & json_in, CalibrationGroup & group_out){
  auto index_name = "groupindex";
  auto index = json_in[index_name].get<int>();
  auto divisions = json_in["intervals"].get<std::vector<std::pair<double, double>>>();
  auto polynomials = json_in["polynomials"].get<std::vector<std::array<double, 4>>>();
  auto size = divisions.size();
  std::vector<CalibrationUnit> elements;
  elements.reserve(size);
  for (size_t i=0; i<size; ++i){
    elements.emplace_back(i, divisions.at(i), polynomials.at(i));
  }
  group_out.index = index;
  group_out.elements = elements;
}


///\brief Calibration JSON serialization
///\param json_out the json object to serialize to
///\param calibration_in the Calibration object to serialize
///\note The name of this function is set by the nlohmann::json library
///      https://json.nlohmann.me/api/adl_serializer/to_json/
[[maybe_unused]] void to_json(nlohmann::json & json_out, const Calibration & calibration_in){
  json_out = nlohmann::json{
    {"Calibration", {
      {"version", calibration_in.version()},
      {"date", calibration_in.dateString()},
      {"info", calibration_in.info()},
      {"instrument", calibration_in.instrument()},
      {"groups", calibration_in.groupCount()},
      {"groupsize", calibration_in.elementCount()},
      {"parameters", calibration_in.groups()}}
    }
  };
}


///\brief Calibration JSON deserialization
///\param json_in the json object to deserialize from
///\param calibration_out the Calibration object to deserialize into
///\note The name of this function is set by the nlohmann::json library
///      https://json.nlohmann.me/api/adl_serializer/from_json/
[[maybe_unused]] void from_json(const nlohmann::json & json_in, Calibration & calibration_out){
  auto json_calibration_in = json_in["Calibration"];
  auto parameters = json_calibration_in["Parameters"].get<Calibration::Groups>();
  auto groups_name = "groups";
  if (auto groups = json_calibration_in[groups_name].get<int>(); groups != static_cast<int>(parameters.size())){
    auto message = fmt::format("Expected {} groups but json specifies {}={} instead!", parameters.size(), groups_name, groups);
    throw std::runtime_error(message);
  }
  auto els = parameters.empty() ? 0 : parameters.front().size();
  auto elements_name = "groupsize"; // "units";
  if (auto elements = json_calibration_in[elements_name].get<int>(); elements != static_cast<int>(els)){
    auto message = fmt::format("Expected {} units per group but json specifies {}={} instead!", els, elements_name, elements);
    throw std::runtime_error(message);
  }
  calibration_out.setVersion(json_calibration_in["version"].get<int>());
  calibration_out.setDate(json_calibration_in["date"].get<std::string>());
  calibration_out.setInfo(json_calibration_in["info"].get<std::string>());
  calibration_out.setInstrument(json_calibration_in["instrument"].get<std::string>());
  calibration_out.setGroups(parameters);
}