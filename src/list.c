#define R_NO_REMAP

#include <string.h>
#include <stdio.h>
#include "rrapply.h"

SEXP C_recurse_list(
    SEXP env,             // evaluation environment
    SEXP Xi,              // current list element
    FunCall f,            // f function call
    FunCall condition,    // condition function call
    FixedArgs *fixedArgs, // fixed arguments
    LocalArgs *localArgs, // variable arguments and counters
    SEXP classes,         // classes argument
    SEXP deflt,           // deflt argumnt (only used if how = "list" or how = "unlist")
    SEXP xsym             // principal argument symbol
)
{
    /* initialize SEXPs */
    SEXP fval = NULL;
    int nprotect = 0;

    /* if Xi is list (and data.frame is treated as list if !dfaslist)
       and !feverywhere recurse, otherwise evaluate functions */
    Rboolean recurse = FALSE;

    if (fixedArgs->feverywhere < 1 && ((Rf_isVectorList(Xi) || Rf_isPairList(Xi)) && TYPEOF(Xi) != NILSXP))
    {
        recurse = TRUE;
        if (fixedArgs->dfaslist < 1)
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
            /* avoid uninitialized warning */
            SEXP xname_val = NULL, xpos_val = NULL, xparents_val = NULL, xsiblings_val = NULL;
            int nargprotect = 0;

            /* define X argument */
            Rf_defineVar(xsym, Xi, env);
            INCREMENT_NAMED(Xi);

            /* update current .xname value */
            if (f.xname || condition.xname)
            {
                if (f.xparents || condition.xparents)
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
            /* update current node info only for pruning and melting */
            if (fixedArgs->how == 3 || fixedArgs->how == 5)
            {
                R_len_t i1 = localArgs->node;
                (localArgs->xinfo_array)[i1] = 1;
                R_len_t i2 = (localArgs->xinfo_array)[i1 + fixedArgs->maxnodes];

                for (int i = localArgs->depth; i > -1; i--)
                {
                    if (i2 > -1)
                    {
                        i1 = i2;
                        (localArgs->xinfo_array)[i1] = 2; // non-terminal node
                        i2 = (localArgs->xinfo_array)[i1 + fixedArgs->maxnodes];
                    }
                    else
                        break;
                }
            }

            /* evaluate f */
            if (f.evaluate && f.nargs > 0)
            {
                fval = PROTECT(R_forceAndCall(f.call, f.nargs, env));
                nprotect++;

                /* update evaluated name */
                if (fixedArgs->how == 9)
                {
                    if (!Rf_isString(fval))
                    {
                        SEXP fname = PROTECT(Rf_coerceVector(fval, STRSXP));
                        SET_STRING_ELT(fixedArgs->ansnames_ptr, localArgs->xpos_vec[localArgs->depth] - 1, !Rf_isNull(STRING_ELT(fname, 0)) ? STRING_ELT(fname, 0) : R_BlankString);
                        UNPROTECT(1);
                    }
                    else
                    {
                        SET_STRING_ELT(fixedArgs->ansnames_ptr, localArgs->xpos_vec[localArgs->depth] - 1, !Rf_isNull(STRING_ELT(fval, 0)) ? STRING_ELT(fval, 0) : R_BlankString);
                    }
                    fval = Xi;
                }
            }
            else
            {
                fval = Xi;
            }

            /* recurse further with new value if feverywhere == 2 or dfaslist == -1 */
            if ((fixedArgs->feverywhere == 2 || fixedArgs->dfaslist == -1) && ((Rf_isVectorList(fval) || Rf_isPairList(fval)) && TYPEOF(fval) != NILSXP))
            {
                recurse = TRUE;
            }
            else /* otherwise return current value */
            {
                UNPROTECT(nprotect);
                return fval;
            }
        }
        else if ((fixedArgs->feverywhere > 0 || fixedArgs->dfaslist == -1) && !emptysymbol && ((Rf_isVectorList(Xi) || Rf_isPairList(Xi)) && TYPEOF(Xi) != NILSXP))
        {
            /* recurse further */
            fval = Xi;
            recurse = TRUE;
        }
        else
        {
            if (fixedArgs->how == 1 || fixedArgs->how == 2)
            {
                UNPROTECT(nprotect);
                return Rf_lazy_duplicate(deflt);
            }
            else
            {
                UNPROTECT(nprotect);
                return Rf_lazy_duplicate(Xi);
            }
        }
    }
    else
    {
        fval = Xi;
    }

    /* recurse further into list-like element */
    if (recurse)
    {
        /* avoid unitialized warning */
        SEXP ans = NULL, ansptr = NULL, ansnames = NULL;

        /* current element information */
        R_len_t n = Rf_length(fval);
        SEXP names = PROTECT(Rf_getAttrib(fval, R_NamesSymbol));
        SEXP fvalptr = Rf_isPairList(fval) ? fval : NULL;

        /* allocate new output list */
        if ((Rf_isVectorList(fval) && (fixedArgs->how == 1 || fixedArgs->how == 2 || fixedArgs->how == 9)) || (Rf_isPairList(fval) && fixedArgs->how > 0))
        {
            ans = PROTECT(Rf_allocVector(VECSXP, n));
            C_copyAttrs(fval, ans, names, !Rf_isPairList(fval));
            if (Rf_isPairList(fval))
                Rf_copyMostAttrib(fval, ans);

            if (fixedArgs->how == 9)
            {
                if (Rf_isNull(names))
                    ansnames = PROTECT(Rf_allocVector(STRSXP, n));
                else
                    ansnames = PROTECT(Rf_duplicate(names));

                fixedArgs->ansnames_ptr = ansnames;
                nprotect++;
            }
        }
        else
        {
            ans = PROTECT(Rf_shallow_duplicate(fval));
            if (Rf_isPairList(fval))
                ansptr = ans;
        }
        nprotect += 2;

        /* current value of .xsiblings argument */
        if (f.xsiblings || condition.xsiblings)
            localArgs->xsiblings_ptr = fval;

        /* increment current depth */
        (localArgs->depth)++;

        /* reallocate .xpos and .xparent vectors */
        if (fixedArgs->feverywhere == 2 || fixedArgs->dfaslist == -1)
        {
            if (localArgs->depth > 100)
                Rf_error("a hard limit of maximum 100 nested layers is enforced to avoid infinite recursion"); /* stop with error if depth too large */

            if (localArgs->depth >= fixedArgs->depthmax)
            {
                if (f.xpos || condition.xpos || fixedArgs->how == 9)
                    localArgs->xpos_vec = (R_len_t *)S_realloc((char *)localArgs->xpos_vec, 2 * fixedArgs->depthmax, fixedArgs->depthmax, sizeof(R_len_t));

                if (f.xparents || condition.xparents)
                {
                    SEXP xparent_new = PROTECT(Rf_allocVector(STRSXP, 2 * fixedArgs->depthmax));
                    for (R_len_t i = 0; i < fixedArgs->depthmax; i++)
                        SET_STRING_ELT(xparent_new, i, STRING_ELT(localArgs->xparent_ptr, i));

                    REPROTECT(localArgs->xparent_ptr = xparent_new, localArgs->xparent_ipx);
                    UNPROTECT(1);
                }
                fixedArgs->depthmax *= 2;
            }
        }

        /* update variable arguments and counters */
        R_len_t parent = 0; // avoid uninitialized warning
        if (fixedArgs->how == 3 || fixedArgs->how == 5)
            parent = localArgs->node;

        for (R_len_t i = 0; i < n; i++)
        {
            /* update .xpos argument */
            if (f.xpos || condition.xpos || fixedArgs->how == 9)
                (localArgs->xpos_vec)[localArgs->depth] = i + 1;

            /* update .xparents and/or .xname arguments */
            if (localArgs->xparent_ptr)
            {
                SEXP iname = PROTECT(Rf_isNull(names) ? C_int2char(i + 1, FALSE) : STRING_ELT(names, i));
                SET_STRING_ELT(localArgs->xparent_ptr, (f.xparents || condition.xparents) ? localArgs->depth : 0, iname);
                UNPROTECT(1);
            }

            if (fixedArgs->how == 3 || fixedArgs->how == 5)
            {
                localArgs->node++;                                                        // global node counter
                (localArgs->xinfo_array)[localArgs->node + fixedArgs->maxnodes] = parent; // parent node counter
                (localArgs->xinfo_array)[localArgs->node + 2 * fixedArgs->maxnodes] = i;  // child node counter
            }

            /* main recursion part */
            if (Rf_isVectorList(fval))
            {
                SET_VECTOR_ELT(ans, i, C_recurse_list(env, VECTOR_ELT(fval, i), f, condition, fixedArgs, localArgs, classes, deflt, xsym));
            }
            else if (Rf_isPairList(fval))
            {
                if (fixedArgs->how < 1)
                {
                    SETCAR(ansptr, C_recurse_list(env, CAR(fvalptr), f, condition, fixedArgs, localArgs, classes, deflt, xsym));
                    ansptr = CDR(ansptr);
                }
                else
                {
                    SET_VECTOR_ELT(ans, i, C_recurse_list(env, CAR(fvalptr), f, condition, fixedArgs, localArgs, classes, deflt, xsym));
                }
                fvalptr = CDR(fvalptr);
            }

            /* reset .xsiblings argument */
            if (f.xsiblings || condition.xsiblings)
                localArgs->xsiblings_ptr = fval;

            /* reset names */
            if (fixedArgs->how == 9)
                fixedArgs->ansnames_ptr = ansnames;
        }

        /* decrement current depth */
        (localArgs->depth)--;

        /* update names */
        if (fixedArgs->how == 9)
            Rf_setAttrib(ans, R_NamesSymbol, ansnames);

        UNPROTECT(nprotect);
        return ans;
    }
    else
    {
        /* should not normally be reached */
        UNPROTECT(nprotect);
        return R_NilValue;
    }
}

/* Helper function to recursively prune nested list */
SEXP C_prune_list(SEXP Xi, R_len_t *xinfo, R_len_t *buf, R_len_t node, R_len_t maxnodes, R_len_t newmaxnodes, R_len_t ibuf)
{
    if (Rf_isVectorList(Xi))
    {
        R_len_t m = 0;
        R_len_t maxparent = node;
        for (R_len_t inode = node + 1; inode < newmaxnodes; inode++)
        {
            /* check if direct child of node and node is evaluated */
            if (xinfo[inode + maxnodes] == node && xinfo[inode])
            {
                buf[ibuf + m] = inode;
                m++;
            }
            /* stop if no longer (indirect) child of node */
            if (xinfo[inode + maxnodes] < node || xinfo[inode + maxnodes] > maxparent)
                break;
            /* update maximum allowed parent node */
            maxparent++;
        }

        /* descend one level */
        if (m > 0)
        {
            /* populate sublist*/
            SEXP ans = PROTECT(Rf_allocVector(VECSXP, m));
            for (R_len_t j = 0; j < m; j++)
                SET_VECTOR_ELT(ans, j, C_prune_list(VECTOR_ELT(Xi, xinfo[buf[ibuf + j] + 2 * maxnodes]), xinfo, buf, buf[ibuf + j], maxnodes, newmaxnodes, ibuf + m));

            /* add name attribute */
            SEXP names = PROTECT(Rf_getAttrib(Xi, R_NamesSymbol));
            if (!Rf_isNull(names))
            {
                SEXP ansnames = PROTECT(Rf_allocVector(STRSXP, m));

                for (R_len_t j = 0; j < m; j++)
                    SET_STRING_ELT(ansnames, j, STRING_ELT(names, xinfo[buf[ibuf + j] + 2 * maxnodes]));

                Rf_setAttrib(ans, R_NamesSymbol, ansnames);
                UNPROTECT(1);
            }
            /* copy other list attributes */
            Rf_copyMostAttrib(Xi, ans);

            UNPROTECT(2);
            return ans;
        }
        else
        {
            /* should not be reached normally */
            return Xi;
        }
    }
    else
    {
        return Xi;
    }
}
