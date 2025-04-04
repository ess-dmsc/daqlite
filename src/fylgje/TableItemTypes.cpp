// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
//
/// \file
//
/// \brief GUI table interface data types for Calibration information for the BIFROST detector
//===----------------------------------------------------------------------===//
#include "TableItemTypes.h"

IntTableItem::IntTableItem(qint32 data, bool editable){
  data_ = data;
  auto flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  if (editable){
  flags |= Qt::ItemIsEditable;
  }
  setFlags(flags);
  setSelected(false);
}

QVariant IntTableItem::data(int role) const {
  if (role == Qt::EditRole) return data_;
  if (role == Qt::DisplayRole) return data_;
  if (role == Qt::TextAlignmentRole) return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
  return QTableWidgetItem::data(role);
}

void IntTableItem::setData(int role, const QVariant &value){
  if (role == Qt::EditRole) {
    data_ = value.toInt();
    tableWidget()->itemChanged(this);
  }
}

bool IntTableItem::operator<(const IntTableItem & other) const {
  return data_ < other.data_;
}

OptIntItem::OptIntItem(std::optional<int> data){
  data_ = data;
  setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
  setSelected(false);
}

QVariant OptIntItem::data(int role) const {
  if (role == Qt::EditRole) return data_.has_value() ? data_.value() : QVariant();
  if (role == Qt::DisplayRole) return data_.has_value() ? data_.value() : QVariant();
  if (role == Qt::TextAlignmentRole) return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
  return QTableWidgetItem::data(role);
}

void OptIntItem::setData(int role, const QVariant &value){
  if (role == Qt::EditRole) {
    auto v = value.toInt();
    if (v == data_.value_or(std::numeric_limits<int>::lowest())){
      data_ = std::nullopt;
    } else {
      data_ = v;
    }
    setCalibrationUnit();
    tableWidget()->itemChanged(this);
  }
}

bool OptIntItem::operator<(const OptIntItem & other) const {
  if (data_.has_value() && other.data_.has_value()){
    return data_.value() < other.data_.value();
  }
  if (data_.has_value()) return false;
  return true;
}

FloatTableItem::FloatTableItem(float data, bool editable){
  data_ = data;
  auto flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  if (editable){
    flags |= Qt::ItemIsEditable;
  }
  setFlags(flags);
  setSelected(false);
}

QVariant FloatTableItem::data(int role) const {
  if (role == Qt::EditRole) return data_;
  if (role == Qt::DisplayRole) return data_;
  if (role == Qt::TextAlignmentRole) return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
  return QTableWidgetItem::data(role);
}

void FloatTableItem::setData(int role, const QVariant &value){
  if (role == Qt::EditRole) {
    data_ = value.toFloat();
    setCalibrationUnit();
    tableWidget()->itemChanged(this);
  }
}

bool FloatTableItem::operator<(const FloatTableItem & other) const {
  return data_ < other.data_;
}

OptDoubleItem::OptDoubleItem(std::optional<double> data){
  data_ = data;
  setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
  setSelected(false);
}

QVariant OptDoubleItem::data(int role) const {
  if (role == Qt::EditRole) return data_.has_value() ? data_.value() : QVariant();
  if (role == Qt::DisplayRole) return data_.has_value() ? data_.value() : QVariant();
  if (role == Qt::TextAlignmentRole) return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
  return QTableWidgetItem::data(role);
}

void OptDoubleItem::setData(int role, const QVariant &value){
  if (role == Qt::EditRole) {
    auto v = value.toDouble();
    if (v == data_.value_or(std::numeric_limits<double>::lowest())){
      data_ = std::nullopt;
    } else {
      data_ = v;
    }
    setCalibrationUnit();
    tableWidget()->itemChanged(this);
  }
}

bool OptDoubleItem::operator<(const OptDoubleItem & other) const {
  if (data_.has_value() && other.data_.has_value()){
    return data_.value() < other.data_.value();
  }
  if (data_.has_value()) return false;
  return true;
}