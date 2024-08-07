// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file CDTGraph
///
/// \brief Class for handling plotting of CDT readouts
///
//===----------------------------------------------------------------------===//

#include <CDTGraph.h>
#include <fmt/format.h>


bool CDTGraph::ignoreEntry(int Ring, int FEN) {
  //std::vector<int> FENS {12, 10, 10, 6, 6, 6, 12, 8, 9, 9, 9};
  if ((FEN >= 4) or (Ring > 4)) {
    return true;
  }
  return false;
}


void CDTGraph::setupPlot(QGridLayout * Layout) {
  for (int i = 0; i < NumChannels; i++) {
    x.push_back(i);
    y0.push_back(0);
    y1.push_back(0);
  }

  for (int Ring = 0; Ring < 11; Ring++) {
    for (int FEN = 0; FEN < 12; FEN++) {
      if (ignoreEntry(Ring, FEN)) {
        continue;
      }
      addGraph(Layout, Ring, FEN);
    }
  }

  // Add final row of buttons (some inherited from base)
  QPushButton *btnDead = new QPushButton("Dead");
  QPushButton *btnClear = new QPushButton("Clear");

  QHBoxLayout *hblayout = new QHBoxLayout();
  hblayout->addWidget(btnToggle);
  hblayout->addWidget(btnToggleLegend);
  hblayout->addWidget(btnLogLin);
  Layout->addLayout(hblayout, 11, 0);

  QHBoxLayout *hblayout2 = new QHBoxLayout();
  hblayout2->addWidget(btnDead);
  hblayout2->addWidget(btnClear);
  hblayout2->addWidget(btnQuit);
  Layout->addLayout(hblayout2, 11, 1);

  connect(btnDead, SIGNAL(clicked()), this, SLOT(dead()));
  connect(btnClear, SIGNAL(clicked()), this, SLOT(clear()));


  /// Update timer
  QTimer *timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &CDTGraph::updatePlots);
  timer->start(1000);
}


  /// \brief
void CDTGraph::addGraph(QGridLayout * Layout, int Ring, int FEN) {
  QCustomPlot * QCP = new QCustomPlot();
  int GraphKey = Ring * 256 + FEN;
  Graphs[GraphKey] = QCP;

  addText(QCP, fmt::format("R{}/F{}", Ring, FEN));
  QCP->legend->setBorderPen(QPen(Qt::transparent));
  if (ToggleLegend) {
    QCP->legend->setVisible(true);
  }
  QCP->xAxis->setRange(0, NumChannels - 1);
  QCP->yAxis->setRange(0, 5);
  QCP->addGraph();
  QCP->graph(0)->setName("catode");
  QCP->graph(0)->setData(x, y0);
  QCP->graph(0)->setLineStyle(QCPGraph::LineStyle::lsStepLeft);
  QCP->graph(0)->setBrush(QBrush(QColor(20,50,255,40)));
  QCP->addGraph();
  QCP->graph(1)->setName("anode");
  QCP->graph(1)->setData(x, y1);
  QCP->graph(1)->setLineStyle(QCPGraph::LineStyle::lsStepLeft);
  QCP->graph(1)->setBrush(QBrush(QColor(255,50,20,40)));
  Layout->addWidget(QCP, Ring, FEN);
}


void CDTGraph::updatePlots() {
  for (int Ring = 0; Ring < 11; Ring++) {
    for (int FEN = 0; FEN < 12; FEN++) {
      if (ignoreEntry(Ring, FEN)) {
        continue;
      }
      int GraphKey = Ring * 256 + FEN;
      auto qp = Graphs[GraphKey];

      for (int i= 0; i < NumChannels; i++) {
        y0[i] = WThread->Consumer->CDTHistogram[Ring][FEN][0][i];
        y1[i] = WThread->Consumer->CDTHistogram[Ring][FEN][1][i];
      }

      qp->graph(0)->setData(x, y0);
      qp->graph(1)->setData(x, y1);

      updatePlotPresentation(qp);

      qp->replot();
    }
  }
}


///\brief Button signals below
void CDTGraph::dead() {

  for (int Ring = 0; Ring < 11; Ring++) {
    for (int FEN = 0; FEN < 12; FEN++) {
      if (ignoreEntry(Ring, FEN)) {
        continue;
      }

      int DeadCathodes{0};
      int DeadAnodes{0};
      for (int i= 0; i < NumChannels; i++) {
        if (WThread->Consumer->CDTHistogram[Ring][FEN][0][i] == 0) {
          DeadCathodes++;
        }
        if (WThread->Consumer->CDTHistogram[Ring][FEN][1][i] == 0) {
          DeadAnodes++;
        }
      }
      qDebug("Ring %d, FEN %d - dead cathodes %d, dead anodes %d", Ring, FEN, DeadCathodes, DeadAnodes);
    }
  }
}


void CDTGraph::clear() {
  memset(WThread->Consumer->CDTHistogram, 0,
    sizeof(WThread->Consumer->CDTHistogram));
  updatePlots();
}
