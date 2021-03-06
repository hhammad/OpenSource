#include "genfft.h"
#include <string.h>

/**
*   NAME:     rc1fft
*
*   DESCRIPTION: real to complex FFT
*
*   USAGE:
*	      void rc1fft(REAL *rdata, complex *cdata, int n, int sign)
*
*   INPUT:  - *rdata: real 1D input vector 
*           -      n: number of (real) samples in input vector data rdata
*           -   sign: sign of the Fourier kernel 
*
*   OUTPUT: - *cdata: complex 1D output vector unscaled 
*
*   Notice in the preceding formula that there are n real input values,
*     and n/2 + 1 complex output values.  This property is characteristic of
*     real-to-complex FFTs.
*
*   NOTES: Optimized system dependent FFT's implemented for:
*          - SGI/CRAY ORIGIN 2000 (scsl)
*          - AMD ACML 4.4.0
*          - inplace FFT from Mayer and SU (see file fft_mayer.c)
*
*   AUTHOR:
*           Jan Thorbecke (janth@xs4all.nl)
*           The Netherlands
*
*----------------------------------------------------------------------
*  REVISION HISTORY:
*  VERSION        AUTHOR          DATE         COMMENT
*    1.0       Jan Thorbecke    Feb  '94    Initial version (TU Delft)
*    1.1       Jan Thorbecke    June '94    faster in-place FFT 
*    2.0       Jan Thorbecke    July '97    added Cray SGI calls 
*
----------------------------------------------------------------------*/
#if defined(ACML440)
	#if defined(DOUBLE) 
		#define acmlrcfft dzfft
	#else
		#define acmlrcfft scfft
	#endif
#endif

void rc1fft(REAL *rdata, complex *cdata, int n, int sign)
{
#if defined(HAVE_LIBSCS) || defined(ACML440)
	static int nprev=0;
	int   ntable, nwork, zero=0, one=1, i;
	static int isys;
	static REAL *work, *table, scale=1.0;
	REAL scl, *data;
#endif

#if defined(HAVE_LIBSCS)
	if (n != nprev) {
		isys   = 0;
		ntable = n + 15;
		nwork  = n + 2;
		if (work) free(work);
		work = (float *)malloc(nwork*sizeof(float));
		if (work == NULL) fprintf(stderr,"rc1fft: memory allocation error\n");
		if (table) free(table);
		table = (float *)malloc(ntable*sizeof(float));
		if (table == NULL) fprintf(stderr,"rc1fft: memory allocation error\n");
		scfft_(&zero, &n, &scale, rdata, cdata, table, work, &isys);
		nprev = n;
	}
	scfft_(&sign, &n, &scale, rdata, cdata, table, work, &isys);
#elif defined(ACML440)
	data = (REAL *)malloc(n*sizeof(REAL));
	memcpy(data, rdata, n*sizeof(float));
	if (n != nprev) {
		isys   = 0;
		nwork  = 3*n + 100;
		if (work) free(work);
		work = (REAL *)malloc(nwork*sizeof(REAL));
		if (work == NULL) fprintf(stderr,"rc1fft: memory allocation error\n");
		acmlrcfft(zero, n, data, work, &isys);
		nprev = n;
	}
	acmlrcfft(one, n, data, work, &isys);
	scl = sqrt(n);
	for (i=0; i<n/2+1;i++) {
		cdata[i].r=scl*data[i];
	}
	cdata[0].i=0.0;
	for (i=1; i<((n-1)/2)+1; i++) {
		cdata[i].i=-sign*scl*data[n-i];
	}
	cdata[n/2].i=0.0;
	free(data);
#else
	rc1_fft(rdata, cdata, n, sign);
#endif

	return;
}


/****************** NO COMPLEX DEFINED ******************/

void Rrc1fft(REAL *rdata, REAL *cdata, int n, int sign)
{
	rc1fft(rdata, (complex *)cdata, n , sign);
	return;
}

/****************** FORTRAN SHELL *****************/

#ifdef DF_CAPFNAMES
#define nrc1fft	FNAME(RC1FFTF)
#else
#define nrc1fft	FNAME(rc1fftf)
#endif

void nrc1fft(REAL *rdata, complex *cdata, int *n, int *sign)
{
	rc1fft(rdata, cdata, *n, *sign);

	return;
}

