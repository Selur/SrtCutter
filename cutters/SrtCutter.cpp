/*
 * SrtCutter.cpp
 *
 *  Created on: 05.07.2013
 *      Author: Selur
 */

#include "SrtCutter.h"

#include <iostream>
using namespace std;

SrtCutter::SrtCutter(QObject *parent, QString output, bool debug) :
    Cutter(parent, output, debug)
{
  this->setObjectName("SrtCutter");
  m_acceptedExtensions << "srt";
}

SrtCutter::~SrtCutter()
{
}

QList<SubtitleEntry> SrtCutter::tokenize(const QString content)
{
  cout << " tokenizing subtitle content,.." << endl;
  QList<SubtitleEntry> tokens;
  QStringList lines = content.split("\n");

  bool gotLineNumber = false;
  bool gotFromTo = false;
  bool gotContent = false;
  SubtitleEntry currentEntry;
  QStringList fromTo;
  foreach(QString line, lines)
  {
    line = line.trimmed();
    if (line.isEmpty()) {
      if (m_debug) {
        cout << "looking at an empty line" << endl;
        cout << " gotLineNumber " << ((gotLineNumber) ? "true" : "false") << endl;
        cout << " gotContent " << ((gotContent) ? "true" : "false") << endl;
        cout << " gotFromTo " << ((gotFromTo) ? "true" : "false") << endl;
      }
      if (!gotLineNumber || !gotContent || !gotFromTo) {
        if (m_debug) {
          cout << " -> resetting" << endl;
        }
        gotLineNumber = false;
        gotFromTo = false;
        gotContent = false;
        currentEntry.from = -1;
        currentEntry.to = -1;
        currentEntry.text = QString();
        continue;
      }
      currentEntry.fixed = false;
      if (m_debug) {
        cout << " -> appending current entry" << endl;
      }
      tokens.append(currentEntry);
      gotLineNumber = false;
      gotFromTo = false;
      gotContent = false;
      currentEntry.from = -1;
      currentEntry.to = -1;
      currentEntry.text = QString();
      continue;
    }
    if (m_debug) {
      cout << "looking at line " << qPrintable(line) << endl;
      cout << " line.toInt(): " << line.toInt() << endl;
    }
    if (line.toInt() != 0 && !gotLineNumber) {
      gotLineNumber = true;
      continue;
    }
    if (line.contains(" --> ") && !gotFromTo) {
      fromTo = line.split(" --> ");
      currentEntry.from = this->timeToSeconds(fromTo.at(0));
      if (m_debug) {
        cout << "  current from " << currentEntry.from << endl;
      }
      currentEntry.to = this->timeToSeconds(fromTo.at(1));
      if (m_debug) {
        cout << "  current to " << currentEntry.to << endl;
      }
      gotFromTo = true;
      continue;
    }
    if (!currentEntry.text.isEmpty()) {
      currentEntry.text += "\n";
    }
    currentEntry.text += line;
    if (m_debug) {
      cout << "  current text: " << qPrintable(currentEntry.text) << endl;
    }
    gotContent = true;
  }
  if (gotLineNumber && gotContent && gotFromTo) {
    if (m_debug) {
      cout << " -> appending current entry" << endl;
    }
    tokens.append(currentEntry);
  }
  return tokens;
}

QString SrtCutter::cutContent(const QString content, const QStringList cutList)
{
  QList<SubtitleEntry> tokens = this->tokenize(content);
  if (m_debug) {
    cout << " -> " << qPrintable(this->objectName()) << " found " << tokens.count();
    cout << " subtitle entries" << endl;
  }
  this->adjustEntries(tokens, cutList, m_debug);

  QString subtitles;
  int i = 1;
  foreach(SubtitleEntry entry, tokens)
  {
    subtitles += QString::number(i++);
    subtitles += "\r\n";
    subtitles += secondsToHMSZZZ(entry.from) + " ---> " + secondsToHMSZZZ(entry.to);
    subtitles += "\r\n";
    subtitles += entry.text;
    subtitles += "\r\n";
  }
  return subtitles;
}
