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
#pragma once

#include <QDialog>
#include <QTimer>

#include "aslogger.h"
#include "db.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class ASUPSDlg;
}
QT_END_NAMESPACE

class ASUPSDlg : public QDialog
{
    Q_OBJECT

public:
    ASUPSDlg(QWidget* parent = nullptr);
    ~ASUPSDlg();

private Q_SLOTS:
    void doQueryTimeout();
    void doQueryDBTimeout();

    void on_pbQuery_clicked();

    void on_cbQueryPeriodically_stateChanged(int arg1);

    void on_pbUpdateDB_clicked();

    void on_cbAutoUpdateDB_stateChanged(int arg1);

    void on_pbUpdate_clicked();

    void on_pbLastUpdate_clicked();

private:
    Ui::ASUPSDlg* ui;
    ASLogger m_asLogger;
    QTimer m_updateTimer;
    QTimer m_updateDBTimer;
    void update();
    void updatedb();

    LogDB db;
};
