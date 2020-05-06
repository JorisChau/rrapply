#include <R.h>
#include <Rinternals.h>

/* copies only name attribute or all attributes */
void do_copyAttrs(SEXP obj, SEXP ans, SEXP names, Rboolean copyAttrs)
{
	if (!isNull(names))
		setAttrib(ans, R_NamesSymbol, names);
	if (copyAttrs)
	{
		copyMostAttrib(obj, ans);
		setAttrib(ans, R_DimSymbol, getAttrib(ans, R_DimSymbol));
		setAttrib(ans, R_DimNamesSymbol, getAttrib(ans, R_DimNamesSymbol));
	}
}

/* adapted from R_data_class in Defn.h */
Rboolean do_matchClass(SEXP obj, SEXP classes)
{
	SEXP klass = getAttrib(obj, R_ClassSymbol);
	int n = xlength(klass);

	Rboolean matched = FALSE;
	/* match classes to R_ClassSymbol attribute */
	if (n > 0)
	{
		for (int i = 0; i < n; i++)
			for (int j = 0; j < xlength(classes); j++)
				if (strcmp(CHAR(STRING_ELT(klass, i)), CHAR(STRING_ELT(classes, j))) == 0)
					matched = TRUE;

		return matched;
	}
	else
	{
		/* match to specific types */
		SEXP dim = getAttrib(obj, R_DimSymbol);
		int nd = xlength(dim);
		if (nd > 0)
		{
			if (nd == 2)
			{
				for (int j = 0; j < xlength(classes); j++)
					if (strcmp(CHAR(STRING_ELT(classes, j)), "matrix") == 0 ||
						strcmp(CHAR(STRING_ELT(classes, j)), "array") == 0)
						matched = TRUE;
			}
			else
			{
				for (int j = 0; j < xlength(classes); j++)
					if (strcmp(CHAR(STRING_ELT(classes, j)), "array") == 0)
						matched = TRUE;
			}
		}
		else
		{
			SEXPTYPE type = TYPEOF(obj);
			const char *typename;
			/* excluded LANGSXP type */
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
			default:
				typename = CHAR(type2str(type));
			}

			for (int j = 0; j < xlength(classes); j++)
				if (strcmp(CHAR(STRING_ELT(classes, j)), typename) == 0)
					matched = TRUE;
		}

		return matched;
	}
}

R_xlen_t do_rrcount(SEXP X, R_xlen_t n, R_xlen_t *maxNodes, R_xlen_t *maxDepth, R_xlen_t depth)
{
	SEXP Xi;
	for (R_xlen_t i = 0; i < n; i++)
	{
		*maxNodes += 1;
		Xi = VECTOR_ELT(X, i);
		/* descend one level */
		if (isVectorList(Xi))
		{
			depth += 1;
			/* increment maxDepth if current depth is higher than maxDepth */
			*maxDepth += depth > *maxDepth;
			depth = do_rrcount(Xi, xlength(Xi), maxNodes, maxDepth, depth);
		}
	}
	return depth - 1;
}

void do_updateNode(int *xinfo, int node, Rboolean doEval, int parent, int depth, int child)
{
	/* current node counter */
	xinfo[node * 5] = node;
	/* evaluate node */
	xinfo[node * 5 + 1] = doEval;
	/* parent node */
	xinfo[node * 5 + 2] = parent;
	/* depth layer */
	xinfo[node * 5 + 3] = depth;
	/* child counter */
	xinfo[node * 5 + 4] = child;
}

SEXP do_rreval_list(SEXP env, SEXP enclos, SEXP Xi, SEXP fcall, SEXP pcall, SEXP classes,
					SEXP deflt, SEXP xname, SEXP xsym, SEXP xpos, SEXP xnameChar,
					int howInt, int **xloc, int depth, R_xlen_t maxDepth, Rboolean useFun,
					Rboolean usePred, Rboolean dfList)
{
	SEXP predVal, funVal;

	/* if Xi is list (and data.frame is treated as list) recurse, otherwise call function */
	Rboolean doRecurse = FALSE;

	if (isVectorList(Xi))
	{
		doRecurse = TRUE;
		if (!dfList && do_matchClass(Xi, ScalarString(mkChar("data.frame"))))
			doRecurse = FALSE;
	}

	if (doRecurse)
	{
		/* descend one level */
		depth += 1;
		R_xlen_t m = xlength(Xi);
		SEXP names = getAttrib(Xi, R_NamesSymbol);

		if (howInt == 0)
		{
			funVal = PROTECT(shallow_duplicate(Xi));
		}
		else
		{
			/* VECEXP initializes with R_NilValues */
			funVal = PROTECT(allocVector(VECSXP, m));
			do_copyAttrs(Xi, funVal, names, TRUE);
		}

		/* reallocate array if max is reached */
		if (depth == maxDepth)
		{
			maxDepth *= 2;
			*xloc = (int *)S_realloc((char *)*xloc, maxDepth, depth, sizeof(int));
		}

		for (R_xlen_t j = 0; j < m; j++)
		{
			/* increment location */
			(*xloc)[depth] = j + 1;

			/* evaluate list element */
			SET_VECTOR_ELT(funVal, j, do_rreval_list(env, enclos, VECTOR_ELT(Xi, j), fcall, pcall, classes, deflt, xname, xsym, xpos, 
						   isNull(names) ? NA_STRING : STRING_ELT(names, j), howInt, xloc, depth, maxDepth, useFun, usePred, dfList));
		}

		UNPROTECT(1);
		return funVal;
	}
	else
	{
		SEXP xname_val, xpos_val;

		/* define X argument */
		defineVar(xsym, Xi, env);
		INCREMENT_NAMED(Xi);

		/* define .xname argument */
		xname_val = PROTECT(ScalarString(xnameChar));
		defineVar(xname, xname_val, enclos);
		INCREMENT_NAMED(xname_val);
		UNPROTECT(1);

		/* define .xpos argument */
		xpos_val = PROTECT(allocVector(INTSXP, depth + 1));

		for (R_xlen_t k = 0; k < (depth + 1); k++)
			SET_INTEGER_ELT(xpos_val, k, (*xloc)[k]);

		defineVar(xpos, xpos_val, enclos);
		INCREMENT_NAMED(xpos_val);
		UNPROTECT(1);

		/* evaluate predicate */
		Rboolean doEval = TRUE;
		Rboolean matched = FALSE;

		if (strcmp(CHAR(STRING_ELT(classes, 0)), "ANY") == 0) /* ASCII */
			matched = TRUE;
		else
			matched = do_matchClass(Xi, classes);

		if (usePred)
		{
			/* set default to FALSE */
			doEval = FALSE;
			/* evaluate pred function call */
			SEXP predVal = PROTECT(R_forceAndCall(pcall, 1, env));
			if (isLogical(predVal) && xlength(predVal) == 1)
			{
				Rboolean predValBool = LOGICAL_ELT(predVal, 0);
				if (!(predValBool == NA_LOGICAL) && predValBool)
				{
					doEval = TRUE;
				}
			}
			UNPROTECT(1);
		}

		/* evaluate function call */
		if (doEval && matched)
		{
			if (useFun)
			{
				funVal = PROTECT(R_forceAndCall(fcall, 1, env));

				if (MAYBE_REFERENCED(funVal))
					funVal = lazy_duplicate(funVal);

				UNPROTECT(1);

				return funVal;
			}
			else
			{
				return lazy_duplicate(Xi);
			}
		}
		else
		{
			if (howInt == 0) /* replace */
			{
				return lazy_duplicate(Xi);
			}
			else /* fill default */
			{
				return lazy_duplicate(deflt);
			}
		}
	}
}

void do_rreval_flat(SEXP env, SEXP enclos, SEXP Xflat, SEXP Xnames, SEXP Xi, SEXP fcall, SEXP pcall,
					SEXP classes, SEXP deflt, SEXP xname, SEXP xsym, SEXP xpos, SEXP xnameChar,
					int *xinfo, int *xloc, int depth, int *node, int parent, Rboolean useFun,
					Rboolean usePred, Rboolean dfList, int howInt)
{
	/* if Xi is list (and data.frame is treated as list) recurse, otherwise call function */
	Rboolean doRecurse = FALSE;

	if (isVectorList(Xi))
	{
		doRecurse = TRUE;
		if (!dfList && do_matchClass(Xi, ScalarString(mkChar("data.frame"))))
			doRecurse = FALSE;
	}

	if (doRecurse)
	{
		/* descend one level */
		depth += 1;
		parent = node[0];
		R_xlen_t m = xlength(Xi);
		SEXP names = getAttrib(Xi, R_NamesSymbol);

		for (R_xlen_t j = 0; j < m; j++)
		{
			/* increment location */
			xloc[depth] = j + 1;

			/* update current node info */
			node[0] += 1;
			do_updateNode(xinfo, node[0], FALSE, parent, depth, j);

			/* update name attribute */
			if (!isNull(names))
				SET_STRING_ELT(Xnames, node[0], STRING_ELT(names, j));

			/* evaluate list element */
			do_rreval_flat(env, enclos, Xflat, Xnames, VECTOR_ELT(Xi, j), fcall, pcall,
						   classes, deflt, xname, xsym, xpos, isNull(names) ? NA_STRING : STRING_ELT(names, j),
						   xinfo, xloc, depth, node, parent, useFun, usePred, dfList, howInt);
		}
	}
	else
	{
		SEXP xname_val, xpos_val;

		/* define X argument */
		defineVar(xsym, Xi, env);
		INCREMENT_NAMED(Xi);

		/* define .xname argument */
		xname_val = PROTECT(ScalarString(xnameChar));
		defineVar(xname, xname_val, enclos);
		INCREMENT_NAMED(xname_val);
		UNPROTECT(1);

		/* define .xpos argument */
		xpos_val = PROTECT(allocVector(INTSXP, depth + 1));

		for (R_xlen_t k = 0; k < (depth + 1); k++)
			SET_INTEGER_ELT(xpos_val, k, xloc[k]);

		defineVar(xpos, xpos_val, enclos);
		INCREMENT_NAMED(xpos_val);
		UNPROTECT(1);

		/* evaluate predicate */
		Rboolean doEval = TRUE;
		Rboolean matched = FALSE;

		if (strcmp(CHAR(STRING_ELT(classes, 0)), "ANY") == 0) /* ASCII */
			matched = TRUE;
		else
			matched = do_matchClass(Xi, classes);

		if (usePred)
		{
			/* set default to FALSE */
			doEval = FALSE;
			/* evaluate pred function call */
			SEXP predVal = PROTECT(R_forceAndCall(pcall, 1, env));
			if (isLogical(predVal) && xlength(predVal) == 1)
			{
				Rboolean predValBool = LOGICAL_ELT(predVal, 0);
				if (!(predValBool == NA_LOGICAL) && predValBool)
				{
					doEval = TRUE;
				}
			}
			UNPROTECT(1);
		}

		/* update current node evaluation info */
		int i1 = node[0];
		xinfo[i1 * 5 + 1] = doEval && matched;

		/* evaluate function call */
		if (doEval && matched)
		{
			/* update parent node info only for pruned list */
			if (howInt == 3)
			{
				int i2 = xinfo[i1 * 5 + 2];

				while (i1 != i2)
				{
					i1 = i2;
					xinfo[i1 * 5 + 1] = TRUE;
					i2 = xinfo[i1 * 5 + 2];
				}
			}

			/* update name attribute */
			SET_STRING_ELT(Xnames, node[0], xnameChar);

			if (useFun)
			{
				SEXP funVal = PROTECT(R_forceAndCall(fcall, 1, env));

				if (MAYBE_REFERENCED(funVal))
					funVal = lazy_duplicate(funVal);

				SET_VECTOR_ELT(Xflat, node[0], funVal);
				UNPROTECT(1);
			}
			else
			{
				SET_VECTOR_ELT(Xflat, node[0], lazy_duplicate(Xi));
			}
		}
	}
}

SEXP do_rrfill(SEXP Xflat, SEXP Xnames, SEXP Xi, int *xinfo, int maxNodes, int depth, int node, Rboolean useNames)
{
	R_xlen_t buf[maxNodes - node];
	int m = 0;
	for (R_xlen_t inode = node + 1; inode < maxNodes; inode++)
	{
		/* exit loop if smaller equal current depth */
		if (xinfo[inode * 5 + 3] <= depth)
			break;

		if (xinfo[inode * 5 + 3] == (depth + 1) && xinfo[inode * 5 + 1])
		{
			buf[m] = xinfo[inode * 5];
			m++;
		}
	}

	/* descend one level */
	if (m > 0)
	{
		/* populate sublist*/
		SEXP ans = PROTECT(allocVector(VECSXP, m));
		for (int j = 0; j < m; j++)
		{
			SET_VECTOR_ELT(ans, j, do_rrfill(Xflat, Xnames, VECTOR_ELT(Xi, xinfo[buf[j] * 5 + 4]), xinfo, maxNodes, depth + 1, buf[j], useNames));
		}

		/* add name attribute */
		if (useNames)
		{
			SEXP ansNames = PROTECT(allocVector(STRSXP, m));

			for (int j = 0; j < m; j++)
			{
				SET_STRING_ELT(ansNames, j, STRING_ELT(Xnames, buf[j]));
			}
			setAttrib(ans, R_NamesSymbol, ansNames);
			UNPROTECT(1);
		}
		/* copy other list attributes */
		copyMostAttrib(Xi, ans);

		UNPROTECT(1);
		return ans;
	}
	else
	{
		return VECTOR_ELT(Xflat, node);
	}
}

SEXP do_rrapply(SEXP env, SEXP enclos, SEXP X, SEXP FUN, SEXP PRED, SEXP classes, SEXP how, SEXP deflt, SEXP dfAsList)
{
	SEXP ans, names, xsym, xname, xpos, R_fcall, R_pcall;

	/* initialize arguments */
	int nprotect = 1;
	R_xlen_t n = xlength(X);
	names = getAttrib(X, R_NamesSymbol);
	Rboolean useFun = isFunction(FUN);
	Rboolean usePred = isFunction(PRED);
	Rboolean dfList = LOGICAL_ELT(dfAsList, 0);

	/* install arguments and initialize call objects */
	xsym = install("X");
	xname = install(".xname");
	xpos = install(".xpos");

	if (useFun)
	{
		R_fcall = PROTECT(lang3(FUN, xsym, R_DotsSymbol));
		nprotect++;
	}

	if (usePred)
	{
		R_pcall = PROTECT(lang3(PRED, xsym, R_DotsSymbol));
		nprotect++;
	}

	/* indices how:
	   0: replace (return nested list)
	   1: list (fill default, return nested list)
	   2: unlist (fill default, return nested list, unlist in R-function)
	   3: prune (return nested list)
	   4: flatten (prune, return flat list)    */

	int howInt = INTEGER_ELT(how, 0);

	if (howInt < 3) /* replace nodes or fill nodes by default values and return nested list */
	{
		/* allocate array to store location info */
		R_xlen_t maxDepth = 16;
		int *xloc = (int *)S_alloc(maxDepth, sizeof(int));

		/* allocate output list */
		if (howInt == 0)
		{
			ans = PROTECT(shallow_duplicate(X));
		}
		else
		{
			ans = PROTECT(allocVector(TYPEOF(X), n));
			do_copyAttrs(X, ans, names, TRUE);
		}

		/* traverse list to evaluate function calls */
		for (R_xlen_t i = 0; i < n; i++)
		{
			/* increment location counter */
			xloc[0] += 1;
			/* evaluate list element */
			SET_VECTOR_ELT(ans, i, do_rreval_list(env, enclos, VECTOR_ELT(X, i), R_fcall, R_pcall, classes, deflt, xname, xsym, xpos, 
						   isNull(names) ? NA_STRING : STRING_ELT(names, i), howInt, &xloc, 0, maxDepth, useFun, usePred, dfList));
		}

        /* restore initial .xpos and .xvar */
		defineVar(xname, findVar(xname, env), enclos);
		defineVar(xpos, findVar(xpos, env), enclos);

		UNPROTECT(nprotect);
		return ans;
	}
	else /* filter nodes and return nested list or filter nodes and return flat list */
	{
		SEXP Xnames, Xflat, ansNames;

		/* allocate array for additional location info */
		R_xlen_t maxNodes = 0;
		R_xlen_t maxDepth = 0;

		/* traverse list once for max nodes and max depth */
		int depth = do_rrcount(X, n, &maxNodes, &maxDepth, 0) + 1;

		/* allocate arrays to store location info */
		int *xloc = (int *)S_alloc(maxDepth + 1, sizeof(int));
		int *xinfo = (int *)S_alloc(maxNodes * 5, sizeof(int));
		int *node = (int *)S_alloc(1, sizeof(int));

		/* allocate flat list */
		Xflat = PROTECT(allocVector(VECSXP, maxNodes));
		/* allocate names vector (STRSXP initializes to "") */
		/* if names present on zero-th layer */
		Xnames = PROTECT(allocVector(STRSXP, maxNodes));
		nprotect += 2;

		for (R_xlen_t i = 0; i < n; i++)
		{
			/* increment location counter */
			xloc[0] += 1;
			/* update current node info */
			node[0] += (i > 0);
			do_updateNode(xinfo, node[0], FALSE, node[0], 0, i);
			/* store name attribute */
			if (!isNull(names))
				SET_STRING_ELT(Xnames, node[0], STRING_ELT(names, i));
			/* evaluate list element */
			do_rreval_flat(env, enclos, Xflat, Xnames, VECTOR_ELT(X, i), R_fcall, R_pcall,
						   classes, deflt, xname, xsym, xpos, isNull(names) ? NA_STRING : STRING_ELT(names, i),
						   xinfo, xloc, 0, node, node[0], useFun, usePred, dfList, howInt);
		}

		/* detect nodes to filter */
		R_xlen_t buf[maxNodes];
		int m = 0;
		for (R_xlen_t i = 0; i < maxNodes; i++)
		{
			/* doEval != 0 and depth == 0 only if howInt == 3 */
			if (xinfo[i * 5 + 1] && (howInt == 3 ? xinfo[i * 5 + 3] < 1 : TRUE))
			{
				buf[m] = xinfo[i * 5];
				m++;
			}
		}

		/* allocate pruned list */
		ans = PROTECT(allocVector(VECSXP, m));

		if (howInt == 3)
		{
			/* populate nested list */
			for (R_xlen_t j = 0; j < m; j++)
			{
				SET_VECTOR_ELT(ans, j, do_rrfill(Xflat, Xnames, VECTOR_ELT(X, xinfo[buf[j] * 5 + 4]), xinfo, maxNodes, 0, buf[j], !isNull(names)));
			}
			/* copy other list attributes */
			copyMostAttrib(X, ans);
		}
		else
		{
			/* populate flat list */
			for (R_xlen_t j = 0; j < m; j++)
			{
				SET_VECTOR_ELT(ans, j, VECTOR_ELT(Xflat, buf[j]));
			}
		}

		/* add names attribute */
		if (!isNull(names))
		{
			ansNames = PROTECT(allocVector(STRSXP, m));
			nprotect++;

			for (R_xlen_t j = 0; j < m; j++)
			{
				SET_STRING_ELT(ansNames, j, STRING_ELT(Xnames, buf[j]));
			}
			setAttrib(ans, R_NamesSymbol, ansNames);
		}

		/* restore initial .xpos and .xvar */
		defineVar(xname, findVar(xname, env), enclos);
		defineVar(xpos, findVar(xpos, env), enclos);

		UNPROTECT(nprotect);
		return ans;
	}
}
