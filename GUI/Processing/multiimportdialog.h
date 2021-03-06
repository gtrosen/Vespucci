/*******************************************************************************
    Copyright (C) 2015 Wright State University - All Rights Reserved
    Daniel P. Foose - Author

    This file is part of Vespucci.

    Vespucci is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Vespucci is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Vespucci.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/
#ifndef MULTIIMPORTDIALOG_H
#define MULTIIMPORTDIALOG_H

#include <QDialog>
#include "Global/vespucciworkspace.h"

namespace Ui {
class MultiImportDialog;
}

class MultiImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MultiImportDialog(QWidget *parent, VespucciWorkspace * ws);
    ~MultiImportDialog();

private slots:
    void on_rowSpinBox_valueChanged(int arg1);

    void on_colSpinBox_valueChanged(int arg1);

    void on_addFilesPushButton_clicked();

    void on_buttonBox_accepted();

private:
    Ui::MultiImportDialog *ui;
    VespucciWorkspace *workspace;
    QTableWidget *filename_table_;
    QLabel *count_label_;
    QLineEdit *name_box_;
    QLineEdit *abscissa_label_box_;
    QLineEdit *ordinate_label_box_;

};

#endif // MULTIIMPORTDIALOG_H
