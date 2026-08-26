#ifndef __TAMU_ANOVA_H
#define __TAMU_ANOVA_H

/*
One way ANOVA
This package defines ANOVA for one factor.
This package was written by:
Andrew Redd
Krista Rister
Elizabeth Young
*/ 
  
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_errno.h>
#include <stdio.h>		//This is required for the tamu_anova_printtable function.   
#include <stdlib.h>
  
/*This structure contains all the data that will be included in a one-way ANOVA table.*/ 
  struct tamu_anova_table
{
  
long dfTr, 
dfE, 
dfT;	//Degrees of freedom for the treatment, error, and total
  double SSTr, 
SSE, 
SST;	//Sum of squares for the treatment, error, and total
  double MSTr, 
MSE;		//Mean squares for the treatment and error
  double F;			//F test statistic corresponding to the test
  double p;			//p value (upper tail probability) corresponding with the F statistic
};


/*This function runs one-way ANOVA after being passed the following arguments:
 -data:   An array of doubles representing all the observations to be included in the test.
 -factor: An array of long integers representing the group that the corresponding data value
          belongs in. For example, if factor[i]=1, then data[i] belongs in the first group or
					first treatment level.
 -I:      The number of data values included in the test.
 -J:      The number of groups or levels of the treatment. Each member of the factor array
          must be an integer value between 1 and J, inclusive.

*/ 
  struct tamu_anova_table
tamu_anova (double data[], long factor[], long I, long J) 
{
  
    //Error checking//////////////////////////////////////////////////////////////////////////////
    //This for loop checks to make sure each member of the factor array is valid.
  int k;
  
for (k = 0; k < I; k++)
    {
      
if (factor[k] < 1 || factor[k] > J)
	{
	  
	    //Cannot use GSL_ERROR since it assumes an integer return value which our fuction does not do
	    //but it may do it in the future.
	    //GSL_ERROR("Invalid factor passed to tamu_anova. All members of factor array must be between 1 and J, inclusive.", 4444);
	    gsl_error
	    ("Invalid factor passed to tamu_anova. All members of factor array must be between 1 and J, inclusive.",
	     __FILE__, __LINE__, GSL_EINVAL);
	  k = I;
	};
    };
  
    //Declarations////////////////////////////////////////////////////////////////////////////////
  struct tamu_anova_table rtntbl;	//Table to be returned by function
  long i, j;			//We will use i and j as iterators to iterate over I and J respectively.
  double grandsum = 0;		//Sum of all I data values
  double cf;			//Correction factor used for calculating sum of squares
  long *n = (long *)malloc(J * sizeof(long));			//The n array holds the number of data values belonging to each of the J groups or treatement levels.
  double *subsum = (double *)malloc(J * sizeof(double));		//The subsum array is used to hold the sum of the data in each of the J groups.

  if (!n || !subsum) {
      if (n) free(n);
      if (subsum) free(subsum);
      gsl_error("Out of memory in tamu_anova", __FILE__, __LINE__, GSL_ENOMEM);
      return rtntbl; 
  }
  
    //Computes the grandsum, the sum of all I data values//////////////////////////////////////////
    for (i = 0; i < I; i++)
    {
      grandsum += data[i];
    };
  
    //Calculates the correction factor cf using the grandsum///////////////////////////////////////
    cf = grandsum * grandsum / I;
  
    //Computes sum of squares for the treatment (SSTr)/////////////////////////////////////////////
    //First, calculate the sum of the data and find the number of data in each group///////////////////
    for (j = 0; j < J; j++)
    {
      subsum[j] = 0;
      n[j] = 0;
    };				//Initialize subsum to zero for all j.
  
#ifdef TAMU_ANOVA_DEBUG
    printf ("DEBUG INFO:: contents of submean and n arrays:\n");
  for (j = 0; j < J; j++)
    {
      printf ("%ld  %f\n", n[j], subsum[j]);
    };
  
#endif	/* tamu_anova_debug */
    for (i = 0; i < I; i++)
    {
      subsum[factor[i] - 1] += data[i];
      n[factor[i] - 1]++;	//This increments the number of observations in the group determined by factor[i].
    };
  
    //Now, using subsum and n arrays and correction factor cf, calculates SSTr/////////////////////
    rtntbl.SSTr = 0;		//Initializes SSTr to 0
  for (j = 0; j < J; j++)
    {
      rtntbl.SSTr += subsum[j] * subsum[j] / n[j];
    };
  rtntbl.SSTr -= cf;
  
#ifdef TAMU_ANOVA_DEBUG
    printf ("DEBUG INFO:: contents of submean and n arrays:\n");
  for (j = 0; j < J; j++)
    {
      printf ("%ld  %f\n", n[j], subsum[j]);
    };
  
#endif	/* tamu_anova_debug */
    
    //Computes the sum of squares for the error (SSE) and the total sum of squares (SST)///////////
    rtntbl.SSE = 0;		//Initializes SSE to 0
  rtntbl.SST = 0;		//Initializes SST to 0
  for (i = 0; i < I; i++)
    
    {
      rtntbl.SST += (data[i]) * (data[i]);
    };
  rtntbl.SST -= cf;
  rtntbl.SSE = rtntbl.SST - rtntbl.SSTr;	//Uses SST and SSTr to calculate SSE
  
    //Computes the degrees of freedom
    rtntbl.dfTr = J - 1;
  rtntbl.dfE = I - J;
  rtntbl.dfT = I - 1;
  
    //Computes the mean squares (MSTr and MSE)////////////////////////////////////////////////////
    rtntbl.MSTr = rtntbl.SSTr / rtntbl.dfTr;
  rtntbl.MSE = rtntbl.SSE / rtntbl.dfE;
  
    //Computes the F test statistic///////////////////////////////////////////////////////////////
    rtntbl.F = rtntbl.MSTr / rtntbl.MSE;
  
    //Computes 'p'-value using F cdf from GSL/////////////////////////////////////////////////////
#ifdef TAMU_ANOVA_DEBUG
    printf ("rtntbl.dfTr as double: %f\nrtntbl.dfE as double: %f\n",
	    (double) rtntbl.dfTr, (double) rtntbl.dfE);
  
#endif	/* tamu_anova_debug */
    rtntbl.p =
    gsl_cdf_fdist_Q (rtntbl.F, (double) rtntbl.dfTr, (double) rtntbl.dfE);
  
    free(n);
    free(subsum);

    //Return entire table
    return rtntbl;
};


/*This function prints a formatted version of the tamu_anova_table passed to it.*/ 
  void
tamu_anova_printtable (struct tamu_anova_table t)
{
  
printf ("source     d.f.  SS          MS           F           P\n");
  
printf ("treatments %4li  %10f %10f %10f %10f\n", t.dfTr, t.SSTr, t.MSTr,
	   t.F, t.p);
  
printf ("error      %4li  %10f %10f\n", t.dfE, t.SSE, t.MSE);
  
printf ("total      %4li  %10f\n", t.dfT, t.SST);

};



#endif	/*__TAMU_ANOVA_H*/
