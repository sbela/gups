/*
 * Copyright 2022. 03. 07. sbela
 * @ AutoSys KnF Kft.
 * web: http://www.autosys.hu
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#ifndef ASLOGGER_H
#define ASLOGGER_H

#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QTimer>

class ASLogger : public QObject
{
    Q_OBJECT
public:
    explicit ASLogger(QObject* parent = nullptr);
    virtual ~ASLogger() override;

    void AddToLogFile(const QString& message);
    void AddToLog(const QString& message);
    void AddToLog(const std::string& message);
    const QStringList GetLast100();

private Q_SLOTS:
    void syncTimeout();

Q_SIGNALS:
    void doSync();

private:
    QDate m_logDate;
    QStringList m_logMessages;
    QTimer m_syncTimer;
    int m_syncTimeout { 10000 };
    QMutex m_messageMutex;
};

#endif // ASLOGGER_H
