/***************************************************************************
   File                 : fit_gsl.h
   Project              : QtiPlot
   Description          : Built-in data fit models for QtiPlot
   --------------------------------------------------------------------
   Copyright            : (C) 2004-2010 Ion Vasilief (ion_vasilief*yahoo.fr)
						  (replace * with @ in the email address)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  As a special exception, you may omit the above copyright notice when   *
 *  distributing modified copies of this file (for instance, when using it *
 *  as a template for your own fit plugin).                                *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#ifndef FIT_GSL_H
#define FIT_GSL_H

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>

class Fit;
	
//! Structure for fitting data
struct FitData {
  int n;// number of points to be fitted (size of X, Y and sigma arrays)
  int p;// number of fit parameters
  double * X;// the data to be fitted (abscissae) 
  double * Y; // the data to be fitted (ordinates)
  double * sigma; // the weighting data
  Fit *fitter; //pointer to the fitter object (used only for the NonLinearFit class)
};

int expd3_fdf (const gsl_vector * x, void *params, gsl_vector * f, gsl_matrix * J);
int expd3_df (const gsl_vector * x, void *params, gsl_matrix * J);
int expd3_f (const gsl_vector * x, void *params, gsl_vector * f);
double expd3_d (const gsl_vector * x, void *params);

int expd2_fdf (const gsl_vector * x, void *params, gsl_vector * f, gsl_matrix * J);
int expd2_df (const gsl_vector * x, void *params, gsl_matrix * J);
int expd2_f (const gsl_vector * x, void *params, gsl_vector * f);
double expd2_d (const gsl_vector * x, void *params);

int exp_fdf (const gsl_vector * x, void *params, gsl_vector * f, gsl_matrix * J);
int exp_df (const gsl_vector * x, void *params, gsl_matrix * J);
int exp_f (const gsl_vector * x, void *params, gsl_vector * f);
double exp_d (const gsl_vector * x, void *params);

int boltzmann_fdf (const gsl_vector * x, void *params, gsl_vector * f, gsl_matrix * J);
int boltzmann_df (const gsl_vector * x, void *params, gsl_matrix * J);
int boltzmann_f (const gsl_vector * x, void *params, gsl_vector * f);
double boltzmann_d (const gsl_vector * x, void *params);

int logistic_fdf (const gsl_vector * x, void *params, gsl_vector * f, gsl_matrix * J);
int logistic_df (const gsl_vector * x, void *params, gsl_matrix * J);
int logistic_f (const gsl_vector * x, void *params, gsl_vector * f);
double logistic_d (const gsl_vector * x, void *params);

int gauss_fdf (const gsl_vector * x, void *params, gsl_vector * f, gsl_matrix * J);
int gauss_df (const gsl_vector * x, void *params, gsl_matrix * J);
int gauss_f (const gsl_vector * x, void *params,gsl_vector * f);
double gauss_d (const gsl_vector * x, void *params);

int gauss_multi_peak_f (const gsl_vector * x, void *params, gsl_vector * f);
double gauss_multi_peak_d (const gsl_vector * x, void *params);
int gauss_multi_peak_df (const gsl_vector * x, void *params, gsl_matrix * J);
int gauss_multi_peak_fdf (const gsl_vector * x, void *params, gsl_vector * f, gsl_matrix * J);

int lorentz_multi_peak_f (const gsl_vector * x, void *params, gsl_vector * f);
double lorentz_multi_peak_d (const gsl_vector * x, void *params);
int lorentz_multi_peak_df (const gsl_vector * x, void *params, gsl_matrix * J);
int lorentz_multi_peak_fdf (const gsl_vector * x, void *params, gsl_vector * f, gsl_matrix * J);

int user_f(const gsl_vector * x, void *params, gsl_vector * f);
double user_d(const gsl_vector * x, void *params);
int user_df(const gsl_vector * x, void *params,gsl_matrix * J);
int user_fdf(const gsl_vector * x, void *params,gsl_vector * f, gsl_matrix * J);

#endif
