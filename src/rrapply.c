#define R_NO_REMAP

#include <string.h>
#include <stdio.h>
#include "rrapply.h"

SEXP C_rrapply(SEXP env, SEXP X, SEXP FUN, SEXP argsFun, SEXP PRED, SEXP argsPred, SEXP classes, SEXP R_how, SEXP deflt, SEXP R_dfaslist, SEXP R_feverywhere, SEXP options)
{
	SEXP ans, ansnames = NULL, ansnamecols = NULL, ansptr, xptr, names, xsym, xname, xpos, xparents, xsiblings;

	/* protect calls */
	int nprotect = 0;

	/* install arguments */
	xsym = Rf_install("X");
	xname = Rf_install(".xname");
	xpos = Rf_install(".xpos");
	xparents = Rf_install(".xparents");
	xsiblings = Rf_install(".xsiblings");

	/* initialize f call, definition depends on special arguments */
	FunCall f = {
		.call = NULL,
		.evaluate = Rf_isFunction(FUN),
		.nargs = 0,
		.xname = INTEGER_ELT(argsFun, 0) > 0,
		.xpos = INTEGER_ELT(argsFun, 1) > 0,
		.xparents = INTEGER_ELT(argsFun, 2) > 0,
		.xsiblings = INTEGER_ELT(argsFun, 3) > 0};

	f.nargs = 1 + f.xname + f.xpos + f.xparents + f.xsiblings;

	if (f.evaluate)
	{
		switch (f.nargs)
		{
		case 1:
			f.call = PROTECT(Rf_lang3(FUN, xsym, R_DotsSymbol));
			break;
		case 2:
			f.call = PROTECT(Rf_lang4(FUN, xsym, R_NilValue, R_DotsSymbol));
			break;
		case 3:
			f.call = PROTECT(Rf_lang5(FUN, xsym, R_NilValue, R_NilValue, R_DotsSymbol));
			break;
		case 4:
			f.call = PROTECT(Rf_lang6(FUN, xsym, R_NilValue, R_NilValue, R_NilValue, R_DotsSymbol));
			break;
		case 5:
			f.call = PROTECT(C_lang7(FUN, xsym, R_NilValue, R_NilValue, R_NilValue, R_NilValue, R_DotsSymbol));
			break;
		default:
			f.call = PROTECT(Rf_lang3(FUN, xsym, R_DotsSymbol)); // not reached
		}

		SEXP fcdr = CDR(f.call);

		if (f.xname)
		{
			fcdr = CDR(fcdr);
			SET_TAG(fcdr, xname);
		}
		if (f.xpos)
		{
			fcdr = CDR(fcdr);
			SET_TAG(fcdr, xpos);
		}
		if (f.xparents)
		{
			fcdr = CDR(fcdr);
			SET_TAG(fcdr, xparents);
		}
		if (f.xsiblings)
		{
			SET_TAG(CDR(fcdr), xsiblings);
		}

		nprotect++;
	}

	/* initialize condition call, definition depends on special arguemnts */
	FunCall condition = {
		.call = NULL,
		.nargs = 0,
		.xname = INTEGER_ELT(argsPred, 0) > 0,
		.xpos = INTEGER_ELT(argsPred, 1) > 0,
		.xparents = INTEGER_ELT(argsPred, 2) > 0,
		.xsiblings = INTEGER_ELT(argsPred, 3) > 0,
		.evaluate = Rf_isFunction(PRED)};

	condition.nargs = 1 + condition.xname + condition.xpos + condition.xparents + condition.xsiblings;

	if (condition.evaluate)
	{
		switch (condition.nargs)
		{
		case 1:
			condition.call = PROTECT(Rf_lang3(PRED, xsym, R_DotsSymbol));
			break;
		case 2:
			condition.call = PROTECT(Rf_lang4(PRED, xsym, R_NilValue, R_DotsSymbol));
			break;
		case 3:
			condition.call = PROTECT(Rf_lang5(PRED, xsym, R_NilValue, R_NilValue, R_DotsSymbol));
			break;
		case 4:
			condition.call = PROTECT(Rf_lang6(PRED, xsym, R_NilValue, R_NilValue, R_NilValue, R_DotsSymbol));
			break;
		case 5:
			condition.call = PROTECT(C_lang7(PRED, xsym, R_NilValue, R_NilValue, R_NilValue, R_NilValue, R_DotsSymbol));
			break;
		default:
			condition.call = PROTECT(Rf_lang3(PRED, xsym, R_DotsSymbol)); // not reached
		}

		SEXP pcdr = CDR(condition.call);

		if (condition.xname)
		{
			pcdr = CDR(pcdr);
			SET_TAG(pcdr, xname);
		}
		if (condition.xpos)
		{
			pcdr = CDR(pcdr);
			SET_TAG(pcdr, xpos);
		}
		if (condition.xparents)
		{
			pcdr = CDR(pcdr);
			SET_TAG(pcdr, xparents);
		}
		if (condition.xsiblings)
		{
			SET_TAG(CDR(pcdr), xsiblings);
		}

		nprotect++;
	}

	/* fixed values */
	FixedArgs fixedArgs = {
		.ans_ptr = NULL,
		.ansnames_ptr = NULL,
		.how = INTEGER_ELT(R_how, 0) - 1,
		.dfaslist = INTEGER_ELT(R_dfaslist, 0),
		.feverywhere = INTEGER_ELT(R_feverywhere, 0),
		.depthmax = 1,
		.maxnodes = 0,
		.maxleafs = 0,
		.anynames = FALSE,
		.anysymbol = FALSE,
		.ans_flags = 0,
		.ans_sep = NULL,
		.ans_depthmax = 0,
		.ans_depthpivot = -1,
		.ans_maxrows = 0};

	/* variable values and counters */
	PROTECT_INDEX ipx = 0; // initialize to avoid warning

	LocalArgs localArgs = {
		.xparent_ptr = NULL,
		.xsiblings_ptr = NULL,
		.node = -1,
		.depth = 0,
		.ans_idx = 0,
		.xpos_vec = NULL,
		.xinfo_array = NULL,
		.ans_row = 0,
		.xparent_ipx = ipx};

	/* fill remaining values, depends on how argument
	   and presence of special arguments */
	R_len_t n = Rf_length(X);
	names = PROTECT(Rf_getAttrib(X, R_NamesSymbol));
	nprotect++;

	/* how = 'flatten' */
	if (fixedArgs.how == 4)
	{
		if (!Rf_isNull(names))
			fixedArgs.anynames = TRUE;

		/* name separator */
		SEXP namesep = STRING_ELT(VECTOR_ELT(options, 0), 0);
		if (namesep != NA_STRING)
			fixedArgs.ans_sep = CHAR(namesep);
	}

	/* how = 'bind' */
	if (fixedArgs.how == 6)
	{
		/* include name-column(s) */
		fixedArgs.ans_namecols = LOGICAL_ELT(VECTOR_ELT(options, 2), 0);

		/* traverse list once for max nodes, max depth and
		   pivot depth for more accurate initialization,
		   computational effort is negligible */
		C_traverse_bind(&fixedArgs, X, 0);

		/* override with user pivot depth */
		int coldepth = INTEGER_ELT(VECTOR_ELT(options, 3), 0) - 1;
		if (coldepth > -1)
			fixedArgs.ans_depthpivot = coldepth;

		/* detect maximum number of binding rows */
		C_count_rows(&fixedArgs, X, 0);

		/* name separator */
		SEXP namesep = STRING_ELT(VECTOR_ELT(options, 0), 0);
		if (namesep != NA_STRING)
			fixedArgs.ans_sep = CHAR(namesep);
		else
			fixedArgs.ans_sep = ".";
	}
	else
	{
		/* traverse list once for max nodes and max depth
		   for more accurate initialization, computational
		   effort is negligible */
		C_traverse(&fixedArgs, X, 0);
	}

	/* how = 'prune' */
	if (fixedArgs.how == 3)
		localArgs.xinfo_array = (R_len_t *)S_alloc(3 * fixedArgs.maxnodes, sizeof(R_len_t));
	else if (fixedArgs.how == 6)
		localArgs.xinfo_array = (R_len_t *)S_alloc(fixedArgs.maxleafs, sizeof(R_len_t));

	/* current value of .xpos argument */
	if (f.xpos || condition.xpos || fixedArgs.how == 9)
		localArgs.xpos_vec = (R_len_t *)S_alloc(fixedArgs.depthmax, sizeof(R_len_t));

	/* current value of .xparents and/or .xname arguments */
	if (f.xparents || condition.xparents || fixedArgs.ans_sep || fixedArgs.how == 5 || fixedArgs.how == 6)
	{
		PROTECT_WITH_INDEX(localArgs.xparent_ptr = Rf_allocVector(STRSXP, fixedArgs.depthmax), &ipx);
		for (R_len_t j = 0; j < fixedArgs.depthmax; j++)
			SET_STRING_ELT(localArgs.xparent_ptr, j, NA_STRING);
		nprotect++;
	}
	else if (f.xname || condition.xname || fixedArgs.how == 4)
	{
		PROTECT_WITH_INDEX(localArgs.xparent_ptr = Rf_ScalarString(NA_STRING), &ipx);
		nprotect++;
	}

	/* current value of .xsiblings argument */
	if (f.xsiblings || condition.xsiblings)
		localArgs.xsiblings_ptr = X;

	/* allocate output objects */
	ansptr = NULL; /* avoid unitialized warning */
	xptr = Rf_isPairList(X) ? X : NULL;

	if (fixedArgs.how == 4 || fixedArgs.how == 5 || fixedArgs.how == 6)
	{
		ans = PROTECT(Rf_allocVector(VECSXP, fixedArgs.maxleafs));
		fixedArgs.ans_ptr = ans;

		if (fixedArgs.how == 5)
			ansnames = PROTECT(Rf_allocVector(VECSXP, fixedArgs.maxleafs));
		else
			ansnames = PROTECT(Rf_allocVector(STRSXP, fixedArgs.maxleafs));

		fixedArgs.ansnames_ptr = ansnames;
		nprotect += 2;

		/* temp object to hold extra binding name-columns
		   (only used if namecols == TRUE) */
		if (fixedArgs.how == 6 && fixedArgs.ans_namecols)
		{
			ansnamecols = PROTECT(Rf_allocVector(VECSXP, fixedArgs.ans_depthpivot));
			SEXP namecol = PROTECT(Rf_allocVector(STRSXP, fixedArgs.ans_maxrows));
			for (R_len_t j = 0; j < fixedArgs.ans_depthpivot; j++)
				SET_VECTOR_ELT(ansnamecols, j, Rf_duplicate(namecol));
			UNPROTECT(1);
			fixedArgs.ansnamecols_ptr = ansnamecols;
			nprotect++;
		}
	}
	else if ((Rf_isVectorList(X) && (fixedArgs.how == 1 || fixedArgs.how == 2)) || (Rf_isPairList(X) && fixedArgs.how > 0))
	{
		ans = PROTECT(Rf_allocVector(VECSXP, n));
		C_copyAttrs(X, ans, names, !Rf_isPairList(X));
		if (Rf_isPairList(X))
			Rf_copyMostAttrib(X, ans);
		nprotect++;
	}
	else
	{
		ans = PROTECT(Rf_shallow_duplicate(X));
		if (Rf_isPairList(X))
			ansptr = ans;
		nprotect++;

		if (fixedArgs.how == 9)
		{
			if (Rf_isNull(names))
				ansnames = PROTECT(Rf_allocVector(STRSXP, n));
			else
				ansnames = PROTECT(Rf_duplicate(names));

			fixedArgs.ansnames_ptr = ansnames;
			nprotect++;
		}
	}

	/* traverse list to evaluate function calls */
	for (R_len_t i = 0; i < n; i++)
	{
		/* update variable arguments and counters */
		if (f.xpos || condition.xpos || fixedArgs.how == 9)
			(localArgs.xpos_vec[0])++; // increment .xpos

		if (localArgs.xparent_ptr)
		{
			SEXP iname = PROTECT(Rf_isNull(names) ? C_int2char(i + 1, FALSE) : STRING_ELT(names, i));
			SET_STRING_ELT(localArgs.xparent_ptr, 0, iname); // update .xparents and/or .xname
			UNPROTECT(1);

			/* clean-up dangling names when melting */
			if (fixedArgs.how == 5 && fixedArgs.depthmax > 1)
			{
				for (R_len_t j = 1; j < fixedArgs.depthmax; j++)
					SET_STRING_ELT(localArgs.xparent_ptr, j, NA_STRING);
			}
		}

		if (fixedArgs.how == 3)
		{
			localArgs.node++;													// global node counter
			localArgs.xinfo_array[localArgs.node + fixedArgs.maxnodes] = -1;	// parent node counter
			localArgs.xinfo_array[localArgs.node + 2 * fixedArgs.maxnodes] = i; // child node counter
		}

		/* main recursion part */
		if (Rf_isVectorList(X))
		{
			switch (fixedArgs.how)
			{
			case 4:
			case 5:
			case 6:
				C_recurse_flatten(env, VECTOR_ELT(X, i), f, condition, &fixedArgs, &localArgs, classes, xsym);
				break;
			default:
				SET_VECTOR_ELT(ans, i, C_recurse_list(env, VECTOR_ELT(X, i), f, condition, &fixedArgs, &localArgs, classes, deflt, xsym));
				break;
			}
		}
		else if (Rf_isPairList(X))
		{
			switch (fixedArgs.how)
			{
			case 0:
				SETCAR(ansptr, C_recurse_list(env, CAR(xptr), f, condition, &fixedArgs, &localArgs, classes, deflt, xsym));
				ansptr = CDR(ansptr);
				break;
			case 1:
			case 2:
			case 3:
			case 9:
				SET_VECTOR_ELT(ans, i, C_recurse_list(env, CAR(xptr), f, condition, &fixedArgs, &localArgs, classes, deflt, xsym));
				break;
			case 4:
			case 5:
			case 6:
				C_recurse_flatten(env, CAR(xptr), f, condition, &fixedArgs, &localArgs, classes, xsym);
				break;
			}
			xptr = CDR(xptr);
		}

		/* reset .xsiblings argument */
		if (f.xsiblings || condition.xsiblings)
			localArgs.xsiblings_ptr = X;

		/* update assignments how = 'bind' */
		if (fixedArgs.how == 6 && fixedArgs.ans_depthpivot == 1)
		{
			/* bump row only if non-empty */
			if (localArgs.ans_idx > 0 && (localArgs.xinfo_array)[localArgs.ans_idx - 1] == localArgs.ans_row)
			{
				/* assign binding name columns */
				if (fixedArgs.ans_namecols && localArgs.ans_row < fixedArgs.ans_maxrows)
					SET_STRING_ELT(VECTOR_ELT(fixedArgs.ansnamecols_ptr, 0), localArgs.ans_row, STRING_ELT(localArgs.xparent_ptr, 0));
				localArgs.ans_row++;
			}
		}

		/* reset names */
		if (fixedArgs.how == 9)
			fixedArgs.ansnames_ptr = ansnames;
	}

	if (fixedArgs.how == 3) // prune list
	{
		/* detect nodes to filter */
		R_len_t newmaxnodes = localArgs.node + 1;
		R_len_t *buf = (R_len_t *)R_alloc((size_t)newmaxnodes, sizeof(R_len_t));
		R_len_t m = 0;
		for (R_len_t i = 0; i < fixedArgs.maxnodes; i++)
		{
			/* if nested list filter only level zero evaluated nodes,
			   otherwise filter evaluated terminal nodes */
			if (localArgs.xinfo_array[i] && localArgs.xinfo_array[i + fixedArgs.maxnodes] == -1)
			{
				buf[m] = i;
				m++;
			}
		}

		/* populate pruned list */
		SEXP newans = PROTECT(Rf_allocVector(VECSXP, m));
		nprotect++;

		/* populate nested list */
		for (R_len_t j = 0; j < m; j++)
			SET_VECTOR_ELT(newans, j, C_prune_list(VECTOR_ELT(ans, localArgs.xinfo_array[buf[j] + 2 * fixedArgs.maxnodes]), localArgs.xinfo_array, buf, buf[j], fixedArgs.maxnodes, newmaxnodes, m));

		/* add names attribute */
		if (!Rf_isNull(names))
		{
			SEXP newnames = PROTECT(Rf_allocVector(STRSXP, m));
			for (R_len_t j = 0; j < m; j++)
				SET_STRING_ELT(newnames, j, STRING_ELT(names, localArgs.xinfo_array[buf[j] + 2 * fixedArgs.maxnodes]));
			Rf_setAttrib(newans, R_NamesSymbol, newnames);
			UNPROTECT(1);
		}
		/* copy other list attributes */
		Rf_copyMostAttrib(ans, newans);

		UNPROTECT(nprotect);
		return newans;
	}
	else if (fixedArgs.how == 4 || fixedArgs.how == 5) // unlist, flatten or melt list
	{
		SEXP newans = ans;

		if (LOGICAL_ELT(VECTOR_ELT(options, 1), 0))
		{
			/* coerce type */
			SEXPTYPE mode = VECSXP;
			if (fixedArgs.ans_flags & 256)
				mode = VECSXP;
			else if (fixedArgs.ans_flags & 128)
				mode = STRSXP;
			else if (fixedArgs.ans_flags & 64)
				mode = CPLXSXP;
			else if (fixedArgs.ans_flags & 32)
				mode = REALSXP;
			else if (fixedArgs.ans_flags & 16)
				mode = INTSXP;
			else if (fixedArgs.ans_flags & 2)
				mode = LGLSXP;

			/* create return object */
			newans = PROTECT(Rf_allocVector(mode, localArgs.ans_idx));
			nprotect++;

			/* populate  return object and coerce to flagged type */
			C_coerceList(ans, newans, localArgs.ans_idx, mode);
		}

		if (fixedArgs.how == 4)
		{
			/* add names attribute and return result */
			if (fixedArgs.anynames)
			{
				SEXP newnames = PROTECT(Rf_allocVector(STRSXP, localArgs.ans_idx));
				for (R_len_t j = 0; j < localArgs.ans_idx; j++)
					SET_STRING_ELT(newnames, j, STRING_ELT(ansnames, j));
				Rf_setAttrib(newans, R_NamesSymbol, newnames);
				UNPROTECT(1);
			}

			UNPROTECT(nprotect);
			return newans;
		}
		else
		{
			/* melted return object */
			SEXP newans1 = PROTECT(Rf_allocVector(VECSXP, fixedArgs.ans_depthmax + 2));
			SEXP newnames_col = PROTECT(Rf_allocVector(STRSXP, localArgs.ans_idx));
			SEXP *newnames_col_ptr = STRING_PTR(newnames_col);
			nprotect += 3;

			/* populate columns */
			SET_VECTOR_ELT(newans1, fixedArgs.ans_depthmax + 1, newans);

			for (R_len_t j = 0; j < fixedArgs.ans_depthmax + 1; j++)
			{
				for (R_len_t i = 0; i < localArgs.ans_idx; i++)
					newnames_col_ptr[i] = STRING_ELT(VECTOR_ELT(ansnames, i), j);
				/* deep copy of column */
				SET_VECTOR_ELT(newans1, j, Rf_duplicate(newnames_col));
			}

			/* add attribute checking if data.frame conversion is possible in R < 4.0.0 */
			Rf_setAttrib(newans1, Rf_install("anysymbol"), PROTECT(Rf_ScalarLogical((int)fixedArgs.anysymbol)));

			UNPROTECT(nprotect);
			return newans1;
		}
	}
	else if (fixedArgs.how == 6) // row-bind list
	{
		/* traverse flat list */
		R_len_t *col_idx_array = (R_len_t *)R_alloc((size_t)localArgs.ans_idx, sizeof(R_len_t));
		SEXP ansnames_uniq = PROTECT(Rf_allocVector(STRSXP, localArgs.ans_idx));
		nprotect++;

		/* initialize arguments */
		R_len_t icol = 0, ncol = 0;

		if (localArgs.ans_idx > 0)
		{
			SEXP *ansnames_uniq_ptr = STRING_PTR(ansnames_uniq);
			SEXP *ansnames_new_ptr = STRING_PTR(ansnames);

			ansnames_uniq_ptr[0] = ansnames_new_ptr[0];
			col_idx_array[ncol++] = icol++;

			for (R_len_t i = 1; i < localArgs.ans_idx; i++)
			{
				/* fast check, if fails default to slow check */
				if (icol < ncol && strcmp(CHAR(ansnames_new_ptr[i]), CHAR(ansnames_uniq_ptr[icol])) == 0)
				{
					col_idx_array[i] = icol;
					icol = icol < (ncol - 1) ? icol + 1 : 0;
				}
				else
				{
					/* slow check */
					for (R_len_t j = 0; j < ncol; j++)
					{
						/* match already observed column */
						if (strcmp(CHAR(ansnames_new_ptr[i]), CHAR(ansnames_uniq_ptr[j])) == 0)
						{
							col_idx_array[i] = j;
							icol = j < (ncol - 1) ? j + 1 : 0;
							break;
						}
						/* add new column */
						if (j == (ncol - 1))
						{
							ansnames_uniq_ptr[j + 1] = ansnames_new_ptr[i];
							col_idx_array[i] = j + 1;
							icol = 0;
							ncol++;
							break;
						}
					}
				}
			}
		}

		/* create and populate return object */
		R_len_t ncol1 = fixedArgs.ans_namecols ? fixedArgs.ans_depthpivot : 0;
		SEXP newans = PROTECT(Rf_allocVector(VECSXP, ncol + ncol1));
		nprotect++;

		/* set default column values to NA_LOGICAL */
		if (localArgs.ans_idx > 0)
		{
			R_len_t nrow = localArgs.ans_row > 0 ? localArgs.ans_row : 1;
			SEXP anscol = PROTECT(Rf_allocVector(VECSXP, nrow));
			for (R_len_t i = 0; i < nrow; i++)
				SET_VECTOR_ELT(anscol, i, Rf_ScalarLogical(NA_LOGICAL));

			for (R_len_t j = 0; j < ncol; j++)
				SET_VECTOR_ELT(newans, j + ncol1, Rf_shallow_duplicate(anscol));
			UNPROTECT(1);

			for (R_len_t i = 0; i < localArgs.ans_idx; i++)
				SET_VECTOR_ELT(VECTOR_ELT(newans, col_idx_array[i] + ncol1), localArgs.xinfo_array[i], VECTOR_ELT(ans, i));

			/* coerce columns */
			int col_flags;
			SEXPTYPE mode;

			for (R_len_t j = 0; j < ncol; j++)
			{
				col_flags = 0;
				for (R_len_t i = 0; i < nrow; i++)
					col_flags |= C_answerType(VECTOR_ELT(VECTOR_ELT(newans, j + ncol1), i));

				if (col_flags < 256)
				{
					/* return type */
					mode = VECSXP;
					if (col_flags & 128)
						mode = STRSXP;
					else if (col_flags & 64)
						mode = CPLXSXP;
					else if (col_flags & 32)
						mode = REALSXP;
					else if (col_flags & 16)
						mode = INTSXP;
					else if (col_flags & 2)
						mode = LGLSXP;

					SEXP newcol = PROTECT(Rf_allocVector(mode, nrow));
					C_coerceList(VECTOR_ELT(newans, j + ncol1), newcol, nrow, mode);
					SET_VECTOR_ELT(newans, j + ncol1, newcol);
					UNPROTECT(1);
				}
			}

			/* add name columns */
			if (fixedArgs.ans_namecols && nrow <= fixedArgs.ans_maxrows)
			{
				for (R_len_t j = 0; j < ncol1; j++)
				{
					if (nrow == fixedArgs.ans_maxrows)
					{
						SET_VECTOR_ELT(newans, j, Rf_duplicate(VECTOR_ELT(ansnamecols, j)));
					}
					else /* clip column lengths */
					{
						SEXP namecol = PROTECT(Rf_allocVector(STRSXP, nrow));
						for (R_len_t i = 0; i < nrow; i++)
							SET_STRING_ELT(namecol, i, STRING_ELT(VECTOR_ELT(ansnamecols, j), i));
						SET_VECTOR_ELT(newans, j, namecol);
						UNPROTECT(1);
					}
				}
			}
		}

		/* add attribute checking if data.frame conversion is possible in R < 4.0.0 */
		Rf_setAttrib(newans, Rf_install("anysymbol"), PROTECT(Rf_ScalarLogical((int)fixedArgs.anysymbol)));

		/* add column names */
		SEXP newnames = PROTECT(Rf_allocVector(STRSXP, ncol + ncol1));
		nprotect += 2;

		for (R_len_t j = 0; j < ncol; j++)
			SET_STRING_ELT(newnames, j + ncol1, STRING_ELT(ansnames_uniq, j));

		if (fixedArgs.ans_namecols)
		{
			for (R_len_t j = 0; j < ncol1; j++)
				SET_STRING_ELT(newnames, j, C_int2char(j + 1, TRUE));
		}

		Rf_setAttrib(newans, R_NamesSymbol, newnames);

		UNPROTECT(nprotect);
		return newans;
	}
	else if (fixedArgs.how == 9)
	{
		/* update names */
		Rf_setAttrib(ans, R_NamesSymbol, ansnames);

		UNPROTECT(nprotect);
		return ans;
	}
	else // other 'how' options
	{
		UNPROTECT(nprotect);
		return ans;
	}
}
