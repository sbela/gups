/*
 * Copyright 2023. 02. 18. sbela
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

#include "db.h"
#include "asfx.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>

LogDB::LogDB()
{
    PRINT("LOGDB");
    QSettings s(QStringLiteral("aslog.conf"), QSettings::IniFormat);
    QString server = s.value(QStringLiteral("SERVER"), QStringLiteral("192.168.100.70")).toString();
    QString dbName = s.value(QStringLiteral("DB"), QStringLiteral("ups")).toString();
    QString un     = s.value(QStringLiteral("UN"), QStringLiteral("postgres")).toString();
    QString pw     = s.value(QStringLiteral("PW"), QStringLiteral("sysauto")).toString();
    int port       = s.value(QStringLiteral("PORT"), 5432).toInt();

    m_db = QSqlDatabase::addDatabase(QStringLiteral("QPSQL"),
                                     QString("ASLOG_%1").arg(QDateTime::currentDateTime().toString("yyyyMMhhmmsszzz")));
    m_db.setHostName(server);
    m_db.setDatabaseName(dbName);
    m_db.setUserName(un);
    m_db.setPassword(pw);
    m_db.setPort(port);
    PRINT("LOGDB INIT DONE");
}

LogDB::~LogDB()
{
    if (m_db.isOpen())
        m_db.close();
    PRINT("~LOGDB");
}

void LogDB::addToDB(const QStringList& items)
{
    if (initDB())
    {
        if (m_db.transaction())
        {
            QString battery_voltage_high, battery_voltage_low, battery_voltage_nominal, device_mfr, device_model,
                device_type, driver_name, driver_parameter_bus, driver_parameter_pollinterval, driver_parameter_port,
                driver_parameter_product, driver_parameter_productid, driver_parameter_serial,
                driver_parameter_synchronous, driver_parameter_vendor, driver_parameter_vendorid, driver_version,
                driver_version_internal, input_current_nominal, input_frequency, ups_beeper_status, ups_delay_shutdown,
                ups_delay_start, ups_firmware, ups_load, ups_mfr, ups_model, ups_productid, ups_type, ups_vendorid;

            constexpr auto setItem = [](const QString& item, const QString& elem, QString& value, bool isText = true) {
                if (item.contains(elem))
                {
                    auto items { item.split(':', Qt::SkipEmptyParts) };
                    if (items.size() > 1)
                        value = (isText ? "'" : "") + items.at(1).trimmed() + (isText ? "'" : "");
                }
                else if (value.isEmpty())
                    value = "0";                
            };

            for (int i = 0; i < items.size(); ++i)
            {
                auto item { items.at(i) };
                setItem(item, QStringLiteral("battery voltage high"), battery_voltage_high, false);
                setItem(item, QStringLiteral("battery voltage low"), battery_voltage_low, false);
                setItem(item, QStringLiteral("battery voltage nominal"), battery_voltage_nominal, false);
                setItem(item, QStringLiteral("device mfr"), device_mfr);
                setItem(item, QStringLiteral("device model"), device_model);
                setItem(item, QStringLiteral("device type"), device_type);
                setItem(item, QStringLiteral("driver name"), driver_name);
                setItem(item, QStringLiteral("driver parameter bus"), driver_parameter_bus);
                setItem(item, QStringLiteral("driver parameter pollinterval"), driver_parameter_pollinterval);
                setItem(item, QStringLiteral("driver parameter port"), driver_parameter_port);
                setItem(item, QStringLiteral("driver parameter product"), driver_parameter_product);
                setItem(item, QStringLiteral("driver parameter productid"), driver_parameter_productid);
                setItem(item, QStringLiteral("driver parameter serial"), driver_parameter_serial);
                setItem(item, QStringLiteral("driver parameter synchronous"), driver_parameter_synchronous);
                setItem(item, QStringLiteral("driver parameter vendor"), driver_parameter_vendor);
                setItem(item, QStringLiteral("driver parameter vendorid"), driver_parameter_vendorid);
                setItem(item, QStringLiteral("driver version"), driver_version);
                setItem(item, QStringLiteral("driver version internal"), driver_version_internal);
                setItem(item, QStringLiteral("input current nominal"), input_current_nominal, false);
                setItem(item, QStringLiteral("input frequency"), input_frequency, false);
                setItem(item, QStringLiteral("ups beeper status"), ups_beeper_status);
                setItem(item, QStringLiteral("ups delay shutdown"), ups_delay_shutdown);
                setItem(item, QStringLiteral("ups delay start"), ups_delay_start);
                setItem(item, QStringLiteral("ups firmware"), ups_firmware);
                setItem(item, QStringLiteral("ups load"), ups_load);
                setItem(item, QStringLiteral("ups mfr"), ups_mfr);
                setItem(item, QStringLiteral("ups model"), ups_model);
                setItem(item, QStringLiteral("ups productid"), ups_productid);
                setItem(item, QStringLiteral("ups type"), ups_type);
                setItem(item, QStringLiteral("ups vendorid"), ups_vendorid);
            }

            QSqlQuery qry(m_db);
            if (qry.prepare("insert into ups(datum, battery_voltage_high, battery_voltage_low, "
                            "battery_voltage_nominal, device_mfr, device_model,"
                            "device_type, driver_name, driver_parameter_bus, driver_parameter_pollinterval, "
                            "driver_parameter_port, driver_parameter_product,"
                            "driver_parameter_productid, driver_parameter_serial, driver_parameter_synchronous, "
                            "driver_parameter_vendor,"
                            "driver_parameter_vendorid, driver_version, driver_version_internal, "
                            "input_current_nominal, input_frequency,"
                            "ups_beeper_status, ups_delay_shutdown, ups_delay_start, ups_firmware, ups_load, ups_mfr, "
                            "ups_model, ups_productid,"
                            "ups_type, ups_vendorid) "
                            "values(CURRENT_TIMESTAMP, :battery_voltage_high, :battery_voltage_low, "
                            ":battery_voltage_nominal, :device_mfr, :device_model,"
                            ":device_type, :driver_name, :driver_parameter_bus, :driver_parameter_pollinterval, "
                            ":driver_parameter_port, :driver_parameter_product,"
                            ":driver_parameter_productid, :driver_parameter_serial, :driver_parameter_synchronous, "
                            ":driver_parameter_vendor,"
                            ":driver_parameter_vendorid, :driver_version, :driver_version_internal, "
                            ":input_current_nominal, :input_frequency,"
                            ":ups_beeper_status, :ups_delay_shutdown, :ups_delay_start, :ups_firmware, :ups_load, "
                            ":ups_mfr, :ups_model, :ups_productid,"
                            ":ups_type, :ups_vendorid)"))
            {
                qry.bindValue(":battery_voltage_high", battery_voltage_high);
                qry.bindValue(":battery_voltage_low", battery_voltage_low);

                qry.bindValue(":battery_voltage_nominal", battery_voltage_nominal);
                qry.bindValue(":device_mfr", device_mfr);
                qry.bindValue(":device_model", device_model);

                qry.bindValue(":device_type", device_type);
                qry.bindValue(":driver_name", driver_name);
                qry.bindValue(":driver_parameter_bus", driver_parameter_bus);
                qry.bindValue(":driver_parameter_pollinterval", driver_parameter_pollinterval);

                qry.bindValue(":driver_parameter_port", driver_parameter_port);
                qry.bindValue(":driver_parameter_product", driver_parameter_product);

                qry.bindValue(":driver_parameter_productid", driver_parameter_productid);
                qry.bindValue(":driver_parameter_serial", driver_parameter_serial);
                qry.bindValue(":driver_parameter_synchronous", driver_parameter_synchronous);

                qry.bindValue(":driver_parameter_vendor", driver_parameter_vendor);

                qry.bindValue(":driver_parameter_vendorid", driver_parameter_vendorid);
                qry.bindValue(":driver_version", driver_version);
                qry.bindValue(":driver_version_internal", driver_version_internal);

                qry.bindValue(":input_current_nominal", input_current_nominal);
                qry.bindValue(":input_frequency", input_frequency);

                qry.bindValue(":ups_beeper_status", ups_beeper_status);
                qry.bindValue(":ups_delay_shutdown", ups_delay_shutdown);
                qry.bindValue(":ups_delay_start", ups_delay_start);
                qry.bindValue(":ups_firmware", ups_firmware);
                qry.bindValue(":ups_load", ups_load);

                qry.bindValue(":ups_mfr", ups_mfr);
                qry.bindValue(":ups_model", ups_model);
                qry.bindValue(":ups_productid", ups_productid);

                qry.bindValue(":ups_type", ups_type);
                qry.bindValue(":ups_vendorid", ups_vendorid);

                if (qry.exec())
                {
                    m_db.commit();
                }
                else
                {
                    PRINT("EXEC FAILED: DBERR:{} QRYERR:{} SQL:[{}]", m_db.lastError().text(), qry.lastError().text(),
                          qry.lastQuery());
                    m_db.rollback();
                }
            }
            else
                PRINT("PREPARE FAILED: [{}] [{}]", m_db.lastError().text(), qry.lastError().text());
        }
        else
            PRINT("CAN NOT INIT TRANSACTION: {}", m_db.lastError().text());
    }
    else
        PRINT("CAN NOT INIT DB: {}", m_db.lastError().text());
}

QVector<QPair<QDateTime, QString>> LogDB::getData()
{
    if (initDB())
    {
        QSqlQuery qry(m_db);
        if (qry.prepare("select to_char(datum, 'yyyymmddHH24MISSMS'), "
                        "battery_charge, "
                        "input_voltage_fault, "
                        "output_voltage from ups_data"))
        {
            if (not qry.exec())
            {
                PRINT("getData FAILED! [{}]:[{}]", qry.lastError().text().toUtf8().constData(),
                      qry.executedQuery().toUtf8().constData());
            }
            else
            {
                QVector<QPair<QDateTime, QString>> data;
                while (qry.next())
                {
                    QDateTime utc { QDateTime::fromString(qry.value(0).toString(), "yyyyMMddhhmmsszzz") };
                    utc.setTimeZone(QTimeZone::UTC);
                    data.append(qMakePair(utc.toLocalTime(), QString("%1:%2:%3")
                                                                 .arg(qry.value(1).toString(), qry.value(2).toString(),
                                                                      qry.value(3).toString())));
                }
                return data;
            }
        }
        else
        {
            PRINT("getData prepare FAILED! [{}]", qry.lastError().text().toLocal8Bit().constData());
        }
    }
    return {};
}

QVector<QPair<QDateTime, QString>> LogDB::getData(const QDateTime& from, const QDateTime& to)
{
    if (initDB())
    {
        QSqlQuery qry(m_db);
        if (qry.prepare(QStringLiteral("select to_char(datum, 'yyyymmddHH24MISSMS'), "
                                       "battery_charge, "
                                       "input_voltage_fault, "
                                       "output_voltage from ups_data "
                                       "where datum >= '%1' and datum <= '%2'")
                            .arg(from.toString("yyyy-MM-dd hh:mm:ss"), to.toString("yyyy-MM-dd hh:mm:ss"))))
        {
            if (not qry.exec())
            {
                PRINT("getData FAILED! [{}]:[{}]", qry.lastError().text().toUtf8().constData(),
                      qry.executedQuery().toUtf8().constData());
            }
            else
            {
                QVector<QPair<QDateTime, QString>> data;
                while (qry.next())
                {
                    QDateTime utc { QDateTime::fromString(qry.value(0).toString(), "yyyyMMddhhmmsszzz") };
                    utc.setTimeZone(QTimeZone::UTC);
                    data.append(qMakePair(utc.toLocalTime(), QString("%1:%2:%3")
                                                                 .arg(qry.value(1).toString(), qry.value(2).toString(),
                                                                      qry.value(3).toString())));
                }
                return data;
            }
        }
        else
        {
            PRINT("getData prepare FAILED! [{}]", qry.lastError().text().toLocal8Bit().constData());
        }
    }
    return {};
}

bool LogDB::isConnected()
{
    return m_db.isOpen();
}

bool LogDB::initDB()
{
    if (m_db.isOpen())
        return true;
    return m_db.open();
}
