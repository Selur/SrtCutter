/*
 * Cutter.cpp
 *
 *  Created on: 05.07.2013
 *      Author: Selur
 */

#include "Cutter.h"
#include <QDir>
#include <QFile>
#include <QTextCodec>
#include <QTextStream>

#define UTF8BOM "\xEF\xBB\xBF"
#define UTF8 "UTF-8"
#define UTF16LEBOM "\xFF\xFE"
#define UTF16LE "UTF-16LE"
#define UTF16BEBOM "\xFE\xFF"
#define UTF16BE "UTF-16BE"

#include <iostream>
using namespace std;

Cutter::Cutter(QObject *parent, QString output, bool debug) :
    QObject(parent), m_input(QString()), m_output(output), m_acceptedExtensions(), m_debug(debug)
{
}

Cutter::~Cutter()
{
}

int Cutter::saveTextTo(QString text, QString to)
{
  if (text.isEmpty()) {
    cerr << "Save text called with empty text!" << endl;
    return -1;
  }
  cout << " saving subtitles to: " << qPrintable(to) << endl;
  text = text.replace("\r\n", "\n");
  QFile file(to);
  file.remove();
  if (file.open(QIODevice::WriteOnly)) {
    bool avs = to.endsWith(".avs", Qt::CaseInsensitive);
    bool idx = to.endsWith(".idx", Qt::CaseInsensitive);
    bool d2v = to.endsWith(".d2v", Qt::CaseInsensitive);
    bool qp = to.endsWith(".qp", Qt::CaseInsensitive);
    bool meta = to.endsWith(".meta", Qt::CaseInsensitive);
#ifdef Q_OS_WIN
    bool ttxt = to.endsWith(".ttxt", Qt::CaseInsensitive);
    if (!ttxt && !avs && !meta && !qp && !d2v) {
#else
    if (!avs && !meta && !qp && !d2v) {
#endif
      file.write(UTF8BOM);
      file.setTextModeEnabled(true);
    }
    QTextStream out(&file);
    if (avs || meta || idx || qp || d2v) {
      out.setCodec(QTextCodec::codecForLocale());
    } else {
      out.setCodec("UTF-8");
    }
    out << text;
    if (file.exists()) {
      file.close();
      return 0;
    } else {
      cerr << "Couldn't save file content!" << endl;
    }
  } else {
    cerr << "Couldn't open file for writing!" << endl;
  }
  return -1;
}

void Cutter::cut(const QString input, const QStringList cutList)
{

  QString extension = input;
  extension = extension.trimmed();
  extension = extension.remove(0, extension.lastIndexOf(".") + 1);
  extension = extension.toLower();
  if (m_debug) {
    cout << "  input file extension " << qPrintable(extension) << endl;
  }
  bool supported = m_acceptedExtensions.contains(extension);
  if (!supported) {
    cerr << " -> " << qPrintable(this->objectName());
    cerr << " does not support " << qPrintable(input) << endl;
    emit close();
    return;
  }
  if (m_debug) {
    cout << " -> " << qPrintable(this->objectName());
    cout << " used for input " << qPrintable(input) << endl;
  }
  QString content = this->readAll(input).trimmed();
  if (content.isEmpty()) {
    cerr << " input " << qPrintable(input) << " is empty!" << endl;
    emit close();
    return;
  }
  QString cutContent = this->cutContent(content, cutList);
  if (cutContent.isEmpty()) {
    cerr << " cut content is empty!" << endl;
    emit close();
    return;
  }
  QString output = m_output;
  if (output.trimmed().isEmpty()) {
    output = input;
    output = output.insert(output.lastIndexOf("."), ".cut");
  }
  this->saveTextTo(cutContent, output);
  cout << "finished!";
  emit close();
  return;
}

QString Cutter::removeLastSeparatorFromPath(QString input)
{
  input = input.trimmed();
  if (input.isEmpty()) {
    return input;
  }
  input = QDir::toNativeSeparators(input);
  int size = input.size();
  if (!input.endsWith(QDir::separator())) {
    return input;
  } else if (size == 1) { //input only consists of the separator
    return QString();
  }
  return input.remove(size - 1, 1);
}

QString Cutter::getDirectory(const QString input)
{
  if (input.isEmpty()) {
    return QString();
  }
  QString path = input;
  QFileInfo info(path);
  if (info.isDir()) {
    return removeLastSeparatorFromPath(path);
  }
  QString output = path;
  output = output.replace("\\", "/");
  int index = output.lastIndexOf("/");
  if (index == -1) {
    return QString();
  }
  output = output.remove(index, output.size());
  return QDir::toNativeSeparators(output);
}

QString Cutter::readAll(const QString fileName, QString type)
{
  QString input = fileName;
  QFile file(input);
  if (!file.exists()) {
    return QString();
  }
  if (!file.open(QIODevice::ReadOnly)) {
    return QString();
  }
  QTextStream stream(&file);
  if (type == "auto") {
    stream.autoDetectUnicode();
  } else {
    stream.setCodec(type.toUtf8());
  }
  input = stream.readAll();
  file.close();
  return input;
}

QString Cutter::secondsToHMSZZZ(double seconds)
{
  if (seconds == 0) {
    return "00:00:00";
  }
  QString time = QString();
  int hrs = 0;
  if (seconds >= 3600) { //Stunden
    hrs = int(seconds) / 3600;
  } else if (seconds == 3600) {
    hrs = 1;
  }
  time += QString((hrs < 10) ? "0" : QString()) + QString::number(hrs);

  int min = 0;
  seconds = seconds - 3600 * hrs;
  if (seconds >= 60) { //Minuten
    min = int(seconds) / 60;
  }
  time += ":" + QString((min < 10) ? "0" : QString()) + QString::number(min);

  int sec = 0;
  seconds = seconds - 60 * min;
  if (seconds > 0) { //Sekunden
    sec = int(seconds);
  }
  time += ":";
  time += QString((sec < 10) ? "0" : QString());
  time += QString::number(sec);
  int milliseconds = int(1000 * (seconds - (sec * 1.0)) + 0.5);
  if (milliseconds == 0) {
    time += ".000";
    return time;
  }
  time += ".";
  if (milliseconds < 10) {
    time += "00";
  } else if (milliseconds < 100) {
    time += "0";
  }
  time += QString::number(milliseconds);
  return time;
}

double Cutter::timeToSeconds(QTime time)
{
  return ((time.hour() * 60.0 + time.minute()) * 60.0 + time.second()) + (time.msec() / 1000.0);
}

/**
 * converts a time string to seconds
 **/
double Cutter::timeToSeconds(QString value)
{
  QString tmp = value.trimmed();
  double dtime = tmp.toDouble();
  int index = tmp.indexOf(":");
  bool tripple = tmp.count(":") == 3;
  if (index == -1) {
    return 0;
  }
  if (index == 1) {
    tmp = "0" + tmp;
  }
  bool dot = true;
  index = tmp.indexOf(".");
  if (index == -1) {
    index = tmp.indexOf(",");
    dot = false;
  }
  if (index != -1) {
    int length = tmp.length();
    QTime time;
    if ((length - index) == 2) {
      if (tripple) {
        time = QTime::fromString(tmp, "hh:mm:ss:z");
      } else if (dot) {
        time = QTime::fromString(tmp, "hh:mm:ss.z");
      } else {
        time = QTime::fromString(tmp, "hh:mm:ss,z");
      }
    } else {
      if ((length - index) == 3) {
        tmp += "0";
      }
      if (tripple) {
        time = QTime::fromString(tmp, "hh:mm:ss:zzz");
      } else if (dot) {
        time = QTime::fromString(tmp, "hh:mm:ss.zzz");
      } else {
        time = QTime::fromString(tmp, "hh:mm:ss,zzz");
      }
    }
    dtime = this->timeToSeconds(time);
  } else {
    QTime time;
    if (dot) {
      time = QTime::fromString(tmp, "hh:mm:ss");
    } else {
      time = QTime::fromString(tmp, "hh:mm:ss");
    }
    dtime = (time.hour() * 60.0 + time.minute()) * 60.0 + time.second();
  }
  return dtime;
}

void adjustSubtitlesFor(const double cStart, const double cEnd, double tempCend,
                        QList<SubtitleEntry> &entries, const bool &debug)
{
  if (debug) {
    cout << "adjusting subtitle";
    cout << " for ";
    cout << qPrintable(QString::number(cStart)+"-"+QString::number(cEnd));
    cout << " and previous end " << tempCend;
    cout << endl;
  }
  QList<SubtitleEntry> adjustedEntries;
  foreach(SubtitleEntry entry, entries)
  {
    if (entry.fixed) {
      if (debug) {
        cout << " keeping ";
        cout << qPrintable(QString::number(entry.from)+"-"+QString::number(entry.to));
        cout << endl;
      }
      tempCend = entry.to;
      adjustedEntries.append(entry);
      continue;
    }
    if (entry.to <= cStart) {
      if (debug) {
        cout << " dropping ";
        cout << qPrintable(QString::number(entry.from)+"-"+QString::number(entry.to));
        cout << endl;
      }
      continue;
    }
    // to > cStart
    if (entry.from >= cEnd) { // after current cut
      if (debug) {
        cout << " adjusted ";
        cout << qPrintable(QString::number(entry.from)+"-"+QString::number(entry.to));
      }
      entry.from -= cEnd;
      entry.to -= cEnd;
      if (debug) {
        cout << " to ";
        cout << qPrintable(QString::number(entry.from)+"-"+QString::number(entry.to));
        cout << endl;
      }
      adjustedEntries.append(entry);
      continue;
    } // from <= cEnd
    if (debug) {
      cout << " adjusted ";
      cout << qPrintable(QString::number(entry.from)+"-"+QString::number(entry.to));
    }
    if (entry.from < cStart) {
      entry.from = cStart;
      if (entry.to < cEnd) {
        if (debug) {
          cout << " tempCend " << tempCend << endl;
          cout << " entry.to " << entry.to << endl;
          cout << " entry.from " << entry.from << endl;
        }
        entry.to = tempCend + entry.to - entry.from;
      }
      entry.from = tempCend;
      entry.fixed = true;
    } else if (entry.to > cEnd) {
      entry.from -= cEnd;
      entry.to -= cEnd;
    } else {
      entry.from = cStart;
      if (debug) {
        cout << " tempCend " << tempCend << endl;
        cout << " entry.to " << entry.to << endl;
        cout << " entry.from " << entry.from << endl;
      }
      entry.to = tempCend + entry.to - entry.from;
      entry.from = tempCend;
      entry.fixed = true;
    }
    if (debug) {
      cout << " to ";
      cout << qPrintable(QString::number(entry.from)+"-"+QString::number(entry.to));
      cout << endl;
    }
    adjustedEntries.append(entry);
  }
  entries = adjustedEntries;
}

void adjustCutListFor(const double cStart, const double cEnd, double &previousEnd,
                      QStringList &cutList, const bool debug)
{
  if (debug) {
    cout << "adjusting cutList";
    cout << " for ";
    cout << qPrintable(QString::number(cStart)+"-"+QString::number(cEnd));
    cout << endl;
  }
  QString current;
  QStringList cutElements, adjustedCuts;
  double currentStart = 0, currentEnd = 0, newEnd;
  for (int i = 0, c = cutList.count(); i < c; ++i) {
    current = cutList.at(i);
    cutElements = current.split("-");
    currentStart = cutElements.at(0).toDouble();
    currentEnd = cutElements.at(1).toDouble();
    if (cStart >= currentEnd) {
      if (debug) {
        cout << " keeping ";
        cout << qPrintable(QString::number(currentStart)+"-"+QString::number(currentEnd));
        cout << endl;
      }
      adjustedCuts.append(current);
      previousEnd = currentEnd;
      continue;
    }
    if (debug) {
      cout << " adjusted ";
      cout << qPrintable(QString::number(currentStart)+"-"+QString::number(currentEnd));
    }
    if (cStart == currentStart) {
      if (debug) {
        cout << "  currentEnd " << currentEnd << endl;
        cout << "  currentStart " << currentStart << endl;
        cout << "  previousEnd " << previousEnd << endl;
      }
      newEnd = currentEnd - currentStart + previousEnd;
      if (debug) {
        cout << "  -> newEnd " << newEnd << endl;
      }
      currentEnd = newEnd;
      currentStart = previousEnd;
      if (debug) {
        cout << "  -> newStart " << currentStart << endl;
      }

    } else {
      currentEnd -= cEnd;
      if (debug) {
        cout << "  -> newEnd " << currentEnd << endl;
      }
      currentStart -= cEnd;
      if (debug) {
        cout << "  -> newStart " << currentStart << endl;
      }
    }
    if (debug) {
      cout << " to ";
      cout << qPrintable(QString::number(currentStart)+"-"+QString::number(currentEnd));
      cout << endl;
    }
    adjustedCuts.append(QString::number(currentStart) + "-" + QString::number(currentEnd));
  }
  cutList = adjustedCuts;
}

void adjustSubtitlesToLastCut(const double cStart, const double cEnd, QList<SubtitleEntry> &entries,
                              const bool &debug)
{
  if (debug) {
    cout << "adjusting subtitles to last cut ";
    cout << qPrintable(QString::number(cStart)+"-"+QString::number(cEnd));
    cout << endl;
  }
  QList<SubtitleEntry> adjustedEntries;
  foreach(SubtitleEntry entry, entries)
  {
    if (debug) {
      cout << " looking at ";
      cout << qPrintable(QString::number(entry.from)+"-"+QString::number(entry.to));
      cout << endl;
    }
    if (entry.to <= cEnd) {
      if (debug) {
        cout << " -> keep: entry.to <= cEnd" << endl;
      }
      adjustedEntries.append(entry);
      continue;
    } // entry.to > cEnd
    if (entry.from >= cEnd) {
      if (debug) {
        cout << " -> drop: entry.from >= cEnd" << endl;
      }
      continue;
    } // entry.from < cEnd && entry.to > cEnd
    if (entry.from < cStart) {
      entry.from = cStart;
      if (debug) {
        cout << "  -> adjust: entry.from < cStart -> entry.from adjusted to: " << cStart << endl;
      }
    }
    entry.to = cEnd;
    if (debug) {
      cout << " -> keep" << endl;
    }
    adjustedEntries.append(entry);
  }
  entries = adjustedEntries;
}

void Cutter::adjustEntries(QList<SubtitleEntry> &entries, QStringList cutList, const bool &debug)
{
  if (entries.isEmpty()) {
    cerr << "No entries to adjust!" << endl;
    return;
  }
  cout << " adjusting subtitle entries to cut list,.." << endl;
  // remove all unneeded at the end
  QString cut = cutList.last();
  QStringList adjustedCuts;
  QStringList cutElements = cut.split("-");
  double cStart = cutElements.at(0).toDouble();
  double cEnd = cutElements.at(1).toDouble();
  double cOldEnd = 0;
  double previousEnd = 0;
  adjustSubtitlesToLastCut(cStart, cEnd, entries, debug);
  SubtitleEntry currentEntry;
  for (int i = 0, c = cutList.count(); i < c; ++i) {
    cut = cutList.at(i);
    cutElements = cut.split("-");
    cStart = cutElements.at(0).toDouble() - cOldEnd;
    cEnd = cutElements.at(1).toDouble() - cOldEnd;
    adjustCutListFor(cStart, cEnd, previousEnd, cutList, debug);
    adjustSubtitlesFor(cStart, cEnd, previousEnd, entries, debug);
  }
  cut = cutList.last();
  cutElements = cut.split("-");
  cStart = cutElements.at(0).toDouble() - cOldEnd;
  cEnd = cutElements.at(1).toDouble() - cOldEnd;
}
