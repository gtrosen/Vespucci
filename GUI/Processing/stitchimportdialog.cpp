#include "stitchimportdialog.h"
#include "ui_stitchimportdialog.h"
#include <Data/Import/binaryimport.h>
#include "Data/Import/textimportqpd.h"

StitchImportDialog::StitchImportDialog(QWidget *parent, VespucciWorkspace *ws) :
    QDialog(parent),
    ui(new Ui::StitchImportDialog)
{
    ui->setupUi(this);
    workspace = ws;

    x_line_edit_ = findChild<QLineEdit*>("xLineEdit");
    y_line_edit_ = findChild<QLineEdit*>("yLineEdit");
    x_units_line_edit_ = findChild<QLineEdit*>("xUnitsLineEdit");
    y_units_line_edit_ = findChild<QLineEdit*>("yUnitsLineEdit");
    filename_line_edit_ = findChild<QLineEdit*>("filenameLineEdit");
    data_format_combo_box_ = findChild<QComboBox*>("formatComboBox");
    name_line_edit_ = findChild<QLineEdit*>("nameLineEdit");
    swap_spatial_check_box_ = findChild<QCheckBox*>("swapSpatialCheckBox");

}

StitchImportDialog::~StitchImportDialog()
{
    delete ui;
}

void StitchImportDialog::on_browsePushButton_clicked()
{
    QString filename =
            QFileDialog::getOpenFileName(this,
                                         "Select Instruction File",
                                         workspace->directory(),
                                         "Instruction File (*.csv)");
    filename_line_edit_->setText(filename);
}

bool StitchImportDialog::LoadDatasets(field<string> filenames, mat &spectra, vec &x, vec &y, vec &abscissa, bool swap_spatial, QString type)
{
    //follows the vespuccci binary field format:
    field<field<mat> > datasets(filenames.n_rows, filenames.n_cols);
    mat current_spectra;
    vec current_x, current_y, current_abscissa;
    bool ok;
    bool two_dim = datasets.n_rows > 1 && datasets.n_cols > 1;
    QProgressDialog progress(this);
    for (uword j = 0; j < filenames.n_cols; ++j){
        for (uword i = 0; i < filenames.n_rows; ++i){
            cout << "i = " << i << endl << "j = " << j << endl;
            QString filename;
            if (two_dim)
                filename = path_ + "/" + QString::fromStdString(filenames(i, j));
            else
                filename = path_ + "/" + QString::fromStdString(filenames(i));
            if (type == "Vespucci Dataset"){

                ok = BinaryImport::ImportVespucciBinary(filename.toStdString(),
                                                        current_spectra,
                                                        current_abscissa,
                                                        current_x,
                                                        current_y);
            }
            else if (type == "Wide Text"){
                ok = TextImport::ImportWideText(filename,
                                                current_spectra,
                                                current_abscissa,
                                                current_x,
                                                current_y,
                                                swap_spatial,
                                                &progress,
                                                "\t");

            }
            else if (type == "Wide CSV"){
                ok = TextImport::ImportWideText(filename,
                                                current_spectra,
                                                current_abscissa,
                                                current_x,
                                                current_y,
                                                swap_spatial,
                                                &progress,
                                                ",");
            }
            else if (type == "Long Text" || "Long CSV"){
                ok = TextImport::ImportLongText(filename,
                                                current_spectra,
                                                current_abscissa,
                                                current_x,
                                                current_y,
                                                swap_spatial,
                                                &progress);
            }
            else{
                cout << "Improper!" << endl;
                return false;
            }



            if (ok){
                field<mat> dataset(4);
                dataset(0) = current_spectra;
                dataset(1) = current_abscissa;
                dataset(2) = current_x;
                dataset(3) = current_y;
                datasets(i, j) = dataset;
            }
        }
    }

    try{
        ok = Vespucci::StitchDatasets(datasets, spectra, x, y, abscissa);
    }catch(exception e){
        workspace->main_window()->DisplayExceptionWarning("Vespucci::StitchDatasets", e);
        ok = false;
    }

    return ok;
}

void StitchImportDialog::on_buttonBox_accepted()
{
    QString filename = filename_line_edit_->text();
    path_ = QFileInfo(filename).absolutePath();
    QString data_format = data_format_combo_box_->currentText();
    bool swap_spatial = swap_spatial_check_box_->isChecked();
    bool ok;
    mat spectra;
    vec x, y, abscissa;
    field<string> instruction;
    ok = instruction.load(filename.toStdString());
    if (!ok){
        QMessageBox::warning(this,
                             "Failed to Load Instructions",
                             "The instruction file could not be loaded");
    }
    try{
        ok = LoadDatasets(instruction,
                          spectra, x, y, abscissa,
                          swap_spatial, data_format);
    }catch (exception e){
        workspace->main_window()->DisplayExceptionWarning(e);
        ok = false;
    }
    if (!ok){
        QMessageBox::warning(this,
                             "Failed to Load Dataset",
                             "One or more datasets could not be loaded, or "
                             "datasets of incompatible spatial coordinates.");
    }
    else{
        QString x_description = x_line_edit_->text();
        QString x_units = x_units_line_edit_->text();
        QString y_description = y_line_edit_->text();
        QString y_units = y_units_line_edit_->text();
        QString name = name_line_edit_->text();
        QSharedPointer<VespucciDataset>
                dataset(new VespucciDataset(name,
                                            workspace->main_window(),
                                            workspace->directory_ptr(),
                                            workspace->CreateLogFile(name)));
        dataset->SetData(spectra, abscissa, x, y);
        dataset->SetXDescription(x_description + " (" + x_units + ")");
        dataset->SetYDescription(y_description + " (" + y_units + ")");
        workspace->AddDataset(dataset);
    }
}
