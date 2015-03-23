#ifndef ACCESSORY_IMPL_H
#define ACCESSORY_IMPL_H
//Implementation of template functions
#include <Math/Accessory/accessory.h>
namespace Vespucci
{
    namespace Math
    {
        template <typename T> T conv_fft(T &A, T &B, std::string type);
        template <typename T> T rotate(T X, arma::uword shift_by, bool slide_back = true);
        template <typename T> T diff(T X, arma::uword deriv_order = 1);
    }
}


///
/// \brief Vespucci::Math::diff
/// \param X A vector represetning a signal or a arma::matrix where each column is a
/// signal
/// \param deriv_order The derivative order to calculate
/// \return The deriv_order-th derivative of X, a vector with deriv_order fewer
/// elements than X.
///
/// Perform arithmetic differentiation of a vector or arma::matrix (if X is a arma::matrix,
/// the differentiation is performed to all the columns).
///
template <typename T> T Vespucci::Math::diff(T X, arma::uword deriv_order)
{
        T return_value;
        if (deriv_order == 0){
            return_value = X;
        }

        else if (deriv_order > 0){ //perform first derivative of input
            T offset(X.n_rows, X.n_cols);
            offset.zeros();
            offset.rows(1, offset.n_rows - 1) = X.rows(0, X.n_rows - 2);
            //offset.insert_rows(0, 1);
            //offset.row(0) = arma::zeros(1, X.n_cols);
            //offset.shed_row(offset.n_rows - 1);
            T difference;
            difference = X - offset;
            difference.shed_row(0); //difference will have one less row
            return_value = diff(difference, deriv_order - 1);
        }

        return return_value;
}



template <typename T> T Vespucci::Math::rotate(T X, arma::uword shift_by, bool slide_back = true)
{
    T start;
    T end;

    if (shift_by >= X.n_rows){
        shift_by = shift_by - X.n_rows;
    }

    if (shift_by == 0){
        return X;
    }
    else if (shift_by == 1){
        if (slide_back){
            start = X.rows(1, X.n_rows - 1);
            end = X.row(0);
        }
        else{
            start = X.row(X.n_rows - 1);
            end = X.rows(0, X.n_rows - 2);
        }
    }
    else{
        if (slide_back){
            start = X.rows(shift_by, X.n_rows - 1);
            end = X.rows(0, shift_by - 1);
        }
        else{
            start = X.rows(X.n_rows - shift_by, X.n_rows - 1);
            end = X.rows(0, X.n_rows - shift_by - 1);
        }
    }

    return join_vert(start, end);
}

//Column-wise 1-D convolution (may re-write with 2d fft later)
template <typename T> T Vespucci::Math::conv_fft(T &A, T &B, std::string type = "circular")
{

    if (A.n_rows < B.n_rows){
        throw std::invalid_argument("B must have the same number of rows or fewer rows than A");
    }

    if (type == "filter"){
        arma::uword next_p2 = std::ceil(std::log(A.n_elem) / std::log(2));
        arma::uword n = std::pow(2, next_p2);
        arma::cx_mat A_hat = fft(A, n);
        arma::cx_mat B_hat = fft(B, n);
        arma::cx_mat w = ifft(A_hat % B_hat, n);
        w = w.rows(0, A.n_rows - 1);
        return real(w);
    }
    else if (type == "circular"){

        T A_padded = join_vert(A, arma::zeros(B.n_rows- 1));
        T B_padded = join_vert(B, arma::zeros(A.n_rows - 1));

        arma::uword next_p2 = std::ceil(std::log(A.n_elem) / std::log(2));
        arma::uword n = std::pow(2, next_p2);

        arma::cx_mat A_hat = fft(A_padded, n);
        arma::cx_mat B_hat = fft(B_padded, n);
        arma::cx_mat w = ifft(A_hat % B_hat, n);
        //w = w.rows(0, A.n_rows - 1);
        return real(w);
    }
    else
        throw(std::invalid_argument("argument 'type' invalid"));
}
#endif // ACCESSORY_IMPL_H
