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
#ifndef ACCESSORY_H
#define ACCESSORY_H
#include <mlpack/core.hpp>
#include "Math/Accessory/accessory_impl.h"
namespace Vespucci
{
    namespace Math
    {
        //general Math functions
        arma::sp_mat LocalMaxima(const arma::mat &X);
        arma::sp_mat LocalMaxima(const arma::mat &X, const arma::mat &dX, const arma::mat &d2X);
        arma::sp_mat LocalMaxima(const arma::mat &dX, const arma::mat &d2X);

        arma::sp_mat LocalMinima(const arma::mat &X);
        arma::sp_mat LocalMinima(const arma::mat &X, const arma::mat &dX, const arma::mat &d2X);
        arma::sp_mat LocalMinima(const arma::mat &dX, const arma::mat &d2X);

        arma::sp_mat LocalMinimaWindow(const arma::mat &X, const int window_size);
        arma::sp_mat LocalMaximaWindow(const arma::mat &X, const int window_size);


        arma::sp_mat LocalMinimaCWT(arma::mat coefs, arma::uvec scales, arma::uword min_window_size, double amplitude_threshold);
        arma::sp_mat LocalMaximaCWT(arma::mat coefs, arma::uvec scales, arma::uword min_window_size);

        double quantile (arma::vec &data, double probs);
        double mad (arma::vec &data);
        arma::vec ExtendToNextPow(arma::vec X, arma::uword n);
        arma::uword NextPow(arma::uword number, arma::uword power);

        arma::mat spdiags(arma::mat B, arma::ivec d, arma::uword m, arma::uword n);
        arma::mat orth(arma::mat X); //this is implemented better in mlpack


        arma::uword min(arma::uword a, arma::uword b);
        arma::uword max(arma::uword a, arma::uword b);

        void position(arma::uword index,
                      arma::uword n_rows, arma::uword n_cols,
                      arma::uword &i, arma::uword &j);
        arma::umat to_row_column(arma::uvec indices, arma::uword n_rows, arma::uword n_cols);

        double RecalculateAverage(double new_value, double old_average, double old_count);
        double RecalculateStdDev(double new_value, double old_mean, double old_stddev, double old_count);

        arma::umat GetClosestValues(arma::vec query, arma::vec target, const arma::uword k=5);

        double CalcPoly(const double x, const arma::vec &coefs);

        arma::vec WavelengthToFrequency(const arma::vec &x, double freq_factor, double wl_factor);
        arma::vec FrequencyToWavelength(const arma::vec &x, double wl_factor, double freq_factor);
        arma::vec FrequencyToEnergy(const arma::vec &x, double energy_factor, double freq_factor);
        arma::vec EnergyToFrequency(const arma::vec &x, double freq_factor, double energy_factor);
        arma::vec WavenumberToFrequency(const arma::vec &x, double freq_factor, double wn_factor);
        arma::vec FrequencyToWavenumber(const arma::vec &x, double wn_factor, double freq_factor);
        arma::vec WavenumberToWavelength(const arma::vec &x, double wn_factor, double wl_factor);
        arma::vec WavelengthToWavenumber(const arma::vec &x, double wl_factor, double wn_factor);
        arma::vec WavelengthToEnergy(const arma::vec &x, double energy_factor, double wl_factor);
        arma::vec EnergyToWavelength(const arma::vec &x, double wl_factor, double energy_factor);
        arma::vec EnergyToWavenumber(const arma::vec &x, double wn_factor, double energy_factor);
        arma::vec WavenumberToEnergy(const arma::vec &x, double energy_factor, double wn_factor);

    }
}


#endif //ACCESSORY_H
