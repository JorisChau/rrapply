#define R_NO_REMAP

#include <string.h>
#include <stdio.h>
#include "rrapply.h"

void C_recurse_flatten(
    SEXP env,             // evaluation environment
    SEXP Xi,              // current list element
    FunCall f,            // f function call
    FunCall condition,    // condition function call
    FixedArgs *fixedArgs, // fixed arguments
    LocalArgs *localArgs, // variable arguments and counters
    SEXP classes,         // classes argument
    SEXP xsym             // principal argument symbol
)
{
    /* if Xi is list (and data.frame is treated as list if !dfaslist)
       and !feverywhere recurse, otherwise evaluate functions */
    Rboolean recurse = FALSE;

    if (fixedArgs->feverywhere < 1 && ((Rf_isVectorList(Xi) || Rf_isPairList(Xi)) && TYPEOF(Xi) != NILSXP))
    {
        recurse = TRUE;
        if (!fixedArgs->dfaslist)
        {
            if (C_matchClass(Xi, PROTECT(Rf_ScalarString(Rf_mkChar("data.frame")))))
                recurse = FALSE;
            UNPROTECT(1);
        }
    }

    /* do not recurse further, i.e. apply f on node
        if condition and classes are satisfied */
    if (!recurse)
    {
        /* evaluate condition and/or f calls */
        Rboolean eval = TRUE;
        Rboolean matched = (strcmp(CHAR(STRING_ELT(classes, 0)), "ANY") == 0) ? TRUE : C_matchClass(Xi, classes);
        Rboolean emptysymbol = FALSE;

        if (Rf_isSymbol(Xi))
            emptysymbol = strlen(CHAR(PRINTNAME(Xi))) < 1;

        if (((condition.evaluate && condition.nargs > 0) ||
             (f.evaluate && f.nargs > 0)) && // valid condition or f functions
            matched &&                       // matches classes argument
            !emptysymbol)                    // Xi not an empty symbol
        {
            /* avoid unitialized warning */
            SEXP xname_val = NULL, xpos_val = NULL, xparents_val = NULL, xsiblings_val = NULL;
            int nargprotect = 0;

            /* define X argument */
            Rf_defineVar(xsym, Xi, env);
            INCREMENT_NAMED(Xi);

            /* update current .xname value */
            if (f.xname || condition.xname)
            {
                if (f.xparents || condition.xparents || fixedArgs->how > 4)
                    xname_val = PROTECT(Rf_ScalarString(STRING_ELT(localArgs->xparent_ptr, localArgs->depth)));
                else
                    xname_val = PROTECT(Rf_duplicate(localArgs->xparent_ptr));
                nargprotect++;
            }

            /* update current .xpos value */
            if (f.xpos || condition.xpos)
            {
                xpos_val = PROTECT(Rf_allocVector(INTSXP, localArgs->depth + 1));
                for (R_len_t k = 0; k < (localArgs->depth + 1); k++)
                    SET_INTEGER_ELT(xpos_val, k, (int)(localArgs->xpos_vec)[k]);
                nargprotect++;
            }

            /* update current .xparents value */
            if (f.xparents || condition.xparents)
            {
                xparents_val = PROTECT(Rf_allocVector(STRSXP, localArgs->depth + 1));
                for (R_len_t k = 0; k < (localArgs->depth + 1); k++)
                    SET_STRING_ELT(xparents_val, k, STRING_ELT(localArgs->xparent_ptr, k));
                nargprotect++;
            }

            /* update current .xsiblings value */
            if (f.xsiblings || condition.xsiblings)
            {
                if (Rf_isPairList(localArgs->xsiblings_ptr))
                {
                    R_len_t n = Rf_length(localArgs->xsiblings_ptr);
                    xsiblings_val = PROTECT(Rf_allocVector(VECSXP, n));
                    SEXP xptr = localArgs->xsiblings_ptr;
                    for (R_len_t i = 0; i < n; i++)
                    {
                        SET_VECTOR_ELT(xsiblings_val, i, CAR(xptr));
                        xptr = CDR(xptr);
                    }
                    Rf_copyMostAttrib(localArgs->xsiblings_ptr, xsiblings_val);
                    Rf_setAttrib(xsiblings_val, R_NamesSymbol, PROTECT(Rf_getAttrib(localArgs->xsiblings_ptr, R_NamesSymbol)));
                    UNPROTECT(1);
                }
                else
                {
                    xsiblings_val = PROTECT(Rf_shallow_duplicate(localArgs->xsiblings_ptr));
                }
                nargprotect++;
            }

            /* define f special arguments */
            if (f.evaluate && f.nargs > 1)
            {
                SEXP fcdr = CDDR(f.call);

                if (f.xname)
                {
                    SETCAR(fcdr, xname_val);
                    fcdr = CDR(fcdr);
                }
                if (f.xpos)
                {
                    SETCAR(fcdr, xpos_val);
                    fcdr = CDR(fcdr);
                }
                if (f.xparents)
                {
                    SETCAR(fcdr, xparents_val);
                    fcdr = CDR(fcdr);
                }
                if (f.xsiblings)
                {
                    SETCAR(fcdr, xsiblings_val);
                }
            }

            /* define condition special arguments */
            if (condition.evaluate && condition.nargs > 1)
            {
                SEXP pcdr = CDDR(condition.call);

                if (condition.xname)
                {
                    SETCAR(pcdr, xname_val);
                    pcdr = CDR(pcdr);
                }
                if (condition.xpos)
                {
                    SETCAR(pcdr, xpos_val);
                    pcdr = CDR(pcdr);
                }
                if (condition.xparents)
                {
                    SETCAR(pcdr, xparents_val);
                    pcdr = CDR(pcdr);
                }
                if (condition.xsiblings)
                {
                    SETCAR(pcdr, xsiblings_val);
                }
            }

            UNPROTECT(nargprotect);
        }

        /* evaluate condiiton call */
        if (condition.evaluate && condition.nargs > 0 && matched && !emptysymbol)
        {
            /* set default to FALSE */
            eval = FALSE;

            /* evaluate pred function call */
            SEXP pval = PROTECT(R_forceAndCall(condition.call, condition.nargs, env));

            if (Rf_isLogical(pval) && Rf_length(pval) == 1)
            {
                int pval_lgl = LOGICAL_ELT(pval, 0);
                if (!(pval_lgl == NA_LOGICAL) && pval_lgl)
                {
                    eval = TRUE;
                }
            }
            UNPROTECT(1);
        }

        /* evaluate f call and decide what to return or recurse further */
        if (eval && matched && !emptysymbol)
        {
            SEXP fval;

            /* evaluate f and assign result to flat list */
            if (f.evaluate && f.nargs > 0)
                fval = PROTECT(R_forceAndCall(f.call, f.nargs, env));
            else
                fval = PROTECT(Rf_lazy_duplicate(Xi));

            /* update return type flag */
            fixedArgs->ans_flags |= C_answerType(fval);

            SET_VECTOR_ELT(fixedArgs->ans_ptr, localArgs->ans_idx, fval);

            /* update names attributes or columns */
            if (fixedArgs->how == 4)
            {
                if (fixedArgs->ans_sep)
                {
                    SET_STRING_ELT(fixedArgs->ansnames_ptr, localArgs->ans_idx, PROTECT(C_strcat(localArgs->xparent_ptr, 0, localArgs->depth, fixedArgs->ans_sep)));
                }
                else if (f.xparents || condition.xparents)
                    SET_STRING_ELT(fixedArgs->ansnames_ptr, localArgs->ans_idx, PROTECT(STRING_ELT(localArgs->xparent_ptr, localArgs->depth)));
                else
                    SET_STRING_ELT(fixedArgs->ansnames_ptr, localArgs->ans_idx, PROTECT(STRING_ELT(localArgs->xparent_ptr, 0)));

                UNPROTECT(1);
            }
            else if (fixedArgs->how == 5)
            {
                SET_VECTOR_ELT(fixedArgs->ansnames_ptr, localArgs->ans_idx, PROTECT(Rf_duplicate(localArgs->xparent_ptr)));
                UNPROTECT(1);

                /* update maximum observed depth */
                if (localArgs->depth > fixedArgs->ans_depthmax)
                    fixedArgs->ans_depthmax = localArgs->depth;

                /* check if any symbol is present as no format method is available for symbols in R < 4.0.0,
                   which produces an error in list to data.frame conversion */
                if (!fixedArgs->anysymbol && Rf_isSymbol(fval))
                    fixedArgs->anysymbol = TRUE;
            }
            else if (fixedArgs->how == 6)
            {
                int start = (fixedArgs->ans_depthpivot <= localArgs->depth) ? fixedArgs->ans_depthpivot : 0;

                if (fixedArgs->ans_sep)
                    SET_STRING_ELT(fixedArgs->ansnames_ptr, localArgs->ans_idx, PROTECT(C_strcat(localArgs->xparent_ptr, start, localArgs->depth, fixedArgs->ans_sep)));
                else
                    SET_STRING_ELT(fixedArgs->ansnames_ptr, localArgs->ans_idx, PROTECT(C_strcat(localArgs->xparent_ptr, start, localArgs->depth, ".")));

                UNPROTECT(1);

                (localArgs->xinfo_array)[localArgs->ans_idx] = localArgs->ans_row;

                /* same as for melting */
                if (!fixedArgs->anysymbol && Rf_isSymbol(fval))
                    fixedArgs->anysymbol = TRUE;
            }

            (localArgs->ans_idx)++;
            UNPROTECT(1);
        }
        else if (fixedArgs->feverywhere > 0 && !emptysymbol && ((Rf_isVectorList(Xi) || Rf_isPairList(Xi)) && TYPEOF(Xi) != NILSXP))
        {
            /* recurse further */
            recurse = TRUE;
        }
    }

    /* recurse further into list-like element */
    if (recurse)
    {
        /* current element information */
        SEXP iname;
        R_len_t n = Rf_length(Xi);
        SEXP xptr = Rf_isPairList(Xi) ? Xi : NULL;
        SEXP names = PROTECT(Rf_getAttrib(Xi, R_NamesSymbol));
        fixedArgs->anynames = !Rf_isNull(names) || fixedArgs->anynames;

        /* current value of .xsiblings argument */
        if (f.xsiblings || condition.xsiblings)
            localArgs->xsiblings_ptr = Xi;

        /* increment current depth */
        (localArgs->depth)++;

        for (R_len_t i = 0; i < n; i++)
        {
            /* update .xpos argument */
            if (f.xpos || condition.xpos)
                (localArgs->xpos_vec)[localArgs->depth] = i + 1;

            /* update .xparents and/or .xname arguments */

            iname = PROTECT(Rf_isNull(names) ? C_int2char(i + 1, FALSE) : STRING_ELT(names, i));
            SET_STRING_ELT(localArgs->xparent_ptr, (f.xparents || condition.xparents || fixedArgs->how != 4 || fixedArgs->ans_sep) ? localArgs->depth : 0, iname);
            UNPROTECT(1);

            /* clean-up dangling names when melting */
            if (fixedArgs->how == 5 && localArgs->depth < (fixedArgs->depthmax - 1))
            {
                for (R_len_t j = localArgs->depth + 1; j < fixedArgs->depthmax; j++)
                    SET_STRING_ELT(localArgs->xparent_ptr, j, NA_STRING);
            }

            /* main recursion part */
            if (Rf_isVectorList(Xi))
            {
                C_recurse_flatten(env, VECTOR_ELT(Xi, i), f, condition, fixedArgs, localArgs, classes, xsym);
            }
            else if (Rf_isPairList(Xi))
            {
                C_recurse_flatten(env, CAR(xptr), f, condition, fixedArgs, localArgs, classes, xsym);
                xptr = CDR(xptr);
            }

            /* reset .xsiblings argument */
            if (f.xsiblings || condition.xsiblings)
                localArgs->xsiblings_ptr = Xi;

            /* update assignments how = 'bind' */
            if (fixedArgs->how == 6 && fixedArgs->ans_depthpivot == localArgs->depth + 1)
            {
                /* assign binding name columns */
                if (fixedArgs->ans_namecols && localArgs->ans_row < fixedArgs->ans_maxrows)
                {
                    for (R_len_t j = 0; j < fixedArgs->ans_depthpivot; j++)
                        SET_STRING_ELT(VECTOR_ELT(fixedArgs->ansnamecols_ptr, j), localArgs->ans_row, STRING_ELT(localArgs->xparent_ptr, j));
                }
                localArgs->ans_row++;
            }
        }

        UNPROTECT(1);

        /* decrement current depth */
        (localArgs->depth)--;
    }
}
