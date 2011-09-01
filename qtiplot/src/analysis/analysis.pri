###############################################################
################# Module: Analysis ##############################
###############################################################

INCLUDEPATH += src/analysis/
INCLUDEPATH += src/analysis/dialogs/

	HEADERS += src/analysis/ChiSquareTest.h \
			   src/analysis/Convolution.h \
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
			   src/analysis/ShapiroWilkTest.h \
			   src/analysis/SigmoidalFit.h \
			   src/analysis/SmoothFilter.h \
			   src/analysis/Statistics.h \
			   src/analysis/StatisticTest.h \
			   src/analysis/tTest.h \
			   src/analysis/fft2D.h \
			   src/analysis/fit_gsl.h \

	SOURCES += src/analysis/ChiSquareTest.cpp \
			   src/analysis/Convolution.cpp \
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
			   src/analysis/ShapiroWilkTest.cpp \
			   src/analysis/SigmoidalFit.cpp \
			   src/analysis/SmoothFilter.cpp \
			   src/analysis/Statistics.cpp \
			   src/analysis/StatisticTest.cpp \
			   src/analysis/tTest.cpp \
			   src/analysis/fft2D.cpp \
			   src/analysis/fit_gsl.cpp \

	HEADERS += src/analysis/dialogs/AnovaDialog.h \
			   src/analysis/dialogs/BaselineDialog.h \
			   src/analysis/dialogs/ExpDecayDialog.h \
			   src/analysis/dialogs/FFTDialog.h \
			   src/analysis/dialogs/FitDialog.h \
			   src/analysis/dialogs/FilterDialog.h \
			   src/analysis/dialogs/FrequencyCountDialog.h \
			   src/analysis/dialogs/IntDialog.h \
			   src/analysis/dialogs/IntegrationDialog.h \
			   src/analysis/dialogs/InterpolationDialog.h \
			   src/analysis/dialogs/PolynomFitDialog.h \
			   src/analysis/dialogs/SmoothCurveDialog.h \
			   src/analysis/dialogs/SubtractDataDialog.h \
			   src/analysis/dialogs/StudentTestDialog.h \

	SOURCES += src/analysis/dialogs/AnovaDialog.cpp \
			   src/analysis/dialogs/BaselineDialog.cpp \
			   src/analysis/dialogs/ExpDecayDialog.cpp \
			   src/analysis/dialogs/FFTDialog.cpp \
			   src/analysis/dialogs/FitDialog.cpp \
			   src/analysis/dialogs/FilterDialog.cpp \
			   src/analysis/dialogs/FrequencyCountDialog.cpp \
			   src/analysis/dialogs/IntDialog.cpp \
			   src/analysis/dialogs/IntegrationDialog.cpp \
			   src/analysis/dialogs/InterpolationDialog.cpp \
			   src/analysis/dialogs/PolynomFitDialog.cpp \
			   src/analysis/dialogs/SmoothCurveDialog.cpp \
			   src/analysis/dialogs/SubtractDataDialog.cpp \
			   src/analysis/dialogs/StudentTestDialog.cpp \

!isEmpty(ALGLIB_LIBS) {
	HEADERS += src/analysis/dialogs/GriddingDialog.h
	SOURCES += src/analysis/dialogs/GriddingDialog.cpp
}

!isEmpty(TAMUANOVA_LIBS) {
	HEADERS += src/analysis/Anova.h
	SOURCES += src/analysis/Anova.cpp
}
