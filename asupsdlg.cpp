/*
 * Copyright 2023. 03. 12. sbela
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
#include "asupsdlg.h"
#include "asfx.h"
#include "ui_asupsdlg.h"

#include <QProcess>
#include <QSettings>

ASUPSDlg::ASUPSDlg(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ASUPSDlg)
{
    Globals::pLog = &m_asLogger;
    PRINT("ASUPSDlg");
    ui->setupUi(this);
    setWindowFlags(Qt::Window);

    connect(&m_updateTimer, &QTimer::timeout, this, &ASUPSDlg::doQueryTimeout);
    connect(&m_updateDBTimer, &QTimer::timeout, this, &ASUPSDlg::doQueryDBTimeout);

    QSettings s(QStringLiteral("asups.ini"), QSettings::IniFormat);
    ui->cbQueryPeriodically->blockSignals(true);
    ui->cbQueryPeriodically->setChecked(s.value(QStringLiteral("periodically"), false).toBool());
    ui->cbQueryPeriodically->blockSignals(false);
    ui->twData->setColumnWidth(1, 160);
    if (ui->cbQueryPeriodically->isChecked())
    {
        update();
        m_updateTimer.start(10000);
    }
    ui->cbAutoUpdateDB->blockSignals(true);
    ui->cbAutoUpdateDB->setChecked(s.value(QStringLiteral("updatedb"), false).toBool());
    ui->cbAutoUpdateDB->blockSignals(false);
    ui->frGraph->setLocale(QLocale(QLocale::Hungarian, QLocale::Hungary));
    ui->frGraph->legend->setVisible(true);
    ui->frGraph->addGraph(nullptr, ui->frGraph->yAxis2);
    ui->frGraph->graph(0)->setPen(QPen(Qt::red));
    ui->frGraph->graph(0)->setName(QStringLiteral("Charge [%]"));
    ui->frGraph->addGraph();
    ui->frGraph->graph(1)->setPen(QPen(Qt::green));
    ui->frGraph->graph(1)->setName(QStringLiteral("Input voltage [V]"));
    ui->frGraph->addGraph();
    ui->frGraph->graph(2)->setName(QStringLiteral("Output voltage [V]"));
    ui->frGraph->xAxis->setLabel(QStringLiteral("t"));
    ui->frGraph->yAxis->setLabel(QStringLiteral("Ubatt"));
    ui->frGraph->yAxis2->setLabel(QStringLiteral("%batt"));
    ui->frGraph->yAxis2->setVisible(true);
    ui->frGraph->yAxis2->setRange(0, 110);
    ui->frGraph->yAxis->setRange(180, 250);
    QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
    dateTicker->setDateTimeFormat(QStringLiteral("hh:mm:ss.zzz"));
    dateTicker->setTickOrigin(QDateTime::fromString(QStringLiteral("1. Jan 1970, 00:00 UTC")));
    ui->frGraph->xAxis->setTicker(dateTicker);
    ui->frGraph->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems);
    ui->frGraph->setSelectionRectMode(QCP::srmNone);
    if (ui->cbAutoUpdateDB->isChecked())
        m_updateDBTimer.start(10000);
    updatedb();
    restoreGeometry(s.value(QStringLiteral("geometry")).toByteArray());
}

ASUPSDlg::~ASUPSDlg()
{
    QSettings s(QStringLiteral("asups.ini"), QSettings::IniFormat);
    s.setValue(QStringLiteral("geometry"), saveGeometry());
    delete ui;
    PRINT("~ASUPSDlg");
    Globals::pLog = nullptr;
}

void ASUPSDlg::doQueryTimeout()
{
    m_updateTimer.stop();
    update();
    if (ui->cbQueryPeriodically->isChecked())
        m_updateTimer.start(10000);
}

void ASUPSDlg::doQueryDBTimeout()
{
    m_updateDBTimer.stop();
    updatedb();
    if (ui->cbAutoUpdateDB->isChecked())
        m_updateDBTimer.start(10000);
}

void ASUPSDlg::on_pbQuery_clicked()
{
    update();
}

void ASUPSDlg::on_cbQueryPeriodically_stateChanged(int arg1)
{
    QSettings s(QStringLiteral("asups.ini"), QSettings::IniFormat);
    s.setValue(QStringLiteral("periodically"), arg1);
    if (arg1)
        m_updateTimer.start(10000);
}

void ASUPSDlg::update()
{
    QProcess pr;
    QStringList args;
    args << ui->leURL->text();
    pr.start(QStringLiteral("upsc"), args);
    pr.waitForFinished();
    auto ret = QString("%1").arg(pr.readAllStandardOutput().constData()).split('\n', Qt::SkipEmptyParts);
    ui->twData->setRowCount(ret.size());
    for (int i = 0; i < ret.size(); ++i)
    {
        auto items = ret.at(i).split(':');
        if (items.size())
            ui->twData->setItem(i, 0, new QTableWidgetItem(items[0].replace('.', ' ')));
        if (items.size() > 1)
            ui->twData->setItem(i, 1, new QTableWidgetItem(items.at(1)));
    }
}

void ASUPSDlg::updatedb()
{
    QVector<QPair<QDateTime, QString>> data { db.getData() };
    if (data.size())
    {
        QVector<double> dates;
        QVector<double> charge;
        QVector<double> inputVoltage;
        QVector<double> outputVoltage;
        for (const QPair<QDateTime, QString>& d : data)
        {
            dates.append(QCPAxisTickerDateTime::dateTimeToKey(d.first));
            QStringList items { d.second.split(':') };
            charge.append(items.at(0).toInt());
            inputVoltage.append(items.at(1).toDouble());
            outputVoltage.append(items.at(2).toDouble());
        }

        ui->frGraph->graph(0)->data().clear();
        ui->frGraph->graph(0)->addData(dates, charge);
        ui->frGraph->graph(1)->data().clear();
        ui->frGraph->graph(1)->addData(dates, inputVoltage);
        ui->frGraph->graph(2)->data().clear();
        ui->frGraph->graph(2)->addData(dates, outputVoltage);
        ui->frGraph->xAxis->setRange(dates.first(), dates.last());
        ui->frGraph->replot();
    }
}

void ASUPSDlg::on_pbUpdateDB_clicked()
{
    updatedb();
    QStringList items;
    for (int i = 0; i < ui->twData->rowCount(); ++i)
        items << QString("%1:%2").arg(ui->twData->item(i, 0)->text(), ui->twData->item(i, 1)->text());
    db.addToDB(items);
}

void ASUPSDlg::on_cbAutoUpdateDB_stateChanged(int arg1)
{
    QSettings s(QStringLiteral("asups.ini"), QSettings::IniFormat);
    s.setValue(QStringLiteral("updatedb"), arg1);
    if (arg1)
        m_updateDBTimer.start(10000);
}
