#ifndef SPECTRUMSELECTIONDIALOG_H
#define SPECTRUMSELECTIONDIALOG_H

#include <QDialog>
#include "Data/Dataset/vespuccidataset.h"
#include "GUI/QAbstractItemModel/spectratablemodel.h"
class MainWindow;
class SpectraTableModel;
class VespucciWorkspace;

namespace Ui {
class SpectrumSelectionDialog;
}

class SpectrumSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SpectrumSelectionDialog(QWidget *parent, MainWindow *main_window, QSharedPointer<VespucciDataset> dataset);
    ~SpectrumSelectionDialog();

private slots:
    void on_tableView_clicked(const QModelIndex &index);
    void SpectrumRemoved(int row);

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::SpectrumSelectionDialog *ui;
    SpectraTableModel *table_model_;
    QTableView *table_view_;
    QSharedPointer<VespucciDataset> dataset_;
    SpectrumViewer *spectrum_viewer_;
    MainWindow *main_window_;
    VespucciWorkspace *workspace;
};

#endif // SPECTRUMSELECTIONDIALOG_H
