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
#ifndef LINLEASTSQ_H
#define LINLEASTSQ_H
#include "Math/VespucciMath.h"
namespace Vespucci{
    namespace Math{
        /// \namespace Vespucci::Math:::LinLeastSq
        /// \brief A namespace for math functions relating to linear
        /// least squares regression.
        ///
        ///
        namespace LinLeastSq{
            arma::uword IModPoly(const arma::vec &spectrum, const arma::vec &abscissa,
                           arma::vec &baseline, arma::vec &corrected,
                           double &err,
                           const arma::uword poly_order,
                           const arma::uword max_it,
                           const double threshold);

            double CalcDev(const arma::vec &spectrum, const arma::vec &fit);
            arma::uvec NonPeakInd(const arma::vec &spectrum, const double dev);
            arma::vec CalcPoly(const arma::vec &coefs, const arma::vec &x);
            arma::mat OrdinaryLeastSquares(const arma::mat &X, const arma::mat &y);
            arma::mat OrdinaryLeastSquares(const arma::mat &X, const arma::mat &y, arma::mat &coef_errors, arma::mat &predicted);
            arma::vec OrdinaryLeastSquares(const arma::mat &X, const arma::vec &y, arma::vec &fit, std::map<std::string, double> stats);
            arma::mat Vandermonde(const arma::vec &x, const int poly_order);
            double CalcErr(const double &dev, const double &prev_dev);
        }
    }

}


#endif // LINLEASTSQ_H
