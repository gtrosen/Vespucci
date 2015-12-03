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
#include "Data/Dataset/vespuccidataset.h" //VespucciDataset includes all necessary headers.
#include <mlpack/methods/kmeans/kmeans.hpp>
#include <mlpack/methods/quic_svd/quic_svd.hpp>


using namespace arma;
using namespace std;


///
/// \brief VespucciDataset::~VespucciDataset
/// Destructor deletes everything allocated with new that isn't a smart pointer
VespucciDataset::~VespucciDataset()
{
    //Delete heap-allocated objects that are not stored in smart pointers
    /*
    if (principal_components_calculated_)
        delete principal_components_data_;
    if (partial_least_squares_calculated_)
        delete partial_least_squares_data_;
    if (vertex_components_calculated_)
        delete vertex_components_data_;
    */
    //make sure all mfaps are delted properly.
    map_list_model_->ClearMaps();

    //This will be called for each dataset in the list model when it is removed.
    DestroyLogFile();
    //delete map_list_model_;

}

bool VespucciDataset::Save(QString filename)
{
    bool success;
    try{
        arma::field<mat> dataset(4);
        dataset(0) = spectra_;
        dataset(1) = abscissa_;
        dataset(2) = x_;
        dataset(3) = y_;
        success = dataset.save(filename.toStdString(), arma_binary);
    }
    catch(exception e){
        cerr << "See armadillo exception" << endl;

        string str = "VespucciDataset::Save: " + string(e.what());
        throw std::runtime_error(str);
    }

    return success;

}

bool VespucciDataset::SaveSpectrum(QString filename, uword column, file_type type)
{
    std::string filename_stdstring = filename.toStdString();
    vec spectrum;
    bool success;
    try{
        spectrum = spectra_.col(column);
        success = spectrum.save(filename_stdstring, type);
    }
    catch(exception e){
        main_window_->DisplayExceptionWarning("VespucciDataset::SaveSpectrum", e);
        success = false;
    }
    return success;

}

bool VespucciDataset::SaveLogFile(QString filename)
{
    return log_file_->copy(filename);
}



///
/// \brief VespucciDataset::DestroyLogFile
/// Deletes log file unless user decides to save it elsewhere
void VespucciDataset::DestroyLogFile()
{
    QMessageBox::StandardButton reply =
            QMessageBox::question(main_window_,
                                  "Save log?",
                                  "Would you like to save the log for " + name_
                                  + "?",
                                  QMessageBox::Yes|QMessageBox::No);
    QString filename;


    if (reply == QMessageBox::No){
        log_file_->remove();
        return;
    }
    else{
        filename = QFileDialog::getSaveFileName(0, QTranslator::tr("Save File"),
                                   *directory_,
                                   QTranslator::tr("Text Files (*.txt)"));        
        bool success = log_file_->copy(filename);
        if (!success){
            bool remove_success = QFile::remove(filename); //delete old file
            if (!remove_success){
                QMessageBox::warning(main_window_, "Failure", "Previous file could not be removed");
            }
            success = log_file_->copy(filename);
        }
        //new log file falls out of scope
        if(log_file_->isOpen()){log_file_->close();}
        QFile::remove(log_file_->fileName());

        if (success)
            QMessageBox::information(main_window_, "Success!", "File " + filename + " written successfully");
        else
            QMessageBox::warning(main_window_, "Failure", "File not written successfully.");
    }
    return;
}

void VespucciDataset::SetOldCopies()
{
    spectra_old_ = spectra_;
    x_old_ = x_;
    y_old_ = y_;
    abscissa_old_ = abscissa_;
}

///
/// \brief VespucciDataset::VespucciDataset
/// \param binary_filename The filename of the binary input file
/// \param main_window The main window of the program
/// \param directory The current working directory
/// \param name The name of the dataset displayed to the user
/// \param log_file The log file
/// Constructor for loading saved spectral/spatial data in armadillo format
VespucciDataset::VespucciDataset(QString vespucci_binary_filename,
                                 MainWindow *main_window,
                                 QString *directory,
                                 QFile *log_file,
                                 QString name,
                                 QString x_axis_description,
                                 QString y_axis_description)
    : log_stream_(log_file)
{
    QDateTime datetime = QDateTime::currentDateTimeUtc();
    log_file_ = log_file;
    log_stream_ << "Vespucci, a free, cross-platform tool for spectroscopic imaging" << endl;
    log_stream_ << "Version 1.0" << endl << endl;
    log_stream_ << "Dataset " << name << "created "
                << datetime.date().toString("yyyy-MM-dd") << "T"
                << datetime.time().toString("hh:mm:ss") << "Z" << endl;
    log_stream_ << "Imported from binary file " << vespucci_binary_filename << endl << endl;

    non_spatial_ = false;
    meta_ = false;
    //Set up variables unrelated to hyperspectral data:
    map_list_model_ = new MapListModel(main_window, this);

    map_loading_count_ = 0;
    principal_components_calculated_ = false;
    mlpack_pca_calculated_ = false;
    partial_least_squares_calculated_ = false;
    vertex_components_calculated_ = false;
    k_means_calculated_ = false;
    z_scores_calculated_ = false;
    directory_ = directory;
    main_window_ = main_window;
    x_axis_description_ = x_axis_description;
    y_axis_description_ = y_axis_description;
    name_ = name;
    try{
        BinaryImport::ImportVespucciBinary(vespucci_binary_filename.toStdString(),
                                           spectra_,
                                           abscissa_,
                                           x_, y_);
        indices_.set_size(x_.n_elem);
        for (uword i = 0; i < indices_.n_elem; ++i)
            indices_(i) = i;

    }
    catch(exception e) {
        string str = "BinaryImport: " + string(e.what());
        throw std::runtime_error(str);

    }

}




///
/// \brief VespucciDataset::VespucciDataset
/// \param text_filename The filename of the text file from which data is imported
/// \param main_window The main window of the program
/// \param directory The current global working directory
/// \param log_file The log file
/// \param name The name of the dataset displayed to the user
/// \param x_axis_description A description of the spectral abscissa
/// \param y_axis_description A description of the spectral ordinate
/// \param swap_spatial Whether or not y is located in the first column instead of the second (some Horiba spectrometers do this).
/// Main function for processing data from text files to create VespucciDataset objects.
/// Currently written to accept files in "wide" format, will be expanded to deal
/// with different ASCII formats later with conditionals.
VespucciDataset::VespucciDataset(QString text_filename,
                                 MainWindow *main_window,
                                 QString *directory,
                                 QFile *log_file,
                                 QString name,
                                 QString x_axis_description,
                                 QString y_axis_description,
                                 bool swap_spatial,
                                 std::string format)
    : log_stream_(log_file)
{
    QDateTime datetime = QDateTime::currentDateTimeUtc();
    log_file_ = log_file;


    log_stream_ << "Dataset " << name << " created "
                << datetime.date().toString("yyyy-MM-dd") << "T"
                << datetime.time().toString("hh:mm:ss") << "Z" << endl;
    log_stream_ << "Imported from text file " << text_filename << endl << endl;


    non_spatial_ = false;
    meta_ = false;
    //Set up variables unrelated to hyperspectral data:
    map_list_model_ = new MapListModel(main_window, this);
    map_loading_count_ = 0;
    principal_components_calculated_ = false;
    mlpack_pca_calculated_ = false;
    partial_least_squares_calculated_ = false;
    vertex_components_calculated_ = false;
    k_means_calculated_ = false;
    z_scores_calculated_ = false;
    directory_ = directory;
    flipped_ = swap_spatial;

    QProgressDialog progress("Loading Dataset...", "Cancel", 0, 100, NULL);
    vec indices_temp;


    if (format == "WideTabDel"){
        try{
            constructor_canceled_ = TextImport::ImportWideText(text_filename,
                                                                   spectra_,
                                                                   abscissa_,
                                                                   x_, y_,
                                                                   swap_spatial,
                                                                   &progress,
                                                                   "\t");
            indices_.set_size(x_.n_elem);
            for (uword i = 0; i < indices_.n_elem; ++i)
                indices_(i) = i;

        }
        catch(exception e){
            string str = "Text import constructor:" +  string(e.what());
            throw std::runtime_error(str);
        }
    }
    else if (format == "WideCSV"){
        try{
            constructor_canceled_ = TextImport::ImportWideText(text_filename,
                                                               spectra_,
                                                               abscissa_,
                                                               x_, y_,
                                                               swap_spatial,
                                                               &progress,
                                                               ",");
            indices_.set_size(x_.n_elem);
            for (uword i = 0; i < indices_.n_elem; ++i)
                indices_(i) = i;

        }
        catch(exception e){
            string str = "Text import constructor: " + string(e.what());
            throw std::runtime_error(str);
        }
    }
    else if (format == "LongTabDel" || format == "LongCSV"){
        try{
            constructor_canceled_ = TextImport::ImportLongText(text_filename,
                                                               spectra_,
                                                               abscissa_,
                                                               x_, y_,
                                                               swap_spatial,
                                                               &progress);
            indices_.set_size(x_.n_elem);
            for (uword i = 0; i < indices_.n_elem; ++i)
                indices_(i) = i;
        }
        catch(exception e){
            string str = "Text import constructor: " + string(e.what());
            throw std::runtime_error(str);
        }
    }
    else{
        throw std::runtime_error("Invalid file format for text import constructor");
    }

    constructor_canceled_ = false;
    name_ = name;
    x_axis_description_ = x_axis_description;
    y_axis_description_ = y_axis_description;
    main_window_ = main_window;
}

VespucciDataset::VespucciDataset(map<pair<int, int>, string> text_filenames,
                                 MainWindow *main_window,
                                 QString *directory,
                                 QFile *log_file,
                                 QString name,
                                 QString x_axis_description,
                                 QString y_axis_description,
                                 int rows, int cols)
     : log_stream_(log_file)
{
    QDateTime datetime = QDateTime::currentDateTimeUtc();
    log_stream_ << "Vespucci, a free, cross-platform tool for spectroscopic imaging" << endl;
    log_stream_ << "Version 1.0" << endl << endl;
    log_stream_ << "Dataset " << name << "created "
                << datetime.date().toString("yyyy-MM-dd") << "T"
                << datetime.time().toString("hh:mm:ss") << "Z" << endl;
    log_stream_ << "Created from multiple text files"<< endl;
    non_spatial_ = false;
    meta_ = false;
    //Set up variables unrelated to hyperspectral data:
    map_list_model_ = new MapListModel(main_window, this);
    map_loading_count_ = 0;
    principal_components_calculated_ = false;
    mlpack_pca_calculated_ = false;
    partial_least_squares_calculated_ = false;
    vertex_components_calculated_ = false;
    k_means_calculated_ = false;
    z_scores_calculated_ = false;
    directory_ = directory;
    flipped_ = false;
    try{
        constructor_canceled_ = TextImport::ImportMultiplePoints(text_filenames,
                                                                 rows, cols,
                                                                 spectra_,
                                                                 abscissa_,
                                                                 x_,
                                                                 y_);

    }
    catch(exception e){
        string reason = "Multi import constructor: " + string(e.what());
        throw std::runtime_error(reason.c_str());
    }
    name_ = name;
    x_axis_description_ = x_axis_description;
    y_axis_description_ = y_axis_description;
    main_window_ = main_window;
}

///
/// \brief VespucciDataset::VespucciDataset
/// \param name The name of the dataset displayed to the user
/// \param main_window The main window of the program
/// \param directory The current global working directory
/// \param log_file The log file
/// \param original The dataset from which this dataset is "extracted"
/// \param indices The indices of the spectra in the previous dataset that will form this one
/// This is a constructor to create a new dataset by "extracting" spectra from a
/// another dataset based on values on an image.
VespucciDataset::VespucciDataset(QString name,
                                 MainWindow *main_window,
                                 QString *directory,
                                 QFile *log_file,
                                 QSharedPointer<VespucciDataset> original,
                                 uvec indices)
    : log_stream_(log_file)

{
    log_file_ = log_file;
    map_list_model_ = new MapListModel(main_window, this);
    QDateTime datetime = QDateTime::currentDateTimeUtc();
    log_stream_ << "Vespucci, a free, cross-platform tool for spectroscopic imaging" << endl;
    log_stream_ << "Version 1.0" << endl << endl;
    log_stream_ << "Dataset " << name << "created "
                << datetime.date().toString("yyyy-MM-dd") << "T"
                << datetime.time().toString("hh:mm:ss") << "Z" << endl;
    log_stream_ << "Created from previous dataset " << original->name() << endl;

    non_spatial_ = true;
    meta_ = original->meta();
    map_loading_count_ = 0;
    principal_components_calculated_ = false;
    mlpack_pca_calculated_ = false;
    partial_least_squares_calculated_ = false;
    vertex_components_calculated_ = false;
    k_means_calculated_ = false;
    z_scores_calculated_ = false;
    directory_ = directory;
    vec parent_indices;

    try{
        spectra_ = original->spectra(indices);
        abscissa_ = original->wavelength();
        x_ = original->x(indices);
        y_ = original->y(indices);
        parent_indices = original->indices();
        indices_ = parent_indices(indices);
    }
    catch(exception e){
        string str = "Extraction constructor: " + string(e.what());
        throw std::runtime_error(str);
    }

    name_ = name;
    main_window_ = main_window;
    directory_ = directory;
}

///
/// \brief VespucciDataset::VespucciDataset
/// \param name Dataset name
/// \param main_window The
/// \param directory
/// \param log_file
/// Constructor to create a dataset without spatial and spectral data set (i.e.
/// when using MetaDataset).
VespucciDataset::VespucciDataset(QString name,
                                 MainWindow *main_window,
                                 QString *directory,
                                 QFile *log_file)
    : log_stream_(log_file)
{
    map_list_model_ = new MapListModel(main_window, this);
    log_file_ = log_file;
    non_spatial_ = true;
    meta_ = true;
    map_loading_count_ = 0;
    principal_components_calculated_ = false;
    mlpack_pca_calculated_ = false;
    partial_least_squares_calculated_ = false;
    vertex_components_calculated_ = false;
    k_means_calculated_ = false;
    z_scores_calculated_ = false;
    directory_ = directory;
    name_ = name;
    main_window_ = main_window;
    directory_ = directory;
}


// PRE-PROCESSING FUNCTIONS //
///
/// \brief VespucciDataset::Undo Swap spectra_ and spectra_old_ to undo an action.
/// Calling this function again re-does the action that was undid.
///
void VespucciDataset::Undo()
{

    try{
        mat spectra_buffer = spectra_;
        colvec x_buffer = x_;
        colvec y_buffer = y_;
        vec abscissa_buffer = abscissa_;

        spectra_.swap(spectra_old_);
        x_.swap(x_old_);
        y_.swap(y_old_);
        abscissa_.swap(abscissa_old_);

        //spectra_ = spectra_old_;
        //x_ = x_old_;
        //y_ = y_old_;
        //abscissa_ = abscissa_old_;

        //spectra_old_ = spectra_buffer;
        //x_old_ = x_buffer;
        //y_old_ = y_buffer;
        //abscissa_old_ = abscissa_buffer;


    }
    catch(exception e){
        string str = "Undo: " + string(e.what());
        throw std::runtime_error(str);
    }

    log_stream_ << "Undo: " << last_operation_ << endl << endl;
    last_operation_ = "Undo";
}


///
/// \brief VespucciDataset::CropSpectra
/// Crops spectra_ based on
/// \param x_min value of x below which spectra are deleted
/// \param x_max value of x above which spectra are deleted
/// \param y_min value of y below which spectra are deleted
/// \param y_max value of y above which spectra are deleted
/// Removes all data points outside of the range. Cannot be undone. It is preferable
/// to create a new dataset rather than crop an old one.
void VespucciDataset::CropSpectra(double x_min, double x_max,
                                  double y_min, double y_max,
                                  double wl_min, double wl_max)
{

    SetOldCopies();
    if (!std::isnan(x_min) && !std::isnan(x_max) && !std::isnan(y_min) && !std::isnan(y_max)){
        uvec valid_indices = find ((x_ >= x_min) && (x_ <= x_max));
        try{
            spectra_ = spectra_.cols(valid_indices);
            y_ = y_.rows(valid_indices);
            x_ = x_.rows(valid_indices);
            valid_indices = find((y_ >= y_min) && (y_ <= y_max));
            y_ = y_.rows(valid_indices);
            x_ = x_.rows(valid_indices);
            spectra_ = spectra_.cols(valid_indices);
        }catch(exception e){
            string str = "CropSpectra: " + string(e.what());
            throw std::runtime_error(str);
        }
    }
    if (!std::isnan(wl_min) && !std::isnan(wl_max)){
        uvec valid_indices = find(abscissa_ >= wl_min && abscissa_ <= wl_max);
        try{
            spectra_ = spectra_.rows(valid_indices);
            abscissa_ = abscissa_.rows(valid_indices);

        }catch(exception e){
            string str = "CropSpectra: " + string(e.what());
            throw std::runtime_error(str);
        }
    }

    last_operation_ = "crop";
    log_stream_ << "CropSpectra" << endl;
    log_stream_ << "x_min == " << x_min << endl;
    log_stream_ << "x_max == " << x_max << endl;
    log_stream_ << "y_min == " << y_min << endl;
    log_stream_ << "y_max == " << y_max << endl;
    log_stream_ << "wl_min == " << wl_min << endl;
    log_stream_ << "wl_max == " << wl_max << endl << endl;
}


///
/// \brief VespucciDataset::MinMaxNormalize
/// Normalizes each column of spectra_ so that the highest value is 1 and lowest
/// value is 0.
void VespucciDataset::MinMaxNormalize()
{
    try{
        SetOldCopies();
        vec spectrum_buffer;
        vec extrema_buffer;
        for (uword i = 0; i < spectra_.n_cols; ++i){
            spectrum_buffer = spectra_.col(i);
            extrema_buffer = spectrum_buffer.min()*ones<vec>(spectra_.n_rows);
            spectrum_buffer -= extrema_buffer;
            extrema_buffer = spectrum_buffer.max()*ones<vec>(spectra_.n_rows);
            spectrum_buffer /= extrema_buffer;
            spectra_.col(i) = spectrum_buffer;
        }
    }
    catch(exception e){
        string str = "MinMaxNormalize: " + string(e.what());
        throw std::runtime_error(str);
    }
    last_operation_ = "min/max normalize";
    log_stream_ << "MinMaxNormalize" << endl << endl;

}

void VespucciDataset::VectorNormalize()
{
    log_stream_ << "Vector Normalization" << endl << endl;
    SetOldCopies();
    try{
        spectra_ = normalise(spectra_old_);
        spectra_ = spectra_;
    }
    catch(exception e){
        string str = "VectorNormalize()" + string(e.what());
        throw std::runtime_error(str);
    }
    last_operation_ = "vector normalize";
}

///
/// \brief VespucciDataset::MeanCenter
/// Subtract the mean of all spectra in the dataset at each wavelength from each
/// wavelength (i.e. in preparation for PCA)
void VespucciDataset::MeanCenter()
{
    log_stream_ << "Mean Centering" << endl << endl;
    SetOldCopies();
    try{
        vec mean_intensity = mean(spectra_, 1);
        spectra_.each_col() -= mean_intensity;
    }
    catch(exception e){
        throw std::runtime_error("MeanCenter");
    }
    last_operation_ = "mean centering";
}

///
/// \brief VespucciDataset::UnitAreaNormalize
///normalizes the spectral data so that the area under each point spectrum is 1
void VespucciDataset::UnitAreaNormalize()
{
    log_stream_ << "UnitAreaNormalize" << endl << endl;
    SetOldCopies();
    uword num_rows = spectra_.n_rows;
    uword num_cols = spectra_.n_cols;

    try{
        for (uword j = 0; j < num_cols; ++j){
            vec spectrum = spectra_.col(j);
            double spectrum_sum = sum(spectrum);
            for (uword i = 0; i < num_rows; ++i){
                spectra_(i, j) = spectra_(i, j) / spectrum_sum;
            }
        }
    }
    catch(exception e){
        string str = "UnitAreaNormalize: " + string(e.what());
        throw std::runtime_error(str);
    }

    last_operation_ = "unit area normalize";
}

///
/// \brief VespucciDataset::PeakIntensityNormalize
/// \param left_bound
/// \param right_bound
///
void VespucciDataset::PeakIntensityNormalize(double left_bound, double right_bound)
{
    SetOldCopies();
    vec positions;
    vec peak_maxes = Vespucci::Math::Quantification::FindPeakMaxMat(spectra_, abscissa_, left_bound, right_bound, positions);
    for (uword j = 0; j < spectra_.n_cols; ++j){
        spectra_.col(j) /= peak_maxes(j);
    }
    last_operation_ = "Peak intensity normalize";
}

///
/// \brief VespucciDataset::Booleanize
/// \param min
/// \param max
/// \param keep_inside
/// \param oneify
///
void VespucciDataset::Booleanize(double min, double max, bool keep_inside, bool oneify)
{
    SetOldCopies();
    last_operation_ = "Booleanize";
    log_stream_
            << "Booleanize" << endl << "min = " << min << endl <<"max = "
            << max << endl
            << (keep_inside ? "keep values in range" : "zero values in range")
            << endl
            << (oneify ? "set non-zero to one" : "retain magnitudes of retained values") << endl;

    //set all values on the outside (or inside) of the range to zero
    try{
        if (keep_inside)
            spectra_.elem(find(spectra_ <= min && spectra_ >= max) ).zeros();
        else
            spectra_.elem(find(spectra_ >= min && spectra_ <= max) ).zeros();


        //set all non-zero values equal to one
        if (oneify)
            spectra_.elem( find(spectra_) ).ones();

    }
    catch (exception e){
        cerr << e.what();
        runtime_error f("Booleanize");
        main_window_->DisplayExceptionWarning(f);
    }

}

///
/// \brief VespucciDataset::Clamp
/// \param min
/// \param max
///
void VespucciDataset::Clamp(double min, double max)
{
    SetOldCopies();
    last_operation_ = "Clamp";
    log_stream_ << "Clamp" << endl << "min = "<< min << endl << "max = " << max << endl;

    try{
        spectra_ = clamp(spectra_, min, max);
    }
    catch(exception e){
        cerr << e.what();
        runtime_error f("Clamp");
        main_window_->DisplayExceptionWarning(f);
    }
}

void VespucciDataset::ShedZeroSpectra()
{
    SetOldCopies();
    vec current_col;
    uvec indices;
    uvec indices_buffer;
    for (uword i = 0; i < spectra_.n_cols; ++i){
        current_col = spectra_.col(i);
        indices_buffer = find(current_col, 1);
        if (indices_buffer.n_elem > 0)
            indices << i;
    }
    try{
        spectra_ = spectra_.cols(indices);
        x_ = x_.rows(indices);
        y_ = y_.rows(indices);
    }
    catch(exception e){
        main_window_->DisplayExceptionWarning("VespucciDataset::ShedZeroSpectra", e);
    }
}

void VespucciDataset::ShedZeroWavelengths()
{
    log_stream_ << "ShedZeroWavelengths" << endl;
    SetOldCopies();
    vec current_row;
    uvec indices, indices_buffer;
    for (uword i = 0; i < spectra_.n_rows; ++i){
        current_row = spectra_.row(i);
        indices_buffer = find(current_row, 1);
        if (indices_buffer.n_elem > 0)
            indices << i;
    }
    try{
        spectra_ = spectra_.rows(indices);
        abscissa_ = abscissa_.rows(indices);
    }
    catch(exception e){
        main_window_->DisplayExceptionWarning("VespucciDataset::ShedZeroWavelengths", e);
    }
}

///
/// \brief VespucciDataset::ZScoreNormCopy
/// For when you want to Z-score normalize without changing spectra_
/// \return A normalized copy of the matrix.
///
mat VespucciDataset::ZScoreNormCopy()
{

    mat normalized;
    //mat normalized_copy(num_rows, num_cols);
    try{
        normalized = Vespucci::Math::Normalization::StandardScore(spectra_);
    }
    catch(exception e){
        string str = "ZScoreNormCopy: " + string(e.what());
        throw std::runtime_error(str);
    }
    return trans(normalized);

}

///
/// \brief VespucciDataset::ZScoreNormalize
/// Normalizes each row using the score of the normal distribution
///
void VespucciDataset::ZScoreNormalize()
{
    log_stream_ << "ZScoreNormalize" << endl;
    SetOldCopies();

    try{
        spectra_ = Vespucci::Math::Normalization::StandardScoreMat(spectra_);
    }
    catch(exception e){
        string str = "ZScoreNormalize: " + string(e.what());
        throw std::runtime_error(str);
    }

    z_scores_calculated_ = true;
    last_operation_ = "Z-score normalize";

}


void VespucciDataset::SNVNormalize(double offset, bool center)
{
    log_stream_ << "SNVNormalize" << endl;
    log_stream_ << "offset = " << offset << endl;
    log_stream_ << "center (Z-score) = " << (center ? "true" : "false") << endl;
    SetOldCopies();
    try{
        spectra_ = Vespucci::Math::Normalization::SNVNorm(spectra_, offset, center);
    }
    catch(exception e){
        string str = "SNVNormalize: " + string(e.what());
        throw std::runtime_error(str);
    }

    last_operation_ = "SNVNormalize";
}

void VespucciDataset::AbsoluteValue()
{
    log_stream_ << "AbsoluteValue" << endl;
    SetOldCopies();

    try{
        spectra_ = arma::abs(spectra_);
    }
    catch(exception e){
        string str =
        "AbsoluteValue: " + string(e.what());
        throw std::runtime_error(str);
    }

    last_operation_ = "Absolute value";

}


///
/// \brief VespucciDataset::SubtractBackground
/// Subtracts a known background spectrum. This can be extracted from a control
/// map using this software (using * component analysis endmember extraction or
/// average spectrum).
/// \param background A matrix consisting of a single spectrum representing the
/// background.
///
void VespucciDataset::SubtractBackground(mat background, QString filename)
{
    log_stream_ << "SubtractBackground" << endl;
    log_stream_ << "filename == " << filename << endl << endl;
    SetOldCopies();
    if (background.n_rows != spectra_.n_rows){
        QMessageBox::warning(0,
                             "Improper Dimensions!",
                             "The background spectrum has a different number of"
                             " points than the map data."
                             " No subtraction can be performed");
        return;
    }
    else{
        try{
            spectra_.each_col() -= background.col(0);
        }
        catch(exception e){
            string str = "SubtractBackground: " + string(e.what());
            throw std::runtime_error(str);
        }
    }
    last_operation_ = "background correction";
}

///
/// \brief VespucciDataset::Baseline
/// Baseline-adjusts the data. This function uses a median filter with a large
/// window to determine the baseline on the assumption that the median value
/// is more likely to be basline than spectrum. This will complicate things if
/// there are many peaks. Additionally, this significantly narrows the shape of
/// peaks. Other baseline methods may be implemented later.
/// \param method
/// \param window_size
///
void VespucciDataset::MFBaseline(int window_size, int iterations)
{
    log_stream_ << "MFBaseline" << endl;
    log_stream_ << "window_size == " << window_size << endl;
    SetOldCopies();
    try{
        baselines_ = spectra_;
        for (uword i = 0; i < iterations; ++i){
            baselines_ = Vespucci::Math::Smoothing::MedianFilterMat(baselines_, window_size);
        }
        spectra_ -= baselines_;
    }
    catch(exception e){
        string str = "MFBaseline: " + string(e.what());
        throw std::runtime_error(str);
    }

    last_operation_ = "baseline correction (median filter)";
}

void VespucciDataset::CWTBaseline(int lambda, int penalty_order, double SNR_threshold, double peak_shape_threshold)
{

}

void VespucciDataset::IModPolyBaseline(const uword poly_order, const uword max_it, double threshold)
{
    log_stream_ << "IModPolyBaseline" << endl;
    log_stream_ << "poly_order == " << poly_order << endl;
    log_stream_ << "max_it == " << max_it << endl;
    log_stream_ << "threshold == " << threshold << endl;
    SetOldCopies();
    mat baselines(spectra_.n_rows, spectra_.n_cols);
    vec baseline, corrected;
    QProgressDialog *progress = new QProgressDialog(main_window_);
    progress->setMinimum(0);
    progress->setMaximum(spectra_.n_cols-1);
    progress->setWindowTitle("Correcting Baseline");
    progress->setWindowModality(Qt::WindowModal);
    QString colcount = " of " + QString::number(spectra_.n_cols);
    progress->setLabelText("Spectrum 0" + colcount);
    progress->show();
    double err;
    try{
        for (uword i = 0; i < spectra_.n_cols; ++i){
            progress->setValue(i);
            progress->setLabelText("Spectrum " + QString::number(i+1) + colcount);
            Vespucci::Math::LinLeastSq::IModPoly(spectra_.col(i),
                                                 abscissa_, baseline,
                                                 corrected, err,
                                                 poly_order, max_it, threshold);
            baselines.col(i) = baseline;
            spectra_.col(i) = corrected;
            if (progress->wasCanceled()){
                progress->close();
                break;
                Undo();
            }
        }
    }catch(exception e){
          main_window_->DisplayExceptionWarning("VespucciDataset::IModPolyBaseline", e);
    }
    progress->close();
    if (!progress->wasCanceled()){
        AddAnalysisResult("IModPoly Baselines", baselines);
    }

}

///
/// \brief VespucciDataset::RemoveClippedSpectra
/// \param threshold
/// Removes spectra with a maximum value at or above the threshold
void VespucciDataset::RemoveClippedSpectra(double threshold)
{
    SetOldCopies();
    rowvec spectra_max = max(spectra_, 0);

    uvec valid_indices = find(spectra_max < threshold);
    if (valid_indices.n_elem == 0)
        return;

    try{
        spectra_ = spectra_.cols(valid_indices);
        x_ = x_.rows(valid_indices);
        y_ = y_.rows(valid_indices);
    }
    catch(exception e){
        string str = "RemoveClipped Spectra: " + string(e.what());
        throw std::runtime_error(str);
    }

    if (spectra_.n_rows != spectra_old_.n_rows)
        non_spatial_ = true;
}

///
/// \brief VespucciDataset::RemoveFlatSpectra
/// \param threshold
/// Removes "flat" spectra whose maximum is less than the threshold
void VespucciDataset::RemoveFlatSpectra(double threshold)
{
    SetOldCopies();
    rowvec spectra_max = max(spectra_, 0);

    uvec valid_indices = find(spectra_max > threshold);
    if (valid_indices.n_elem == 0)
        return;

    try{
        spectra_ = spectra_.cols(valid_indices);
        x_ = x_.rows(valid_indices);
        y_ = y_.rows(valid_indices);
    }
    catch(exception e){
        string str = "RemoveClipped Spectra: " + string(e.what());
        throw std::runtime_error(str);
    }

    if (spectra_.n_rows != spectra_old_.n_rows)
        non_spatial_ = true;
}

//Filtering functions
///
/// \brief VespucciDataset::MedianFilter
/// performs median filtering on the spectral data.  Entries near the boundaries
/// of spectra are not processed. See also VespucciDataset::LinearMovingAverage
/// \param window_size - an odd number representing the width of the window.

void VespucciDataset::MedianFilter(unsigned int window_size)
{
    log_stream_ << "MedianFilter" << endl;
    log_stream_ << "window_size == " << window_size << endl << endl;
    mat processed;
    SetOldCopies();
    try{
        processed = Vespucci::Math::Smoothing::MedianFilterMat(spectra_, window_size);
        spectra_ = processed;
    }
    catch(exception e){
        string str = "MedianFilter: " + string(e.what());
        throw std::runtime_error(str);
    }

    last_operation_ = "median filter";
}

///
/// \brief VespucciDataset::LinearMovingAverage
/// Performs moving average filtering on the spectral data.  Entries near the
/// boundaries of spectra are not processed.  See also VespucciDataset::MedianFilter.
/// \param window_size - an odd number representing the width of the window.

void VespucciDataset::LinearMovingAverage(unsigned int window_size)
{
    log_stream_ << "LinearMovingAverage" << endl;
    log_stream_ << "window_size == " << window_size << endl << endl;

    SetOldCopies();
    try{
        vec filter = Vespucci::Math::Smoothing::CreateMovingAverageFilter(window_size);
        for (uword j = 0; j < spectra_.n_cols; ++j){
            spectra_.col(j) = Vespucci::Math::Smoothing::ApplyFilter(spectra_.col(j), filter);
        }
    }
    catch(exception e){
        string str = "LinearMovingAverage: " + string(e.what());
        throw std::runtime_error(str);
        Undo();
    }

    last_operation_ = "moving average filter";
}

///
/// \brief VespucciDataset::SingularValue
/// Denoises the spectra matrix using a truncated singular value decomposition.
/// The first singular_values singular values are used to "reconstruct" the
/// spectra matrix. The function used to find the truncated SVD is
/// Vespucci::Math::DimensionReduction::svds.
/// \param singular_values Number of singular values to use.
///
void VespucciDataset::SingularValue(unsigned int singular_values)
{
    log_stream_ << "SingularValue" << endl;
    log_stream_ << "singular_values == " << singular_values << endl << endl;
    SetOldCopies();
    mat U;
    vec s;
    mat V;
    try{
        Vespucci::Math::DimensionReduction::svds(spectra_, singular_values, U, s, V);
        spectra_ = -1 * U * diagmat(s) * V.t();
    }
    catch(exception e){
        string str = "SingularValue: " + string(e.what());
        throw std::runtime_error(str);
    }

    last_operation_ = "truncated SVD de-noise";
}


///
/// \brief VespucciDataset::QUIC_SVD
/// \param epsilon Error tolerance fraction for calculated subspace
/// Use the QUIC-SVD algorithm to denoise the spectra by finding a lower-rank approximation
/// of the matrix.
int VespucciDataset::QUIC_SVD(double epsilon)
{
    SetOldCopies();
    int SVD_rank;
    log_stream_ << "QUIC_SVD" << endl;
    log_stream_ << "epsilon == " << epsilon << endl << endl;
    mat u, sigma, v, copy;
    cout << "Call QUIC_SVD" << endl;
    mlpack::svd::QUIC_SVD svd_obj(spectra_, u, v, sigma, epsilon, 0.1);
    cout << "create copy" << endl;
    SVD_rank = u.n_cols;
    log_stream_ << "rank of approximation = " << SVD_rank << endl;
    spectra_ = u * sigma * v.t();
    last_operation_ = "truncated SVD de-noise";
    cout << "Copy operations" << endl;
    spectra_ = copy;
    return SVD_rank;

}

///
/// \brief VespucciDataset::SavitzkyGolay
/// Performs derivatization/Savitzky-Golay smoothing
/// \param derivative_order The order of the derivative.
/// \param polynomial_order The order of the polynomial
/// \param window_size The size of the filter window.
///
void VespucciDataset::SavitzkyGolay(unsigned int derivative_order,
                                 unsigned int polynomial_order,
                                 unsigned int window_size)
{
    log_stream_ << "SavitzkyGolay" << endl;
    log_stream_ << "derivative_order == " << derivative_order << endl;
    log_stream_ << "polynomial_order == " << polynomial_order << endl;
    log_stream_ << "window_size == " << window_size << endl << endl;
    SetOldCopies();

    try{
        spectra_ = Vespucci::Math::Smoothing::sgolayfilt(spectra_,
                                        polynomial_order,
                                        window_size,
                                        derivative_order,
                                        1);
    }
    catch(exception e){
        string str = "SavitzkyGolay: " + string(e.what());
        throw std::runtime_error(str);
    }

    last_operation_ = "Savitzky-Golay filtering";
}

void VespucciDataset::Scale(double scaling_factor)
{
    log_stream_ << "Scale" << endl;
    log_stream_ << "scaling_factor = " << scaling_factor << endl;
    SetOldCopies();

    try{
        spectra_ = spectra_ * scaling_factor;
    }catch(exception e){
     string str = "Scale(): " + string(e.what());
     throw std::runtime_error(str);
    }

    last_operation_ = "Scaling";
}

///
/// \brief VespucciDataset::ShedSpectrum
/// \param index Index of spectrum to shed.
/// Removes a spectrum from the dataset
void VespucciDataset::ShedSpectrum(const uword index)
{
    cout << "VespucciDataset::ShedSpectrum" << endl;
    cout << "index = " << index << "x = " << x_(index) << " y = " << y_(index) << endl;
    log_stream_ << "Shed Spectrum" << endl;
    log_stream_ << "index = " << index << "x = " << x_(index) << " y = " << y_(index) << endl;
    SetOldCopies();
    cout << "spectra columns = " << spectra_.n_cols;
    try{
        spectra_.shed_col(index);
        x_.shed_row(index);
        y_.shed_row(index);
        indices_.shed_row(index);
    }
    catch(exception e){
        cout << e.what();
        std::runtime_error exc("VespucciDataset::ShedSpectrum");
        main_window_->DisplayExceptionWarning(exc);
    }
    cout << "spectra_ columns (post) = " << spectra_.n_cols;
}

///
/// \brief VespucciDataset::HySime
/// \return Dimensionality predicted by HySime algorithm
///
int VespucciDataset::HySime()
{
    cout << "VespucciDataset::HySime" << endl;
    wall_clock timer;
    mat noise, noise_correlation, subspace;
    cout << "call EstimateAdditiveNoise" << endl;
    timer.tic();
    Vespucci::Math::DimensionReduction::EstimateAdditiveNoise(noise, noise_correlation, spectra_);
    cout << "took " << timer.toc() << " seconds." << endl;
    cout << "Call HySime" << endl;
    timer.tic();
    int k = Vespucci::Math::DimensionReduction::HySime(spectra_, noise, noise_correlation, subspace);
    cout << "Took " << timer.toc() << " seconds." << endl;
    return k;
}

///
/// \brief VespucciDataset::TransformAbscissa
/// \param input_units
/// \param input_factor
/// \param output_units
/// \param output_factor
///
void VespucciDataset::TransformAbscissa(QString input_units, double input_factor, QString output_units, double output_factor, QString description)
{
    log_stream_ << "TransformAbscissa" << endl;
    log_stream_ << "Input Units: " << input_units << endl;
    log_stream_ << "Input Factor: " << input_factor << endl;
    log_stream_ << "Output Units: " << output_units << endl;
    log_stream_ << "Output Factor: " << output_factor << endl;
    log_stream_ << "Old Label: " << x_axis_description_ << endl;
    log_stream_ << "New Label: " << description << endl;

    if (input_units == "Wavelength"){
        if (output_units == "Energy"){
            SetOldCopies();
            abscissa_ = Vespucci::Math::WavelengthToEnergy(abscissa_, output_factor, input_factor);
            x_axis_description_ = description;
            last_operation_ = "Abscissa Transform";
        }
        else if (output_units == "Wavenumber"){
            SetOldCopies();
            abscissa_ = Vespucci::Math::WavelengthToWavenumber(abscissa_, output_factor, input_factor);
            x_axis_description_ = description;
            last_operation_ = "Abscissa Transform";
        }
        else if (output_units == "Frequency"){
            SetOldCopies();
            abscissa_ = Vespucci::Math::WavelengthToFrequency(abscissa_, output_factor, input_factor);
            x_axis_description_ = description;
            last_operation_ = "Abscissa Transform";
        }
        else if (output_units == "Wavelength"){
            SetOldCopies();
            abscissa_ = (input_factor * output_factor) * abscissa_;
            x_axis_description_ = description;
            last_operation_ = "Abscissa Transform";
        }
        else{return;}
    }
    else if (input_units == "Energy"){
        if (output_units == "Energy"){
            SetOldCopies();
            abscissa_ = (input_factor * output_factor) * abscissa_;
            x_axis_description_ = description;
            last_operation_ = "Abscissa Transform";
        }
        else if (output_units == "Wavenumber"){
            SetOldCopies();
            abscissa_ = Vespucci::Math::EnergyToWavenumber(abscissa_, output_factor, input_factor);
            x_axis_description_ = description;
            last_operation_ = "Abscissa Transform";
        }
        else if (output_units == "Frequency"){
            SetOldCopies();
            abscissa_ = Vespucci::Math::EnergyToFrequency(abscissa_, output_factor, input_factor);
            x_axis_description_ = description;
            last_operation_ = "Abscissa Transform";
        }
        else if (output_units == "Wavelength"){
            SetOldCopies();
            abscissa_ = Vespucci::Math::EnergyToWavelength(abscissa_, output_factor, input_factor);
            x_axis_description_ = description;
            last_operation_ = "Abscissa Transform";
        }
        else{return;}
    }
    else if (input_units == "Wavenumber"){
        if (output_units == "Energy"){
            SetOldCopies();
            abscissa_ = Vespucci::Math::WavenumberToEnergy(abscissa_, output_factor, input_factor);
            x_axis_description_ = description;
            last_operation_ = "Abscissa Transform";
        }
        else if (output_units == "Wavenumber"){
            SetOldCopies();
            abscissa_ = (input_factor * output_factor) * abscissa_;
            x_axis_description_ = description;
            last_operation_ = "Abscissa Transform";
        }
        else if (output_units == "Frequency"){
            SetOldCopies();
            abscissa_ = Vespucci::Math::WavenumberToFrequency(abscissa_, output_factor, input_factor);
            x_axis_description_ = description;
            last_operation_ = "Abscissa Transform";
        }
        else if (output_units == "Wavelength"){
            SetOldCopies();
            abscissa_ = Vespucci::Math::WavenumberToWavelength(abscissa_, output_factor, input_factor);
            x_axis_description_ = description;
            last_operation_ = "Abscissa Transform";
        }
        else{return;}
    }
    else if (input_units == "Frequency"){
        if (output_units == "Energy"){
            SetOldCopies();
            abscissa_ = Vespucci::Math::FrequencyToEnergy(abscissa_, output_factor, input_factor);
            x_axis_description_ = description;
            last_operation_ = "Abscissa Transform";
        }
        else if (output_units == "Wavenumber"){
            SetOldCopies();
            abscissa_ = Vespucci::Math::FrequencyToWavenumber(abscissa_, output_factor, input_factor);
            x_axis_description_ = description;
            last_operation_ = "Abscissa Transform";
        }
        else if (output_units == "Frequency"){
            SetOldCopies();
            abscissa_ = (input_factor * output_factor) * abscissa_;
            x_axis_description_ = description;
            last_operation_ = "Abscissa Transform";
        }
        else if (output_units == "Wavelength"){
            SetOldCopies();
            abscissa_ = Vespucci::Math::FrequencyToWavelength(abscissa_, output_factor, input_factor);
            x_axis_description_ = description;
            last_operation_ = "Abscissa Transform";
        }
        else{return;}//do nothing for invalid input
    }
    else{return;}
}

///
/// \brief VespucciDataset::InterpolateToNewAbscissa
/// \param new_abscissa
/// \param polynomial_order
/// \param window_size
/// Spline interpolation to new abscissa
void VespucciDataset::InterpolateToNewAbscissa(const vec &new_abscissa, unsigned int poly_order, unsigned int window_size)
{
    log_stream_ << "InterpolateToNewAbscissa (spline)" << endl;
    log_stream_ << "Old Abscissa Size: " << abscissa_.n_rows << endl;
    log_stream_ << "New Abscissa Size: " << new_abscissa.n_rows << endl;
    mat new_spectra;
    try{
        new_spectra = Vespucci::Math::Smoothing::InterpolateToNewAbscissa(spectra_, abscissa_, new_abscissa, window_size, poly_order);
    }catch(exception e){
        string str = "InterpolateToNewAbscissa(vec, uint, uint): " + string(e.what());
        throw std::runtime_error(str);
    }
    SetOldCopies();
    abscissa_ = new_abscissa;
    spectra_ = new_spectra;
    last_operation_ = "Abscissa Interpolation";
}

///
/// \brief VespucciDataset::InterpolateToNewAbscissa
/// \param new_abscissa
/// Linear interpolation to new abscissa
void VespucciDataset::InterpolateToNewAbscissa(const vec &new_abscissa)
{
    log_stream_ << "InterpolateToNewAbscissa (linear)" << endl;
    log_stream_ << "Old Abscissa Size: " << abscissa_.n_rows << endl;
    log_stream_ << "New Abscissa Size: " << new_abscissa.n_rows << endl;
    mat new_spectra;
    try{
        new_spectra = Vespucci::Math::Smoothing::InterpolateToNewAbscissa(spectra_, abscissa_, new_abscissa);
    }catch(exception e){
        string str = "InterpolateToNewAbscissa(vec): " + string(e.what());
        throw std::runtime_error(str);
    }
    SetOldCopies();
    abscissa_ = new_abscissa;
    spectra_ = new_spectra;
    last_operation_ = "Abscissa Interpolation";
}

///
/// \brief VespucciDataset::FourierTransform
/// \param n
///
void VespucciDataset::FourierTransform(int n)
{
    log_stream_ << "FourierTransform" << endl;
    log_stream_ << "n = " << n << endl;
    //cx_mat complex_spectra = cx_mat(spectra_, spectra_imag_);
    cx_mat f_spectra(spectra_.n_rows, spectra_.n_cols);
    vec f_abscissa(abscissa_.n_rows);
    try{
        Vespucci::Math::Transform::fft_mat(spectra_, abscissa_,
                                           f_spectra, f_abscissa,
                                           n);
    }catch(exception e){
        string str = "FourierTransform: " + string(e.what());
        throw std::runtime_error(str);
    }
    SetOldCopies();
    spectra_ = real(f_spectra);
    spectra_imag_ = imag(f_spectra);
    abscissa_ = f_abscissa;
    x_axis_description_ = "Frequency (Hz)";
}

void VespucciDataset::InverseFourierTransform(int n)
{
    log_stream_ << "InverseFouerierTransform" << endl;
    log_stream_ << "n = " << n << endl;
    cx_mat t_spectra(spectra_.n_rows, spectra_.n_cols);
    vec t_abscissa(abscissa_.n_rows);
    try{
        Vespucci::Math::Transform::ifft_mat(cx_spectra(),
                                            abscissa_,
                                            t_spectra,
                                            t_abscissa,
                                            n);
    }catch(exception e){
        string str = "InverseFourierTransform: " + string(e.what());
        throw std::runtime_error(str);
    }
    SetOldCopies();
    spectra_ = real(t_spectra);
    spectra_imag_ = imag(t_spectra);
    abscissa_ = t_abscissa;
    x_axis_description_ = "Time (s)";
}

///
/// \brief VespucciDataset::ApplyFTWeight
/// \param type
/// \param param
///
void VespucciDataset::ApplyFTWeight(QString type,
                                    double param)
{
    log_stream_ << "ApplyFTWeight" << endl;
    log_stream_ << "type = " << type << endl;
    log_stream_ << "parameter = " << param << endl;
    SetOldCopies();
    string shape;
    if (type == "Exponential"){shape = "exp";}
    else{shape = "gaus";}
    try{
        spectra_ = Vespucci::Math::Transform::ApplyWeights(spectra_, abscissa_, shape, param);
    }catch(exception e){
        main_window_->DisplayExceptionWarning("VespucciDataset::ApplyFTWeight", e);
    }
}

///
/// \brief VespucciDataset::ApplyFTWeight
/// \param type
/// \param start_offset
/// \param end_offset
/// \param power
///
void VespucciDataset::ApplyFTWeight(double start_offset,
                                    double end_offset,
                                    double power)
{
    log_stream_ << "ApplyFTWeight (sine bell)" << endl;
    log_stream_ << "start_offset = " << start_offset << endl;
    log_stream_ << "end_offset = " << end_offset << endl;
    log_stream_ << "power = " << power << endl;
    SetOldCopies();
    try{
        spectra_ = Vespucci::Math::Transform::ApplySBWeights(spectra_,
                                                             abscissa_,
                                                             start_offset,
                                                             end_offset,
                                                             power);
    }catch(exception e){
        main_window_->DisplayExceptionWarning("VespucciDataset::ApplyFTWeight", e);
    }

}

// ANALYIS/MAPPING FUNCTIONS //

///
/// \brief VespucciDataset::Univariate
/// \param min
/// \param max
/// \param name
/// \param method
/// \param window
/// \param plot
/// \param gradient_index
///
void VespucciDataset::Univariate(double min, double max, QString name, 
                                 UnivariateMethod::Method method, 
                                 uword window, bool plot, uword gradient_index)
{
    QSharedPointer<UnivariateData> univariate_data(new UnivariateData(QSharedPointer<VespucciDataset>(this), name));
    univariate_data->Apply(min, max, window, method);
    univariate_datas_.append(univariate_data);

    log_stream_ << "Univariate" << endl;
    log_stream_ << "min == " << min << endl;
    log_stream_ << "max == " << max << endl;
    log_stream_ << "name == " << name << endl;
    log_stream_ << "method == " << univariate_data->method_description() << endl;

    if (plot){
        QSharedPointer<MapData> new_map(new MapData(x_axis_description_,
                                                    y_axis_description_,
                                                    x_, y_,
                                                    univariate_data->results(),
                                                    QSharedPointer<VespucciDataset>(this),
                                                    directory_, GetGradient(gradient_index),
                                                    map_list_model_->rowCount(QModelIndex()),
                                                    6,
                                                    main_window_));
        switch (method){
          case UnivariateMethod::Area :
          case UnivariateMethod::Derivative :
            new_map->set_baselines(univariate_data->baselines());
            break;
          case UnivariateMethod::FWHM :
            new_map->set_fwhm(univariate_data->baselines(), univariate_data->midlines());
            break;
          default:
            ;//do nothing
        }

        map_list_model_->AddMap(new_map);
        new_map->ShowMapWindow();
    }


}

///
/// \brief VespucciDataset::BandRatio
/// \param first_min
/// \param first_max
/// \param second_min
/// \param second_max
/// \param name
/// \param method
/// \param window
/// \param plot
/// \param gradient_index
///
void VespucciDataset::BandRatio(double first_min, double first_max, double second_min, double second_max, QString name, UnivariateMethod::Method method, uword window, bool plot, unsigned int gradient_index)
{
    QSharedPointer<UnivariateData> univariate_data(new UnivariateData(QSharedPointer<VespucciDataset>(this), name));
    univariate_data->Apply(first_min, first_max, second_min, second_max, window, method);
    univariate_datas_.append(univariate_data);

    log_stream_ << "Band Ratio" << endl;
    log_stream_ << "first_min == " << first_min << endl;
    log_stream_ << "first_max == " << first_max << endl;
    log_stream_ << "second_min == " << second_min << endl;
    log_stream_ << "second_max == " << second_max << endl;
    log_stream_ << "name == " << name << endl;
    log_stream_ << "method == " << univariate_data->method_description() << endl;

    if (plot){
        QSharedPointer<MapData> new_map(new MapData(x_axis_description_,
                                                    y_axis_description_,
                                                    x_, y_,
                                                    univariate_data->results(),
                                                    QSharedPointer<VespucciDataset>(this),
                                                    directory_, GetGradient(gradient_index),
                                                    map_list_model_->rowCount(QModelIndex()),
                                                    6,
                                                    main_window_));

        if (method == UnivariateMethod::AreaRatio || method == UnivariateMethod::DerivativeRatio)
            new_map->set_baselines(univariate_data->first_baselines(), univariate_data->second_baselines());

        map_list_model_->AddMap(new_map);
        new_map->ShowMapWindow();
    }
}

///
/// \brief VespucciDataset::Correlation
/// \param control
/// \param name
/// \param gradient_index
///
void VespucciDataset::Correlation(vec control, QString name, uword gradient_index, bool plot)
{
    QSharedPointer<UnivariateData> univariate_data(new UnivariateData(QSharedPointer<VespucciDataset>(this), name, control));
    univariate_data->Apply(0, 0, 0, UnivariateMethod::Correlation);
    univariate_datas_.append(univariate_data);

    log_stream_ << "Correlation" << endl;
    log_stream_ << "name == " << name << endl;

    if (plot){
        QSharedPointer<MapData> new_map(new MapData(x_axis_description_,
                                                    y_axis_description_,
                                                    x_, y_,
                                                    univariate_data->results(),
                                                    QSharedPointer<VespucciDataset>(this),
                                                    directory_, GetGradient(gradient_index),
                                                    map_list_model_->rowCount(QModelIndex()),
                                                    6,
                                                    main_window_));
        map_list_model_->AddMap(new_map);
        new_map->ShowMapWindow();
    }
}

///
/// \brief VespucciDataset::PrincipalComponents
/// Performs principal component analysis on the data.  Uses armadillo's pca routine.
/// This function both calculates and plots principal components maps.
/// \param component the PCA component from which the image will be produced
/// \param name the name of the MapData object to be created
/// \param gradient_index the index of the gradient in the master list (in function GetGradient)
///
void VespucciDataset::PrincipalComponents(int component,
                                  QString name,
                                  int gradient_index, bool recalculate)
{
    //if dataset is non spatial, just quit
    if(non_spatial_){
        QMessageBox::warning(0, "Non-spatial dataset", "Dataset is non-spatial or non-contiguous! Mapping functions are not available. Please use the analysis dialog.");
        return;
    }
    log_stream_ << "PrincipalComponents" << endl;
    log_stream_ << "component == " << component << endl;
    log_stream_ << "name == " << name << endl;
    log_stream_ << "gradient_index == " << gradient_index << endl;
    log_stream_ << "recalculate == " << (recalculate ? "true" : "false") << endl << endl;

    if (recalculate || !principal_components_calculated_){
        component--;
        cout << "call to arma::princomp" << endl;
        wall_clock timer;
        timer.tic();
        principal_components_data_ =
                QSharedPointer<PrincipalComponentsData>
                (new PrincipalComponentsData(
                     QSharedPointer<VespucciDataset>(this),
                     directory_));
        try{
            principal_components_data_->Apply(spectra_);
        }
        catch(exception e){
            string str = "PrincipalComponents: " + string(e.what());
            throw std::runtime_error(str);
        }

        int seconds = timer.toc();
        cout << "call to arma::princomp successful. Took " << seconds << " s" << endl;
        principal_components_calculated_ = true;
    }

    QString map_type;
    QTextStream(&map_type) << "(Principal Component " << component + 1 << ")";

    colvec results;
    try{
        results = principal_components_data_->Results(component);
    }
    catch(exception e){
        string str = "PrincipalComponents: " + string(e.what());
        throw std::runtime_error(str);
    }

    QSharedPointer<MapData> new_map(new MapData(x_axis_description_,
                                            y_axis_description_,
                                            x_, y_, results,
                                            QSharedPointer<VespucciDataset>(this), directory_,
                                            GetGradient(gradient_index),
                                            map_list_model_->rowCount(QModelIndex()),
                                            6,
                                            main_window_));
    new_map->set_name(name, map_type);
    AddMap(new_map);
    new_map->ShowMapWindow();
}

///
/// \brief VespucciDataset::PrincipalComponents
/// \param scaleData
/// \param recalculate
/// Perform PCA using MLPACK's implementation. This is designed for dimensionality
/// reduction, not classification, so there is no corresponding imaging function
void VespucciDataset::PrincipalComponents(bool scaleData)
{
    mlpack_pca_data_ = QSharedPointer<MLPACKPCAData>(new MLPACKPCAData(scaleData));
    try{
        mlpack_pca_data_->Apply(spectra_);
    }
    catch(exception e){
        throw std::runtime_error("VespucciDataset::PrincipalComponents");
    }

    mlpack_pca_calculated_ = true;
}

///
/// \brief VespucciDataset::PrincipalComponents
/// Perform PCA without creating an image
void VespucciDataset::PrincipalComponents()
{
    log_stream_ << "PrincipalComponents (no image)" << endl;
    principal_components_data_ =
            QSharedPointer<PrincipalComponentsData>(
                new PrincipalComponentsData(QSharedPointer<VespucciDataset>(this),
                                            directory_));
    try{
        principal_components_data_->Apply(spectra_);
        principal_components_calculated_ = true;
    }catch(exception e){
        string str = "PrincipalComponents: " + string(e.what());
        throw std::runtime_error(str);
    }
}

///
/// \brief VespucciDataset::FindPeaks
/// \param sel The amount above the surrounding data for a peak to be identified
/// \param threshold A value which peaks must be larger than to be maxima
/// \param poly_order Polynomial order for Savitzky-Golay derivatization
/// \param window_size Window size for Savitzky-Golay derivatization
///
void VespucciDataset::FindPeaks(double sel, double threshold, uword poly_order, uword window_size)
{
    log_stream_ << "FindPeaks" << endl;
    log_stream_ << "sel = " << sel << endl;
    log_stream_ << "threshold = " << threshold << endl;
    log_stream_ << "poly_order = " << poly_order << endl;
    log_stream_ << "window_size = " << window_size << endl;

    mat peak_magnitudes;
    mat peak_positions;


    try{
        peak_positions =
                Vespucci::Math::PeakFinding::FindPeaksMat(spectra_,
                                       sel, threshold,
                                       poly_order, window_size,
                                       peak_magnitudes);
    }catch(exception e){
        cerr << e.what();
        throw std::runtime_error("FindPeaks");
    }
    QSharedPointer<AnalysisResults> peak_mag(new AnalysisResults(peak_magnitudes));
    QSharedPointer<AnalysisResults> peak_pos(new AnalysisResults(peak_positions));
    analysis_results_.insert("Peak Positions", peak_pos);
    analysis_results_.insert("Peak Magnitudes", peak_mag);
}

void VespucciDataset::FindPeaksCWT(string wavelet,
                                   uword max_scale,
                                   uword gap_threshold,
                                   uword min_ridge_length,
                                   uword search_width,
                                   double noise_threshold,
                                   string noise_method,
                                   uword noise_window_size,
                                   bool save_coefs,
                                   bool save_coef_plots,
                                   bool save_ridge_plots,
                                   bool save_ridge_plot_data,
                                   bool estimate_width,
                                   QString save_directory,
                                   QString image_format,
                                   QCPColorGradient gradient)
{
    cwt_peak_data_ = QSharedPointer<CWTData>(new CWTData(QSharedPointer<VespucciDataset>(this)));
    try{
        cwt_peak_data_->Apply(wavelet,
                              max_scale,
                              gap_threshold,
                              min_ridge_length,
                              search_width,
                              noise_threshold,
                              noise_method,
                              noise_window_size,
                              save_coefs,
                              save_coef_plots,
                              save_ridge_plots,
                              save_ridge_plot_data,
                              estimate_width,
                              save_directory,
                              image_format,
                              gradient);
    }
    catch(exception e){
        main_window_->DisplayExceptionWarning("VespucciDataset::FindPeaksCWT", e);
    }

    //mat centers = cwt_peak_data_->centers();
    //QSharedPointer<AnalysisResults> result(new AnalysisResults(centers));
    //analysis_results_.insert("CWT Peak Finding Results", result);
    mat counts = cwt_peak_data_->counts();
    QSharedPointer<AnalysisResults> count_result(new AnalysisResults(counts));
    analysis_results_.insert("Detected Peak Counts", count_result);
}

void VespucciDataset::HasPeaksCWT(const mat &range_list)
{
    //if this pointer evaluates void, stop
    if (!cwt_peak_data_){
        QMessageBox::warning(main_window_,
                             "Peak Finding Not Performed",
                             "You must perform peak finding before determining "
                             "peak populations.");
        return;
    }

    mat peak_existence = cwt_peak_data_->HasPeaks(range_list);
    mat peak_pop = trans(sum(peak_existence, 0));
    peak_pop.insert_cols(0, range_list);

    QSharedPointer<AnalysisResults> exist_result(new AnalysisResults(peak_existence));
    QSharedPointer<AnalysisResults> pop_results(new AnalysisResults(peak_pop));
    analysis_results_.insert("Peak Presence by Spectrum", exist_result);
    analysis_results_.insert("Peak Population", pop_results);
}

///
/// \brief VespucciDataset::VertexComponents
/// \param endmembers
/// \param image_component
/// \param name
/// \param gradient_index
/// \param recalculate
///
void VespucciDataset::VertexComponents(uword endmembers,
                               uword image_component,
                               QString name,
                               unsigned int gradient_index,
                               bool recalculate)
{
    //if dataset is non spatial, just quit
    if(non_spatial_){
        QMessageBox::warning(0, "Non-spatial dataset", "Dataset is non-spatial or non-contiguous! Mapping functions are not available");
        return;
    }
    log_stream_ << "VertexComponents" << endl;
    log_stream_ << "endmembers == " << endmembers << endl;
    log_stream_ << "image_component == " << image_component << endl;
    log_stream_ << "gradient_index == " << gradient_index;
    log_stream_ << "recalculate == " << (recalculate ? "true" : "false") << endl << endl;

    if(endmembers == 0){
        log_stream_ << "Endmember count predicted using HySime algorithm: ";
        QMessageBox alert;
        alert.setText("The HySime algorithm will take a long time.");
        alert.setInformativeText("OK to continue");

        alert.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        alert.setWindowTitle("Vertexf Components Analysis");
        alert.setIcon(QMessageBox::Question);

        int ret = alert.exec();


        if (ret == QMessageBox::Cancel){
            return;
        }

        endmembers = HySime();
        log_stream_ << endmembers << endl;
    }

    QString map_type;
    QTextStream(&map_type) << "(Vertex Component " << image_component << ")";
    try{
        if (recalculate || !vertex_components_calculated_){
            vertex_components_data_ =
                    QSharedPointer<VCAData>(
                        new VCAData(QSharedPointer<VespucciDataset>(this),
                                    directory_));

            vertex_components_data_->Apply(spectra_, endmembers);
            vertex_components_calculated_ = true;
        }
    }
    catch(exception e){
        cout << "VespucciDataset::VertexComponents()" << endl;
        string str = "VertexComponents: " + string(e.what());
        throw std::runtime_error(str);
    }
    colvec results;
    try{
        results = vertex_components_data_->Results(image_component-1);
    }
    catch(exception e){
        string str = "VertexComponents: " + string(e.what());
        throw std::runtime_error(str);
    }

    QSharedPointer<MapData> new_map(new MapData(x_axis_description_,
                                                y_axis_description_,
                                                x_, y_, results,
                                                QSharedPointer<VespucciDataset>(this), directory_,
                                                GetGradient(gradient_index),
                                                map_list_model_->rowCount(QModelIndex()),
                                                6,
                                                main_window_));
    new_map->set_name(name, map_type);
    AddMap(new_map);
    new_map->ShowMapWindow();
}

///
/// \brief VespucciDataset::VertexComponents
/// \param endmembers
/// Perform VCA without creating an image
void VespucciDataset::VertexComponents(uword endmembers)
{
    log_stream_ << "VertexComponents (no image)" << endl;
    log_stream_ << "endmembers == " << endmembers << endl;

    if(endmembers == 0){
        log_stream_ << "Endmember count predicted using HySime algorithm: ";
        QMessageBox alert;
        alert.setText("The HySime algorithm will take a long time.");
        alert.setInformativeText("OK to continue");

        alert.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        alert.setWindowTitle("Principal Components Analysis");
        alert.setIcon(QMessageBox::Question);

        int ret = alert.exec();


        if (ret == QMessageBox::Cancel){
            return;
        }
        endmembers = HySime();
        log_stream_ << endmembers << endl;
    }

    try{
        vertex_components_data_ =
                QSharedPointer<VCAData>(new VCAData(QSharedPointer<VespucciDataset>(this),
                                                    directory_));
        vertex_components_data_->Apply(spectra_, endmembers);
        vertex_components_calculated_ = true;
    }catch(exception e){
        cerr << "VespucciDataset::VertexComponents()" << endl;
        string str = "VertexComponents: " + string(e.what());
        throw std::runtime_error(str);
    }
}


///
/// \brief VespucciDataset::PartialLeastSquares
/// Performs PLS regression on data.  Resulting map is score for one PLS Component,
/// taken from one column of the X loadings.
/// PLS is performed once.  Subsequent maps use data from first call, stored
/// as PartialLeastSquaresData object, unless user requests recalculation.
/// \param components the number of components/response variables of the regression data
/// \param name the name of the MapData object to be created.
/// \param gradient_index the index of the color gradient in the color gradient list
/// \param recalculate whether or not to recalculate PLS regression.
///
void VespucciDataset::PartialLeastSquares(uword components,
                                  uword image_component,
                                  QString name,
                                  unsigned int gradient_index,
                                  bool recalculate)
{
    //if dataset is non spatial, just quit
    if(non_spatial_){
        QMessageBox::warning(0, "Non-spatial dataset", "Dataset is non-spatial or non-contiguous! Mapping functions are not available");
        return;
    }
    log_stream_ << "PartialLeastSqares" << endl;
    log_stream_ << "components == " << components << endl;
    log_stream_ << "image_component == " << image_component << endl;
    log_stream_ << "name == " << name << endl;
    log_stream_ << "gradient_index == " << gradient_index << endl;
    log_stream_ << "recalculate == " << (recalculate ? "true" : "false") << endl << endl;

    if(components == 0){
        log_stream_ << "Component count predicted using HySime algorithm: ";
        components = HySime();
        log_stream_ << components << endl;
        if (image_component > components){
            image_component = components;
        }
    }

    image_component--;
    QString map_type = "Partial Least Squares Map number of components = ";
    try{
        if (recalculate || !partial_least_squares_calculated_){
            map_type += QString::number(components);
            partial_least_squares_data_ =
                    QSharedPointer<PLSData>(new PLSData(QSharedPointer<VespucciDataset>(this),
                                                        directory_));
            bool success = partial_least_squares_data_->Apply(spectra_, abscissa_, components);
            if (success){
                partial_least_squares_calculated_ = true;
            }
        }
    }
    catch(exception e){
        string str = "PartialLeastSquares: " + string(e.what());
        throw std::runtime_error(str);
    }


    bool valid;
    colvec results;
    try{
        results = partial_least_squares_data_->Results(image_component, valid);
        if (!valid){
        QMessageBox::warning(main_window_, "Index out of Bounds",
                             "The component number requested is greater than"
                             "the number of components calculated. Map generated"
                             "corresponds to the highest component number"
                             "calculated");
        }
    }
    catch(exception e){
        string str = "PartialLeastSquares: " + string(e.what());
        throw std::runtime_error(str);
    }


    map_type += QString::number(partial_least_squares_data_->NumberComponents());
    map_type += ". Component number " + QString::number(image_component);

    QSharedPointer<MapData> new_map(new MapData(x_axis_description_,
                                            y_axis_description_,
                                            x_, y_, results,
                                            QSharedPointer<VespucciDataset>(this), directory_,
                                            GetGradient(gradient_index),
                                            map_list_model_->rowCount(QModelIndex()),
                                            6,
                                            main_window_));
    new_map->set_name(name, map_type);
    map_list_model_->AddMap(new_map);
    new_map->ShowMapWindow();
}

///
/// \brief VespucciDataset::PartialLeastSquares
/// \param components
/// Perform partial least squares without creating an image
void VespucciDataset::PartialLeastSquares(uword components)
{
    log_stream_ << "PartialLeastSqares (no image)" << endl;
    log_stream_ << "components == " << components << endl;

    if(components == 0){
        log_stream_ << "Component count predicted using HySime algorithm: ";
        components = HySime();
        log_stream_ << components << endl;
    }


    try{
        partial_least_squares_data_ =
                QSharedPointer<PLSData>(new PLSData(QSharedPointer<VespucciDataset>(this),
                                                    directory_));
        bool success = partial_least_squares_data_->Apply(spectra_, abscissa_, components);
        if (success){
            partial_least_squares_calculated_ = true;
        }
    }catch(exception e){
        string str = "PartialLeastSquares: " + string(e.what());
        throw std::runtime_error(str);
    }
}

///
/// \brief VespucciDataset::KMeans
/// Implements K-means clustering using MLPACK
/// \param clusters Number of clusters to find
/// \param metric Distance metric
/// \param name Name of map in workspace.
///
void VespucciDataset::KMeans(size_t clusters, QString metric_text, QString name)
{
    if(clusters == 0){
        log_stream_ << "Cluster count predicted using HySime algorithm: ";
        clusters = HySime();
        log_stream_ << clusters << endl;
    }

    if (metric_text == "Euclidean"){
        mlpack::kmeans::KMeans<mlpack::metric::EuclideanDistance> k;
        try{
            Col<size_t> assignments;
            mat data = spectra_;
            k.Cluster(data, clusters, assignments);
            k_means_data_.set_size(assignments.n_elem, 1);
            k_means_calculated_ = true;
            //assignments += ones(assignments.n_elem);
            //k_means_data_ = assignments;
            //The assignment operator no longer works in armadillo this is temp kludge
            for (uword i = 0; i < k_means_data_.n_elem; ++i){
                k_means_data_(i) = assignments(i) + 1.0;
            }
        }
        catch(exception e){
            string str = "KMeans: " + string(e.what());
            throw std::runtime_error(str);
        }

    }

    else if (metric_text == "Manhattan"){
        mlpack::kmeans::KMeans<mlpack::metric::ManhattanDistance> k;
        try{
            Col<size_t> assignments;
            mat data = spectra_;
            k.Cluster(data, clusters, assignments);
            k_means_data_.set_size(assignments.n_elem, 1);
            k_means_calculated_ = true;
            //assignments += ones(assignments.n_elem);
            //k_means_data_ = assignments;
            //The assignment operator no longer works in armadillo this is temp kludge
            for (uword i = 0; i < k_means_data_.n_elem; ++i){
                k_means_data_(i) = assignments(i) + 1.0;
            }
        }
        catch(exception e){
            string str = "KMeans: " + string(e.what());
            throw std::runtime_error(str);
        }

    }

    else if (metric_text == "Chebyshev"){
        mlpack::kmeans::KMeans<mlpack::metric::ChebyshevDistance> k;
        try{
            Col<size_t> assignments;
            mat data = spectra_;
            k.Cluster(data, clusters, assignments);
            k_means_data_.set_size(assignments.n_elem, 1);
            k_means_calculated_ = true;
            //assignments += ones(assignments.n_elem);
            //k_means_data_ = assignments;
            //The assignment operator no longer works in armadillo this is temp kludge
            for (uword i = 0; i < k_means_data_.n_elem; ++i){
                k_means_data_(i) = assignments(i) + 1.0;
            }
        }
        catch(exception e){
            string str = "KMeans: " + string(e.what());
            throw std::runtime_error(str);
        }

    }

    else{
        mlpack::kmeans::KMeans<mlpack::metric::SquaredEuclideanDistance> k;
        try{
            Col<size_t> assignments;
            mat data = spectra_;
            k.Cluster(data, clusters, assignments);
            k_means_data_.set_size(assignments.n_elem, 1);
            k_means_calculated_ = true;
            //assignments += ones(assignments.n_elem);
            //k_means_data_ = assignments;
            //The assignment operator no longer works in armadillo this is temp kludge
            for (uword i = 0; i < k_means_data_.n_elem; ++i){
                k_means_data_(i) = assignments(i) + 1.0;
            }
        }
        catch(exception e){
            string str = "KMeans: " + string(e.what());
            throw std::runtime_error(str);
        }

    }

    //if dataset is non spatial, just quit
    if(non_spatial_){
        QMessageBox::warning(0, "Non-spatial dataset", "Dataset is non-spatial or non-contiguous! Mapping functions are not available");
        return;
    }
    log_stream_ << "KMeans" << endl;
    log_stream_ << "clusters == " << clusters << endl;
    log_stream_ << "name == " << name << endl << endl;

    if(clusters == 0){
        log_stream_ << "Cluster count predicted using HySime algorithm: ";
        clusters = HySime();
        log_stream_ << clusters << endl;
    }


    QString map_type = "K-means clustering map. Number of clusters = ";
    map_type += QString::number(clusters);


    QCPColorGradient gradient = GetClusterGradient(clusters);
    QSharedPointer<MapData> new_map(new MapData(x_axis_description_,
                                            y_axis_description_,
                                            x_, y_, k_means_data_.col(0),
                                            QSharedPointer<VespucciDataset>(this), directory_,
                                            gradient,
                                            map_list_model_->rowCount(QModelIndex()),
                                            clusters,
                                            main_window_));
    new_map->set_name(name, map_type);
    new_map->SetCrispClusters(true);
    map_list_model_->AddMap(new_map);
    new_map->ShowMapWindow();

}

void VespucciDataset::PLS_DA(vec labels, uword components, QString name)
{
    //PLSData pls_data(this, workspace->directory_ptr());

}

void VespucciDataset::ClassicalLeastSquares(const mat &standards, QString name)
{
    log_stream_ << "Classical Least Squares" << endl;
    log_stream_ << "name == " << name << endl << endl;
    mat coefs;
    try{
        coefs = Vespucci::Math::LinLeastSq::OrdinaryLeastSquares(standards, spectra_);
    }catch(exception e){
        string str = "Vespucci::Math::LinLeastSq::OrdinaryLeastSquares" + string(e.what());
        throw runtime_error(str);
    }

    AddAnalysisResult("CLS Coefficients", coefs);

}

void VespucciDataset::ClassicalLeastSquares(const mat &standards, uword image_component, QString name, int gradient_index)
{
    if(non_spatial_){
        QMessageBox::warning(0, "Non-spatial dataset", "Dataset is non-spatial or non-contiguous! Mapping functions are not available");
        return;
    }
    log_stream_ << "Classical Least Squares" << endl;
    log_stream_ << "name == " << name << endl << endl;
    mat coefs;
    try{
        coefs = Vespucci::Math::LinLeastSq::OrdinaryLeastSquares(standards, spectra_);
    }catch(exception e){
        string str = "Vespucci::Math::LinLeastSq::OrdinaryLeastSquares" + string(e.what());
        throw runtime_error(str);
    }

    AddAnalysisResult("CLS Coefficients", coefs);

    QString map_type = "CLS Map " + QString::number(image_component);
    QCPColorGradient gradient = GetGradient(gradient_index);

    QSharedPointer<MapData> new_map(new MapData(x_axis_description_,
                                                y_axis_description_,
                                                x_, y_,
                                                coefs.row(image_component).t(),
                                                QSharedPointer<VespucciDataset>(this),
                                                directory_, gradient,
                                                map_list_model_->rowCount(QModelIndex()),
                                                5, main_window_));
    new_map->set_name(name, map_type);
    new_map->SetCrispClusters(true);
    map_list_model_->AddMap(new_map);
    new_map->ShowMapWindow();
}

///
/// \brief VespucciDataset::KMeans
/// \param clusters
/// Perform k-Means clustering without creating an image
void VespucciDataset::KMeans(size_t clusters)
{
    log_stream_ << "KMeans (no image)" << endl;
    log_stream_ << "clusters == " << clusters << endl;

    if(clusters == 0){
        log_stream_ << "Cluster count predicted using HySime algorithm: ";
        clusters = HySime();
        log_stream_ << clusters << endl;
    }

    try{
        Col<size_t> assignments;
        mlpack::kmeans::KMeans<> k;
        mat data = spectra_;
        k.Cluster(data, clusters, assignments);
        k_means_data_.set_size(assignments.n_elem, 1);
        k_means_calculated_ = true;
        for (uword i = 0; i < k_means_data_.n_elem; ++i){
            k_means_data_(i) = assignments(i) + 1.0;
        }
    }
    catch(exception e){
        string str = "KMeans: " + string(e.what());
        throw std::runtime_error(str);
    }
}

// HELPER FUNCTIONS //

///
/// \brief VespucciDataset::FindRange.
/// Finds the index of the wavelength value closest
/// to the specified wavelength range.
/// \param start the first wavelength in the spectral region of interest
/// \param end the second wavelength in the spectral region of interest
/// \return
///
uvec VespucciDataset::FindRange(double start, double end) const
{
    uvec indices(2);
    indices(0) = FindIndex(start);
    indices(1) = FindIndex(end);
    return indices;
}

///
/// \brief VespucciDataset::FindOrigin
/// \return
/// Find the point closest to (0,0) in the map. If the function fails, find the
/// index in the middle of the spatial data.
uword VespucciDataset::FindOrigin() const
{
    double delta = std::max(std::abs((x_(1) - x(0))), std::abs((y_(1) - y_(0))));
    uvec zero_x = find(((0-delta) <= x_) && (0 + delta) >= x_);
    vec sub_y = y_.elem(zero_x);
    uvec zero_y = find(((0-delta) <= sub_y) && ((0+delta) >= sub_y));

    // If for some reason this doesn't work, also find the point halfway down.
    uword mid = ((x_.n_rows % 2 == 0) ? x_.n_rows : (2*(x_.n_rows + 1)) / 2);
    return (zero_y.n_rows > 0 ? zero_y(0) : mid);
}

/// \brief VespucciDataset::PointSpectrum
/// \param index
/// \return
///
QVector<double> VespucciDataset::PointSpectrum(const uword index) const
{
    //perform bounds check.
    std::vector<double> spectrum_stdvector;
    QVector<double> spectrum_qvector;
    cout << "index  = " << index << endl;
    try{
        if (index > spectra_.n_cols){
            spectrum_stdvector =
                    conv_to< std::vector<double> >::from(spectra_.col(spectra_.n_cols - 1));
        }
        else{
             spectrum_stdvector =
                     conv_to< std::vector<double> >::from(spectra_.col(index));
        }

        spectrum_qvector = QVector<double>::fromStdVector(spectrum_stdvector);
    }
    catch(exception e){
        cerr << "exception thrown!" << endl;
        main_window_->DisplayExceptionWarning("VespucciDataset::PointSpectrum", e);
    }

    cout << "end of PointSpectrum" << endl;

    return spectrum_qvector;
}

QVector<double> VespucciDataset::WavelengthQVector() const
{
    std::vector<double> abscissa_stdvector =
            conv_to< std::vector<double> >::from(abscissa_);

    QVector<double> abscissa_qvector =
            QVector<double>::fromStdVector(abscissa_stdvector);

    return abscissa_qvector;
}

uword VespucciDataset::FindIndex(double abscissa_value) const
{
    double delta = std::fabs(abscissa_(1) - abscissa_(0));
    uvec indices = find((abscissa_value - delta) < abscissa_ <= (abscissa_value + delta));
    return indices(0);
}

///
/// \brief VespucciDataset::ValueRange
/// Finds the minima and maxima of y variable to properly set axes
///  of QCustomPlot objects
/// \return
///
QCPRange VespucciDataset::ValueRange() const
{
    double lower = y_.min();
    double upper = y_.max();
    QCPRange range(upper, lower);
    return range;
}
///
/// \brief VespucciDataset::KeyRange
/// Finds the minima and maxima of x variable to properly set axes
///  of QCustomPlot objects
/// \return
///
QCPRange VespucciDataset::KeyRange() const
{
    double lower = x_.min();
    double upper = x_.max();
    QCPRange range(upper, lower);
    return range;
}

///
/// \brief VespucciDataset::KeySize
/// Finds the number of data points in x variable to properly set axes
///  of QCustomPlot objects
/// \return number of unique x values
///
int VespucciDataset::KeySize() const
{
    uword i;
    uword x_count=1;
    double x_buf;

    //loop through x until a value different then the first is met.
    if (!flipped_){
        x_count = 1; //this counts the first entry in x_
        x_buf = x_(0);
        for(i=0; i<x_.n_elem; ++i){
            if(x_(i)!=x_buf){
                ++x_count;
                x_buf=x_(i);
            }
        }
    } else{
        x_count = 0;
        for (i=0; i<x_.n_elem; ++i){
            if(y_(i)!=y_(0)){
                break;
            } else{
                ++x_count;
            }
        }
    }

    return x_count;
}

///
/// \brief VespucciDataset::ValueSize
/// Finds number of unique y values for properly setting QCPAxis
/// \return number of unique y values
///
int VespucciDataset::ValueSize() const
{

    uword i = 0;
    uword y_count;


    //long-text files hold x constant and vary y
    //until x is different, count y
    //reverse if flipped
    if (!flipped_){
        y_count = 0;
        for (i=0; i<x_.n_elem; ++i){
            if(x_(i)!=x_(0)){
                break;
            }
            else{
                ++y_count;
            }
        }
    } else{
        y_count = 1;
        double y_buf = y_(0);
        for(i=0; i<y_.n_elem; ++i){
            if(y_(i)!=y_buf){
                ++y_count;
                y_buf=y_(i);
            }
        }
    }


    return y_count;
}


// MEMBER ACCESS FUNCTIONS //
///
/// \brief VespucciDataset::wavelength
/// \return member abscissa_ (spectrum key values)
///
vec VespucciDataset::wavelength() const
{
    return abscissa_;
}

vec VespucciDataset::abscissa() const
{
    return abscissa_;
}

vec VespucciDataset::wavelength(uvec indices) const
{
    return abscissa_.rows(indices);
}

///
/// \brief VespucciDataset::x
/// \return member x_
///
colvec VespucciDataset::x() const
{
    return x_;
}

///
/// \brief VespucciDataset::indices
/// \return The indices_ vector
///
vec VespucciDataset::indices() const
{
    return indices_;
}

///
/// \brief VespucciDataset::indices_ptr
/// \return A pointer to the indices_ vector
///
mat *VespucciDataset::indices_ptr()
{
    return (mat *) &indices_;
}

///
/// \brief VespucciDataset::SetIndices
/// \param indices A new indices_ vector
///
void VespucciDataset::SetIndices(vec indices)
{
    indices_ = indices;
}

///
/// \brief VespucciDataset::x
/// \param indices Vector of indices
/// \return Subvec of x corresponding to valeus in indices
///
colvec VespucciDataset::x(uvec indices) const
{
    return x_(indices);
}

double VespucciDataset::x(uword index) const
{
    if (index >= x_.n_rows)
        return x_(x_.n_rows - 1);
    else
        return x_(index);
}



///
/// \brief VespucciDataset::y
/// \return member y_
///
colvec VespucciDataset::y() const
{
    return y_;
}

///
/// \brief VespucciDataset::y
/// \param indices Vector of indices
/// \return Subvec of y at indices
///
colvec VespucciDataset::y(uvec indices) const
{
    return y_(indices);
}

double VespucciDataset::y(uword index) const
{
    if (index >= y_.n_rows)
        return y_(y_.n_rows - 1);
    else
        return y_(index);
}

///
/// \brief VespucciDataset::spectra
/// \return member spectra_
///
mat VespucciDataset::spectra() const
{
    return spectra_;
}

cx_mat VespucciDataset::cx_spectra() const
{
    if (spectra_imag_.n_rows == spectra_.n_rows && spectra_imag_.n_cols == spectra_.n_cols)
        return cx_mat(spectra_, spectra_imag_);
    else
        return cx_mat(spectra_, zeros(spectra_.n_rows, spectra_.n_cols));
}

cx_mat VespucciDataset::cx_spectra(uvec indices) const
{
    if (spectra_imag_.n_rows == spectra_.n_rows && spectra_imag_.n_cols == spectra_.n_cols)
        return cx_mat(spectra_.cols(indices), spectra_imag_.cols(indices));
    else
        return cx_mat(spectra_.cols(indices), zeros(spectra_.n_rows, indices.n_rows));
}

///
/// \brief VespucciDataset::spectra
/// \param indices Vector of indices
/// \return Submat of spectra at indices
///
mat VespucciDataset::spectra(uvec indices) const
{
    return spectra_.cols(indices);
}

///
/// \brief VespucciDataset::name
/// \return member name_, the name of the dataset as seen by the user
///
const QString VespucciDataset::name() const
{
    return name_;
}

///
/// \brief VespucciDataset::SetName
/// \param new_name new name of dataset
///
void VespucciDataset::SetName(QString new_name)
{
    name_ = new_name;
}

///
/// \brief VespucciDataset::SetData
/// \param spectra Spectra
/// \param wavelength Spectral abscissa
/// \param x Colvec of horizontal axis spatial data
/// \param y Colvec of vertical axis spatial data
/// Set the data of the dataset. Used by the MetaDataset constructor
void VespucciDataset::SetData(mat spectra, vec wavelength, colvec x, colvec y)
{
    spectra_ = spectra;
    abscissa_ = wavelength;
    x_ = x;
    y_ = y;
}

//MAP HANDLING FUNCTIONS
///
/// \brief VespucciDataset::map_names
/// \return list of names of the maps created from this dataset
///
//QStringList VespucciDataset::map_names()
//{
//    return map_names_;
//}

///
/// \brief VespucciDataset::map_loading_count
/// \return number of maps created for this dataset
///
int VespucciDataset::map_loading_count() const
{
    return map_loading_count_;
}

///
/// \brief VespucciDataset::RemoveMapAt
/// \param i index of map in the relevant lists
///
void VespucciDataset::RemoveMapAt(unsigned int i)
{
    map_list_model_->removeRow(i, QModelIndex());

}


///
/// \brief VespucciDataset::AddMap
/// Adds a map to the list of map pointers and adds its name to relevant lists
/// \param map
///
void VespucciDataset::AddMap(QSharedPointer<MapData> map)
{
    map_list_model_->AddMap(map);
}

///
/// \brief VespucciDataset::WavelengthRange
/// \return the range of the wavlength vector (for plotting point spectra)
///
QCPRange VespucciDataset::WavelengthRange() const
{
    double min = abscissa_.min();
    double max = abscissa_.max();
    QCPRange range(min, max);
    return range;
}

///
/// \brief VespucciDataset::PointSpectrumRange
/// \param i col of spectra_ containing desired spectrum
/// \return the range of y values for the point spectra at i
///
QCPRange VespucciDataset::PointSpectrumRange(int i) const
{
    vec row = spectra_.col(i);
    double min = row.min();
    double max = row.max();

    QCPRange range(min, max);
    return range;
}

///
/// \brief VespucciDataset::GetGradient
/// Selects the color gradient from list of presets
/// \param gradient_number
/// \return
///
QCPColorGradient VespucciDataset::GetGradient(int gradient_number) const
{
    switch (gradient_number)
    {
    case 0: return QCPColorGradient::cbBuGn;
    case 1: return QCPColorGradient::cbBuPu;
    case 2: return QCPColorGradient::cbGnBu;
    case 3: return QCPColorGradient::cbOrRd;
    case 4: return QCPColorGradient::cbPuBu;
    case 5: return QCPColorGradient::cbPuBuGn;
    case 6: return QCPColorGradient::cbPuRd;
    case 7: return QCPColorGradient::cbRdPu;
    case 8: return QCPColorGradient::cbYlGn;
    case 9: return QCPColorGradient::cbYlGnBu;
    case 10: return QCPColorGradient::cbYlOrBr;
    case 11: return QCPColorGradient::cbYlOrRd;
    case 12: return QCPColorGradient::cbBlues;
    case 13: return QCPColorGradient::cbGreens;
    case 14: return QCPColorGradient::cbOranges;
    case 15: return QCPColorGradient::cbPurples;
    case 16: return QCPColorGradient::cbReds;
    case 17: return QCPColorGradient::cbGreys;
    case 18: return QCPColorGradient::gpGrayscale;
    case 19: return QCPColorGradient::gpNight;
    case 20: return QCPColorGradient::gpCandy;
    case 21: return QCPColorGradient::gpIon;
    case 22: return QCPColorGradient::gpThermal;
    case 23: return QCPColorGradient::gpPolar;
    case 24: return QCPColorGradient::gpSpectrum;
    case 25: return QCPColorGradient::gpJet;
    case 26: return QCPColorGradient::gpHues;
    case 27: return QCPColorGradient::gpHot;
    case 28: return QCPColorGradient::gpCold;
    case 29: return QCPColorGradient::cbBrBG;
    case 30: return QCPColorGradient::cbPiYG;
    case 31: return QCPColorGradient::cbPRGn;
    case 32: return QCPColorGradient::cbPuOr;
    case 33: return QCPColorGradient::cbRdBu;
    case 34: return QCPColorGradient::cbRdGy;
    case 35: return QCPColorGradient::cbRdYlBu;
    case 36: return QCPColorGradient::cbRdYlGn;
    case 37: return QCPColorGradient::cbSpectral;
    case 38: return QCPColorGradient::vSpectral;
    default: return QCPColorGradient::gpCold;
    }
}

///
/// \brief VespucciDataset::GetClusterGradient
/// Cluster gradients are slightly different from the continuous gradients. This
/// selects the right gradient based on the number of clusters.
/// \param clusters Number of clusters
/// \return Proper color gradient for number of clusters
///
QCPColorGradient VespucciDataset::GetClusterGradient(int clusters) const
{
    switch (clusters)
    {
    case 2: return QCPColorGradient::cbCluster2;
    case 3: return QCPColorGradient::cbCluster3;
    case 4: return QCPColorGradient::cbCluster4;
    case 5: return QCPColorGradient::cbCluster5;
    case 6: return QCPColorGradient::cbCluster6;
    case 7: return QCPColorGradient::cbCluster7;
    case 8: return QCPColorGradient::cbCluster8;
    case 9: return QCPColorGradient::cbCluster9;
    default: return QCPColorGradient::cbCluster9;
    }
}

///
/// \brief VespucciDataset::ConstructorCancelled
/// Specifies whether or not the constructor has been canceled. The constructor
/// asks this and cleans everything up in case it is canceled.
/// \return
///
bool VespucciDataset::ConstructorCancelled() const
{
    return constructor_canceled_;
}

///
/// \brief VespucciDataset::AverageSpectrum
/// Finds the average of the spectrum. This can be saved by the user.
/// Probably not all that useful, except for determining a spectrum to use as a
/// background spectrum for other maps.
/// \param stats Whether or not to include standard deviations on the second row.
/// \return The average spectrum
///
mat VespucciDataset::AverageSpectrum(bool stats) const
{
    mat spec_mean = mean(spectra_, 1);
    vec spec_stddev;
    //insert stddevs on next line if requested
    if (stats){
        spec_stddev = stddev(spectra_, 0, 1);
        spec_mean.insert_cols(1, spec_stddev);
    }
    return spec_mean;
}



///
/// \brief VespucciDataset::x_axis_description
/// The x_axis_description is printed on the spectrum viewer.
/// \return Spectral abscissa description.
///
const QString VespucciDataset::x_axis_description() const
{
    return x_axis_description_;
}

///
/// \brief VespucciDataset::SetXDescription
/// Sets the value of the spectral abscissa description.
/// \param description
///
void VespucciDataset::SetXDescription(QString description)
{
    x_axis_description_ = description;
}

///
/// \brief VespucciDataset::SetYDescription
/// \param description
/// Sets the value of the spectral ordinate axis description
void VespucciDataset::SetYDescription(QString description)
{
    y_axis_description_ = description;
}

///
/// \brief VespucciDataset::y_axis_description
/// \return The spectral ordinate axis description.
///
const QString VespucciDataset::y_axis_description() const
{
    return y_axis_description_;
}

///
/// \brief VespucciDataset::principal_components_calculated
/// Accessor for principal_components_calculated_. The PCA dialog requests this
/// to make sure that the same PCA is not calculated twice.
/// \return Whether or not PCA has been calculated.
///
bool VespucciDataset::principal_components_calculated() const
{
    return principal_components_calculated_;
}

bool VespucciDataset::mlpack_pca_calculated() const
{
    return mlpack_pca_calculated_;
}


///
/// \brief VespucciDataset::vertex_components_calculated
/// Accessor for vertex_components_calculated_. The VCA dialog requests this to
/// make sure that the same VCA is not calculated twice.
/// \return Whether or not VCA has been computed.
///
bool VespucciDataset::vertex_components_calculated() const
{
    return vertex_components_calculated_;
}

///
/// \brief VespucciDataset::partial_least_squares_calculated
/// Accessor for partial_least_squares_calculated. The PLS dialog requests this
/// to make sure that the same PLS is not calculated twice.
/// \return Whether or not PLS has been computed.
///
bool VespucciDataset::partial_least_squares_calculated() const
{
    return partial_least_squares_calculated_;
}

///
/// \brief VespucciDataset::k_means_calculated
/// Accessor for k_means_calculated_. Used for filling dataviewer.
/// \return Whether or not k means have been calculated.
///
bool VespucciDataset::k_means_calculated() const
{
    return k_means_calculated_;
}

///
/// \brief VespucciDataset::principal_components_data
/// Accessor for principal_components_data_
/// \return Pointer to the class that handles the output of arma::princomp
///
QSharedPointer<PrincipalComponentsData> VespucciDataset::principal_components_data()
{
    return principal_components_data_;
}

QSharedPointer<MLPACKPCAData> VespucciDataset::mlpack_pca_data()
{
    return mlpack_pca_data_;
}

///
/// \brief VespucciDataset::vertex_components_data
/// Accessor for vertex_components_data_
/// \return
///
QSharedPointer<VCAData> VespucciDataset::vertex_components_data()
{
    return vertex_components_data_;
}

///
/// \brief VespucciDataset::partial_least_squares_data
/// Accessor for partial_least_squares_data_;
/// \return
///
QSharedPointer<PLSData> VespucciDataset::partial_least_squares_data()
{
    return partial_least_squares_data_;
}

///
/// \brief VespucciDataset::k_means_data
/// \return Pointer to the matrix containing the k-means assignments
///
mat *VespucciDataset::k_means_data()
{
    return &k_means_data_;
}

///
/// \brief VespucciDataset::univariate_datas
/// \return The container holding smart pointers to the univariate data objects
///
QList<QSharedPointer<UnivariateData> > VespucciDataset::univariate_datas()
{
    return univariate_datas_;
}

///
/// \brief VespucciDataset::spectra_ptr
/// \return
///
mat* VespucciDataset::spectra_ptr()
{
    return &spectra_;
}



///
/// \brief VespucciDataset::Undoable
/// \return Whether or not the last operation can be undone
///
bool VespucciDataset::Undoable() const
{
    return (spectra_old_.n_elem > 0 ? true : false);
}

///
/// \brief VespucciDataset::UnivariateCount
/// \return
/// Number of univariate/band ratio data objects have been created
int VespucciDataset::UnivariateCount() const
{
    return univariate_datas_.size();
}

///
/// \brief VespucciDataset::abscissa_ptr
/// \return
///
mat* VespucciDataset::abscissa_ptr()
{
    return &abscissa_;
}

mat* VespucciDataset::wavelength_ptr()
{
    return &abscissa_;
}

///
/// \brief VespucciDataset::x_ptr
/// \return
///
mat* VespucciDataset::x_ptr()
{
    return &x_;
}

///
/// \brief VespucciDataset::y_ptr
/// \return
///
mat* VespucciDataset::y_ptr()
{
    return &y_;
}

///
/// \brief VespucciDataset::non_spatial
/// \return True if map has empty x_ and y_
///
bool VespucciDataset::non_spatial() const
{
    return non_spatial_;
}

///
/// \brief VespucciDataset::meta
/// \return
/// Whether or not this is an instance of MetaDataset
bool VespucciDataset::meta() const
{
    return meta_;
}

///
/// \brief VespucciDataset::SetParentDatasetIndices
/// \param parent_dataset_indices
/// Set the parent_dataset_indices_ variable. Used by the MetaDataset constructor.
void VespucciDataset::SetParentDatasetIndices(mat parent_dataset_indices)
{
    parent_dataset_indices_ = parent_dataset_indices;
}

mat *VespucciDataset::parent_dataset_indices()
{
    return &parent_dataset_indices_;
}

///
/// \brief VespucciDataset::last_operation
/// \return Description of last pre-processing operation performed
///
const QString VespucciDataset::last_operation() const
{
    return last_operation_;
}

MapListModel* VespucciDataset::map_list_model()
{
    return map_list_model_;
}

///
/// \brief VespucciDataset::AnalysisResultsList
/// \return
/// Interface to analysis_results_
QStringList VespucciDataset::AnalysisResultsList()
{
    return analysis_results_.keys();
}

///
/// \brief VespucciDataset::AnalysisResult
/// \param key
/// \return
/// Interface to analysis_results_
mat *VespucciDataset::AnalysisResult(QString key)
{
    mat *matrix_ptr;
    bool checked = false;
    if (key.split(" ")[0] == "PCA" && principal_components_calculated_){
        matrix_ptr = principal_components_data_->value(key);
    }
    else if (key.split(" ")[0] == "VCA" && vertex_components_calculated_){
        matrix_ptr = vertex_components_data_->value(key);
    }
    else if (key.split(" ")[0] == "PLS" && partial_least_squares_calculated_){
        matrix_ptr = partial_least_squares_data_->value(key);
    }
    else if (key.split(" ")[0] == "MLPACK" && mlpack_pca_calculated_){
        matrix_ptr = mlpack_pca_data_->value(key);
    }
    else if (key.split(" ")[0] == "k-Means" && k_means_calculated_){
        if (key == "k-Means Assignments")
            matrix_ptr = (mat *) &k_means_data_;
        else
            matrix_ptr = NULL;
    }
    else{
        checked = true;
        matrix_ptr = analysis_results_.value(key)->value_ptr();
    }
    //double check if there is an analysis result that starts with searched sequence but not in relevant object
    if (checked && matrix_ptr == NULL)
        matrix_ptr = analysis_results_.value(key)->value_ptr();

    return matrix_ptr;
}

void VespucciDataset::AddAnalysisResult(string key, mat value)
{
    QSharedPointer<AnalysisResults> results(new AnalysisResults(value));
    analysis_results_.insert(QString::fromStdString(key), results);
}

void VespucciDataset::AddAnalysisResults(std::map<string, mat> results)
{
    for (std::map<string, mat>::iterator i = results.begin(); i != results.end(); ++i){
        QSharedPointer<AnalysisResults> results(new AnalysisResults(i->second));
        analysis_results_.insert(QString::fromStdString(i->first), results);
    }
}
