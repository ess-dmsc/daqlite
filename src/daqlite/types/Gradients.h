// Copyright (C) 2020 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file Gradients.h
///
/// Gradients used in daqlite plots
//===----------------------------------------------------------------------===//

#pragma once

#include <QPlot/qcustomplot/qcustomplot.h>

#include <map>

inline std::map<std::string, QCPColorGradient> GRADIENTS = {
  {"candy",     QCPColorGradient::gpCandy},
  {"cold",      QCPColorGradient::gpCold},
  {"geography", QCPColorGradient::gpGeography},
  {"grayscale", QCPColorGradient::gpGrayscale},
  {"hot",       QCPColorGradient::gpHot},
  {"hues",      QCPColorGradient::gpHues},
  {"ion",       QCPColorGradient::gpIon},
  {"jet",       QCPColorGradient::gpJet},
  {"night",     QCPColorGradient::gpNight},
  {"polar",     QCPColorGradient::gpPolar},
  {"spectrum",  QCPColorGradient::gpSpectrum},
  {"thermal",   QCPColorGradient::gpThermal},
};


