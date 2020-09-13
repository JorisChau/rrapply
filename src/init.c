#include <R.h>
#include <Rinternals.h>
#include <stdlib.h> // for NULL
#include <R_ext/Rdynload.h>

/* .Call calls */
void R_init_rrapply(DllInfo *dll);
extern SEXP C_unmelt(SEXP);
extern SEXP C_rrapply(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);

static const R_CallMethodDef CallEntries[] = {
    {"C_unmelt", (DL_FUNC)&C_unmelt, 1},
    {"C_rrapply", (DL_FUNC)&C_rrapply, 11},
    {NULL, NULL, 0}};

void R_init_rrapply(DllInfo *dll)
{
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}