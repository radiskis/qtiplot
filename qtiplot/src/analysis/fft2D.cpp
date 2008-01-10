#include "fft2D.h"
#include <math.h>

double** allocateMatrixData(int rows, int columns)
{
	double** data = new double* [rows];
	for ( int i = 0; i < rows; ++i)
		data[i] = new double [columns];

	return data;
}

void freeMatrixData(double **data, int rows)
{
	for ( int i = 0; i < rows; i++)
		delete [] data[i];

	delete [] data;
}

void fft(double* x_int_re, double* x_int_im, unsigned int taille)
{
    unsigned int size_2=taille>>1,tmp1;
    double tmp, tmpcos, tmpsin, base = 2*M_PI/taille;
    const double SQ_2=sqrt(2);
    double pair_re[size_2], pair_im[size_2], impair_re[size_2], impair_im[size_2];
    for(int i=0; i<size_2; i++){
        tmp1=(i<<1);
        pair_re[i]=x_int_re[tmp1];
        pair_im[i]=x_int_im[tmp1];
        impair_re[i]=x_int_re[tmp1+1];
        impair_im[i]=x_int_im[tmp1+1];
    }

    if(taille>2){
        fft(pair_re,pair_im,size_2);
        fft(impair_re,impair_im,size_2);
    }

    for(int i=0; i<size_2; i++){
        tmp=base*i;
        tmpcos=cos(tmp);
        tmpsin=sin(tmp);
        x_int_re[i]=(pair_re[i]+impair_re[i]*tmpcos+impair_im[i]*tmpsin)/SQ_2;
        x_int_im[i]=(pair_im[i]+impair_im[i]*tmpcos-impair_re[i]*tmpsin)/SQ_2;
        x_int_re[i+size_2]=(pair_re[i]-impair_re[i]*tmpcos-impair_im[i]*tmpsin)/SQ_2;
        x_int_im[i+size_2]=(pair_im[i]-impair_im[i]*tmpcos+impair_re[i]*tmpsin)/SQ_2;
    }
}

void fft_inv(double* x_int_re, double* x_int_im, unsigned int taille)
{
    unsigned int size_2=taille>>1, tmp1;
    double tmp, tmpcos, tmpsin, base=2*M_PI/taille;
    const double SQ_2=sqrt(2);
    double pair_re[size_2], pair_im[size_2], impair_re[size_2], impair_im[size_2];
    for(int i=0; i<size_2; i++){
        tmp1=i<<1;
        pair_re[i]=x_int_re[tmp1];
        pair_im[i]=x_int_im[tmp1];
        impair_re[i]=x_int_re[tmp1+1];
        impair_im[i]=x_int_im[tmp1+1];
    }

    if(taille>2){
        fft_inv(pair_re, pair_im,size_2);
        fft_inv(impair_re, impair_im,size_2);
    }

    for(int i=0; i<size_2; i++){
        tmp=base*i;
        tmpcos=cos(tmp);
        tmpsin=sin(tmp);
        x_int_re[i]=(pair_re[i]+impair_re[i]*tmpcos-impair_im[i]*tmpsin)/SQ_2;
        x_int_im[i]=(pair_im[i]+impair_im[i]*tmpcos+impair_re[i]*tmpsin)/SQ_2;
        x_int_re[i+size_2]=(pair_re[i]-impair_re[i]*tmpcos+impair_im[i]*tmpsin)/SQ_2;
        x_int_im[i+size_2]=(pair_im[i]-impair_im[i]*tmpcos-impair_re[i]*tmpsin)/SQ_2;
    }
}

void fft2d(double **xtre, double **xtim, unsigned int width, unsigned int height)
{
    double **xint_re = allocateMatrixData(height, width);
    double **xint_im = allocateMatrixData(height, width);
    double x_int_l[width], x_int2_l[width], x_int_c[height], x_int2_c[height];
    for(int k=0; k<height; k++){
        for(int j=0; j<width; j++){
            //x_int_l[j] = x[k][j];
            //x_int2_l[j] = 0.0;

            x_int_l[j] = xtre[k][j];
            x_int2_l[j] = xtim[k][j];
        }
        fft(x_int_l, x_int2_l, width);
        for(int j=0; j<width; j++){
            xint_re[k][j]=x_int_l[j];
            xint_im[k][j]=x_int2_l[j];
        }
    }

    for(int k=0; k<width; k++){
        for(int i=0; i<height; i++){
            x_int_c[i]=xint_re[i][k];
            x_int2_c[i]=xint_im[i][k];
        }
        fft(x_int_c,x_int2_c, height) ;
        for(int i=0; i<height; i++){
            xtre[(i+(height>>1))%height][(k+(width>>1))%width]=x_int_c[i];
            xtim[(i+(height>>1))%height][(k+(width>>1))%width]=x_int2_c[i];
        }
    }
    freeMatrixData(xint_re, height);
    freeMatrixData(xint_im, height);
}

void fft2d_inv(double **xtre, double **xtim, double **xrec_re, double **xrec_im, unsigned int width, unsigned int height)
{
    double **xint_re = allocateMatrixData(height, width);
    double **xint_im = allocateMatrixData(height, width);
    double x_int_l[width], x_int2_l[width], x_int_c[height], x_int2_c[height];
    for(int k=0; k<height; k++){
        for(int j=0; j<width; j++){
            x_int_l[j] = xtre[(k-(height>>1))%height][(j+(width>>1))%width];
            x_int2_l[j] = xtim[(k-(height>>1))%height][(j+(width>>1))%width] ;
        }
        fft_inv(x_int_l, x_int2_l, width) ;
        for(int j=0; j<width; j++){
            xint_re[k][j] = x_int_l[j];
            xint_im[k][j] = x_int2_l[j];
        }
    }
    for(int k=0; k<width; k++){
        for(int i=0; i<height; i++){
            x_int_c[i] = xint_re[i][k];
            x_int2_c[i] = xint_im[i][k];
        }
        fft_inv(x_int_c,x_int2_c, height) ;
        for(int i=0; i<height; i++){
            xrec_re[i][k] = x_int_c[i];
            xrec_im[i][k] = x_int2_c[i];
        }
    }
    freeMatrixData(xint_re, height);
    freeMatrixData(xint_im, height);
}
