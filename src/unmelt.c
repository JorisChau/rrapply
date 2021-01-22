#define R_NO_REMAP

#include <string.h>
#include "rrapply.h"

/* prototypes */

static SEXP C_fill_unmelt(SEXP X, SEXP Xval, R_len_t *namesCount, R_len_t lvl, R_len_t nlvls, R_len_t start, R_len_t end);

/* main function */
SEXP C_unmelt(SEXP X)
{
	R_len_t ncols = (R_len_t)Rf_length(X);
	R_len_t nrows = (R_len_t)Rf_length(VECTOR_ELT(X, ncols - 1));
	R_len_t *namesCount = (R_len_t *)R_alloc((size_t)nrows, sizeof(R_len_t));

	return C_fill_unmelt(X, VECTOR_ELT(X, ncols - 1), namesCount, 0, ncols - 2, 0, nrows);
}

/* unmelt data.frame to nested list */
static SEXP C_fill_unmelt(SEXP X, SEXP Xval, R_len_t *namesCount, R_len_t lvl, R_len_t nlvls, R_len_t start, R_len_t end)
{
	// get column
	SEXP Xi = VECTOR_ELT(X, lvl);
	SEXP Xi1 = VECTOR_ELT(X, lvl + 1);
	R_len_t m = 0;

	// count names
	for (R_len_t i = start; i < end; i++)
	{
		if (i == start ||
			lvl >= nlvls ||
			STRING_ELT(Xi1, i) == NA_STRING ||
			STRING_ELT(Xi, i - 1) == NA_STRING ||
			STRING_ELT(Xi, i) == NA_STRING ||
			strcmp(CHAR(STRING_ELT(Xi, i - 1)), CHAR(STRING_ELT(Xi, i))) != 0)
		{
			if (i > start)
				m++;
			namesCount[m] = 0;
		}

		(namesCount[m])++;
	}

	SEXP ansNew = PROTECT(Rf_allocVector(VECSXP, m + 1));
	SEXP ansNames = PROTECT(Rf_allocVector(STRSXP, m + 1));
	SEXPTYPE ansType = (SEXPTYPE)TYPEOF(Xval);
	R_len_t mcum = start;

	for (R_len_t j = 0; j < (m + 1); j++)
	{
		SET_STRING_ELT(ansNames, j, STRING_ELT(Xi, mcum));
		mcum += namesCount[j];
	}

	Rf_setAttrib(ansNew, R_NamesSymbol, ansNames);
	R_len_t jstart = 0;
	R_len_t jend = start;
	R_len_t *jcounts = (R_len_t *)R_alloc((size_t)(m + 1), sizeof(R_len_t));
	memcpy(jcounts, namesCount, (size_t)(m + 1) * sizeof(R_len_t));

	for (R_len_t j = 0; j < (m + 1); j++)
	{
		jstart += (j == 0 ? start : jcounts[j - 1]);
		jend += jcounts[j];

		if ((jend - jstart) > 1 ||
			(lvl < nlvls && STRING_ELT(Xi1, jstart) != NA_STRING))
			SET_VECTOR_ELT(ansNew, j, C_fill_unmelt(X, Xval, namesCount, lvl + 1, nlvls, jstart, jend));
		else
		{
			switch (ansType)
			{
			case VECSXP:
				SET_VECTOR_ELT(ansNew, j, VECTOR_ELT(Xval, jstart));
				break;
			case STRSXP:
				SET_VECTOR_ELT(ansNew, j, Rf_ScalarString(STRING_ELT(Xval, jstart)));
				break;
			case CPLXSXP:
				SET_VECTOR_ELT(ansNew, j, Rf_ScalarComplex(COMPLEX_ELT(Xval, jstart)));
				break;
			case REALSXP:
				SET_VECTOR_ELT(ansNew, j, Rf_ScalarReal(REAL_ELT(Xval, jstart)));
				break;
			case INTSXP:
				SET_VECTOR_ELT(ansNew, j, Rf_ScalarInteger(INTEGER_ELT(Xval, jstart)));
				break;
			case LGLSXP:
				SET_VECTOR_ELT(ansNew, j, Rf_ScalarLogical(LOGICAL_ELT(Xval, jstart)));
				break;
			case RAWSXP:
				SET_VECTOR_ELT(ansNew, j, Rf_ScalarRaw(RAW_ELT(Xval, jstart)));
				break;
			}
		}
	}

	UNPROTECT(2);
	return ansNew;
}
