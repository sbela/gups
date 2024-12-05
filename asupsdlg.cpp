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
    QPalette pal { palette() };
    ui->frGraph->setBackground(pal.window().color().lighter(200));
    QColor itemColor { pal.windowText().color() };

    const auto setColor = [itemColor](QCPAxis* axis) {
        axis->setLabelColor(itemColor);
        axis->setTickLabelColor(itemColor);
        axis->setTickPen(itemColor);
        axis->setSubTickPen(itemColor);
        axis->setBasePen(itemColor);
    };

    setColor(ui->frGraph->xAxis);
    setColor(ui->frGraph->yAxis);
    setColor(ui->frGraph->yAxis2);

    ui->frGraph->legend->setBrush(pal.window().color().lighter());
    ui->frGraph->legend->setBorderPen(itemColor);
    ui->frGraph->legend->setTextColor(itemColor);

    ui->frGraph->setLocale(QLocale(QLocale::Hungarian, QLocale::Hungary));
    ui->frGraph->legend->setVisible(true);
    ui->frGraph->addGraph(nullptr, ui->frGraph->yAxis2);
    ui->frGraph->graph(0)->setPen(QPen(Qt::red));
    ui->frGraph->graph(0)->setBrush(QBrush(QColor(255, 0, 0, 20)));
    ui->frGraph->graph(0)->setName(QStringLiteral("Charge [%]"));
    ui->frGraph->addGraph();
    ui->frGraph->graph(1)->setPen(QPen(Qt::white));
    ui->frGraph->graph(1)->setBrush(QBrush(QColor(255, 255, 0, 20)));
    ui->frGraph->graph(1)->setName(QStringLiteral("Input voltage [V]"));
    ui->frGraph->addGraph();
    ui->frGraph->graph(2)->setPen(QPen(Qt::yellow));
    ui->frGraph->graph(2)->setName(QStringLiteral("Output voltage [V]"));
    ui->frGraph->xAxis->setLabel(QStringLiteral("t"));
    ui->frGraph->yAxis->setLabel(QStringLiteral("Ubatt"));
    ui->frGraph->yAxis2->setLabel(QStringLiteral("%batt"));
    ui->frGraph->yAxis2->setVisible(true);
    ui->frGraph->yAxis2->setRange(0, 110);
    ui->frGraph->yAxis->setRange(180, 250);

    QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
    dateTicker->setDateTimeFormat(QStringLiteral("yy.MM.dd hh:mm"));
    dateTicker->setTickOrigin(QDateTime::fromString(QStringLiteral("1. Jan 1970, 00:00 UTC")));
    ui->frGraph->xAxis->setTicker(dateTicker);
    ui->frGraph->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
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
    on_pbUpdate_clicked();
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
        if (not items.isEmpty())
            ui->twData->setItem(i, 0, new QTableWidgetItem(items[0].replace('.', ' ')));
        if (items.size() > 1)
            ui->twData->setItem(i, 1, new QTableWidgetItem(items.at(1)));
    }
}

void ASUPSDlg::updatedb()
{
    QVector<QPair<QDateTime, QString>> data { db.getData() };
    if (data.isEmpty())
        return;
    QVector<double> dates;
    dates.reserve(data.size());
    QVector<double> charge;
    charge.reserve(data.size());
    QVector<double> inputVoltage;
    inputVoltage.reserve(data.size());
    QVector<double> outputVoltage;
    outputVoltage.reserve(data.size());
    double minValue { 1000 };
    double maxValue { 0 };
    for (const QPair<QDateTime, QString>& d : data)
    {
        dates.append(QCPAxisTickerDateTime::dateTimeToKey(d.first));
        QStringList items { d.second.split(':') };
        charge.append(items.at(0).toInt());
        inputVoltage.append(items.at(1).toDouble());

        if (minValue > items.at(1).toDouble())
            minValue = items.at(1).toDouble();
        if (maxValue < items.at(1).toDouble())
            maxValue = items.at(1).toDouble();

        outputVoltage.append(items.at(2).toDouble());

        if (minValue > items.at(2).toDouble())
            minValue = items.at(2).toDouble();
        if (maxValue < items.at(2).toDouble())
            maxValue = items.at(2).toDouble();
    }

    ui->frGraph->graph(0)->data()->clear();
    ui->frGraph->graph(0)->addData(dates, charge);
    ui->frGraph->graph(1)->data()->clear();
    ui->frGraph->graph(1)->addData(dates, inputVoltage);
    ui->frGraph->graph(2)->data()->clear();
    ui->frGraph->graph(2)->addData(dates, outputVoltage);

    ui->frGraph->yAxis->setRange(minValue - 2, maxValue + 2);

    ui->frGraph->xAxis->setRange(QCPAxisTickerDateTime::dateTimeToKey(QDateTime::currentDateTime().addSecs(-3600)),
                                 QCPAxisTickerDateTime::dateTimeToKey(QDateTime::currentDateTime().addSecs(60)));
    ui->frGraph->replot();
    ui->dtFrom->setDateTime(data.first().first);
    ui->dtTo->setDateTime(data.last().first);
}

void ASUPSDlg::on_pbUpdateDB_clicked()
{
    on_pbUpdate_clicked();
    QStringList items;
    items.reserve(ui->twData->rowCount());
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

void ASUPSDlg::on_pbUpdate_clicked()
{
    QDateTime from(ui->dtFrom->dateTime());
    QDateTime to(ui->dtTo->dateTime());
    if (from > to)
    {
        auto tmp { from };
        from = to;
        to   = tmp;
    }

    QVector<QPair<QDateTime, QString>> data { db.getData(from, to) };
    if (data.isEmpty())
    {
        ui->frGraph->graph(0)->data()->clear();
        ui->frGraph->graph(1)->data()->clear();
        ui->frGraph->graph(2)->data()->clear();
        ui->frGraph->xAxis->setRange(QCPAxisTickerDateTime::dateTimeToKey(from),
                                     QCPAxisTickerDateTime::dateTimeToKey(to));
        ui->frGraph->replot();
        return;
    }
    QVector<double> dates;
    dates.reserve(data.size());
    QVector<double> charge;
    charge.reserve(data.size());
    QVector<double> inputVoltage;
    inputVoltage.reserve(data.size());
    QVector<double> outputVoltage;
    outputVoltage.reserve(data.size());

    double minValue { 1000 };
    double maxValue { 0 };

    for (const QPair<QDateTime, QString>& d : data)
    {
        dates.append(QCPAxisTickerDateTime::dateTimeToKey(d.first));
        QStringList items { d.second.split(':') };
        charge.append(items.at(0).toInt());
        inputVoltage.append(items.at(1).toDouble());
        outputVoltage.append(items.at(2).toDouble());
        if (minValue > items.at(1).toDouble())
            minValue = items.at(1).toDouble();
        if (maxValue < items.at(1).toDouble())
            maxValue = items.at(1).toDouble();
        if (minValue > items.at(2).toDouble())
            minValue = items.at(2).toDouble();
        if (maxValue < items.at(2).toDouble())
            maxValue = items.at(2).toDouble();
    }

    ui->frGraph->graph(0)->data()->clear();
    ui->frGraph->graph(0)->addData(dates, charge);
    ui->frGraph->graph(1)->data()->clear();
    ui->frGraph->graph(1)->addData(dates, inputVoltage);
    ui->frGraph->graph(2)->data()->clear();
    ui->frGraph->graph(2)->addData(dates, outputVoltage);

    ui->frGraph->yAxis->setRange(minValue - 2, maxValue + 2);

    ui->frGraph->xAxis->setRange(
        QCPAxisTickerDateTime::dateTimeToKey(from < data.first().first ? data.first().first : from),
        QCPAxisTickerDateTime::dateTimeToKey(to > data.last().first ? data.last().first : to));
    ui->frGraph->replot();
}

void ASUPSDlg::on_pbLastUpdate_clicked()
{
    QDateTime to(QDateTime::currentDateTime());
    QDateTime from;

    switch (ui->cbLast->currentIndex())
    {
    case 0:
        from = to.addSecs(-5 * 60);
        break;
    case 1:
        from = to.addSecs(-10 * 60);
        break;
    case 2:
        from = to.addSecs(-30 * 60);
        break;
    case 3:
        from = to.addSecs(-60 * 60);
        break;
    case 4:
        from = to.addDays(-1);
        break;
    case 5:
        from = to.addMonths(-1);
        break;
    case 6:
        from = to.addYears(-1);
        break;
    }

    QVector<QPair<QDateTime, QString>> data { db.getData(from.toUTC(), to.toUTC()) };
    if (data.isEmpty())
    {
        ui->frGraph->graph(0)->data()->clear();
        ui->frGraph->graph(1)->data()->clear();
        ui->frGraph->graph(2)->data()->clear();
        ui->frGraph->xAxis->setRange(QCPAxisTickerDateTime::dateTimeToKey(from),
                                     QCPAxisTickerDateTime::dateTimeToKey(to));
        ui->frGraph->replot();
        return;
    }

    QVector<double> dates;
    dates.reserve(data.size());
    QVector<double> charge;
    charge.reserve(data.size());
    QVector<double> inputVoltage;
    inputVoltage.reserve(data.size());
    QVector<double> outputVoltage;
    outputVoltage.reserve(data.size());

    double minValue { 1000 };
    double maxValue { 0 };

    for (const QPair<QDateTime, QString>& d : data)
    {
        dates.append(QCPAxisTickerDateTime::dateTimeToKey(d.first));
        QStringList items { d.second.split(':') };
        charge.append(items.at(0).toInt());
        inputVoltage.append(items.at(1).toDouble());
        outputVoltage.append(items.at(2).toDouble());
        if (minValue > items.at(1).toDouble())
            minValue = items.at(1).toDouble();
        if (maxValue < items.at(1).toDouble())
            maxValue = items.at(1).toDouble();
        if (minValue > items.at(2).toDouble())
            minValue = items.at(2).toDouble();
        if (maxValue < items.at(2).toDouble())
            maxValue = items.at(2).toDouble();
    }

    ui->frGraph->graph(0)->data()->clear();
    ui->frGraph->graph(0)->addData(dates, charge);
    ui->frGraph->graph(1)->data()->clear();
    ui->frGraph->graph(1)->addData(dates, inputVoltage);
    ui->frGraph->graph(2)->data()->clear();
    ui->frGraph->graph(2)->addData(dates, outputVoltage);

    ui->frGraph->yAxis->setRange(minValue - 2, maxValue + 2);

    ui->frGraph->xAxis->setRange(
        QCPAxisTickerDateTime::dateTimeToKey(from < data.first().first ? data.first().first : from),
        QCPAxisTickerDateTime::dateTimeToKey(to > data.last().first ? data.last().first : to));
    ui->frGraph->replot();
}
