#include "GUI/Display/reportmessagedialog.h"
#include "ui_reportmessagedialog.h"

ReportMessageDialog::ReportMessageDialog(QWidget *parent, const QString &title) :
    QDialog(parent),
    ui(new Ui::ReportMessageDialog)
{
    ui->setupUi(this);
    label_ = findChild<QLabel*>("label");
    text_edit_ = findChild<QPlainTextEdit*>("plainTextEdit");
    setWindowTitle(title);
}

void ReportMessageDialog::setLabel(const QString &new_label)
{
    label_->setText(new_label);
}

void ReportMessageDialog::appendPlainText(const QString &text)
{
    text_edit_->appendPlainText(text);
}



ReportMessageDialog::~ReportMessageDialog()
{
    delete ui;
}

void ReportMessageDialog::on_pushButton_clicked()
{
    close();
}
