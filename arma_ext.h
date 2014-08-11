/*******************************************************************************
    Copyright (C) 2014 Daniel P. Foose - All Rights Reserved

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
#ifndef ARMA_EXT_H
#define ARMA_EXT_H
#include "armadillo"
#include <QApplication>
#include <QVector>
#include <QMessageBox>

using namespace arma;

namespace arma_ext
{
mat spdiags(mat B, QVector<int> d, int m, int n);
mat svds(mat X, int k, std::string form);
mat svds(mat X, int k);
mat svds(mat X, std::string form);
mat svds(mat X);
}

#endif // ARMA_EXT_H