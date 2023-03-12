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
#include "aslogger.h"
#include "asfx.h"

ASLogger::ASLogger(QObject* parent)
    : QObject { parent }
{
    PRINTNL("ASLOGGER Created!");
    connect(&m_syncTimer, &QTimer::timeout, this, &ASLogger::syncTimeout);
    connect(this, &ASLogger::doSync, this, &ASLogger::syncTimeout, Qt::QueuedConnection);
    m_logDate = QDateTime::currentDateTime().date().addDays(-1);
    m_syncTimer.start(m_syncTimeout);
}

ASLogger::~ASLogger()
{
    PRINTNL("ASLOGGER Destroyed!");
}

void ASLogger::AddToLog(const QString& message)
{
    if (m_logDate != QDateTime::currentDateTime().date())
    {
        m_logDate = QDateTime::currentDateTime().date();
        syncTimeout();
    }
    QMutexLocker m(&m_messageMutex);
    m_logMessages.append(message);
}

void ASLogger::AddToLog(const std::string& message)
{
    AddToLog(QString::fromStdString(message));
}

const QStringList ASLogger::GetLast100()
{
    QString strPath { QDateTime::currentDateTime().toString("yy/MM/dd") };

    QFile f(QString("log/%1/error.log").arg(strPath));
    if (f.open(QIODevice::ReadOnly))
    {
        const QByteArray file = f.readAll();
        QStringList content;
        QString line;
        for (const char& c : file)
        {
            if (c != '\n')
            {
                line.append(c);
            }
            else
            {
                if (line.length())
                {
                    content.append(line);
                    line.clear();
                }
            }
        }
        QStringList ret;
        for (int i = content.size() - 1, count = 0; i > -1; --i)
        {
            ret.append(content.at(i));
            if (++count > 99)
                break;
        }
        return ret;
    }
    return {};
}

void ASLogger::syncTimeout()
{
    m_syncTimer.stop();
    QMutexLocker m(&m_messageMutex);
    if (m_logMessages.size())
    {
        QString strPath { QDateTime::currentDateTime().toString("yy/MM/dd") };
        QDir d;
        d.mkpath(QString("log/%1").arg(strPath));
        QFile f(QString("log/%1/error.log").arg(strPath));
        if (f.open(QIODevice::Append))
        {
            PRINTNL("SYNCING LOG TO FS {}", f.fileName());
            f.write(m_logMessages.join('\n').append('\n').toUtf8());
            m_logMessages.clear();
        }
    }
    m_syncTimer.start(m_syncTimeout);
}
