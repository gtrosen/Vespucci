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
#include "GUI/Display/spectrumviewer.h"
#include "Global/vespucci.h"
#include "Global/global.h"
#include "ui_spectrumviewer.h"

///
/// \brief SpectrumViewer::SpectrumViewer
/// The constructor for when a spectrum viewer is linked to a MapViewer
/// \param parent
/// \param map_data
/// \param x_axis_description
/// \param y_axis_description
/// \param dataset
/// \param widget_size
/// \param directory
///
SpectrumViewer::SpectrumViewer(MapViewer *parent,
                               MapData *map_data,
                               const QString x_axis_description,
                               const QString y_axis_description,
                               QSharedPointer<VespucciDataset> dataset,
                               QSize widget_size,
                               QString directory) :
    QDialog(parent),
    ui(new Ui::SpectrumViewer)
{
    ui->setupUi(this);
    linked_to_map_ = true;
    map_data_ = map_data;
    hold_check_box_ = findChild<QCheckBox *>("holdCheckBox");
    spectrum_plot_ = findChild<QCustomPlot *>("spectrum");
    spectrum_plot_->addGraph();
    spectrum_plot_->xAxis->setLabel(x_axis_description);
    spectrum_plot_->yAxis->setLabel(y_axis_description);

    spectrum_plot_->replot();

    QVector<double> plot_data = dataset->PointSpectrum(0);
    QVector<double> wavelength = dataset->WavelengthQVector();

    coordinate_label_ = findChild<QLabel *>("coordinateLabel");
    value_label_ = findChild<QLabel *>("valueLabel");

    spectrum_plot_->graph(0)->addData(wavelength, plot_data);


    spectrum_plot_->xAxis->setRange(dataset->WavelengthRange());
    spectrum_plot_->yAxis->setRange(dataset->PointSpectrumRange(0));
    dataset_ = dataset;
    widget_size_ = widget_size;
    directory_ = directory;

    spectrum_plot_->setInteraction(QCP::iRangeDrag, true);
    spectrum_plot_->setInteraction(QCP::iRangeZoom, true);
}

///
/// \brief SpectrumViewer::SpectrumViewer
/// A constructor for using a spectrum viewer to view VCA endmembers
/// \param parent
/// \param dataset
/// \param endmember
/// \param directory
///
SpectrumViewer::SpectrumViewer(DataViewer *parent,
                               QSharedPointer<VespucciDataset> dataset,
                               int endmember,
                               QString directory,
                               QString type) :
    QDialog(parent),
    ui(new Ui::SpectrumViewer)
{
    ui->setupUi(this);
    dataset_ = dataset;
    hold_check_box_ = findChild<QCheckBox *>("holdCheckBox");
    coordinate_label_ = findChild<QLabel *>("coordinateLabel");
    value_label_ = findChild<QLabel *>("valueLabel");
    coordinate_label_->setVisible(false);
    value_label_->setVisible(false);
    spectrum_plot_ = findChild<QCustomPlot *>("spectrum");
    spectrum_plot_->addGraph();
    spectrum_plot_->xAxis->setLabel(dataset->x_axis_description());
    spectrum_plot_->yAxis->setLabel(dataset->y_axis_description());
    QVector<double> plot_data;
    spectrum_plot_->replot();
    if (type == "VCA"){
        plot_data = dataset_->vertex_components_data()->EndmemberQVec(endmember);
    }

    QVector<double> wavelength = dataset->WavelengthQVector();
    coordinate_label_ = findChild<QLabel *>("coordinateLabel");
    value_label_ = findChild<QLabel *>("valueLabel");
    spectrum_plot_->graph(0)->addData(wavelength, plot_data);
    directory_ = directory;

    spectrum_plot_->setInteraction(QCP::iRangeDrag, true);
    spectrum_plot_->setInteraction(QCP::iRangeZoom, true);
    spectrum_plot_->rescaleAxes();
}

SpectrumViewer::SpectrumViewer(SpectrumSelectionDialog *parent,
                               QSharedPointer<VespucciDataset> dataset):
    QDialog(parent),
    ui(new Ui::SpectrumViewer)
{
    ui->setupUi(this);
    cout << "alternative SpectrumViewer constructor" << endl;
    dataset_ = dataset;
    hold_check_box_ = findChild<QCheckBox *>("holdCheckBox");

    cout << "find labels " << endl;
    coordinate_label_ = findChild<QLabel *>("coordinateLabel");
    value_label_ = findChild<QLabel *>("valueLabel");
    cout << "edit labels" << endl;
    coordinate_label_->setVisible(false);
    value_label_->setVisible(false);

    cout << "find plot" << endl;
    spectrum_plot_ = findChild<QCustomPlot *>("spectrum");

    cout << "set labels" << endl;
    spectrum_plot_->addGraph();
    spectrum_plot_->xAxis->setLabel(dataset->x_axis_description());
    spectrum_plot_->yAxis->setLabel(dataset->y_axis_description());

    cout << "set data" << endl;
    QVector<double> wavelength = dataset->WavelengthQVector();
    QVector<double> plot_data = dataset->PointSpectrum(0);
    spectrum_plot_->graph(0)->addData(wavelength, plot_data);
    dataset_ = dataset;

    spectrum_plot_->setInteraction(QCP::iRangeDrag, true);
    spectrum_plot_->setInteraction(QCP::iRangeZoom, true);
    spectrum_plot_->rescaleAxes();
    cout << "end of constructor" << endl;
}

SpectrumViewer::SpectrumViewer(DataViewer *parent,
                               QSharedPointer<VespucciDataset> dataset,
                               vec y, QString ordinate_label):
    QDialog(parent),
    ui(new Ui::SpectrumViewer)
{
    ui->setupUi(this);
    cout << "alternative SpectrumViewer constructor" << endl;
    dataset_ = dataset;
    hold_check_box_ = findChild<QCheckBox *>("holdCheckBox");

    cout << "find labels " << endl;
    coordinate_label_ = findChild<QLabel *>("coordinateLabel");
    value_label_ = findChild<QLabel *>("valueLabel");
    cout << "edit labels" << endl;
    coordinate_label_->setVisible(false);
    value_label_->setVisible(false);

    cout << "find plot" << endl;
    spectrum_plot_ = findChild<QCustomPlot *>("spectrum");

    cout << "set labels" << endl;
    spectrum_plot_->addGraph();
    spectrum_plot_->xAxis->setLabel(dataset->x_axis_description());
    spectrum_plot_->yAxis->setLabel(ordinate_label);

    cout << "set data" << endl;
    QVector<double> wavelength = dataset->WavelengthQVector();
    QVector<double> plot_data =
            QVector<double>::fromStdVector(conv_to<std::vector<double> >::from(y));
    spectrum_plot_->graph(0)->addData(wavelength, plot_data);
    dataset_ = dataset;

    spectrum_plot_->setInteraction(QCP::iRangeDrag, true);
    spectrum_plot_->setInteraction(QCP::iRangeZoom, true);
    spectrum_plot_->rescaleAxes();
}

\



SpectrumViewer::~SpectrumViewer()
{
    delete ui;
}

void SpectrumViewer::ResetPlot()
{
    spectrum_plot_->clearGraphs();
}

///
/// \brief SpectrumViewer::SetPlot
/// \param wavelength
/// \param intensity
/// Set the plot of the spectrum viewer
void SpectrumViewer::SetPlot(QVector<double> wavelength,
                             QVector<double> intensity)
{
    spectrum_plot_->clearGraphs();
    spectrum_plot_->addGraph();
    spectrum_plot_->graph(0)->setData(wavelength, intensity);
    spectrum_plot_->rescaleAxes();
    spectrum_plot_->replot();
}

void SpectrumViewer::AddPlot(QVector<double> abscissa, QVector<double> intensity)
{
    spectrum_plot_->addGraph(spectrum_plot_->graph(0)->keyAxis(), spectrum_plot_->graph(0)->valueAxis());
    spectrum_plot_->graph(spectrum_plot_->graphCount() - 1)->addData(abscissa, intensity);
    spectrum_plot_->rescaleAxes();
    spectrum_plot_->replot();
}

///
/// \brief SpectrumViewer::MapClicked
/// \param plottable The QCPColorMap
/// \param event The QMouseEvent
/// A slot which receives the locations
void SpectrumViewer::MapClicked(QCPAbstractPlottable *plottable, QMouseEvent *event)
{
    QCPColorMap *color_map = qobject_cast<QCPColorMap*>(plottable);
    current_x_ = color_map->keyAxis()->pixelToCoord(event->x());
    current_y_ = color_map->valueAxis()->pixelToCoord(event->y());
    current_z_ = color_map->data()->data(current_x_, current_y_);
    arma::uvec row = arma::find(map_data_->results_ == current_z_);
    if (row.n_elem == 0)
        return;
    else
        current_index_ = row(0);

    QVector<double> wavelength = dataset_->WavelengthQVector();
    QVector<double> intensities = dataset_->PointSpectrum(current_index_);
    double x_value = dataset_->x(current_index_);
    double y_value = dataset_->y(current_index_);

    coordinate_label_->setText("(" +
                               QString::number(x_value) +
                               ", " +
                               QString::number(y_value) +
                               ")");
    value_label_->setText(QString::number(current_z_));

    if (hold_check_box_->isChecked()){
        AddPlot(wavelength, intensities);
    }
    else{
        SetPlot(wavelength, intensities);
    }

    if (map_data_->univariate_area()){
        AddPlot(map_data_->abscissa(current_index_), map_data_->baseline(current_index_));
    }
    if (map_data_->univariate_bandwidth()){
        AddPlot(map_data_->abscissa(current_index_), map_data_->baseline(current_index_));
        AddPlot(map_data_->midline_abscissa(current_index_), map_data_->midline(current_index_));
    }
    if (map_data_->band_ratio_area()){
        AddPlot(map_data_->first_abscissa(current_index_), map_data_->first_baseline(current_index_));
        AddPlot(map_data_->second_abscissa(current_index_), map_data_->second_baseline(current_index_));
    }
}

///
/// \brief SpectrumViewer::on_pushButton_clicked
/// Exports the data or a picture of the plot
void SpectrumViewer::on_pushButton_clicked()
{
    if (!linked_to_map_){
        //only images can be saved in this view
        QString filename =
                QFileDialog::getSaveFileName(this,
                                             tr("Save File"),
                                             directory_,
                                             tr("Scalable Vector Graphics (*.svg);; "
                                                "Enhanced Windows Metafile (*.emf);; "
                                                "Portable Document Format (*.pdf);; "
                                                "Tagged Image File Format (*.tif);; "
                                                "Windows Bitmap (*.bmp);; "
                                                "Portable Network Graphics (*.png);; "
                                                "JPEG (*.jpg)"));
        bool success = Vespucci::SavePlot(spectrum_plot_, filename);
        if(success)
            QMessageBox::information(this, "Success!", "File " + filename + " written successfully");
        else
            QMessageBox::warning(this, "File Save Failed", "File " + filename + " was not written successfully");
        return; //don't do any of the other stuff
    }





    QString filename =
            QFileDialog::getSaveFileName(this,
                                         "Save As...",
                                         directory_,
                                         tr("Tab-delimited text (*.txt);;"
                                            "Comma-separated Values (*.csv);;"
                                            "Scalable Vector Graphics (*.svg);; "
                                            "Enhanced Windows Metafile (*.emf);; "
                                            "Portable Document Format (*.pdf);; "
                                            "Tagged Image File Format (*.tif);; "
                                            "Windows Bitmap (*.bmp);; "
                                            "Portable Network Graphics (*.png);; "
                                            "JPEG (*.jpg)"
                                            ));
    bool success;
    QFileInfo file_info(filename);

    QString extension = file_info.suffix();

    vec spectrum = dataset_->spectra_ptr()->col(current_index_);
    vec wavelength = dataset_->wavelength();
    mat results;
    results.insert_rows(0, wavelength);
    results.insert_rows(1, spectrum);
    results = results.t();

    if ( (extension== "pdf")
       ||(extension == "tif")
       ||(extension == "bmp")
       ||(extension == "jpg")
       ||(extension == "png")
       ||(extension == "svg")
       ||(extension == "emf")){
        success = Vespucci::SavePlot(spectrum_plot_, filename);
    }

    else if (extension == "csv")
        success = results.save(filename.toStdString(), csv_ascii);
    else if (extension == "arma")
        success = spectrum.save(filename.toStdString(), arma_binary);
    else
        success = results.save(filename.toStdString(), raw_ascii);

    if (success)
        QMessageBox::information(this, "Success!", "Saving " + filename + " successful");
    else
        QMessageBox::warning(this, "Failure", "Saving " + filename + " not successful");



}

void SpectrumViewer::SpectrumChanged(QVector<double> &wavelength, QVector<double> &intensity)
{
    SetPlot(wavelength, intensity);
}
