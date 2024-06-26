#define R_NO_REMAP

#include <string.h>
#include <stdio.h>
#include "rrapply.h"

/* initialize part of 'FixedArgs' */
void C_traverse(FixedArgs *fixedArgs, SEXP X, int depth)
{
    SEXP Xi;
    SEXP xptr = X;
    /* increment max depth if current depth is higher than max depth */
    R_len_t n = Rf_length(X);
    depth++;
    fixedArgs->maxnodes += n;
    fixedArgs->depthmax += (depth > fixedArgs->depthmax);

    for (R_len_t i = 0; i < n; i++)
    {
        /* X is either a list or pairlist */
        if (Rf_isVectorList(X))
        {
            Xi = VECTOR_ELT(X, i);
        }
        else
        {
            Xi = CAR(xptr);
            xptr = CDR(xptr);
        }
        /* descend one level */
        if (TYPEOF(Xi) != NILSXP && (Rf_isVectorList(Xi) || Rf_isPairList(Xi))) // skip NILSXP
        {
            C_traverse(fixedArgs, Xi, depth);
        }
        else
        {
            (fixedArgs->maxleafs)++;
        }
    }
}

/* initialize part of 'FixedArgs' for how = 'bind' */
void C_traverse_bind(FixedArgs *fixedArgs, SEXP X, int depth)
{
    SEXP Xi;
    SEXP xptr = X;
    /* increment max depth if current depth is higher than max depth */
    R_len_t n = Rf_length(X);
    depth++;
    fixedArgs->maxnodes += n;
    fixedArgs->depthmax += (depth > fixedArgs->depthmax);

    for (R_len_t i = 0; i < n; i++)
    {
        /* X is either a list or pairlist */
        if (Rf_isVectorList(X))
        {
            Xi = VECTOR_ELT(X, i);
        }
        else
        {
            Xi = CAR(xptr);
            xptr = CDR(xptr);
        }
        /* descend one level */
        if (TYPEOF(Xi) != NILSXP && (Rf_isVectorList(Xi) || Rf_isPairList(Xi))) // skip NILSXP
        {
            C_traverse_bind(fixedArgs, Xi, depth);
        }
        else
        {
            (fixedArgs->maxleafs)++;
            if (fixedArgs->ans_depthpivot == -1 || (depth - 1) < fixedArgs->ans_depthpivot)
                fixedArgs->ans_depthpivot = depth - 1;
        }
    }
}

/* initialize number of rows for how = 'bind' */
void C_count_rows(FixedArgs *fixedArgs, SEXP X, int depth)
{
    SEXP Xi;
    SEXP xptr = X;
    R_len_t n = Rf_length(X);

    if (depth < fixedArgs->ans_depthpivot - 1)
    {
        for (R_len_t i = 0; i < n; i++)
        {
            /* X is either a list or pairlist */
            if (Rf_isVectorList(X))
            {
                Xi = VECTOR_ELT(X, i);
            }
            else
            {
                Xi = CAR(xptr);
                xptr = CDR(xptr);
            }
            /* descend one level */
            if (TYPEOF(Xi) != NILSXP && (Rf_isVectorList(Xi) || Rf_isPairList(Xi))) // skip NILSXP
            {
                C_count_rows(fixedArgs, Xi, depth + 1);
            }
        }
    }
    else if (depth == fixedArgs->ans_depthpivot - 1)
    {
        fixedArgs->ans_maxrows += n;
    }
}

/* create long language object */
SEXP C_lang7(SEXP s, SEXP t, SEXP u, SEXP v, SEXP w, SEXP x, SEXP y)
{
    PROTECT(s);
    s = Rf_lcons(s, Rf_list6(t, u, v, w, x, y));
    UNPROTECT(1);
    return s;
}

/* convert integer to character */
SEXP C_int2char(int i, Rboolean prefix)
{
    char buff[64]; // fixed buffer size
    if (prefix)
        snprintf(buff, 64, "L%d", i);
    else
        snprintf(buff, 64, "%d", i);
    return Rf_mkChar(buff);
}

/* concatenate names */
SEXP C_strcat(SEXP names, int start, int end, const char *sep)
{
    size_t BUFFER_SIZE = 8192; // fixed buffer size
    char buff[8192];

    // do not copy unnamed elements at pivot depth
    strncpy(buff, STRING_ELT(names, start) == NA_STRING ? "" : CHAR(STRING_ELT(names, start)), BUFFER_SIZE - 1);
    buff[BUFFER_SIZE - 1] = '\0';
    size_t nbuff = strlen(buff);

    for (int j = start + 1; j < end + 1; j++)
    {
        if (nbuff < (BUFFER_SIZE - 1))
        {
            if (STRING_ELT(names, j) != NA_STRING)
            {
                if (nbuff > 0)
                    strncat(buff, sep, BUFFER_SIZE - nbuff - 1);

                strncat(buff, CHAR(STRING_ELT(names, j)), BUFFER_SIZE - strlen(buff) - 1);
                nbuff = strlen(buff);
            }
        }
        else
            break; // not reached normally
    }

    return Rf_mkChar(buff);
}

/* copies only name attribute or all attributes */
void C_copyAttrs(SEXP obj, SEXP ans, SEXP names, Rboolean copyAttrs)
{
    if (!Rf_isNull(names))
        Rf_setAttrib(ans, R_NamesSymbol, names);
    if (copyAttrs)
    {
        Rf_copyMostAttrib(obj, ans);
        Rf_setAttrib(ans, R_DimSymbol, PROTECT(Rf_getAttrib(ans, R_DimSymbol)));
        Rf_setAttrib(ans, R_DimNamesSymbol, PROTECT(Rf_getAttrib(ans, R_DimNamesSymbol)));
        UNPROTECT(2);
    }
}

/* adapted from R_data_class in Defn.h */
Rboolean C_matchClass(SEXP obj, SEXP classes)
{
    SEXP klass = PROTECT(Rf_getAttrib(obj, R_ClassSymbol));
    R_len_t n = Rf_length(klass);

    Rboolean matched = FALSE;
    /* match classes to R_ClassSymbol attribute */
    if (n > 0)
    {
        for (R_len_t i = 0; i < n; i++)
            for (R_len_t j = 0; j < Rf_length(classes); j++)
                if (strcmp(CHAR(STRING_ELT(klass, i)), CHAR(STRING_ELT(classes, j))) == 0)
                    matched = TRUE;
    }
    else
    {
        /* match to specific types */
        SEXP dim = PROTECT(Rf_getAttrib(obj, R_DimSymbol));
        R_len_t nd = Rf_length(dim);
        if (nd > 0)
        {
            if (nd == 2)
            {
                for (R_len_t j = 0; j < Rf_length(classes); j++)
                    if (strcmp(CHAR(STRING_ELT(classes, j)), "matrix") == 0 ||
                        strcmp(CHAR(STRING_ELT(classes, j)), "array") == 0)
                        matched = TRUE;
            }
            else
            {
                for (R_len_t j = 0; j < Rf_length(classes); j++)
                    if (strcmp(CHAR(STRING_ELT(classes, j)), "array") == 0)
                        matched = TRUE;
            }
        }
        else
        {
            SEXPTYPE type = (SEXPTYPE)TYPEOF(obj);
            const char *typename;
            switch (type)
            {
            case CLOSXP:
            case SPECIALSXP:
            case BUILTINSXP:
                typename = "function";
                break;
            case REALSXP:
                typename = "numeric";
                break;
            case SYMSXP:
                typename = "name";
                break;
            case EXPRSXP:
                typename = "expression";
                break;
            case LISTSXP:
                typename = "pairlist";
                break;
            case LANGSXP:
                typename = "language";
                break;
            default:
                typename = CHAR(Rf_type2str(type));
            }

            for (R_len_t j = 0; j < Rf_length(classes); j++)
                if (strcmp(CHAR(STRING_ELT(classes, j)), typename) == 0)
                    matched = TRUE;
        }
        UNPROTECT(1);
    }
    UNPROTECT(1);
    return matched;
}

/* determine answer type, adapted from AnswerType() in bind.c */
int C_answerType(SEXP x)
{
    if (Rf_length(x) == 1)
    {
        switch (TYPEOF(x))
        {
        // could include RAWSXP as well here, but Rf_as* do not implement RAWSXP
        case LGLSXP:
            return 2;
        case INTSXP:
            return 16;
        case REALSXP:
            return 32;
        case CPLXSXP:
            return 64;
        case STRSXP:
            return 128;
        default:
            return 256;
        }
    }
    else
    {
        return 256;
    }
}

/* populate and coerce flat list, adapted from coerceVectorList() in coerce.c */
void C_coerceList(SEXP ans, SEXP newans, R_len_t newlen, SEXPTYPE type)
{
    /* initialize outside of switch statement */
    SEXP newString;

    switch (type)
    {
    case VECSXP:
        for (R_len_t j = 0; j < newlen; j++)
            SET_VECTOR_ELT(newans, j, VECTOR_ELT(ans, j));
        break;
    case STRSXP:
        for (R_len_t j = 0; j < newlen; j++)
        {
            if (!Rf_isString(VECTOR_ELT(ans, j)))
            {
                newString = PROTECT(Rf_coerceVector(VECTOR_ELT(ans, j), STRSXP));
                SET_STRING_ELT(newans, j, STRING_ELT(newString, 0));
                UNPROTECT(1);
            }
            else
                SET_STRING_ELT(newans, j, STRING_ELT(VECTOR_ELT(ans, j), 0));
        }
        break;
    case CPLXSXP:
        for (R_len_t j = 0; j < newlen; j++)
            SET_COMPLEX_ELT(newans, j, Rf_asComplex(VECTOR_ELT(ans, j)));
        break;
    case REALSXP:
        for (R_len_t j = 0; j < newlen; j++)
            SET_REAL_ELT(newans, j, Rf_asReal(VECTOR_ELT(ans, j)));
        break;
    case INTSXP:
        for (R_len_t j = 0; j < newlen; j++)
            SET_INTEGER_ELT(newans, j, Rf_asInteger(VECTOR_ELT(ans, j)));
        break;
    case LGLSXP:
        for (R_len_t j = 0; j < newlen; j++)
            SET_LOGICAL_ELT(newans, j, Rf_asLogical(VECTOR_ELT(ans, j)));
        break;
    default: // not reached normally
        for (R_len_t j = 0; j < newlen; j++)
            SET_VECTOR_ELT(newans, j, VECTOR_ELT(ans, j));
        break;
    }
}
