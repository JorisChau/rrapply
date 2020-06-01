#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>
#include <string.h>

/* ---------------------- */

/* prototypes */
static void do_copyAttrs(SEXP obj, SEXP ans, SEXP names, Rboolean copyAttrs);
static int do_matchClass(SEXP obj, SEXP classes);
static R_xlen_t do_rrcount(SEXP X, R_xlen_t n, R_xlen_t *maxNodes, R_xlen_t *maxDepth, R_xlen_t depth);
static void do_updateNode(R_xlen_t *xinfo, R_xlen_t node, int doEval, R_xlen_t parent, R_xlen_t depth, R_xlen_t child);
static SEXP do_rreval_list(SEXP env, SEXP Xi, SEXP fcall, int fArgs, int fxname, int fxpos, SEXP pcall, int predArgs, int pxname, int pxpos, SEXP classes, SEXP deflt, SEXP xsym, SEXP xnameChar, int howInt, R_xlen_t **xloc, R_xlen_t depth, R_xlen_t maxDepth, int dfList, int feverywhere);
static void do_rreval_flat(SEXP env, SEXP Xflat, SEXP Xnames, SEXP Xi, SEXP fcall, int fArgs, int fxname, int fxpos, SEXP pcall, int predArgs, int pxname, int pxpos, SEXP classes, SEXP deflt, SEXP xsym, SEXP xnameChar, R_xlen_t *xinfo, R_xlen_t *xloc, R_xlen_t depth, R_xlen_t *node, R_xlen_t parent, int dfList, int howInt, int feverywhere);
static SEXP do_rrfill(SEXP Xflat, SEXP Xnames, SEXP Xi, R_xlen_t *xinfo, R_xlen_t maxNodes, R_xlen_t depth, R_xlen_t node, Rboolean useNames);
SEXP do_rrapply(SEXP env, SEXP X, SEXP FUN, SEXP argsFun, SEXP PRED, SEXP argsPred, SEXP classes, SEXP how, SEXP deflt, SEXP R_dfaslist, SEXP R_feverywhere);

/* ---------------------- */

/* Main function */

SEXP do_rrapply(SEXP env, SEXP X, SEXP FUN, SEXP argsFun, SEXP PRED, SEXP argsPred, SEXP classes, SEXP how, SEXP deflt, SEXP R_dfaslist, SEXP R_feverywhere)
{
	SEXP ans, names, xsym, xname, xpos, R_fcall, R_pcall;

	/* initialize arguments */
	int nprotect = 1;
	int fArgs = 0;
	int predArgs = 0;
	R_xlen_t n = Rf_xlength(X);
	names = PROTECT(Rf_getAttrib(X, R_NamesSymbol));
	int dfList = LOGICAL_ELT(R_dfaslist, 0);
	int feverywhere = LOGICAL_ELT(R_feverywhere, 0);

	/* install arguments and initialize call objects */
	xsym = Rf_install("X");
	xname = Rf_install(".xname");
	xpos = Rf_install(".xpos");
	int fxname = INTEGER_ELT(argsFun, 0) > 0;  /* .xname present in f */
	int fxpos = INTEGER_ELT(argsFun, 1) > 0;   /* .xpos present in f */
	int pxname = INTEGER_ELT(argsPred, 0) > 0; /* .xname present in condition */
	int pxpos = INTEGER_ELT(argsPred, 1) > 0;  /* .xpos present in condition */

	if (Rf_isFunction(FUN))
	{
		/* call definitions depend on presence of .xname and.xpos arguments */
		if (fxname && fxpos)
		{
			fArgs += 3;
			R_fcall = PROTECT(Rf_lang5(FUN, xsym, R_NilValue, R_NilValue, R_DotsSymbol));
			SET_TAG(CDDR(R_fcall), xname);
			SET_TAG(CDDDR(R_fcall), xpos);
		}
		else if (fxname)
		{
			fArgs += 2;
			R_fcall = PROTECT(Rf_lang4(FUN, xsym, R_NilValue, R_DotsSymbol));
			SET_TAG(CDDR(R_fcall), xname);
		}
		else if (fxpos)
		{
			fArgs += 2;
			R_fcall = PROTECT(Rf_lang4(FUN, xsym, R_NilValue, R_DotsSymbol));
			SET_TAG(CDDR(R_fcall), xpos);
		}
		else
		{
			fArgs += 1;
			R_fcall = PROTECT(Rf_lang3(FUN, xsym, R_DotsSymbol));
		}
		nprotect++;
	}
	else
	{
		R_fcall = FUN;
	}

	if (Rf_isFunction(PRED))
	{
		if (pxname && pxpos)
		{
			predArgs += 3;
			R_pcall = PROTECT(Rf_lang5(PRED, xsym, R_NilValue, R_NilValue, R_DotsSymbol));
			SET_TAG(CDDR(R_pcall), xname);
			SET_TAG(CDDDR(R_pcall), xpos);
		}
		else if (pxname)
		{
			predArgs += 2;
			R_pcall = PROTECT(Rf_lang4(PRED, xsym, R_NilValue, R_DotsSymbol));
			SET_TAG(CDDR(R_pcall), xname);
		}
		else if (pxpos)
		{
			predArgs += 2;
			R_pcall = PROTECT(Rf_lang4(PRED, xsym, R_NilValue, R_DotsSymbol));
			SET_TAG(CDDR(R_pcall), xpos);
		}
		else
		{
			predArgs += 1;
			R_pcall = PROTECT(Rf_lang3(PRED, xsym, R_DotsSymbol));
		}
		nprotect++;
	}
	else
	{
		R_pcall = PRED;
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
		R_xlen_t *xloc = (R_xlen_t *)S_alloc(maxDepth, sizeof(R_xlen_t));

		/* allocate output list */
		if (howInt == 0)
		{
			ans = PROTECT(Rf_shallow_duplicate(X));
		}
		else
		{
			ans = PROTECT(Rf_allocVector((SEXPTYPE)TYPEOF(X), n));
			do_copyAttrs(X, ans, names, TRUE);
		}
		nprotect++;

		/* traverse list to evaluate function calls */
		for (R_xlen_t i = 0; i < n; i++)
		{
			/* increment location counter */
			xloc[0] += 1;
			/* evaluate list element */
			SET_VECTOR_ELT(ans, i, do_rreval_list(env, VECTOR_ELT(X, i), R_fcall, fArgs, fxname, fxpos, R_pcall, predArgs, 
											      pxname, pxpos, classes, deflt, xsym, Rf_isNull(names) ? NA_STRING : STRING_ELT(names, i), 
												  howInt, &xloc, 0, maxDepth, dfList, feverywhere));
		}

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
		R_xlen_t depth = do_rrcount(X, n, &maxNodes, &maxDepth, 0) + 1;
		/* reset current depth to zero */
		depth = 0;

		/* allocate arrays to store location info */
		R_xlen_t *xloc = (R_xlen_t *)S_alloc(maxDepth + 1, sizeof(R_xlen_t));
		R_xlen_t *xinfo = (R_xlen_t *)S_alloc(maxNodes * 5, sizeof(R_xlen_t));
		R_xlen_t *node = (R_xlen_t *)S_alloc(1, sizeof(R_xlen_t));

		/* allocate flat list */
		Xflat = PROTECT(Rf_allocVector(VECSXP, maxNodes));
		/* allocate names vector (STRSXP initializes to "") */
		/* if names present on zero-th layer */
		Xnames = PROTECT(Rf_allocVector(STRSXP, maxNodes));
		nprotect += 2;

		for (R_xlen_t i = 0; i < n; i++)
		{
			/* increment location counter */
			xloc[0] += 1;
			/* update current node info */
			node[0] += (i > 0);
			do_updateNode(xinfo, node[0], FALSE, node[0], 0, i);
			/* store name attribute */
			if (!Rf_isNull(names))
				SET_STRING_ELT(Xnames, node[0], STRING_ELT(names, i));
			/* evaluate list element */
			do_rreval_flat(env, Xflat, Xnames, VECTOR_ELT(X, i), R_fcall, fArgs, fxname, fxpos, R_pcall, 
						   predArgs, pxname, pxpos, classes, deflt, xsym, Rf_isNull(names) ? NA_STRING : STRING_ELT(names, i),
						   xinfo, xloc, depth, node, node[0], dfList, howInt, feverywhere);
		}

		/* detect nodes to filter */
		R_xlen_t buf[maxNodes];
		R_xlen_t m = 0;
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
		ans = PROTECT(Rf_allocVector(VECSXP, m));
		nprotect++;

		if (howInt == 3)
		{
			/* populate nested list */
			for (R_xlen_t j = 0; j < m; j++)
			{
				SET_VECTOR_ELT(ans, j, do_rrfill(Xflat, Xnames, VECTOR_ELT(X, xinfo[buf[j] * 5 + 4]), xinfo, maxNodes, 0, buf[j], !Rf_isNull(names)));
			}
			/* copy other list attributes */
			Rf_copyMostAttrib(X, ans);
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
		if (!Rf_isNull(names))
		{
			ansNames = PROTECT(Rf_allocVector(STRSXP, m));
			nprotect++;

			for (R_xlen_t j = 0; j < m; j++)
			{
				SET_STRING_ELT(ansNames, j, STRING_ELT(Xnames, buf[j]));
			}
			Rf_setAttrib(ans, R_NamesSymbol, ansNames);
		}

		UNPROTECT(nprotect);
		return ans;
	}
}

/* Helper functions */

/* copies only name attribute or all attributes */
static void do_copyAttrs(SEXP obj, SEXP ans, SEXP names, Rboolean copyAttrs)
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
static int do_matchClass(SEXP obj, SEXP classes)
{
	SEXP klass = PROTECT(Rf_getAttrib(obj, R_ClassSymbol));
	int n = (int)Rf_xlength(klass);

	int matched = FALSE;
	/* match classes to R_ClassSymbol attribute */
	if (n > 0)
	{
		for (int i = 0; i < n; i++)
			for (int j = 0; j < Rf_xlength(classes); j++)
				if (strcmp(CHAR(STRING_ELT(klass, i)), CHAR(STRING_ELT(classes, j))) == 0)
					matched = 1;
	}
	else
	{
		/* match to specific types */
		SEXP dim = PROTECT(Rf_getAttrib(obj, R_DimSymbol));
		int nd = (int)Rf_xlength(dim);
		if (nd > 0)
		{
			if (nd == 2)
			{
				for (int j = 0; j < Rf_xlength(classes); j++)
					if (strcmp(CHAR(STRING_ELT(classes, j)), "matrix") == 0 ||
						strcmp(CHAR(STRING_ELT(classes, j)), "array") == 0)
						matched = TRUE;
			}
			else
			{
				for (int j = 0; j < Rf_xlength(classes); j++)
					if (strcmp(CHAR(STRING_ELT(classes, j)), "array") == 0)
						matched = TRUE;
			}
		}
		else
		{
			SEXPTYPE type = (SEXPTYPE)TYPEOF(obj);
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
				typename = CHAR(Rf_type2str(type));
			}

			for (int j = 0; j < Rf_xlength(classes); j++)
				if (strcmp(CHAR(STRING_ELT(classes, j)), typename) == 0)
					matched = TRUE;
		}
		UNPROTECT(1);
	}
	UNPROTECT(1);
	return matched;
}

static R_xlen_t do_rrcount(SEXP X, R_xlen_t n, R_xlen_t *maxNodes, R_xlen_t *maxDepth, R_xlen_t depth)
{
	SEXP Xi;
	for (R_xlen_t i = 0; i < n; i++)
	{
		*maxNodes += 1;
		Xi = VECTOR_ELT(X, i);
		/* descend one level */
		if (Rf_isVectorList(Xi))
		{
			depth += 1;
			/* increment maxDepth if current depth is higher than maxDepth */
			*maxDepth += depth > *maxDepth;
			depth = do_rrcount(Xi, Rf_xlength(Xi), maxNodes, maxDepth, depth);
		}
	}
	return depth - 1;
}

static void do_updateNode(R_xlen_t *xinfo, R_xlen_t node, int doEval, R_xlen_t parent, R_xlen_t depth, R_xlen_t child)
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

static SEXP do_rreval_list(SEXP env, SEXP Xi, SEXP fcall, int fArgs, int fxname, int fxpos,
						   SEXP pcall, int predArgs, int pxname, int pxpos, SEXP classes,
						   SEXP deflt, SEXP xsym, SEXP xnameChar,  int howInt, R_xlen_t **xloc, 
						   R_xlen_t depth, R_xlen_t maxDepth, int dfList, int feverywhere)
{
	SEXP funVal;

	/* if Xi is list (and data.frame is treated as list) recurse, otherwise call function */
	Rboolean doRecurse = FALSE;

	if (!feverywhere && Rf_isVectorList(Xi))
	{
		doRecurse = TRUE;
		if (!dfList)
		{
			SEXP df = PROTECT(Rf_ScalarString(Rf_mkChar("data.frame")));
			if (do_matchClass(Xi, df))
				doRecurse = FALSE;
			UNPROTECT(1);
		}
	}

	if (!doRecurse)
	{
		SEXP xname_val, xpos_val;

		/* define X argument */
		Rf_defineVar(xsym, Xi, env);
		INCREMENT_NAMED(Xi);

		if (fxname > 0 || pxname > 0)
		{
			/* update current .xname value */
			xname_val = PROTECT(Rf_ScalarString(xnameChar));
			if (fArgs > 1 && fxname > 0)
				SETCADDR(fcall, xname_val);
			if (predArgs > 1 && pxname > 0)
				SETCADDR(pcall, xname_val);
			UNPROTECT(1);
		}

		if (fxpos > 0 || pxpos > 0)
		{
			/* update current .xpos value */
			xpos_val = PROTECT(Rf_allocVector(INTSXP, depth + 1));
			for (R_xlen_t k = 0; k < (depth + 1); k++)
				SET_INTEGER_ELT(xpos_val, k, (int)((*xloc)[k]));

			if (fArgs > 1 && fxpos > 0)
			{
				if (fxname > 0)
					SETCADDDR(fcall, xpos_val);
				else
					SETCADDR(fcall, xpos_val);
			}
			if (predArgs > 1 && pxpos > 0)
			{
				if (pxname > 0)
					SETCADDDR(pcall, xpos_val);
				else
					SETCADDR(pcall, xpos_val);
			}
			UNPROTECT(1);
		}

		/* evaluate predicate */
		int doEval = TRUE;
		int matched = FALSE;

		if (strcmp(CHAR(STRING_ELT(classes, 0)), "ANY") == 0) /* ASCII */
			matched = TRUE;
		else
			matched = do_matchClass(Xi, classes);

		if (predArgs > 0)
		{
			/* set default to FALSE */
			doEval = FALSE;

			/* evaluate pred function call */
			SEXP predVal = PROTECT(R_forceAndCall(pcall, predArgs, env));
			if (Rf_isLogical(predVal) && Rf_xlength(predVal) == 1)
			{
				int predValBool = LOGICAL_ELT(predVal, 0);
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
			if (fArgs > 0)
			{
				funVal = PROTECT(R_forceAndCall(fcall, fArgs, env));
				if (MAYBE_REFERENCED(funVal))
					funVal = Rf_lazy_duplicate(funVal);
				UNPROTECT(1);
				return funVal;
			}
			else
			{
				return Rf_lazy_duplicate(Xi);
			}
		}
		else if (feverywhere && Rf_isVectorList(Xi))
		{
			/* recurse further if feverywhere and Xi is a list */
			doRecurse = TRUE;
		}
		else
		{
			/* replace Xi in list*/
			if (howInt == 0)
			{
				return Rf_lazy_duplicate(Xi);
			}
			else /* fill list by default */
			{
				return Rf_lazy_duplicate(deflt);
			}
		}
	}

	if (doRecurse)
	{
		/* descend one level */
		depth += 1;
		R_xlen_t m = Rf_xlength(Xi);
		SEXP names = PROTECT(Rf_getAttrib(Xi, R_NamesSymbol));

		if (howInt == 0)
		{
			funVal = PROTECT(Rf_shallow_duplicate(Xi));
		}
		else
		{
			/* VECEXP initializes with R_NilValues */
			funVal = PROTECT(Rf_allocVector(VECSXP, m));
			do_copyAttrs(Xi, funVal, names, TRUE);
		}

		/* reallocate array if max is reached */
		if (depth == maxDepth)
		{
			maxDepth *= 2;
			*xloc = (R_xlen_t *)S_realloc((char *)*xloc, maxDepth, depth, sizeof(R_xlen_t));
		}

		for (R_xlen_t j = 0; j < m; j++)
		{
			/* increment location */
			(*xloc)[depth] = j + 1;

			/* evaluate list element */
			SET_VECTOR_ELT(funVal, j, do_rreval_list(env, VECTOR_ELT(Xi, j), fcall, fArgs, fxname, fxpos, pcall, 
													 predArgs, pxname, pxpos, classes, deflt, xsym, 
													 Rf_isNull(names) ? NA_STRING : STRING_ELT(names, j), howInt, 
													 xloc, depth, maxDepth, dfList, feverywhere));
		}

		UNPROTECT(2);
		return funVal;
	}
	else
	{
		/* default return value */
		return R_NilValue;
	}
}

static void do_rreval_flat(SEXP env, SEXP Xflat, SEXP Xnames, SEXP Xi, SEXP fcall, int fArgs, int fxname, int fxpos,
						   SEXP pcall, int predArgs, int pxname, int pxpos, SEXP classes, SEXP deflt, SEXP xsym, 
						   SEXP xnameChar,  R_xlen_t *xinfo, R_xlen_t *xloc, R_xlen_t depth, R_xlen_t *node, 
						   R_xlen_t parent, int dfList, int howInt, int feverywhere)
{
	/* if Xi is list (and data.frame is treated as list) recurse, otherwise evaluate function calls */
	Rboolean doRecurse = FALSE;

	if (!feverywhere && Rf_isVectorList(Xi))
	{
		doRecurse = TRUE;
		if (!dfList)
		{
			SEXP df = PROTECT(Rf_ScalarString(Rf_mkChar("data.frame")));
			if (do_matchClass(Xi, df))
				doRecurse = FALSE;
			UNPROTECT(1);
		}
	}

	if (!doRecurse)
	{
		SEXP xname_val, xpos_val;

		/* define X argument */
		Rf_defineVar(xsym, Xi, env);
		INCREMENT_NAMED(Xi);

		if (fxname > 0 || pxname > 0)
		{
			/* update current .xname value */
			xname_val = PROTECT(Rf_ScalarString(xnameChar));
			if (fArgs > 1 && fxname > 0)
				SETCADDR(fcall, xname_val);
			if (predArgs > 1 && pxname > 0)
				SETCADDR(pcall, xname_val);
			UNPROTECT(1);
		}

		if (fxpos > 0 || pxpos > 0)
		{
			/* update current .xpos value */
			xpos_val = PROTECT(Rf_allocVector(INTSXP, depth + 1));
			for (R_xlen_t k = 0; k < (depth + 1); k++)
				SET_INTEGER_ELT(xpos_val, k, (int)(xloc[k]));

			if (fArgs > 1 && fxpos > 0)
			{
				if (fxname > 0)
					SETCADDDR(fcall, xpos_val);
				else
					SETCADDR(fcall, xpos_val);
			}
			if (predArgs > 1 && pxpos > 0)
			{
				if (pxname > 0)
					SETCADDDR(pcall, xpos_val);
				else
					SETCADDR(pcall, xpos_val);
			}
			UNPROTECT(1);
		}

		/* evaluate predicate */
		int doEval = TRUE;
		int matched = FALSE;

		if (strcmp(CHAR(STRING_ELT(classes, 0)), "ANY") == 0) /* ASCII */
			matched = TRUE;
		else
			matched = do_matchClass(Xi, classes);

		if (predArgs > 0)
		{
			/* set default to FALSE */
			doEval = FALSE;

			/* evaluate pred function call */
			SEXP predVal = PROTECT(R_forceAndCall(pcall, predArgs, env));
			if (Rf_isLogical(predVal) && Rf_xlength(predVal) == 1)
			{
				int predValBool = LOGICAL_ELT(predVal, 0);
				if (!(predValBool == NA_LOGICAL) && predValBool)
				{
					doEval = TRUE;
				}
			}
			UNPROTECT(1);
		}

		/* update current node evaluation info */
		R_xlen_t i1 = node[0];
		xinfo[i1 * 5 + 1] = doEval && matched;

		if (doEval && matched)
		{
			/* update parent node info only for pruned list */
			if (howInt == 3)
			{
				R_xlen_t i2 = xinfo[i1 * 5 + 2];

				while (i1 != i2)
				{
					i1 = i2;
					xinfo[i1 * 5 + 1] = TRUE;
					i2 = xinfo[i1 * 5 + 2];
				}
			}

			/* update name attribute */
			SET_STRING_ELT(Xnames, node[0], xnameChar);

			if (fArgs > 0)
			{
				/* evaluate function call */
				SEXP funVal = PROTECT(R_forceAndCall(fcall, fArgs, env));
				if (MAYBE_REFERENCED(funVal))
					funVal = Rf_lazy_duplicate(funVal);
				SET_VECTOR_ELT(Xflat, node[0], funVal);
				UNPROTECT(1);
			}
			else
			{
				SET_VECTOR_ELT(Xflat, node[0], Rf_lazy_duplicate(Xi));
			}
		}
		else if (feverywhere && Rf_isVectorList(Xi))
		{
			/* otherwise recurse further if feverywhere */
			doRecurse = TRUE;
		}
	}

	if (doRecurse)
	{
		/* descend one level */
		depth += 1;
		parent = node[0];
		R_xlen_t m = Rf_xlength(Xi);
		SEXP names = PROTECT(Rf_getAttrib(Xi, R_NamesSymbol));

		for (R_xlen_t j = 0; j < m; j++)
		{
			/* increment location */
			xloc[depth] = j + 1;

			/* update current node info */
			node[0] += 1;
			do_updateNode(xinfo, node[0], FALSE, parent, depth, j);

			/* update name attribute */
			if (!Rf_isNull(names))
				SET_STRING_ELT(Xnames, node[0], STRING_ELT(names, j));

			/* evaluate list element */
			do_rreval_flat(env, Xflat, Xnames, VECTOR_ELT(Xi, j), fcall, fArgs, fxname, fxpos, pcall, predArgs,
			               pxname, pxpos, classes, deflt, xsym, Rf_isNull(names) ? NA_STRING : STRING_ELT(names, j),
						   xinfo, xloc, depth, node, parent, dfList, howInt, feverywhere);
		}
		UNPROTECT(1);
	}
}

static SEXP do_rrfill(SEXP Xflat, SEXP Xnames, SEXP Xi, R_xlen_t *xinfo, R_xlen_t maxNodes, R_xlen_t depth, R_xlen_t node, Rboolean useNames)
{
	R_xlen_t buf[maxNodes - node];
	R_xlen_t m = 0;
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
		SEXP ans = PROTECT(Rf_allocVector(VECSXP, m));
		for (R_xlen_t j = 0; j < m; j++)
		{
			SET_VECTOR_ELT(ans, j, do_rrfill(Xflat, Xnames, VECTOR_ELT(Xi, xinfo[buf[j] * 5 + 4]), xinfo, maxNodes, depth + 1, buf[j], useNames));
		}

		/* add name attribute */
		if (useNames)
		{
			SEXP ansNames = PROTECT(Rf_allocVector(STRSXP, m));

			for (R_xlen_t j = 0; j < m; j++)
			{
				SET_STRING_ELT(ansNames, j, STRING_ELT(Xnames, buf[j]));
			}
			Rf_setAttrib(ans, R_NamesSymbol, ansNames);
			UNPROTECT(1);
		}
		/* copy other list attributes */
		Rf_copyMostAttrib(Xi, ans);

		UNPROTECT(1);
		return ans;
	}
	else
	{
		return VECTOR_ELT(Xflat, node);
	}
}
