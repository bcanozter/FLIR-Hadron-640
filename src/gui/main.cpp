#include <QApplication>

#include "MainWindow.h"

// qtlogging
constexpr char kMessagePattern[] =
    "%{time hh:mm:ss.zzz} \x1b[90m(%{file}:%{line})\x1b[0m"
    "%{if-debug}\x1b[90m[D] %{message}\x1b[0m%{endif}"
    "%{if-info}\x1b[36m[I] %{message}\x1b[0m%{endif}"
    "%{if-warning}\x1b[33m[W] %{message}\x1b[0m%{endif}"
    "%{if-critical}\x1b[31m[C] %{message}\x1b[0m%{endif}"
    "%{if-fatal}\x1b[97;41m[F] %{message}\x1b[0m%{endif}";

int main(int argc, char *argv[]) {

  qSetMessagePattern(kMessagePattern);

  QApplication app(argc, argv);
  QApplication::setApplicationName("Hadron Thermal Lab");

  MainWindow window;
  window.show();

  return QApplication::exec();
}
