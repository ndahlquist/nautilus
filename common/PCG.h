#pragma once

#define PCG_ITER 1000
#define PCG_TOL 0.001

void PCG(int nrows, float *sra, int*clm, int *fnz, float *x, float *b, int maxiter = PCG_ITER, float tol = PCG_TOL);