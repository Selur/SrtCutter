/*
 * Cutter.h
 *
 *  Created on: 05.07.2013
 *      Author: Selur
 */

#ifndef CUTTER_H_
#define CUTTER_H_

#include <QTime>
#include <QObject>
#include <QString>
#include <QStringList>

struct SubtitleEntry
{
    double from, to;
    QString text;
    bool fixed;
};

class Cutter : public QObject
{
  Q_OBJECT
  public:
    Cutter(QObject *parent, QString output = QString(), bool debug = false);
    virtual ~Cutter();
    void cut(const QString input, const QStringList cutList);
    static QString readAll(const QString fileName, QString type = "auto");

  protected:
    virtual QString cutContent(const QString content, const QStringList cutList) = 0;
    virtual QList<SubtitleEntry> tokenize(const QString content) = 0;
    bool m_debug;
    QString m_input, m_output;
    QStringList m_acceptedExtensions;
    void adjustEntries(QList<SubtitleEntry> &entries, QStringList cutList, const bool &debug);
    QString secondsToHMSZZZ(double seconds);
    double timeToSeconds(QString value);

  private:
    QString getDirectory(const QString input);
    QString removeLastSeparatorFromPath(QString input);
    int saveTextTo(QString text, QString to);
    double timeToSeconds(QTime time);

  signals:
    void close();
};

#endif /* CUTTER_H_ */

