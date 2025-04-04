// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Custom QTableWidgetItem types for the Fylgje application
//===----------------------------------------------------------------------===//
#pragma once
#include <QTableWidgetItem>
#include <optional>
#include <iostream>
#include "Calibration.h"

///\brief Base class for all table items in the Fylgje application
class FylgjeTableItem: public QTableWidgetItem {
public:
  std::string CalibrationUnitStr(){return "";}
protected:
  virtual void setCalibrationUnit(){}
};

///\brief Table item for integer values
class IntTableItem: public FylgjeTableItem {
public:
  IntTableItem(qint32 data=0, bool editable=true);
  QVariant data(int role) const;
  void setData(int role, const QVariant &value);
  bool operator<(const IntTableItem & other) const;
protected:
  qint32 data_;
};

///\brief Table item for optional integer values
class OptIntItem: public FylgjeTableItem {
public:
  OptIntItem(std::optional<int> data);
  QVariant data(int role) const;
  void setData(int role, const QVariant &value);
  bool operator<(const OptIntItem & other) const;
protected:
  std::optional<int> data_;
};

///\brief Table item for float values
class FloatTableItem: public FylgjeTableItem {
public:
  FloatTableItem(float data=0, bool editable=true);
  QVariant data(int role) const;
  void setData(int role, const QVariant &value);
  bool operator<(const FloatTableItem & other) const;
protected:
  float data_;
};

///\brief Table item for optional double values
class OptDoubleItem: public FylgjeTableItem {
public:
  OptDoubleItem(std::optional<double> data=0);
  QVariant data(int role) const;
  void setData(int role, const QVariant &value);
  bool operator<(const OptDoubleItem & other) const;
protected:
  std::optional<double> data_;
};

///\brief Table float value with callback to update the calibration unit left limit value
class CalibrationUnitLeftItem: public FloatTableItem {
public:
  CalibrationUnitLeftItem(CalibrationUnit * unit): FloatTableItem(unit->left), unit_(unit){}
protected:
  void setCalibrationUnit(){
    unit_->left = data_;
  }
  CalibrationUnit * unit_;
};

///\brief Table float value with callback to update the calibration unit right limit value
class CalibrationUnitRightItem: public FloatTableItem {
public:
  CalibrationUnitRightItem(CalibrationUnit * unit): FloatTableItem(unit->right), unit_(unit){}
protected:
  void setCalibrationUnit(){
    unit_->right = data_;
  }
  CalibrationUnit * unit_;
};

///\brief Table double value with callback to update the calibration unit constant position correction value
class CalibrationUnitC0Item: public OptDoubleItem {
public:
  CalibrationUnitC0Item(CalibrationUnit * unit): OptDoubleItem(unit->c0), unit_(unit){}
protected:
  void setCalibrationUnit(){
    unit_->c0 = data_;
  }
  CalibrationUnit * unit_;
};

///\brief Table double value with callback to update the calibration unit linear position correction value
class CalibrationUnitC1Item: public OptDoubleItem {
public:
  CalibrationUnitC1Item(CalibrationUnit * unit): OptDoubleItem(unit->c1), unit_(unit){}
protected:
  void setCalibrationUnit(){
    unit_->c1 = data_;
  }
  CalibrationUnit * unit_;
};

///\brief Table double value with callback to update the calibration unit quadratic position correction value
class CalibrationUnitC2Item: public OptDoubleItem {
public:
  CalibrationUnitC2Item(CalibrationUnit * unit): OptDoubleItem(unit->c2), unit_(unit){}
protected:
  void setCalibrationUnit(){
    unit_->c2 = data_;
  }
  CalibrationUnit * unit_;
};

///\brief Table double value with callback to update the calibration unit cubic position correction value
class CalibrationUnitC3Item: public OptDoubleItem {
public:
  CalibrationUnitC3Item(CalibrationUnit * unit): OptDoubleItem(unit->c3), unit_(unit){}
protected:
  void setCalibrationUnit(){
    unit_->c3 = data_;
  }
  CalibrationUnit * unit_;
};
