// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file HelpWindow.cpp
///
//===----------------------------------------------------------------------===//

#include <HelpWindow.h>

#include <fmt/format.h>

#include <QGuiApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QScreen>
#include <QScrollBar>
#include <QToolButton>

namespace {
std::string HELP_TEXT = R"(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
  table {{
    border-collapse: collapse;
    width: 100%;
    border: 1px solid #ddd;
  }}
  th, td {{
    border: 1px solid #000;
    padding: 4px;
  }}
  th {{
    background-color: #f2f2f2;
  }}
  td:first-child {{
    text-align: left;
  }}
  td:not(:first-child) {{
    text-align: center;
  }}
</style>
</head>
<body>

<table>
  <!-- Header -->
  {}

  <!-- Rows -->
  {}
</table>

</body>
</html>
)";

std::string HEADER = R"(
  <tr>
    <th>{}</th>
    <th>{}</th>
    <th>{}</th>
  </tr>
)";

std::string ROW = R"(
  <tr>
    <td>{}</td>
    <td>{}</td>
    <td>{}</td>
  </tr>
)";
}


HelpWindow::HelpWindow(QWidget *parent)
  : QTextEdit(parent) {
  // Properties
  setWindowTitle("Help");
  setReadOnly(true);

  // Scale to smaller font
  auto f = font();
  f.setPointSizeF(f.pointSizeF() * std::pow(2.0, -0.25));
  setFont(f);

  // Row entries
  std::vector<std::vector<std::string>> data = {
    {"Reset view",          "Ctrl+R",          "Cmd+R"},
    {"Store current view",  "Ctrl+S",          "Cmd+S"},
    {"Draw zoom rectangle", "Ctrl+Left mouse", "Cmd+Left mouse"},
    {"Invert gradient",     "Alt+I",           "Opt+I"},
    {"Log scale",           "Alt+L",           "Opt+L"},
    {"Auto scale axes",     "Alt+X or Alt+Y",  "Opt+X or Opt+Y"},
    {"Clear the plot",      "Alt+C",           "Opt+C"},
    {"Quit daqlite",        "Alt+Q",           "Opt+Q"},
    {"Show help",           "Alt+H",           "Opt+H"},
  };

  // Setup HTML help text
  std::string header =  fmt::format(HEADER, "Action", "Linux", "Mac");
  std::string rows =  "";
  for (const auto &row: data) {
    rows += fmt::format(ROW, row[0], row[1], row[2]);
  }
  std::string html = fmt::format(HELP_TEXT, header, rows);
  setHtml(QString::fromStdString(html));

  // We extract/steal the clear button from a QLineEdit and use this as
  // hide/close button for the help window
  mLineEdit = new QLineEdit(this);
  mLineEdit->setClearButtonEnabled(true);
  mLineEdit->setText("Daqlite rocks!");
  mLineEdit->hide();

  // Use clear button for hiding help window
  mClearButton = mLineEdit->findChild<QToolButton *>();
  mClearButton->setParent(this);
  mClearButton->disconnect();
  connect(mClearButton, &QToolButton::clicked, this, &HelpWindow::hide);
}

void HelpWindow::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Escape) {
    hide();
  }

  QTextEdit::keyPressEvent(event);
}

void HelpWindow::showEvent(QShowEvent *event) {
  QTextEdit::showEvent(event);

  mClearButton->show();
  updateClearPosition();
}

void HelpWindow::resizeEvent(QResizeEvent *event) {
  QTextEdit::resizeEvent(event);

  updateClearPosition();
}

QSize HelpWindow::sizeHint() const {
  // Adjust size, then calculate new width that includes the clearbutton
  document()->adjustSize();
  int w = document()->idealWidth() + mClearButton->rect().width();
  document()->setTextWidth(w);

  return QSize(w, document()->size().height() + 2);

}

void HelpWindow::updateClearPosition()
{
  const QScrollBar *vs = verticalScrollBar();
  int Delta = vs->isVisible() ? vs->width() : 0;
  auto pos = rect().topRight() - mClearButton->rect().topRight();
  mClearButton->move(pos + QPoint(-2 - Delta, 4));
}

void HelpWindow::placeHelp(const QPoint &pos) {
  const QScreen *screen = QGuiApplication::screenAt(pos);
  const QRect screenRect = screen->geometry();

  QPoint offset(4, 4);

  QPoint p = pos;
  if (p.x() + this->width() > screenRect.x() + screenRect.width())
    p.rx() -= 4 + this->width();
  if (p.y() + this->height() > screenRect.y() + screenRect.height())
    p.ry() -= 24 + this->height();
  if (p.y() < screenRect.y())
    p.setY(screenRect.y());
  if (p.x() + this->width() > screenRect.x() + screenRect.width())
    p.setX(screenRect.x() + screenRect.width() - this->width());
  if (p.x() < screenRect.x())
    p.setX(screenRect.x());
  if (p.y() + this->height() > screenRect.y() + screenRect.height())
    p.setY(screenRect.y() + screenRect.height() - this->height());

  this->move(p);
}