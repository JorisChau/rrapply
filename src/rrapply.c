#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>
#include <string.h>

/* ---------------------- */

/* structures */

typedef struct Args
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
} Args;

typedef struct Depth
{
	int current;	   // current depth
	int max;		   // maximum allowed depth
	R_xlen_t maxnodes; // counter maximum nodes
} Depth;

/* prototypes */

static void C_copyAttrs(SEXP obj, SEXP ans, SEXP names, Rboolean copyAttrs);
static int C_matchClass(SEXP obj, SEXP classes);
static void C_traverse(SEXP X, Depth *depth, R_xlen_t n);
static SEXP C_eval_list(SEXP env, SEXP Xi, SEXP fcall, SEXP pcall, SEXP classes, SEXP deflt, SEXP xsym, SEXP xnameChar, Args args, Depth depth, R_xlen_t (*xinfo)[3], R_xlen_t *xloc, R_xlen_t *node, R_xlen_t parent);
static void C_eval_flat(SEXP env, SEXP Xflat, SEXP Xnames, SEXP Xi, SEXP fcall, SEXP pcall, SEXP classes, SEXP deflt, SEXP xsym, SEXP xnameChar, Args args, Depth depth, R_xlen_t (*xinfo)[3], R_xlen_t *xloc, R_xlen_t *node, R_xlen_t parent);
static SEXP C_fill_list(SEXP Xi, R_xlen_t (*xinfo)[3], R_xlen_t *buf, R_xlen_t node, R_xlen_t maxnodes, R_xlen_t ibuf);
SEXP C_rrapply(SEXP env, SEXP X, SEXP FUN, SEXP argsFun, SEXP PRED, SEXP argsPred, SEXP classes, SEXP how, SEXP deflt, SEXP R_dfaslist, SEXP R_feverywhere);

/* ---------------------- */

/* Main function */

SEXP C_rrapply(SEXP env, SEXP X, SEXP FUN, SEXP argsFun, SEXP PRED, SEXP argsPred, SEXP classes, SEXP R_how, SEXP deflt, SEXP R_dfaslist, SEXP R_feverywhere)
{
	SEXP ans, names, namesFlat, xsym, xname, xpos, R_fcall, R_pcall;

	/* protect calls */
	int nprotect = 0;

	/* integer arguments */
	Args R_args;
	R_args.how_C = INTEGER_ELT(R_how, 0);
	R_args.fArgs = 0;
	R_args.fxname = INTEGER_ELT(argsFun, 0) > 0;
	R_args.fxpos = INTEGER_ELT(argsFun, 1) > 0;
	R_args.pArgs = 0;
	R_args.pxname = INTEGER_ELT(argsPred, 0) > 0;
	R_args.pxpos = INTEGER_ELT(argsPred, 1) > 0;
	R_args.dfaslist = LOGICAL_ELT(R_dfaslist, 0);
	R_args.feverywhere = LOGICAL_ELT(R_feverywhere, 0);

	/* traverse list once for max nodes and max depth
	   to avoid having to reallocate arrays downstream 
	   or allocate unused memory */
	R_xlen_t n = Rf_xlength(X);
	Depth R_depth = {.current = 0, .max = 1, .maxnodes = 0};
	C_traverse(X, &R_depth, n);
	R_depth.current = 0;

	/* allocate arrays to store location info */
	R_xlen_t *xloc = (R_xlen_t *)S_alloc(R_depth.max, sizeof(R_xlen_t));
	R_xlen_t(*xinfo)[3];
	R_xlen_t *inode = (R_xlen_t *)S_alloc(1, sizeof(R_xlen_t));

	if (R_args.how_C > 2)
		xinfo = (R_xlen_t(*)[3])S_alloc(R_depth.maxnodes, sizeof(*xinfo));

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

	/* evaluate names on layer zero */
	names = PROTECT(Rf_getAttrib(X, R_NamesSymbol));
	nprotect++;

	/* allocate output list */
	if (R_args.how_C == 0 || R_args.how_C == 3)
	{
		ans = PROTECT(Rf_shallow_duplicate(X));
		nprotect++;
	}
	else if (R_args.how_C == 4)
	{
		ans = PROTECT(Rf_allocVector(VECSXP, R_depth.maxnodes));
		/* allocate flat names vector (STRSXP initializes to "") */
		namesFlat = PROTECT(Rf_allocVector(STRSXP, R_depth.maxnodes));
		nprotect += 2;
	}
	else
	{
		ans = PROTECT(Rf_allocVector((SEXPTYPE)TYPEOF(X), n));
		C_copyAttrs(X, ans, names, TRUE);
		nprotect++;
	}

	/* traverse list to evaluate function calls */
	for (R_xlen_t i = 0; i < n; i++)
	{
		/* increment location counter */
		xloc[0] += 1;

		/* update current node info for list pruning */
		if (R_args.how_C > 2)
		{
			inode[0] += (i > 0);
			xinfo[inode[0]][1] = -1;  /* parent node */
			xinfo[inode[0]][2] = i;	  /* child counter */
		}

		if (R_args.how_C != 4) /* nested lists */
		{
			SET_VECTOR_ELT(ans, i, C_eval_list(env, VECTOR_ELT(X, i), R_fcall, R_pcall, classes, deflt, xsym, Rf_isNull(names) ? NA_STRING : STRING_ELT(names, i), R_args, R_depth, xinfo, xloc, inode, inode[0]));
		}
		else /* flat list */
		{
			if (!Rf_isNull(names))
				SET_STRING_ELT(namesFlat, inode[0], STRING_ELT(names, i));

			C_eval_flat(env, ans, namesFlat, VECTOR_ELT(X, i), R_fcall, R_pcall, classes, deflt, xsym, Rf_isNull(names) ? NA_STRING : STRING_ELT(names, i), R_args, R_depth, xinfo, xloc, inode, inode[0]);
		}
	}

	/* list pruning */
	if (R_args.how_C > 2)
	{
		/* detect nodes to filter */
		R_xlen_t *buf = (R_xlen_t *)R_alloc((size_t)R_depth.maxnodes, sizeof(R_xlen_t));
		R_xlen_t m = 0;
		for (R_xlen_t i = 0; i < R_depth.maxnodes; i++)
		{
			/* if nested list filter only level zero evaluated nodes, 
			   otherwise filter all evaluated nodes */
			if (xinfo[i][0] && (R_args.how_C == 3 ? xinfo[i][1] == -1 : TRUE))
			{
				buf[m] = i;
				m++;
			}
		}

		/* allocate pruned list */
		SEXP ansNew = PROTECT(Rf_allocVector(VECSXP, m));
		nprotect++;

		for (R_xlen_t j = 0; j < m; j++)
		{
			if (R_args.how_C == 3) /* populate nested list */
				SET_VECTOR_ELT(ansNew, j, C_fill_list(VECTOR_ELT(ans, xinfo[buf[j]][2]), xinfo, buf, buf[j], R_depth.maxnodes, m));
			else /* populate flat list */
				SET_VECTOR_ELT(ansNew, j, VECTOR_ELT(ans, buf[j]));
		}

		/* add names attribute */
		if (!Rf_isNull(names))
		{
			SEXP newNames = PROTECT(Rf_allocVector(STRSXP, m));
			for (R_xlen_t j = 0; j < m; j++)
			{
				if (R_args.how_C == 3) /* nested list names */
					SET_STRING_ELT(newNames, j, STRING_ELT(names, xinfo[buf[j]][2]));
				else /* flat list names */
					SET_STRING_ELT(newNames, j, STRING_ELT(namesFlat, buf[j]));
			}
			Rf_setAttrib(ansNew, R_NamesSymbol, newNames);
			UNPROTECT(1);
		}

		if (R_args.how_C == 3) /* copy other list attributes */
			Rf_copyMostAttrib(ans, ansNew);

		UNPROTECT(nprotect);
		return ansNew;
	}
	else
	{
		UNPROTECT(nprotect);
		return ans;
	}
}

/* Helper functions */

/* copies only name attribute or all attributes */
static void C_copyAttrs(SEXP obj, SEXP ans, SEXP names, Rboolean copyAttrs)
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
static int C_matchClass(SEXP obj, SEXP classes)
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

static void C_traverse(SEXP X, Depth *depth, R_xlen_t n)
{
	SEXP Xi;
	/* increment max depth if current depth is higher than max depth */
	depth->maxnodes += n;
	depth->current += 1;
	depth->max += (depth->current > depth->max);
	for (R_xlen_t i = 0; i < n; i++)
	{
		Xi = VECTOR_ELT(X, i);
		/* descend one level */
		if (Rf_isVectorList(Xi))
		{
			C_traverse(Xi, depth, Rf_xlength(Xi));
		}
	}
	depth->current -= 1;
}

static SEXP C_eval_list(
	SEXP env,			  // evaluation environment
	SEXP Xi,			  // current list layer content
	SEXP fcall,			  // f function call
	SEXP pcall,			  // condition function call
	SEXP classes,		  // classes argument
	SEXP deflt,			  // deflt argument
	SEXP xsym,			  // principal argument symbol
	SEXP xnameChar,		  // current value .xname argument
	Args args,			  // integer arguments
	Depth depth,		  // current and maximum depth information
	R_xlen_t (*xinfo)[3], // array with node position information
	R_xlen_t *xloc,		  // current value .pos argument
	R_xlen_t *node,		  // current node
	R_xlen_t parent		  // parent of current node
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
			if (C_matchClass(Xi, df))
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
			xpos_val = PROTECT(Rf_allocVector(INTSXP, depth.current + 1));
			for (R_xlen_t k = 0; k < (depth.current + 1); k++)
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
			matched = C_matchClass(Xi, classes);

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
			/* update current node info only for list pruning */
			if (args.how_C == 3)
			{
				R_xlen_t i1 = node[0];
				xinfo[i1][0] = TRUE;
				R_xlen_t i2 = xinfo[i1][1];

				for (int i = depth.current; i > -1; i--)
				{
					if (i2 > -1)
					{
						i1 = i2;
						xinfo[i1][0] = TRUE;
						i2 = xinfo[i1][1];
					}
					else
						break;
				}
			}

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
			if (args.how_C == 0 || args.how_C == 3)
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
		depth.current += 1;
		R_xlen_t m = Rf_xlength(Xi);
		SEXP names = PROTECT(Rf_getAttrib(Xi, R_NamesSymbol));

		if (args.how_C == 0 || args.how_C == 3)
		{
			funVal = PROTECT(Rf_shallow_duplicate(Xi));
		}
		else
		{
			/* VECEXP initializes with R_NilValues */
			funVal = PROTECT(Rf_allocVector(VECSXP, m));
			C_copyAttrs(Xi, funVal, names, TRUE);
		}

		for (R_xlen_t j = 0; j < m; j++)
		{
			/* increment location */
			xloc[depth.current] = j + 1;

			/* update current node info */
			if (args.how_C == 3)
			{
				node[0] += 1;
				xinfo[node[0]][1] = parent;  /* parent node */
				xinfo[node[0]][2] = j;		 /* child counter */
			}

			/* evaluate list element */
			SET_VECTOR_ELT(funVal, j, C_eval_list(env, VECTOR_ELT(Xi, j), fcall, pcall, classes, deflt, xsym, Rf_isNull(names) ? NA_STRING : STRING_ELT(names, j), args, depth, xinfo, xloc, node, node[0]));
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

static void C_eval_flat(
	SEXP env,			  // evaluation environment
	SEXP Xflat,			  // flat list with node content
	SEXP Xnames,		  // flat list of node names
	SEXP Xi,			  // current list layer
	SEXP fcall,			  // f function call
	SEXP pcall,			  // condition function call
	SEXP classes,		  // classes argument
	SEXP deflt,			  // deflt argument
	SEXP xsym,			  // principal argument symbol
	SEXP xnameChar,		  // current .xname value
	Args args,			  // integer arguments
	Depth depth,		  // current and max depth information
	R_xlen_t (*xinfo)[3], // array with node position information
	R_xlen_t *xloc,		  // current .xpos value
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
			if (C_matchClass(Xi, df))
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
			xpos_val = PROTECT(Rf_allocVector(INTSXP, depth.current + 1));
			for (R_xlen_t k = 0; k < (depth.current + 1); k++)
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
			matched = C_matchClass(Xi, classes);

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

		if (doEval && matched)
		{
			/* update node evaluation info */
			xinfo[node[0]][0] = TRUE;

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
		depth.current += 1;
		parent = node[0];
		R_xlen_t m = Rf_xlength(Xi);
		SEXP names = PROTECT(Rf_getAttrib(Xi, R_NamesSymbol));

		for (R_xlen_t j = 0; j < m; j++)
		{
			/* increment location */
			xloc[depth.current] = j + 1;

			/* update current node info */
			node[0] += 1;
			xinfo[node[0]][1] = parent;		/* parent node */
			xinfo[node[0]][2] = j;			/* child counter */

			/* update name attribute */
			if (!Rf_isNull(names))
				SET_STRING_ELT(Xnames, node[0], STRING_ELT(names, j));

			/* evaluate list element */
			C_eval_flat(env, Xflat, Xnames, VECTOR_ELT(Xi, j), fcall, pcall, classes, deflt, xsym, Rf_isNull(names) ? NA_STRING : STRING_ELT(names, j), args, depth, xinfo, xloc, node, parent);
		}
		UNPROTECT(1);
	}
}

static SEXP C_fill_list(SEXP Xi, R_xlen_t (*xinfo)[3], R_xlen_t *buf, R_xlen_t node, R_xlen_t maxnodes, R_xlen_t ibuf)
{
	if (Rf_isVectorList(Xi))
	{
		R_xlen_t m = 0;
		R_xlen_t maxparent = node;
		for (R_xlen_t inode = node + 1; inode < maxnodes; inode++)
		{
			/* check if direct child of node and doEval == 1 */
			if (xinfo[inode][1] == node && xinfo[inode][0])
			{
				buf[ibuf + m] = inode;
				m++;
			}
			/* stop if no longer (indirect) child of node */
			if (xinfo[inode][1] < node || xinfo[inode][1] > maxparent)
				break;
			/* update maximum allowed parent node */
			maxparent += 1;
		}

		/* descend one level */
		if (m > 0)
		{
			/* populate sublist*/
			SEXP ans = PROTECT(Rf_allocVector(VECSXP, m));
			for (R_xlen_t j = 0; j < m; j++)
			{
				SET_VECTOR_ELT(ans, j, C_fill_list(VECTOR_ELT(Xi, xinfo[buf[ibuf + j]][2]), xinfo, buf, buf[ibuf + j], maxnodes, ibuf + m));
			}

			/* add name attribute */
			SEXP names = PROTECT(Rf_getAttrib(Xi, R_NamesSymbol));
			if (!Rf_isNull(names))
			{
				SEXP ansNames = PROTECT(Rf_allocVector(STRSXP, m));

				for (R_xlen_t j = 0; j < m; j++)
				{
					SET_STRING_ELT(ansNames, j, STRING_ELT(names, xinfo[buf[ibuf + j]][2]));
				}
				Rf_setAttrib(ans, R_NamesSymbol, ansNames);
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
			return Rf_lazy_duplicate(Xi);
		}
	}
	else
	{
		return Rf_lazy_duplicate(Xi);
	}
}
