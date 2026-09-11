/***************************************************************************
    File                 : GslRAII.h
    Project              : QtiPlot
    Description          : RAII smart pointer wrappers for GSL C data types
 ***************************************************************************/

#ifndef GSL_RAII_H
#define GSL_RAII_H

#include <memory>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_permutation.h>
#include <gsl/gsl_fft_real.h>
#include <gsl/gsl_fft_halfcomplex.h>
#include <gsl/gsl_fft_complex.h>
#include <gsl/gsl_multimin.h>

namespace GslRAII {

struct VectorDeleter {
    void operator()(gsl_vector* v) const noexcept { if (v) gsl_vector_free(v); }
};
using UniqueVector = std::unique_ptr<gsl_vector, VectorDeleter>;

struct MatrixDeleter {
    void operator()(gsl_matrix* m) const noexcept { if (m) gsl_matrix_free(m); }
};
using UniqueMatrix = std::unique_ptr<gsl_matrix, MatrixDeleter>;

struct PermutationDeleter {
    void operator()(gsl_permutation* p) const noexcept { if (p) gsl_permutation_free(p); }
};
using UniquePermutation = std::unique_ptr<gsl_permutation, PermutationDeleter>;

struct FftRealWorkspaceDeleter {
    void operator()(gsl_fft_real_workspace* w) const noexcept { if (w) gsl_fft_real_workspace_free(w); }
};
using UniqueFftRealWorkspace = std::unique_ptr<gsl_fft_real_workspace, FftRealWorkspaceDeleter>;

struct FftRealWavetableDeleter {
    void operator()(gsl_fft_real_wavetable* w) const noexcept { if (w) gsl_fft_real_wavetable_free(w); }
};
using UniqueFftRealWavetable = std::unique_ptr<gsl_fft_real_wavetable, FftRealWavetableDeleter>;

struct FftComplexWorkspaceDeleter {
    void operator()(gsl_fft_complex_workspace* w) const noexcept { if (w) gsl_fft_complex_workspace_free(w); }
};
using UniqueFftComplexWorkspace = std::unique_ptr<gsl_fft_complex_workspace, FftComplexWorkspaceDeleter>;

struct FftComplexWavetableDeleter {
    void operator()(gsl_fft_complex_wavetable* w) const noexcept { if (w) gsl_fft_complex_wavetable_free(w); }
};
using UniqueFftComplexWavetable = std::unique_ptr<gsl_fft_complex_wavetable, FftComplexWavetableDeleter>;

struct MultiminFminimizerDeleter {
    void operator()(gsl_multimin_fminimizer* m) const noexcept { if (m) gsl_multimin_fminimizer_free(m); }
};
using UniqueMultiminFminimizer = std::unique_ptr<gsl_multimin_fminimizer, MultiminFminimizerDeleter>;

} // namespace GslRAII

#endif // GSL_RAII_H
