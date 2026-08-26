#ifndef __GSL_TAMU_ANOVA_TWOWAY_H
#define __GSL_TAMU_ANOVA_TWOWAY_H

/*
Two way ANOVA
This package defines ANOVA for two factors, including:
	two-factor fixed effects
	two-factor random effects
	two-factor mixed model
This package was writen by:
Andrew Redd
Krista Rister
Elizabeth Young
*/


#include <gsl/gsl_cdf.h>
#include <gsl/gsl_errno.h>
#include <math.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_vector.h>
#include <stdio.h>
#include <stdlib.h>

/*This structure contains all the data that will be included in a two-way ANOVA table.*/
struct tamu_anova_table_twoway
{
  long dfA, dfB, dfAB, dfE, dfT;	//Degrees of freedom for factor A, factor B, interaction, error, and total
  double SSA, MSA, FA, pA,	//For factor a, the sum of squares, mean squares, F statistic, and p value
    SSB, MSB, FB, pB,		//For factor b, the sum of squares, mean squares, F statistic, and p value
    SSAB, MSAB, FAB, pAB,	//For interaction, the sum of squares, mean squares, F statistic, and p value
    SSE, MSE,			//For error, the sum of squares and mean squares
    SST;			//The total sum of squares
};

//Definition of three different models
enum gsl_anova_twoway_types
{
  anova_fixed,
  anova_random,
  anova_mixed
};

/*This function runs two-way ANOVA after being passed the following arguments:
 -data:   An array of doubles representing all the observations to be included in the test.
 -factor: A two-dimensional matrix of long integers representing the group that the 
          corresponding data value belongs in. For example, if factor[i][0]=1 and factor[i][1]=2,
          then data[i] belongs in the group pertaining to the first treatment level of factor A
					and the second treatment level of factor B.
 -I:      The number of data values included in the test.
 -J:      An array containing the number of treatment levels of factor A (J[0]) and factor B (J[1]).
          For every i from 1 to I, factor[i][0] must be an integer value between 1 and J[0], and
					factor[i][1] must be an integer value between 1 and J[1], both inclusive.
 -type:   Can take one of three values (anova_fixed, anova_random, or anova_mixed) and defines
          the model of the study.
*/

struct tamu_anova_table_twoway
tamu_anova_twoway (double data[], long factor[][2], long I, long J[2],
		   enum gsl_anova_twoway_types type)
{
  //Error checking/////////////////////////////////////////////////////////////////////////////
  //This for loop checks to make sure each of member of the factor array is valid.
  long k;
  for (k = 0; k < I; k++)
    {
      if ((factor[k][0] < 1 || factor[k][0] > J[0])
	  || (factor[k][1] < 1 || factor[k][1] > J[1]))
	{
	  //Cannot use GSL_ERROR since it assumes an integer return value. but ay use this in the future.
	  //GSL_ERROR("Invalid factor passed to tamu_anova_twoway. All members of factor array must be between 1 and J[0] for factor A and between 1 and J[1] for factor B, both inclusive.",GSL_EINVAL);
	  gsl_error
	    ("Invalid factor passed to tamu_anova_twoway. All members of factor array must be between 1 and J[0] for factor A and between 1 and J[1] for factor B, both inclusive.",
	     __FILE__, __LINE__, GSL_EINVAL);
	  k = I;
	};
    };

  //Declarations///////////////////////////////////////////////////////////////////////////////
  struct tamu_anova_table_twoway rtntbl;	//Table to be returned by function
  int i, j0, j1;		//We will use these to iterate over I, J[0], and J[1], respectively.
  
  long *n = (long *)malloc(J[0] * J[1] * sizeof(long));
  long *nrow = (long *)malloc(J[0] * sizeof(long));
  long *ncol = (long *)malloc(J[1] * sizeof(long));
  double *cellavg = (double *)malloc(J[0] * J[1] * sizeof(double));
  double *cellsum = (double *)malloc(J[0] * J[1] * sizeof(double));
  double *rowavg = (double *)malloc(J[0] * sizeof(double));
  double *rowsum = (double *)malloc(J[0] * sizeof(double));
  double *colavg = (double *)malloc(J[1] * sizeof(double));
  double *colsum = (double *)malloc(J[1] * sizeof(double));
  double *wa = (double *)malloc(J[0] * sizeof(double));
  double *wb = (double *)malloc(J[1] * sizeof(double));

  if (!n || !nrow || !ncol || !cellavg || !cellsum || !rowavg || !rowsum || !colavg || !colsum || !wa || !wb) {
      if (n) free(n); if (nrow) free(nrow); if (ncol) free(ncol);
      if (cellavg) free(cellavg); if (cellsum) free(cellsum);
      if (rowavg) free(rowavg); if (rowsum) free(rowsum);
      if (colavg) free(colavg); if (colsum) free(colsum);
      if (wa) free(wa); if (wb) free(wb);
      gsl_error("Out of memory in tamu_anova_twoway", __FILE__, __LINE__, GSL_ENOMEM);
      return rtntbl;
  }

  //NOTE: Below averages are not the true arithmetic mean, but rather weighted means
  double motheravg = 0;		/*Mean of all data values - used only to calculate an approximation for 
				   the interaction sum of squares when an exact value cannot be found by
				   solving a system of linear equations. */
  gsl_vector *tauvec;		//Pointer to vector tau
  gsl_matrix *cmat;		//Pointer to the coefficient matrix
  gsl_vector *rvec;		//Pointer to the r vector
  gsl_vector *rltvec;		//Pointer to a vector to hold the results

  //Initialization of values//////////////////////////////////////////////////////////////////////
  for (j0 = 0; j0 < J[0]; j0++)
    for (j1 = 0; j1 < J[1]; j1++)
      {
	n[j0 * J[1] + j1] = 0;
      };
  for (j0 = 0; j0 < J[0]; j0++)
    for (j1 = 0; j1 < J[1]; j1++)
      {
	cellsum[j0 * J[1] + j1] = 0;
	cellavg[j0 * J[1] + j1] = 0;
      };
  for (j0 = 0; j0 < J[0]; j0++)
    {
      rowavg[j0] = 0;
      rowsum[j0] = 0;
      nrow[j0] = 0;
    };
  for (j1 = 0; j1 < J[1]; j1++)
    {
      colavg[j1] = 0;
      colsum[j1] = 0;
      ncol[j1] = 0;
    };
  rtntbl.SSA = 0;
  rtntbl.SSB = 0;
  rtntbl.SSAB = 0;
  rtntbl.SSE = 0;
  rtntbl.SST = 0;

  //Calculates sums and number of data values for each group (treatment)///////////////////////////
  for (i = 0; i < I; i++)
    {
      cellsum[(factor[i][0] - 1) * J[1] + (factor[i][1] - 1)] += data[i];
      n[(factor[i][0] - 1) * J[1] + (factor[i][1] - 1)]++;
    };

  //Calculates cell averages, row counts, and column counts////////////////////////////////////////
  for (j0 = 0; j0 < J[0]; j0++)
    {
      for (j1 = 0; j1 < J[1]; j1++)
	{
	  cellavg[j0 * J[1] + j1] = cellsum[j0 * J[1] + j1] / n[j0 * J[1] + j1];
	  nrow[j0] += n[j0 * J[1] + j1];
	  ncol[j1] += n[j0 * J[1] + j1];
	};
    };

#ifdef TAMU_ANOVA_DEBUG
  //Prints out cellsum and avg.
  printf ("cellavg:\n");
  for (j0 = 0; j0 < J[0]; j0++)
    {
      for (j1 = 0; j1 < J[1]; j1++)
	{
	  printf ("%f ", cellavg[j0 * J[1] + j1]);
	};
      printf ("\n");
    };
  printf ("cellsum\n");
  for (j0 = 0; j0 < J[0]; j0++)
    {
      for (j1 = 0; j1 < J[1]; j1++)
	{
	  printf ("%f ", cellsum[j0 * J[1] + j1]);
	};
      printf ("\n");
    };
#endif

  //Calculates row averages, row sums, column averages, and column sums/////////////////////////
  for (j0 = 0; j0 < J[0]; j0++)
    for (j1 = 0; j1 < J[1]; j1++)
      {
	rowavg[j0] += cellavg[j0 * J[1] + j1];
	rowsum[j0] += cellsum[j0 * J[1] + j1];
	colavg[j1] += cellavg[j0 * J[1] + j1];
	colsum[j1] += cellsum[j0 * J[1] + j1];
      };
  for (j0 = 0; j0 < J[0]; j0++)
    {
      rowavg[j0] /= J[1];
    };
  for (j1 = 0; j1 < J[1]; j1++)
    {
      colavg[j1] /= J[0];
    };

  //Calculates average of all data//////////////////////////////////////////////////////////////
  for (j0 = 0; j0 < J[0]; j0++)
    for (j1 = 0; j1 < J[1]; j1++)
      {
	motheravg += cellavg[j0 * J[1] + j1];
      };
  motheravg /= (J[0] * J[1]);

#ifdef TAMU_ANOVA_DEBUG
  //Prints out row avg and col avg.
  printf ("rowavg:\n");
  for (j0 = 0; j0 < J[0]; j0++)
    {
      printf ("%f  ", rowavg[j0]);
    };
  printf ("\n");
  printf ("colavg:\n");
  for (j1 = 0; j1 < J[1]; j1++)
    {
      printf ("%f  ", colavg[j1]);
    };
  printf ("\n");
  printf ("rowsum:\n");
  for (j0 = 0; j0 < J[0]; j0++)
    {
      printf ("%f  ", rowsum[j0]);
    };
  printf ("\n");
  printf ("colsum:\n");
  for (j1 = 0; j1 < J[1]; j1++)
    {
      printf ("%f  ", colsum[j1]);
    };
  printf ("\n");
  printf ("rowsum/nrow:\n");
  for (j0 = 0; j0 < J[0]; j0++)
    {
      printf ("%f  ", rowsum[j0] / nrow[j0]);
    };
  printf ("\n");
  printf ("colsum/ncol:\n");
  for (j1 = 0; j1 < J[1]; j1++)
    {
      printf ("%f  ", colsum[j1] / ncol[j1]);
    };
  printf ("\n");
#endif


  //Calculates sum of squares for factor A (SSA)///////////////////////////////////////////////
  //The calculations are according to formulas found in Searle's _Analysis of Unbalanced Data_
  double sw = 0;
  double *y = (double *)malloc(J[0] * sizeof(double));
  double c = 0;

  if (!y) {
      free(n); free(nrow); free(ncol); free(cellavg); free(cellsum);
      free(rowavg); free(rowsum); free(colavg); free(colsum); free(wa); free(wb);
      gsl_error("Out of memory in tamu_anova_twoway", __FILE__, __LINE__, GSL_ENOMEM);
      return rtntbl;
  }

  for (j0 = 0; j0 < J[0]; j0++)
    {
      wa[j0] = 0;
      y[j0] = 0;
      for (j1 = 0; j1 < J[1]; j1++)
	{
	  wa[j0] += 1.0 / (double) n[j0 * J[1] + j1];
	  y[j0] += cellavg[j0 * J[1] + j1];
	};
      wa[j0] = J[1] * J[1] / wa[j0];
      sw += wa[j0];
      y[j0] /= J[1];
    };

  for (j0 = 0; j0 < J[0]; j0++)
    {
      c += wa[j0] * y[j0] / sw;
    };
  for (j0 = 0; j0 < J[0]; j0++)
    {
      rtntbl.SSA += wa[j0] * pow (y[j0] - c, 2);
    };

  //Calculates sum of squares for factor B (SSB)///////////////////////////////////////////////////
  c = 0;
  sw = 0;
  double *yb = (double *)malloc(J[1] * sizeof(double));

  if (!yb) {
      free(n); free(nrow); free(ncol); free(cellavg); free(cellsum);
      free(rowavg); free(rowsum); free(colavg); free(colsum); free(wa); free(wb);
      free(y);
      gsl_error("Out of memory in tamu_anova_twoway", __FILE__, __LINE__, GSL_ENOMEM);
      return rtntbl;
  }

  for (j1 = 0; j1 < J[1]; j1++)
    {				//finds wi and y~_i..
      wb[j1] = 0;
      yb[j1] = 0;
      for (j0 = 0; j0 < J[0]; j0++)
	{
	  wb[j1] += 1.0 / (double) n[j0 * J[1] + j1];
	  yb[j1] += cellavg[j0 * J[1] + j1];
	};
      wb[j1] = J[0] * J[0] / wb[j1];
      sw += wb[j1];
      yb[j1] /= J[0];
    };

  for (j1 = 0; j1 < J[1]; j1++)
    {
      c += wb[j1] * yb[j1] / sw;
    };
  for (j1 = 0; j1 < J[1]; j1++)
    {
      rtntbl.SSB += wb[j1] * pow (yb[j1] - c, 2);
    };

  //Calculates sum of squares for interaction (SSAB)///////////////////////////////////////////////////
  //Exact calculations, using a system of linear equations and GSL vectors/matrices////////////////////
  rvec = gsl_vector_alloc (J[1] - 1);
  tauvec = gsl_vector_alloc (J[1] - 1);
  cmat = gsl_matrix_alloc (J[1] - 1, J[1] - 1);

  for (j1 = 0; j1 < (J[1] - 1); j1++)
    {
      double x = 0;
      for (j0 = 0; j0 < J[0]; j0++)
	{
	  x += cellsum[j0 * J[1] + j1] - n[j0 * J[1] + j1] * rowsum[j0] / nrow[j0];	//rowavg[j0];
	};
      gsl_vector_set (rvec, j1, x);
    };
#ifdef TAMU_ANOVA_DEBUG
  printf ("rvec:\n");
  for (j1 = 0; j1 < J[1] - 1; j1++)
    {
      printf ("%f\n", gsl_vector_get (rvec, j1));
    };
#endif

  for (j0 = 0; j0 < J[1] - 1; j0++)
    for (j1 = 0; j1 < J[1] - 1; j1++)
      {
	double x = 0;
	if (j0 == j1)
	  {
	    for (i = 0; i < J[0]; i++)
	      {
		x += pow ((double) n[i * J[1] + j1], 2) / (double) nrow[i];
	      };
	    gsl_matrix_set (cmat, j0, j1, (double) ncol[j1] - x);
	  }
	else
	  {
	    for (i = 0; i < J[0]; i++)
	      {
		x += (double) n[i * J[1] + j0] * (double) n[i * J[1] + j1] / (double) nrow[i];
	      };
	    gsl_matrix_set (cmat, j0, j1, -x);
	  };
      };

#ifdef TAMU_ANOVA_DEBUG
  printf ("cmat:\n");
  for (j0 = 0; j0 < J[1] - 1; j0++)
    {
      for (j1 = 0; j1 < J[1] - 1; j1++)
	{
	  printf ("%f  ", gsl_matrix_get (cmat, j0, j1));
	};
      printf ("\n");
    };
#endif

  //Factor C matrix for use in QR.
  int QR_rslt;
  rltvec = gsl_vector_alloc (J[1] - 1);
  QR_rslt = gsl_linalg_QR_decomp (cmat, rltvec);

  if (QR_rslt == 0)
    {
      QR_rslt = gsl_linalg_QR_solve (cmat, rltvec, rvec, tauvec);
#ifdef TAMU_ANOVA_DEBUG
      printf ("tauvec\n");
      for (j1 = 0; j1 < J[1] - 1; j1++)
	{
	  printf ("%f\n", gsl_vector_get (tauvec, j1));
	};
#endif

      //Finally we compute SSAB
      double R = 0;
      for (j0 = 0; j0 < J[0]; j0++)
	for (j1 = 0; j1 < J[1]; j1++)
	  {
	    rtntbl.SSAB += n[j0 * J[1] + j1] * pow (cellavg[j0 * J[1] + j1], 2);
	  };
      for (j0 = 0; j0 < J[0]; j0++)
	{
	  R += pow (rowsum[j0], 2) / nrow[j0];
	};
      for (j1 = 0; j1 < J[1] - 1; j1++)
	{
	  R += gsl_vector_get (tauvec, j1) * gsl_vector_get (rvec, j1);
	};
#ifdef TAMU_ANOVA_DEBUG
      printf ("R:  %f\n", R);
#endif
      rtntbl.SSAB -= R;
    }
  else
    {
      gsl_error
	("Unable to decompose matrix.  Approximate SS interaction being used.",
	 __FILE__, __LINE__, GSL_EFACTOR);
      //Approximating formula to be used if matrix cannot be decomposed
      for (j0 = 0; j0 < J[0]; j0++)
	for (j1 = 0; j1 < J[1]; j1++)
	  {
	    rtntbl.SSAB +=
	      n[j0 * J[1] + j1] * pow (cellavg[j0 * J[1] + j1] - rowavg[j0] - colavg[j1] +
			       motheravg, 2.0);
	  };

    };

  //Cleanup for vectors/////////////////////////////////////////////////////////////////////////////
  gsl_vector_free (rvec);
  gsl_vector_free (tauvec);
  gsl_matrix_free (cmat);
  gsl_vector_free (rltvec);

  //Calculates the error sum of squares (SSE)///////////////////////////////////////////////////////
  for (i = 0; i < I; i++)
    {
      rtntbl.SSE += pow (data[i], 2);
    };
  for (j0 = 0; j0 < J[0]; j0++)
    for (j1 = 0; j1 < J[1]; j1++)
      {
	rtntbl.SSE -= pow (cellavg[j0 * J[1] + j1], 2) * n[j0 * J[1] + j1];
      };

  //Calculates the total sum of squares (SST)///////////////////////////////////////////////////////
  for (i = 0; i < I; i++)
    {
      rtntbl.SST += (data[i]);
    };
  rtntbl.SST = -pow (rtntbl.SST, 2) / I;
  for (j0 = 0; j0 < J[0]; j0++)
    for (j1 = 0; j1 < J[1]; j1++)
      {
	rtntbl.SST += pow (cellavg[j0 * J[1] + j1], 2) * n[j0 * J[1] + j1];
      };
  rtntbl.SST += rtntbl.SSE;

  //Calculates the degrees of freedom////////////////////////////////////////////////////////////////
  rtntbl.dfA = J[0] - 1;
  rtntbl.dfB = J[1] - 1;
  rtntbl.dfT = I - 1;
  rtntbl.dfAB = (J[0] - 1) * (J[1] - 1);
  rtntbl.dfE = I - J[0] * J[1];

  //Calculates the mean squares/////////////////////////////////////////////////////////////////////
  rtntbl.MSA = rtntbl.SSA / rtntbl.dfA;
  rtntbl.MSB = rtntbl.SSB / rtntbl.dfB;
  rtntbl.MSAB = rtntbl.SSAB / rtntbl.dfAB;
  rtntbl.MSE = rtntbl.SSE / rtntbl.dfE;


  //Calculates the F statistic for each of the three different model types////////////////////////////
  switch (type)
    {
    case anova_fixed:
      rtntbl.FA = rtntbl.MSA / rtntbl.MSE;
      rtntbl.FB = rtntbl.MSB / rtntbl.MSE;
      rtntbl.FAB = rtntbl.MSAB / rtntbl.MSE;
      rtntbl.pA =
	gsl_cdf_fdist_Q (rtntbl.FA, (double) rtntbl.dfA, (double) rtntbl.dfE);
      rtntbl.pB =
	gsl_cdf_fdist_Q (rtntbl.FB, (double) rtntbl.dfB, (double) rtntbl.dfE);
      rtntbl.pAB =
	gsl_cdf_fdist_Q (rtntbl.FAB, (double) rtntbl.dfAB,
			 (double) rtntbl.dfE);
      break;
    case anova_random:
      rtntbl.FA = rtntbl.MSA / rtntbl.MSAB;
      rtntbl.FB = rtntbl.MSB / rtntbl.MSAB;
      rtntbl.FAB = rtntbl.MSAB / rtntbl.MSE;
      rtntbl.pA =
	1.0 - gsl_cdf_fdist_P (rtntbl.FA, (double) rtntbl.dfA,
			       (double) rtntbl.dfAB);
      rtntbl.pB =
	1.0 - gsl_cdf_fdist_P (rtntbl.FB, (double) rtntbl.dfB,
			       (double) rtntbl.dfAB);
      rtntbl.pAB =
	1.0 - gsl_cdf_fdist_P (rtntbl.FAB, (double) rtntbl.dfAB,
			       (double) rtntbl.dfE);
      break;
    case anova_mixed:
      rtntbl.FA = rtntbl.MSA / rtntbl.MSAB;
      rtntbl.FB = rtntbl.MSB / rtntbl.MSE;
      rtntbl.FAB = rtntbl.MSAB / rtntbl.MSE;
      rtntbl.pA =
	1.0 - gsl_cdf_fdist_P (rtntbl.FA, (double) rtntbl.dfA,
			       (double) rtntbl.dfAB);
      rtntbl.pB =
	1.0 - gsl_cdf_fdist_P (rtntbl.FB, (double) rtntbl.dfB,
			       (double) rtntbl.dfE);
      rtntbl.pAB =
	1.0 - gsl_cdf_fdist_P (rtntbl.FAB, (double) rtntbl.dfAB,
			       (double) rtntbl.dfE);
      break;
    }

  free(n); free(nrow); free(ncol); free(cellavg); free(cellsum);
  free(rowavg); free(rowsum); free(colavg); free(colsum); free(wa); free(wb);
  free(y); free(yb);

  //Returns entire table
  return rtntbl;
};

/*This function prints a formatted version of the tamu_anova_table_twoway passed to it.*/
void
tamu_anova_printtable_twoway (struct tamu_anova_table_twoway t)
{
  printf ("results from anova twoway test on data\n");
  printf ("source  \tdf\tSS\t\tMS\t\tF \t\tP\n");
  printf ("Factor-A\t%ld\t%f\t%f\t%f\t%f\n", t.dfA, t.SSA, t.MSA, t.FA, t.pA);
  printf ("Factor-B\t%ld\t%f\t%f\t%f\t%f\n", t.dfB, t.SSB, t.MSB, t.FB, t.pB);
  printf ("Interact\t%ld\t%f\t%f\t%f\t%f\n", t.dfAB, t.SSAB, t.MSAB, t.FAB,
	  t.pAB);
  printf ("Error   \t%ld\t%f\t%f\t\n", t.dfE, t.SSE, t.MSE);
  printf ("Total   \t%ld\t%f\n", t.dfT, t.SST);
}

#endif
