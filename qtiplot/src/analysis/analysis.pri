###############################################################
################# Module: Analysis ##############################
###############################################################

INCLUDEPATH += src/analysis/
INCLUDEPATH += src/analysis/dialogs/

    HEADERS += src/analysis/Convolution.h \
			   src/analysis/Correlation.h \
			   src/analysis/Differentiation.h \
			   src/analysis/ExponentialFit.h \
			   src/analysis/FFTFilter.h \
			   src/analysis/FFT.h \
			   src/analysis/Filter.h \
			   src/analysis/Fit.h \
			   src/analysis/FitModelHandler.h \
			   src/analysis/Integration.h \
			   src/analysis/Interpolation.h \
			   src/analysis/LogisticFit.h \
			   src/analysis/MultiPeakFit.h \
			   src/analysis/NonLinearFit.h \
			   src/analysis/PluginFit.h \
			   src/analysis/PolynomialFit.h \
			   src/analysis/SigmoidalFit.h \
			   src/analysis/SmoothFilter.h \
			   src/analysis/fft2D.h \
			   src/analysis/fit_gsl.h \
			   src/analysis/nrutil.h \

	SOURCES += src/analysis/Convolution.cpp \
			   src/analysis/Correlation.cpp \
			   src/analysis/Differentiation.cpp \
			   src/analysis/ExponentialFit.cpp \
			   src/analysis/FFTFilter.cpp \
			   src/analysis/FFT.cpp \
			   src/analysis/Filter.cpp \
			   src/analysis/Fit.cpp \
			   src/analysis/FitModelHandler.cpp \
			   src/analysis/Integration.cpp \
			   src/analysis/Interpolation.cpp \
			   src/analysis/LogisticFit.cpp \
			   src/analysis/MultiPeakFit.cpp \
			   src/analysis/NonLinearFit.cpp \
			   src/analysis/PluginFit.cpp \
			   src/analysis/PolynomialFit.cpp \
			   src/analysis/SigmoidalFit.cpp \
			   src/analysis/SmoothFilter.cpp \
			   src/analysis/fft2D.cpp \
				src/analysis/fit_gsl.cpp \
			   src/analysis/nrutil.cpp \

    HEADERS += src/analysis/dialogs/ExpDecayDialog.h \
			   src/analysis/dialogs/FFTDialog.h \
			   src/analysis/dialogs/FitDialog.h \
			   src/analysis/dialogs/FilterDialog.h \
			   src/analysis/dialogs/FrequencyCountDialog.h \
			   src/analysis/dialogs/IntDialog.h \
			   src/analysis/dialogs/InterpolationDialog.h \
			   src/analysis/dialogs/PolynomFitDialog.h \
			   src/analysis/dialogs/SmoothCurveDialog.h \
			   src/analysis/dialogs/SubtractDataDialog.h \

	SOURCES += src/analysis/dialogs/ExpDecayDialog.cpp \
			   src/analysis/dialogs/FFTDialog.cpp \
			   src/analysis/dialogs/FitDialog.cpp \
			   src/analysis/dialogs/FilterDialog.cpp \
			   src/analysis/dialogs/FrequencyCountDialog.cpp \
			   src/analysis/dialogs/IntDialog.cpp \
			   src/analysis/dialogs/InterpolationDialog.cpp \
			   src/analysis/dialogs/PolynomFitDialog.cpp \
			   src/analysis/dialogs/SmoothCurveDialog.cpp \
			   src/analysis/dialogs/SubtractDataDialog.cpp \
