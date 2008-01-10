#ifndef FOURIER_H
#define FOURIER_H

void fft2d(double **xtre, double **xtim, unsigned int width, unsigned int height);
void fft2d_inv(double **xtre, double **xtim, double **xrec_re, double **xrec_im, unsigned int width, unsigned int height);

double** allocateMatrixData(int rows, int columns);
void freeMatrixData(double **data, int rows);
#endif
