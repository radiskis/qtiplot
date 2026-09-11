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
#include <gsl/gsl_histogram.h>
#include <gsl/gsl_histogram2d.h>
#include <gsl/gsl_spline.h>
#include <gsl/gsl_multifit.h>
#include <gsl/gsl_multifit_nlin.h>
#include <gsl/gsl_integration.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_errno.h>

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

struct FftHalfcomplexWavetableDeleter {
    void operator()(gsl_fft_halfcomplex_wavetable* w) const noexcept { if (w) gsl_fft_halfcomplex_wavetable_free(w); }
};
using UniqueFftHalfcomplexWavetable = std::unique_ptr<gsl_fft_halfcomplex_wavetable, FftHalfcomplexWavetableDeleter>;

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

struct HistogramDeleter {
    void operator()(gsl_histogram* h) const noexcept { if (h) gsl_histogram_free(h); }
};
using UniqueHistogram = std::unique_ptr<gsl_histogram, HistogramDeleter>;

struct Histogram2DDeleter {
    void operator()(gsl_histogram2d* h) const noexcept { if (h) gsl_histogram2d_free(h); }
};
using UniqueHistogram2D = std::unique_ptr<gsl_histogram2d, Histogram2DDeleter>;

struct InterpAccelDeleter {
    void operator()(gsl_interp_accel* a) const noexcept { if (a) gsl_interp_accel_free(a); }
};
using UniqueInterpAccel = std::unique_ptr<gsl_interp_accel, InterpAccelDeleter>;

struct SplineDeleter {
    void operator()(gsl_spline* s) const noexcept { if (s) gsl_spline_free(s); }
};
using UniqueSpline = std::unique_ptr<gsl_spline, SplineDeleter>;

struct MultifitLinearWorkspaceDeleter {
    void operator()(gsl_multifit_linear_workspace* w) const noexcept { if (w) gsl_multifit_linear_free(w); }
};
using UniqueMultifitLinearWorkspace = std::unique_ptr<gsl_multifit_linear_workspace, MultifitLinearWorkspaceDeleter>;

struct MultifitFdfSolverDeleter {
    void operator()(gsl_multifit_fdfsolver* s) const noexcept { if (s) gsl_multifit_fdfsolver_free(s); }
};
using UniqueMultifitFdfSolver = std::unique_ptr<gsl_multifit_fdfsolver, MultifitFdfSolverDeleter>;

struct IntegrationWorkspaceDeleter {
    void operator()(gsl_integration_workspace* w) const noexcept { if (w) gsl_integration_workspace_free(w); }
};
using UniqueIntegrationWorkspace = std::unique_ptr<gsl_integration_workspace, IntegrationWorkspaceDeleter>;

struct RngDeleter {
    void operator()(gsl_rng* r) const noexcept { if (r) gsl_rng_free(r); }
};
using UniqueRng = std::unique_ptr<gsl_rng, RngDeleter>;

// RAII guard to temporarily override or disable GSL error handling and restore previous handler on exit
class ScopedErrorHandler {
public:
    explicit ScopedErrorHandler(gsl_error_handler_t *new_handler = nullptr)
        : d_prev_handler(gsl_set_error_handler(new_handler)) {}
    ~ScopedErrorHandler() {
        gsl_set_error_handler(d_prev_handler);
    }
    ScopedErrorHandler(const ScopedErrorHandler &) = delete;
    ScopedErrorHandler &operator=(const ScopedErrorHandler &) = delete;
private:
    gsl_error_handler_t *d_prev_handler;
};

// Factory functions
inline UniqueVector make_vector(size_t n) {
    return UniqueVector(gsl_vector_alloc(n));
}

inline UniqueMatrix make_matrix(size_t n1, size_t n2) {
    return UniqueMatrix(gsl_matrix_alloc(n1, n2));
}

inline UniquePermutation make_permutation(size_t n) {
    return UniquePermutation(gsl_permutation_alloc(n));
}

inline UniqueHistogram make_histogram(size_t n) {
    return UniqueHistogram(gsl_histogram_alloc(n));
}

inline UniqueHistogram2D make_histogram2d(size_t nx, size_t ny) {
    return UniqueHistogram2D(gsl_histogram2d_alloc(nx, ny));
}

inline UniqueInterpAccel make_interp_accel() {
    return UniqueInterpAccel(gsl_interp_accel_alloc());
}

inline UniqueSpline make_spline(const gsl_interp_type *type, size_t size) {
    return UniqueSpline(gsl_spline_alloc(type, size));
}

inline UniqueMultifitLinearWorkspace make_multifit_linear_workspace(size_t n, size_t p) {
    return UniqueMultifitLinearWorkspace(gsl_multifit_linear_alloc(n, p));
}

inline UniqueIntegrationWorkspace make_integration_workspace(size_t n) {
    return UniqueIntegrationWorkspace(gsl_integration_workspace_alloc(n));
}

inline UniqueRng make_rng(const gsl_rng_type *type = gsl_rng_default) {
    return UniqueRng(gsl_rng_alloc(type));
}

} // namespace GslRAII

#endif // GSL_RAII_H
