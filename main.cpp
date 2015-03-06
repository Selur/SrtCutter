#include "cutters/SrtCutter.h"
#include "cutters/AssCutter.h"

#include <QtCore>
#include <QString>
#include <QStringList>
#include <QCoreApplication>

#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);

  //"-i INPUT -cut from-to;..."
  if (argc < 5) {
    cout << "SubtitleCutter by Selur" << endl;
    cout << "Usage:" << endl;
    cout << "SubtitleCutter -i \"Path to input file\" -cut from-to;..." << endl;
    cout << "SubtitleCutter -i \"Path to input file\" -cut \"Path to cut list file\"" << endl;
    cout << endl;
    cout << "Supported subtitle formats: ";
    cout << "srt";
    cout << endl;
    return -1;
  }
  QString input, tmp, output;
  QStringList cutList;
  bool debug = false;
  for (int i = 0; i < argc; ++i) {
    tmp = argv[i];
    tmp = tmp.trimmed();
    if (tmp == "-i" && i + 1 < argc) {
      i++;
      input = argv[i];
      continue;
    }
    if (tmp == "-cut" && i + 1 < argc) {
      i++;
      tmp = argv[i];
      if (QFile::exists(tmp)) {
        cout << " current cut file: " << qPrintable(tmp) << endl;
        tmp = Cutter::readAll(tmp).trimmed();
      }
      cutList = tmp.split(",");
      continue;
    }
    if (tmp == "-debug") {
      debug = true;
    }
    if (tmp == "-o" && i + 1 < argc) {
      i++;
      output = argv[i];
      continue;
    }
  }
  cout << " current cutList has " << cutList.count() << " elements." << endl;
  if (input.isEmpty()) {
    cout << " -> current subtitle input is empty" << endl;
    return -1;
  }
  cout << " current subtitle input: " << qPrintable(input) << endl;
  if (cutList.isEmpty()) {
    cout << " -> current cut list is empty" << endl;
    return -1;
  }
  if (input.endsWith(".srt", Qt::CaseInsensitive)) {
    SrtCutter cutter(0, output, debug);
    a.connect(&cutter, SIGNAL(close()), &a, SLOT(quit()), Qt::QueuedConnection);
    cutter.cut(input, cutList);
  } else if (input.endsWith(".ass", Qt::CaseInsensitive)
      || input.endsWith(".ssa", Qt::CaseInsensitive)) {
    AssCutter cutter(0, output, debug);
    a.connect(&cutter, SIGNAL(close()), &a, SLOT(quit()), Qt::QueuedConnection);
    cutter.cut(input, cutList);
  } else {
    cout << " -> unsupported subtitle input format,.." << endl;
    return -1;
  }
  return a.exec();
}

