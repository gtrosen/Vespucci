#include "peakfindingdialog.h"
#include "ui_peakfindingdialog.h"
#include "Data/Analysis/cwtdata.h"
PeakFindingDialog::PeakFindingDialog(QWidget *parent, VespucciWorkspace *ws, int row) :
    QDialog(parent),
    ui(new Ui::PeakFindingDialog)
{
    workspace = ws;
    ui->setupUi(this);
    dataset_ = ws->DatasetAt(row);

    save_coefs_box_ = findChild<QCheckBox *>("coefCheckBox");
    gradient_box_ = findChild<QComboBox *>("gradientComboBox");
    format_box_ = findChild<QComboBox *>("formatComboBox");
    method_box_ =findChild<QComboBox *>("methodComboBox");
    save_coef_plots_box_ = findChild<QCheckBox *>("coefPlotCheckBox");
    directory_line_edit_ = findChild<QLineEdit *>("directoryLineEdit");
    method_box_ = findChild<QComboBox *>("methodComboBox");
    save_ridges_box_ = findChild<QCheckBox *>("ridgeCheckBox");
    save_ridge_plots_box_ = findChild<QCheckBox *>("ridgePlotCheckBox");
    width_box_ = findChild<QCheckBox *>("widthCheckBox");
    noise_box_ = findChild<QDoubleSpinBox *>("noiseDoubleSpinBox");
    scale_box_ = findChild<QSpinBox *>("scaleSpinBox");
    search_box_ = findChild<QSpinBox *>("searchSpinBox");
    gap_box_ = findChild<QSpinBox *>("gapSpinBox");
    length_box_ = findChild<QSpinBox *>("lengthSpinBox");

    gradient_box_->setCurrentIndex(18); //set index to QCP Grayscale

}

PeakFindingDialog::~PeakFindingDialog()
{
    delete ui;
}

void PeakFindingDialog::on_browseButton_clicked()
{
    QString directory = "C:/";

    //QString directory = workspace->directory();
    if (directory.isEmpty()){
        directory = QStandardPaths::displayName(QStandardPaths::DocumentsLocation);
    }

    QString save_directory = QFileDialog::getExistingDirectory(this, "Select Save Directory", directory);
    directory_line_edit_->setText(save_directory);

}

void PeakFindingDialog::on_buttonBox_accepted()
{
    QString save_path = directory_line_edit_->text();
    int method_num = method_box_->currentIndex(); //currently discarded as CWT is only implemented method
    bool estimate_width = width_box_->isChecked();
    bool save_coefs = save_coef_plots_box_->isChecked();
    bool save_coef_plots = save_coef_plots_box_->isChecked();
    bool save_ridges = save_ridges_box_->isChecked();
    bool save_ridge_plots = save_ridge_plots_box_->isChecked();
    QCPColorGradient gradient = dataset_->GetGradient(0);

    cout << "length_box->value = " << endl;
    cout << length_box_->value();

    int max_scale = scale_box_->value();
    int min_length = length_box_->value();
    int search_width = search_box_->value();
    int gap_threshold = gap_box_->value();
    double noise_threshold = noise_box_->value();
    QString file_format = format_box_->currentText();

    if (method_num == 0){
        dataset_->FindPeaksCWT("mexh",
                               max_scale, gap_threshold, min_length, search_width,
                               noise_threshold, "quantile", 500,
                               save_coefs, save_coef_plots,
                               save_ridge_plots, save_ridges, estimate_width,
                               save_path, file_format, gradient);

    }
}
