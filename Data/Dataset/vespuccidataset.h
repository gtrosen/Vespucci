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
#ifndef VespucciDataset_H
#define VespucciDataset_H
#include "Math/VespucciMath.h"

#include <QTextStream>
#include <iostream>
#include <QVector>
#include <vector>
#include <qcustomplot.h>
#include "Data/Analysis/principalcomponentsdata.h"
#include "Data/Analysis/plsdata.h"
#include "Data/Analysis/vcadata.h"
#include "Global/vespucciworkspace.h"
#include "Data/Imaging/mapdata.h"
#include <cmath>
#include <fstream>
#include "GUI/mainwindow.h"
#include "Data/Analysis/plsdata.h"
#include "Data/Import/textimport.h"
#include "Data/Import/binaryimport.h"
#include "Data/Import/textimportqpd.h"
#include "GUI/QAbstractItemModel/maplistmodel.h"
#include "Data/Analysis/univariatedata.h"
#include "Data/Analysis/mlpackpcadata.h"
#include "Data/Analysis/analysisresults.h"
#include "Data/Analysis/cwtdata.h"




class MapData;
class PrincipalComponentsData;
class MLPACKPCAData;
class PLSData;
class VCAData;
class SpectrumViewer;
class MainWindow;
class VespucciWorkspace;
class MapListModel;
class UnivariateData;
class AnalysisResults;
class CWTData;



using namespace std;
using namespace arma;



///
/// \brief The VespucciDataset class
/// This is the main class for dealing with hyperspectral data. This handles the
///  import and export of spectra, and the creation of maps. Images are handled
/// by the MapData class.
class VespucciDataset
{
public:
    VespucciDataset(const VespucciDataset &dataset);
    VespucciDataset(QString text_filename,
                    MainWindow *main_window,
                    QString *directory,
                    QFile *log_file,
                    QString name,
                    QString x_axis_description,
                    QString y_axis_description,
                    bool swap_spatial,
                    std::string format);
    VespucciDataset(map<pair<int,int>, string> text_filenames,
                    MainWindow *main_window,
                    QString *directory,
                    QFile *log_file,
                    QString name,
                    QString x_axis_description,
                    QString y_axis_description,
                    int rows, int cols);

    VespucciDataset(QString vespucci_binary_filename,
                    MainWindow *main_window,
                    QString *directory,
                    QFile *log_file,
                    QString name,
                    QString x_axis_description,
                    QString y_axis_description);

    VespucciDataset(QString binary_filename,
                    MainWindow *main_window,
                    QString *directory,
                    QString name,
                    QFile *log_file);


    VespucciDataset(QString name,
                    MainWindow *main_window,
                    QString *directory,
                    QFile *log_file,
                    QSharedPointer<VespucciDataset> original,
                    uvec indices);

    VespucciDataset(QString name,
                    MainWindow *main_window,
                    QString *directory,
                    QFile *log_file);

    ~VespucciDataset();
    // PRE-PROCESSING FUNCTIONS //
    //general
    void Undo();

    // map editing
    void CropSpectra(double x_min, double x_max, double y_min, double y_max, double wl_min, double wl_max);

    // Normalization functions

    mat ZScoreNormCopy();
    void MinMaxNormalize();
    void VectorNormalize();
    void MeanCenter();
    void ZScoreNormalize();
    void SNVNormalize(double offset, bool center);
    void AbsoluteValue();
    void UnitAreaNormalize();
    void PeakIntensityNormalize(double peak_position);
    void PeakIntensityNormalize(double left_bound, double right_bound);
    void Booleanize(double min, double max, bool keep_inside, bool oneify);
    void Clamp(double min, double max);
    void ShedZeroSpectra();
    void ShedZeroWavelengths();

    //Spectral Pre-processing

    void SubtractBackground(mat background, QString filename);
    void MedianFilter(unsigned int window_size);
    void LinearMovingAverage(unsigned int window_size);
    void SavitzkyGolay(unsigned int derivative_order,
                    unsigned int polynomial_order,
                    unsigned int window_size);
    void SingularValue(unsigned int singular_values);
    int QUIC_SVD(double epsilon);
    void MFBaseline(int window_size, int iterations);
    void CWTBaseline(int lambda, int penalty_order, double SNR_threshold, double peak_shape_threshold);
    void IModPolyBaseline(const uword poly_order, const uword max_it, double threshold);
    void RemoveClippedSpectra(double threshold);
    void RemoveFlatSpectra(double threshold);
    void Scale(double scaling_factor);
    void ShedSpectrum(const uword index);
    int HySime();
    void TransformAbscissa(QString input_units, double input_factor,
                           QString output_units, double output_factor, QString description);
    void InterpolateToNewAbscissa(const vec &new_abscissa,
                                  unsigned int poly_order,
                                  unsigned int window_size);
    void InterpolateToNewAbscissa(const vec &new_abscissa);
    void FourierTransform(int n);
    void InverseFourierTransform(int n);
    void ApplyFTWeight(QString type, double param);
    void ApplyFTWeight(double start_offset, double end_offset,
                       double power);



    // HELPER FUNCTIONS //

    // Grabs one line of spectra_, at indices specified, recasts as QVector for QCustomPlot //
    QVector<double> PointSpectrum(const uword index) const;

    // Converts wavelength_ to QVector //
    QVector<double> WavelengthQVector() const;

    // Finds the index of spectra_ with x and y values closest to the input //
    int FindIndex(int x_value, int y_value) const;
    uword FindIndex(double abscissa_value) const;

    // Returns vector containing start and end index within a range
    uvec FindRange(double start, double end) const;

    uword FindOrigin() const;

    // These QCustomPlot sizes apply to all maps created from dataset //
    int KeySize() const;
    int ValueSize() const;
    QCPRange KeyRange() const;
    QCPRange ValueRange() const;

    bool Save(QString filename);
    bool SaveSpectrum(QString filename, uword column, file_type type);
    bool SaveLogFile(QString filename);
    // ANALYSIS/IMAGING FUNCTIONS //
    void Univariate(double min,
                    double max,
                    QString name,
                    UnivariateMethod::Method method,
                    uword window,
                    bool plot,
                    uword gradient_index);
    
    
    void BandRatio(double first_min,
                   double first_max,
                   double second_min,
                   double second_max,
                   QString name,
                   UnivariateMethod::Method method,
                   uword window,
                   bool plot,
                   unsigned int gradient_index);
    
    void Correlation(vec control, QString name, uword gradient_index, bool plot);

    void PartialLeastSquares(uword components, uword image_component,
                             QString name,
                             unsigned int gradient_index, bool recalculate);


    void PrincipalComponents(int component,
                             QString name,
                             int gradient_index,
                             bool recalculate);

    void PrincipalComponents(bool scaleData);

    void VertexComponents(uword endmembers, uword image_component,
                          QString name, unsigned gradient_index, bool recalculate);
    void KMeans(size_t clusters, QString metric_text,
                QString name);

    void IndependentComponents(int image_component,
                               QString name,
                               int gradient_index,
                               bool recalculate);

    void Agglomerative(int clusters,
                      QString distance_metric,
                      QString name);
    void LinearDiscriminantAnalysis(mat in_spectra);
    void PLS_DA(vec labels, uword components, QString name);
    void ClassicalLeastSquares(const mat &standards, QString name);
    void ClassicalLeastSquares(const mat &standards,
                               uword image_component,
                               QString name,
                               int gradient_index);

    //Analysis functions (overloads of imaging functions)
    void PartialLeastSquares(uword components);
    void VertexComponents(uword endmembers);
    void KMeans(size_t clusters);
    void PrincipalComponents();
    void FindPeaks(double sel, double threshold,
                   uword poly_order, uword window_size);
    void FindPeaksCWT(std::string wavelet,
                      uword max_scale, uword gap_threshold,
                      uword min_ridge_length,
                      uword search_width,
                      double noise_threshold,
                      std::string noise_method,
                      uword noise_window_size,
                      bool save_coefs,
                      bool save_coef_plots,
                      bool save_ridge_plots,
                      bool save_ridge_plot_data,
                      bool estimate_width,
                      QString save_directory,
                      QString image_format,
                      QCPColorGradient gradient);
    void HasPeaksCWT(const mat &range_list);


    //MEMBER ACCESS FUNCTIONS:
    vec wavelength() const;
    vec abscissa() const;
    colvec x() const;
    colvec y() const;
    colvec x(uvec indices) const;
    colvec y(uvec indices) const;
    double x(uword index) const;
    double y(uword index) const;
    vec wavelength(uvec indices) const;
    mat spectra(uvec indices) const;
    mat spectra() const;
    cx_mat cx_spectra() const;
    cx_mat cx_spectra(uvec indices) const;
    const QString name() const;
    vec indices() const;
    mat *indices_ptr();

    bool principal_components_calculated() const;
    bool mlpack_pca_calculated() const;
    bool vertex_components_calculated() const;
    bool partial_least_squares_calculated() const;
    bool k_means_calculated() const;

    QSharedPointer<PrincipalComponentsData> principal_components_data();
    QSharedPointer<MLPACKPCAData> mlpack_pca_data();
    QSharedPointer<VCAData> vertex_components_data();
    QSharedPointer<PLSData> partial_least_squares_data();
    mat* k_means_data();
    QList<QSharedPointer<UnivariateData> > univariate_datas();
    const QString x_axis_description() const;
    const QString y_axis_description() const;

    void SetName(QString new_name);
    void SetData(mat spectra, vec wavelength, colvec x, colvec y);

    void SetIndices(vec indices);


    //adds or removes dataset or map to relevant lists
    void AddDataset(VespucciDataset dataset);
    void RemoveDataset(QString name);

    void AddMap(QSharedPointer<MapData> map);
    void RemoveMapAt(unsigned int i);
    int map_loading_count() const;

    void SetXDescription(QString description);
    void SetYDescription(QString description);

    QCPRange WavelengthRange() const;
    QCPRange PointSpectrumRange(int i) const;

    QCPColorGradient GetGradient(int gradient_number) const;
    QCPColorGradient GetClusterGradient(int clusters) const;

    bool ConstructorCancelled() const;
    mat AverageSpectrum(bool stats) const;

    mat *x_ptr();
    mat *y_ptr();
    mat *wavelength_ptr();
    mat *abscissa_ptr();
    mat *spectra_ptr();
    mat *spectra_old_ptr(bool *ok);
    bool non_spatial() const;
    bool meta() const;
    void SetParentDatasetIndices(mat parent_dataset_indices);
    mat *parent_dataset_indices();
    bool Undoable() const;

    int UnivariateCount() const;

    const QString last_operation() const;

    ///
    /// \brief log_stream_ A log of all UI-induced functions called on this dataset
    ///
    QTextStream log_stream_;

    MapListModel *map_list_model();

    QStringList AnalysisResultsList();
    mat *AnalysisResult(QString key);
    void AddAnalysisResult(std::string key, mat value);
    void AddAnalysisResults(std::map<std::string, mat> results);

    void DestroyLogFile();
    void SetOldCopies();

private:

    ///
    /// \brief abscissa_
    /// The spectral abscissa of the spectra in spectra_
    vec abscissa_;
    vec abscissa_old_;

    ///
    /// \brief x_
    /// The spatial horizontal position
    colvec x_;
    colvec x_old_;

    ///
    /// \brief y_
    /// The spatial vertical position
    colvec y_;
    colvec y_old_;

    ///
    /// \brief spectra_
    /// The list of spectra. Every index corresponds to a value for x, a value
    /// for y, and a spectrum.
    mat spectra_;

    ///
    /// \brief spectra_old_
    /// A copy of spectra_, which is not modified by pre-processing functions
    /// until another pre-processing function is called. This allows for undoing
    /// pre-processing functions.
    mat spectra_old_;

    ///
    /// \brief spectra_imag_
    /// The imaginary part of the spectra after fft (makes ifft)
    mat spectra_imag_;


    ///
    /// \brief baselines_
    /// If spectra is baseline corrected, this matrix is the baseline of each
    /// spectrum.
    mat baselines_;

    ///
    /// \brief name_
    /// The name chosen by the user for this dataset.
    QString name_;

    ///
    /// \brief log_file_ Pointer to the QFile associated with the log stream
    /// This is allocated with "new" outside the VespucciDataset object, then
    /// deallocated with delete inside the VespucciDataset destructor
    QFile *log_file_;




    ///
    /// \brief last_operation_
    /// Description of the last operation performed
    QString last_operation_;


    ///
    /// \brief map_list_model_
    /// The model that exposes the list of maps.
    MapListModel *map_list_model_;

    ///
    /// \brief *maps_
    /// A list of the maps created from this data set, managed by the MapListModel
    QList<QSharedPointer<MapData> > *maps_;


    //CWTData *cwt_peak_data_;
    QSharedPointer<CWTData> cwt_peak_data_;

    ///
    /// \brief principal_components_data_
    /// A pointer to a container holding all PCA data and stats.
    //PrincipalComponentsData *principal_components_data_;
    QSharedPointer<PrincipalComponentsData> principal_components_data_;

    ///
    /// \brief mlpack_pca_data_
    /// A pointer to a container holding MLPACK PCA data and stats
    QSharedPointer<MLPACKPCAData> mlpack_pca_data_;

    ///
    /// \brief partial_least_squares_data_
    /// A pointer to a container holding all PLS data and stats
    QSharedPointer<PLSData> partial_least_squares_data_;

    ///
    /// \brief vertex_components_data_
    /// A pointer to a container (on the heap) holding all VCA data and stats.
    QSharedPointer<VCAData> vertex_components_data_;

    ///
    /// \brief k_means_data_
    /// A matrix containing the K-means cluster assignments. A container object
    /// may be used later.
    mat k_means_data_;

    ///
    /// \brief parent_dataset_indices_
    /// The index of the parent dataset, if this is a MetaDataset and this is set.
    /// This is a 3-column matrix. 0 == dataset number, 1 = x, 2 = y.
    mat parent_dataset_indices_;

    ///
    /// \brief map_loading_count_
    /// How many maps have been loaded, used mostly for default names.
    int map_loading_count_;

    ///
    /// \brief x_axis_description_
    /// Spectral abscissa legend
    QString x_axis_description_; //abscissa legend

    ///
    /// \brief y_axis_description_
    /// Spectral ordinate legend
    QString y_axis_description_; //ordinate legend

    ///
    /// \brief directory_
    /// Pointer back to the string in the workspace containing the working path
    QString *directory_;

    ///
    /// \brief indices_
    /// The indices established on dataset import.
    vec indices_;

    ///
    /// \brief z_scores_calculated_
    /// Whether or not Z-scores have been calculated (triggers determination of
    /// "negative" peaks (finding minimums are areas where spectrum is lower than
    /// average)
    bool z_scores_calculated_;

    ///
    /// \brief principal_components_calculated_
    /// Whether or not PCA has been performed
    bool principal_components_calculated_;

    bool mlpack_pca_calculated_;

    ///
    /// \brief partial_least_squares_calculated_
    /// Whether or not PLS has been performed
    bool partial_least_squares_calculated_;

    ///
    /// \brief vertex_components_calculated_
    /// Whether or not VCA has been performed.
    bool vertex_components_calculated_;

    ///
    /// \brief k_means_calculated_
    /// Whether or not k means has been performed.
    bool k_means_calculated_;

    ///
    /// \brief radical_calcualted_
    /// Whether or not RADICAL has been performed.
    bool radical_calcualted_;

    ///
    /// \brief constructor_canceled_
    /// Whether or not cancel button is clicked.
    bool constructor_canceled_;

    ///
    /// \brief flipped_
    /// Whether or not original data set had x in the second column
    bool flipped_;

    ///
    /// \brief main_window_
    /// Pointer back to the main window of the app.
    MainWindow *main_window_;


    ///
    /// \brief non_spatial_
    /// True if the dataset has empty x_ and y_
    bool non_spatial_;

    ///
    /// \brief meta_
    /// Whether or not this dataset is a MetaDataset
    bool meta_;

    ///
    /// \brief univariate_datas_
    /// Container for UnivariateData objects allocated on heap.
    QList<QSharedPointer<UnivariateData> > univariate_datas_;

    ///
    /// \brief analysis_results_
    /// A container for various analyses indexed by name
    QMap<QString, QSharedPointer<AnalysisResults> > analysis_results_;

};
Q_DECLARE_METATYPE(QSharedPointer<VespucciDataset> *)

#endif // VespucciDataset_H
