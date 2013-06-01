#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
void Matrix_multi_Vector(int nrows, float *ax, float *sra, int *clm, int *fnz, float *x){
	for(int i = 0; i < nrows; ++i)
		for(int j = fnz[i]; j < fnz[i+1]; ++j)
			ax[i] += (sra[j] * x[clm[j]]);
}

float dotproduct(int nrows, float *z, float *r){ 
	float temp = 0.0;
	for(int i = 0; i < nrows; ++i)
		temp += (z[i] * r[i]);
	return temp;
}

float maxnorm(int nrows, float *r){
	float temp = abs(r[0]);
	for(int i = 1; i < nrows; ++i)
		if(temp < abs(r[i]))
			temp = abs(r[i]);
	return temp;
}

void PCG(int nrows, float *sra, int*clm, int *fnz, float *x, float *b, int maxiter, float tol){
	// CG Algorithm
	// Theoretically, the CG algorithm can be converged in n time's iteration.
	// But, in consideration of the error of the calculation in computer,
	// we set max iteration time to a very very big number.
	// So, we can always get the solution x smaller then a tolerance number.

	// initialization
	float *r = new float[nrows];
	float *Ap = new float[nrows];
	float *newr = new float[nrows];
	for(int i = 0; i < nrows; ++i){
		r[i] = 0.0;
		Ap[i] = 0.0;
		newr[i] = 0.0;
	}
	// calculate Ax
	Matrix_multi_Vector(nrows,Ap,sra,clm,fnz,x);
	// r = b - Ap
	for(int i = 0; i < nrows; ++i)
		r[i] = b[i] - Ap[i];
	float *p = new float[nrows];
	// initial p = r
	for(int i = 0; i < nrows; ++i)
		p[i] = r[i];

	float pAp = 0.0;
	float rr = 0.0;
	float alpha = 0.0;
	float newrnewr = 0.0;
	float beta = 0.0;
	for(int iter = 0; iter < maxiter; ++iter){
		for(int i = 0; i < nrows; ++i)
			Ap[i] = 0.0;
		// calculate Ap
		Matrix_multi_Vector(nrows,Ap,sra,clm,fnz,p);
		// (p,Ap)
		pAp = dotproduct(nrows,p,Ap);
		if( pAp == 0.0 )
			break;
		// (r,r)
		rr = dotproduct(nrows,r,r);
		alpha = rr/pAp;
		for(int i = 0; i < nrows; ++i){
			// x = x + alpha * p
			x[i] += (alpha * p[i]);
			// r = r - alpha * Ap
			newr[i] = r[i] - alpha * Ap[i];
		}
		
		/*if(!(maxnorm(nrows,r) > tol))
			break;*/
		// (r,r)
		newrnewr = dotproduct(nrows,newr,newr);
		if( newrnewr < tol )
			break;
		beta = newrnewr/rr;
		for(int i = 0; i < nrows; ++i){
			// p = r + beta * p
			p[i] = newr[i] + beta * p[i];
			r[i] = newr[i];
		}
	}
}

//int main(){
//	double start_t,finish_t;
//
//	int nrows = 5000;
//	int iter = 120000;
//	
//	//float *sra = new float[3*nrows-2];
//	//sra[0] = 1.0;
//	//for(int i = 1; i < 3*nrows-2; ++i){
//	//	if(i%3 == 1)
//	//		sra[i] = (int)(i/3) + 2;
//	//	else if(i%3 == 2)
//	//		sra[i] = (int)(i/3) + 2;
//	//	else
//	//		sra[i] = (int)(i/3) + 1;
//	//}
//	//
//	//int *clm = new int[3*nrows-2];
//	//clm[0] = 0;
//	//clm[1] = 1;
//	//for(int i = 2; i < 3*nrows-4; ++i){
//	//	if(i%3 == 2)
//	//		clm[i] = (int)(i/3);
//	//	else if(i%3 == 0)
//	//		clm[i] = (int)(i/3);
//	//	else
//	//		clm[i] = (int)(i/3)+1;
//	//}
//	//clm[3*nrows-4] = nrows-2;
//	//clm[3*nrows-3] = nrows-1;
//
//	//int *fnz = new int[nrows+1];
//	//fnz[0] = 0;
//	//fnz[1] = 2;
//	//for(int i = 2; i < nrows; ++i)
//	//	fnz[i] = fnz[i-1] + 3;
//	//fnz[nrows] = 3*nrows-2;
//	//
//	///*
//	//double sra[28] = {1,2, 2,2,3, 3,3,4, 4,4,5, 5,5,6, 6,6,7, 7,7,8, 8,8,9, 9,9,10, 10,10};
//	//int clm[28] = {0,1, 0,1,2, 1,2,3, 2,3,4, 3,4,5, 4,5,6, 5,6,7, 6,7,8, 7,8,9,  8,9};
//	//int fnz[11] = {0, 2, 5, 8, 11, 14, 17, 20, 23, 26, 28};
//	//*/
//	//float *b = new float[nrows];
//
//	float b[3];
//	float sra[4] = {1,45,32.1,2};
//	int clm[4] = {0,1,0,1};
//	int fnz[3] = {0,2,4};
//
//	
//	for(int i = 0; i < 2; ++i)
//		b[i] = 0;
//
//	printf("The exact solution of x is:\n");
//	float *x = new float[2];
//	for(int i = 0; i < 2; ++i){
//		x[i] = 0.00000;
//	}
//
//	printf("Solving the equation...\n\n");
//
//	// calculate b
//	Matrix_multi_Vector(2,b, sra, clm, fnz, x);
//	printf("%f,%f\n",b[0],b[1]);
//	// set initial x
//	for(int i = 0; i < 2; ++i)
//		x[i] = 0.0;
//
//	// if the max normal of residual r is smaller then a very small number(the constant "tol" below),
//	// then break the iteration in function PCG.
//	double tol = 0.000000001;
//
//	start_t = clock();
//	PCG(2,sra,clm,fnz,x,b,iter,tol);
//	finish_t = clock();
//	double execute_time = (finish_t - start_t)/CLOCKS_PER_SEC;
//
//	for(int i = 0; i < 2; ++i)
//		printf("x[%d] = %f\n",i,x[i]);
//	printf("Total iteration times: %d\n", iter);
//	printf("The execute time is %f second.\n",execute_time);
//	system("pause");
//	return 0;
//}