#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>
#include <string.h>

/* ---------------------- */

/* structures */

struct intArgs
{
	/* indices how:
		0: replace (return nested list)
		1: list (fill default, return nested list)
		2: unlist (fill default, return nested list, unlist in R-function)
		3: prune (return nested list)
		4: flatten (prune, return flat list)    
	*/

	int how_C;		 // how argument
	int fArgs;		 // number of arguments f
	int fxname;		 // .xname present in f
	int fxpos;		 // .xpos present in f
	int pArgs;		 // number of arguments condition
	int pxname;		 // .xname present in condition
	int pxpos;		 // .xpos present in condition
	int dfaslist;	 // dfaslist argument
	int feverywhere; // feverywhere argument
};

/* prototypes */

static void do_copyAttrs(SEXP obj, SEXP ans, SEXP names, Rboolean copyAttrs);
static int do_matchClass(SEXP obj, SEXP classes);
static R_xlen_t do_rrcount(SEXP X, R_xlen_t n, R_xlen_t *maxNodes, R_xlen_t *maxDepth, R_xlen_t depth);
static void do_updateNode(R_xlen_t (*xinfo)[4], R_xlen_t node, int doEval, R_xlen_t parent, R_xlen_t depth, R_xlen_t child);
static SEXP do_rreval_list(SEXP env, SEXP Xi, SEXP fcall, SEXP pcall, struct intArgs args, SEXP classes, SEXP deflt, SEXP xsym, SEXP xnameChar, R_xlen_t **xloc, R_xlen_t depth, R_xlen_t maxDepth);
static void do_rreval_flat(SEXP env, SEXP Xflat, SEXP Xnames, SEXP Xi, SEXP fcall, SEXP pcall, struct intArgs args, SEXP classes, SEXP deflt, SEXP xsym, SEXP xnameChar, R_xlen_t (*xinfo)[4], R_xlen_t *xloc, R_xlen_t depth, R_xlen_t *node, R_xlen_t parent);
static SEXP do_rrfill(SEXP Xflat, SEXP Xnames, SEXP Xi, R_xlen_t (*xinfo)[4], R_xlen_t *buf, R_xlen_t maxNodes, R_xlen_t depth, R_xlen_t node, R_xlen_t ibuf, Rboolean useNames);
SEXP do_rrapply(SEXP env, SEXP X, SEXP FUN, SEXP argsFun, SEXP PRED, SEXP argsPred, SEXP classes, SEXP how, SEXP deflt, SEXP R_dfaslist, SEXP R_feverywhere);

/* ---------------------- */

/* Main function */

SEXP do_rrapply(SEXP env, SEXP X, SEXP FUN, SEXP argsFun, SEXP PRED, SEXP argsPred, SEXP classes, SEXP R_how, SEXP deflt, SEXP R_dfaslist, SEXP R_feverywhere)
{
	SEXP ans, names, xsym, xname, xpos, R_fcall, R_pcall;

	/* integer arguments */
	struct intArgs R_args;
	R_args.how_C = INTEGER_ELT(R_how, 0);
	R_args.fArgs = 0;
	R_args.fxname = INTEGER_ELT(argsFun, 0) > 0;
	R_args.fxpos = INTEGER_ELT(argsFun, 1) > 0;
	R_args.pArgs = 0;
	R_args.pxname = INTEGER_ELT(argsPred, 0) > 0;
	R_args.pxpos = INTEGER_ELT(argsPred, 1) > 0;
	R_args.dfaslist = LOGICAL_ELT(R_dfaslist, 0);
	R_args.feverywhere = LOGICAL_ELT(R_feverywhere, 0);

	/* other arguments */
	R_xlen_t n = Rf_xlength(X);
	names = PROTECT(Rf_getAttrib(X, R_NamesSymbol));
	int nprotect = 1;

	/* install arguments and initialize call objects */
	xsym = Rf_install("X");
	xname = Rf_install(".xname");
	xpos = Rf_install(".xpos");

	if (Rf_isFunction(FUN))
	{
		/* call definitions depend on presence of .xname and.xpos arguments */
		if (R_args.fxname && R_args.fxpos)
		{
			R_args.fArgs += 3;
			R_fcall = PROTECT(Rf_lang5(FUN, xsym, R_NilValue, R_NilValue, R_DotsSymbol));
			SET_TAG(CDDR(R_fcall), xname);
			SET_TAG(CDDDR(R_fcall), xpos);
		}
		else if (R_args.fxname)
		{
			R_args.fArgs += 2;
			R_fcall = PROTECT(Rf_lang4(FUN, xsym, R_NilValue, R_DotsSymbol));
			SET_TAG(CDDR(R_fcall), xname);
		}
		else if (R_args.fxpos)
		{
			R_args.fArgs += 2;
			R_fcall = PROTECT(Rf_lang4(FUN, xsym, R_NilValue, R_DotsSymbol));
			SET_TAG(CDDR(R_fcall), xpos);
		}
		else
		{
			R_args.fArgs += 1;
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
		if (R_args.pxname && R_args.pxpos)
		{
			R_args.pArgs += 3;
			R_pcall = PROTECT(Rf_lang5(PRED, xsym, R_NilValue, R_NilValue, R_DotsSymbol));
			SET_TAG(CDDR(R_pcall), xname);
			SET_TAG(CDDDR(R_pcall), xpos);
		}
		else if (R_args.pxname)
		{
			R_args.pArgs += 2;
			R_pcall = PROTECT(Rf_lang4(PRED, xsym, R_NilValue, R_DotsSymbol));
			SET_TAG(CDDR(R_pcall), xname);
		}
		else if (R_args.pxpos)
		{
			R_args.pArgs += 2;
			R_pcall = PROTECT(Rf_lang4(PRED, xsym, R_NilValue, R_DotsSymbol));
			SET_TAG(CDDR(R_pcall), xpos);
		}
		else
		{
			R_args.pArgs += 1;
			R_pcall = PROTECT(Rf_lang3(PRED, xsym, R_DotsSymbol));
		}
		nprotect++;
	}
	else
	{
		R_pcall = PRED;
	}

	if (R_args.how_C < 3) /* replace nodes or fill nodes by default values and return nested list */
	{
		/* allocate array to store location info */
		R_xlen_t maxDepth = 16;
		R_xlen_t *xloc = (R_xlen_t *)S_alloc(maxDepth, sizeof(R_xlen_t));

		/* allocate output list */
		if (R_args.how_C == 0)
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
			SET_VECTOR_ELT(ans, i, do_rreval_list(env, VECTOR_ELT(X, i), R_fcall, R_pcall, R_args, classes, deflt, xsym, Rf_isNull(names) ? NA_STRING : STRING_ELT(names, i), &xloc, 0, maxDepth));
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
		R_xlen_t *inode = (R_xlen_t *)S_alloc(1, sizeof(R_xlen_t));
		R_xlen_t(*xinfo)[4];
		xinfo = (R_xlen_t(*)[4])S_alloc(maxNodes, sizeof(*xinfo));

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
			inode[0] += (i > 0);
			do_updateNode(xinfo, inode[0], FALSE, inode[0], 0, i);
			/* store name attribute */
			if (!Rf_isNull(names))
				SET_STRING_ELT(Xnames, inode[0], STRING_ELT(names, i));
			/* evaluate list element */
			do_rreval_flat(env, Xflat, Xnames, VECTOR_ELT(X, i), R_fcall, R_pcall, R_args, classes, deflt, xsym, Rf_isNull(names) ? NA_STRING : STRING_ELT(names, i), xinfo, xloc, depth, inode, inode[0]);
		}

		/* detect nodes to filter */
		R_xlen_t *buf = (R_xlen_t *)R_alloc((size_t)maxNodes, sizeof(R_xlen_t));
		R_xlen_t m = 0;
		for (R_xlen_t i = 0; i < maxNodes; i++)
		{
			/* doEval != 0 and depth == 0 only if howInt == 3 */
			if (xinfo[i][0] && (R_args.how_C == 3 ? xinfo[i][2] < 1 : TRUE))
			{
				buf[m] = i;
				m++;
			}
		}

		/* allocate pruned list */
		ans = PROTECT(Rf_allocVector(VECSXP, m));
		nprotect++;

		if (R_args.how_C == 3)
		{
			/* populate nested list */
			for (R_xlen_t j = 0; j < m; j++)
			{
				SET_VECTOR_ELT(ans, j, do_rrfill(Xflat, Xnames, VECTOR_ELT(X, xinfo[buf[j]][3]), xinfo, buf, maxNodes, 0, buf[j], m, !Rf_isNull(names)));
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

static void do_updateNode(R_xlen_t (*xinfo)[4], R_xlen_t node, int doEval, R_xlen_t parent, R_xlen_t depth, R_xlen_t child)
{
	/* evaluate node */
	xinfo[node][0] = doEval;
	/* parent node */
	xinfo[node][1] = parent;
	/* depth layer */
	xinfo[node][2] = depth;
	/* child counter */
	xinfo[node][3] = child;
}

static SEXP do_rreval_list(
	SEXP env,			 // evaluation environment
	SEXP Xi,			 // current list layer content
	SEXP fcall,			 // f function call
	SEXP pcall,			 // condition function call
	struct intArgs args, // integer arguments
	SEXP classes,		 // classes argument
	SEXP deflt,			 // deflt argument
	SEXP xsym,			 // principal argument symbol
	SEXP xnameChar,		 // current value .xname argument
	R_xlen_t **xloc,	 // current value .pos argument
	R_xlen_t depth,		 // current depth
	R_xlen_t maxDepth	 // maximum depth
)
{
	SEXP funVal;

	/* if Xi is list (and data.frame is treated as list) recurse, otherwise call function */
	Rboolean doRecurse = FALSE;

	if (!args.feverywhere && Rf_isVectorList(Xi))
	{
		doRecurse = TRUE;
		if (!args.dfaslist)
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

		if (args.fxname > 0 || args.pxname > 0)
		{
			/* update current .xname value */
			xname_val = PROTECT(Rf_ScalarString(xnameChar));
			if (args.fArgs > 1 && args.fxname > 0)
				SETCADDR(fcall, xname_val);
			if (args.pArgs > 1 && args.pxname > 0)
				SETCADDR(pcall, xname_val);
			UNPROTECT(1);
		}

		if (args.fxpos > 0 || args.pxpos > 0)
		{
			/* update current .xpos value */
			xpos_val = PROTECT(Rf_allocVector(INTSXP, depth + 1));
			for (R_xlen_t k = 0; k < (depth + 1); k++)
				SET_INTEGER_ELT(xpos_val, k, (int)((*xloc)[k]));

			if (args.fArgs > 1 && args.fxpos > 0)
			{
				if (args.fxname > 0)
					SETCADDDR(fcall, xpos_val);
				else
					SETCADDR(fcall, xpos_val);
			}
			if (args.pArgs > 1 && args.pxpos > 0)
			{
				if (args.pxname > 0)
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

		if (args.pArgs > 0)
		{
			/* set default to FALSE */
			doEval = FALSE;

			/* evaluate pred function call */
			SEXP predVal = PROTECT(R_forceAndCall(pcall, args.pArgs, env));
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
			if (args.fArgs > 0)
			{
				funVal = PROTECT(R_forceAndCall(fcall, args.fArgs, env));
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
		else if (args.feverywhere && Rf_isVectorList(Xi))
		{
			/* recurse further if feverywhere and Xi is a list */
			doRecurse = TRUE;
		}
		else
		{
			/* replace Xi in list*/
			if (args.how_C == 0)
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

		if (args.how_C == 0)
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
			SET_VECTOR_ELT(funVal, j, do_rreval_list(env, VECTOR_ELT(Xi, j), fcall, pcall, args, classes, deflt, xsym, Rf_isNull(names) ? NA_STRING : STRING_ELT(names, j), xloc, depth, maxDepth));
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

static void do_rreval_flat(
	SEXP env,			  // evaluation environment
	SEXP Xflat,			  // flat list with node content
	SEXP Xnames,		  // flat list of node names
	SEXP Xi,			  // current list layer
	SEXP fcall,			  // f function call
	SEXP pcall,			  // condition function call
	struct intArgs args,  // integer arguments
	SEXP classes,		  // classes argument
	SEXP deflt,			  // deflt argument
	SEXP xsym,			  // principal argument symbol
	SEXP xnameChar,		  // current .xname value
	R_xlen_t (*xinfo)[4], // array with node position information
	R_xlen_t *xloc,		  // current .xpos value
	R_xlen_t depth,		  // current depth
	R_xlen_t *node,		  // current node
	R_xlen_t parent		  // parent of current node
)
{
	/* if Xi is list (and data.frame is treated as list) recurse, otherwise evaluate function calls */
	Rboolean doRecurse = FALSE;

	if (!args.feverywhere && Rf_isVectorList(Xi))
	{
		doRecurse = TRUE;
		if (!args.dfaslist)
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

		if (args.fxname > 0 || args.pxname > 0)
		{
			/* update current .xname value */
			xname_val = PROTECT(Rf_ScalarString(xnameChar));
			if (args.fArgs > 1 && args.fxname > 0)
				SETCADDR(fcall, xname_val);
			if (args.pArgs > 1 && args.pxname > 0)
				SETCADDR(pcall, xname_val);
			UNPROTECT(1);
		}

		if (args.fxpos > 0 || args.pxpos > 0)
		{
			/* update current .xpos value */
			xpos_val = PROTECT(Rf_allocVector(INTSXP, depth + 1));
			for (R_xlen_t k = 0; k < (depth + 1); k++)
				SET_INTEGER_ELT(xpos_val, k, (int)(xloc[k]));

			if (args.fArgs > 1 && args.fxpos > 0)
			{
				if (args.fxname > 0)
					SETCADDDR(fcall, xpos_val);
				else
					SETCADDR(fcall, xpos_val);
			}
			if (args.pArgs > 1 && args.pxpos > 0)
			{
				if (args.pxname > 0)
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

		if (args.pArgs > 0)
		{
			/* set default to FALSE */
			doEval = FALSE;

			/* evaluate pred function call */
			SEXP predVal = PROTECT(R_forceAndCall(pcall, args.pArgs, env));
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
		xinfo[i1][0] = doEval && matched;

		if (doEval && matched)
		{
			/* update parent node info only for pruned list */
			if (args.how_C == 3)
			{
				R_xlen_t i2 = xinfo[i1][1];

				while (i1 != i2)
				{
					i1 = i2;
					xinfo[i1][0] = TRUE;
					i2 = xinfo[i1][1];
				}
			}

			/* update name attribute */
			SET_STRING_ELT(Xnames, node[0], xnameChar);

			if (args.fArgs > 0)
			{
				/* evaluate function call */
				SEXP funVal = PROTECT(R_forceAndCall(fcall, args.fArgs, env));
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
		else if (args.feverywhere && Rf_isVectorList(Xi))
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
			do_rreval_flat(env, Xflat, Xnames, VECTOR_ELT(Xi, j), fcall, pcall, args, classes, deflt, xsym, Rf_isNull(names) ? NA_STRING : STRING_ELT(names, j), xinfo, xloc, depth, node, parent);
		}
		UNPROTECT(1);
	}
}

static SEXP do_rrfill(SEXP Xflat, SEXP Xnames, SEXP Xi, R_xlen_t (*xinfo)[4], R_xlen_t *buf, R_xlen_t maxNodes, R_xlen_t depth, R_xlen_t node, R_xlen_t ibuf, Rboolean useNames)
{
	R_xlen_t m = 0;
	for (R_xlen_t inode = node + 1; inode < maxNodes; inode++)
	{
		/* exit loop if smaller equal current depth */
		if (xinfo[inode][2] <= depth)
			break;

		if (xinfo[inode][2] == (depth + 1) && xinfo[inode][0])
		{
			buf[ibuf + m] = inode;
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
			SET_VECTOR_ELT(ans, j, do_rrfill(Xflat, Xnames, VECTOR_ELT(Xi, xinfo[buf[ibuf + j]][3]), xinfo, buf, maxNodes, depth + 1, buf[ibuf + j], ibuf + m, useNames));
		}

		/* add name attribute */
		if (useNames)
		{
			SEXP ansNames = PROTECT(Rf_allocVector(STRSXP, m));

			for (R_xlen_t j = 0; j < m; j++)
			{
				SET_STRING_ELT(ansNames, j, STRING_ELT(Xnames, buf[ibuf + j]));
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
