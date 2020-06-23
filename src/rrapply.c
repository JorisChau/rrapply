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

typedef struct CountGlobal
{
	/* counters that should persist between function calls */
	int depthmax;	   // maximum allowed depth
	R_len_t maxnodes;  // maximum allowed node count
	R_len_t node;	   // current node counter (only for pruning)
	Rboolean anynames; // any names present (only for flatten)
} CountGlobal;

typedef struct CountLocal
{
	/* counters that are local to function calls */
	int depth;		// current depth
	R_len_t parent; // current parent counter (only for pruning)
} CountLocal;

/* prototypes */

static void C_copyAttrs(SEXP obj, SEXP ans, SEXP names, Rboolean copyAttrs);
static int C_matchClass(SEXP obj, SEXP classes);
static void C_traverse(SEXP X, CountGlobal *count, int depth);
static SEXP C_eval_list(SEXP env, SEXP Xi, SEXP fcall, SEXP pcall, SEXP classes, SEXP deflt, SEXP xsym, SEXP xnameChar, Args args, CountGlobal *countglobal, CountLocal countlocal, R_len_t (**xinfo)[3], R_len_t **xloc);
static SEXP C_fill_list(SEXP Xi, R_len_t (*xinfo)[3], R_len_t *buf, R_len_t node, R_len_t maxnodes, R_len_t ibuf);
static void C_fill_flat(SEXP ansNew, SEXP Xi, R_len_t (*xinfo)[3], R_len_t *ix, R_len_t *ians);
static void C_fill_flat_names(SEXP ansNew, SEXP newNames, SEXP Xi, SEXP name, R_len_t (*xinfo)[3], R_len_t *ix, R_len_t *ians);
SEXP C_rrapply(SEXP env, SEXP X, SEXP FUN, SEXP argsFun, SEXP PRED, SEXP argsPred, SEXP classes, SEXP how, SEXP deflt, SEXP R_dfaslist, SEXP R_feverywhere);

/* ---------------------- */

/* Main function */

SEXP C_rrapply(SEXP env, SEXP X, SEXP FUN, SEXP argsFun, SEXP PRED, SEXP argsPred, SEXP classes, SEXP R_how, SEXP deflt, SEXP R_dfaslist, SEXP R_feverywhere)
{
	SEXP ans, names, xsym, xname, xpos, R_fcall, R_pcall;

	/* protect calls */
	int nprotect = 0;

	/* integer arguments */
	Args R_args;
	R_args.how_C = INTEGER_ELT(R_how, 0) - 1;
	R_args.fArgs = 0;
	R_args.fxname = INTEGER_ELT(argsFun, 0) > 0;
	R_args.fxpos = INTEGER_ELT(argsFun, 1) > 0;
	R_args.pArgs = 0;
	R_args.pxname = INTEGER_ELT(argsPred, 0) > 0;
	R_args.pxpos = INTEGER_ELT(argsPred, 1) > 0;
	R_args.dfaslist = LOGICAL_ELT(R_dfaslist, 0);
	R_args.feverywhere = INTEGER_ELT(R_feverywhere, 0) - 1;

	/* traverse list once for max nodes and max depth
	   for more accurate initialization, computational 
	   effort is negligible */
	R_len_t n = Rf_length(X);
	CountGlobal initGlobal = {.depthmax = 1, .maxnodes = 0, .node = -1, .anynames = 0};
	CountLocal initLocal = {.depth = 0, .parent = 0};
	C_traverse(X, &initGlobal, 0);

	/* allocate arrays to store location info */
	R_len_t *xloc = (R_len_t *)S_alloc(initGlobal.depthmax, sizeof(R_len_t));
	R_len_t(*xinfo)[3] = NULL; /* avoid unitialized warning */

	if (R_args.how_C > 2)
		xinfo = (R_len_t(*)[3])S_alloc(initGlobal.maxnodes, sizeof(*xinfo));

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

	if (R_args.how_C == 4 && !Rf_isNull(names))
		initGlobal.anynames = TRUE;

	/* allocate output list */
	if (R_args.how_C == 0 || R_args.how_C > 2)
	{
		ans = PROTECT(Rf_shallow_duplicate(X));
	}
	else
	{
		ans = PROTECT(Rf_allocVector((SEXPTYPE)TYPEOF(X), n));
		C_copyAttrs(X, ans, names, TRUE);
	}
	nprotect += 2;

	/* traverse list to evaluate function calls */
	for (R_len_t i = 0; i < n; i++)
	{
		/* increment location counter */
		xloc[0] += 1;

		/* update current node info for list pruning */
		if (R_args.how_C > 2)
		{
			/* reallocate array if necessary in this case */
			if (R_args.feverywhere == 2 && (initGlobal.node + 1) >= initGlobal.maxnodes)
			{
				xinfo = (R_len_t(*)[3])S_realloc((char *)xinfo,  2 * initGlobal.maxnodes, initGlobal.maxnodes, sizeof(*xinfo));
				initGlobal.maxnodes *= 2;
			}

			initGlobal.node++;				// increment node counter
			xinfo[initGlobal.node][1] = -1; // parent node counter
			xinfo[initGlobal.node][2] = i;	// child node counter
		}

		/* main recursion part */
		SET_VECTOR_ELT(ans, i, C_eval_list(env, VECTOR_ELT(X, i), R_fcall, R_pcall, classes, deflt, xsym, Rf_isNull(names) ? NA_STRING : STRING_ELT(names, i), R_args, &initGlobal, initLocal, &xinfo, &xloc));
	}

	/* list pruning */
	if (R_args.how_C > 2)
	{
		/* detect nodes to filter */
		initGlobal.maxnodes = initGlobal.node + 1;
		R_len_t *buf = (R_len_t *)R_alloc((size_t)initGlobal.maxnodes, sizeof(R_len_t));
		R_len_t m = 0;
		for (R_len_t i = 0; i < initGlobal.maxnodes; i++)
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

		if (R_args.how_C == 3)
		{
			/* populate nested list */
			for (R_len_t j = 0; j < m; j++)
				SET_VECTOR_ELT(ansNew, j, C_fill_list(VECTOR_ELT(ans, xinfo[buf[j]][2]), xinfo, buf, buf[j], initGlobal.maxnodes, m));

			/* add names attribute */
			if (!Rf_isNull(names))
			{
				SEXP newNames = PROTECT(Rf_allocVector(STRSXP, m));
				for (R_len_t j = 0; j < m; j++)
					SET_STRING_ELT(newNames, j, STRING_ELT(names, xinfo[buf[j]][2]));
				Rf_setAttrib(ansNew, R_NamesSymbol, newNames);
				UNPROTECT(1);
			}
			/* copy other list attributes */
			Rf_copyMostAttrib(ans, ansNew);
		}
		else
		{
			/* populate flat list */
			int ix = 0;
			int ians = 0;
			if (!initGlobal.anynames)
			{
				for (R_len_t i = 0; i < Rf_length(ans); i++)
				{
					C_fill_flat(ansNew, VECTOR_ELT(ans, i), xinfo, &ix, &ians);
					ix++;
				}
			}
			else
			{
				SEXP newNames = PROTECT(Rf_allocVector(STRSXP, m));
				for (R_len_t i = 0; i < Rf_length(ans); i++)
				{
					C_fill_flat_names(ansNew, newNames, VECTOR_ELT(ans, i), Rf_isNull(names) ? NA_STRING : STRING_ELT(names, i), xinfo, &ix, &ians);
					ix++;
				}
				Rf_setAttrib(ansNew, R_NamesSymbol, newNames);
				UNPROTECT(1);
			}
		}

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
	R_len_t n = Rf_length(klass);

	int matched = FALSE;
	/* match classes to R_ClassSymbol attribute */
	if (n > 0)
	{
		for (R_len_t i = 0; i < n; i++)
			for (R_len_t j = 0; j < Rf_length(classes); j++)
				if (strcmp(CHAR(STRING_ELT(klass, i)), CHAR(STRING_ELT(classes, j))) == 0)
					matched = 1;
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

			for (R_len_t j = 0; j < Rf_length(classes); j++)
				if (strcmp(CHAR(STRING_ELT(classes, j)), typename) == 0)
					matched = TRUE;
		}
		UNPROTECT(1);
	}
	UNPROTECT(1);
	return matched;
}

static void C_traverse(SEXP X, CountGlobal *count, int depth)
{
	SEXP Xi;
	/* increment max depth if current depth is higher than max depth */
	R_len_t n = Rf_length(X);
	depth++;
	count->maxnodes += n;
	count->depthmax += (depth > count->depthmax);
	for (R_len_t i = 0; i < n; i++)
	{
		Xi = VECTOR_ELT(X, i);
		/* descend one level */
		if (Rf_isVectorList(Xi))
		{
			C_traverse(Xi, count, depth);
		}
	}
}

static SEXP C_eval_list(
	SEXP env,				  // evaluation environment
	SEXP Xi,				  // current list layer content
	SEXP fcall,				  // f function call
	SEXP pcall,				  // condition function call
	SEXP classes,			  // classes argument
	SEXP deflt,				  // deflt argument
	SEXP xsym,				  // principal argument symbol
	SEXP xnameChar,			  // current value .xname argument
	Args args,				  // integer arguments
	CountGlobal *countglobal, // global node counters
	CountLocal countlocal,	  // local node counters
	R_len_t (**xinfo)[3],	  // array with node position information
	R_len_t **xloc			  // current value .pos argument
)
{
	SEXP funVal = NULL; /* avoid unitialized warning */
	int nprotect = 0;

	/* if Xi is list (and data.frame is treated as list if !dfaslist)
	   and !feverywhere recurse, otherwise evaluate functions */
	int doRecurse = 0;

	if (args.feverywhere < 1 && Rf_isVectorList(Xi))
	{
		doRecurse = 1;
		if (!args.dfaslist)
		{
			SEXP df = PROTECT(Rf_ScalarString(Rf_mkChar("data.frame")));
			if (C_matchClass(Xi, df))
				doRecurse = 0;
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
			xpos_val = PROTECT(Rf_allocVector(INTSXP, countlocal.depth + 1));
			for (R_len_t k = 0; k < (countlocal.depth + 1); k++)
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
			matched = C_matchClass(Xi, classes);

		if (args.pArgs > 0)
		{
			/* set default to FALSE */
			doEval = FALSE;

			/* evaluate pred function call */
			SEXP predVal = PROTECT(R_forceAndCall(pcall, args.pArgs, env));

			if (Rf_isLogical(predVal) && Rf_length(predVal) == 1)
			{
				int predValBool = LOGICAL_ELT(predVal, 0);
				if (!(predValBool == NA_LOGICAL) && predValBool)
				{
					doEval = TRUE;
				}
			}
			UNPROTECT(1);
		}

		/* evaluate f and decide what to return or recurse further */
		if (doEval && matched)
		{
			/* update current node info only for nested list pruning */
			if (args.how_C == 3)
			{
				R_len_t i1 = countglobal->node;
				(*xinfo)[i1][0] = TRUE;
				R_len_t i2 = (*xinfo)[i1][1];

				for (int i = countlocal.depth; i > -1; i--)
				{
					if (i2 > -1)
					{
						i1 = i2;
						(*xinfo)[i1][0] = TRUE;
						i2 = (*xinfo)[i1][1];
					}
					else
						break;
				}
			}

			/* evaluate f */
			if (args.fArgs > 0)
			{
				funVal = PROTECT(R_forceAndCall(fcall, args.fArgs, env));
				nprotect++;

				if (MAYBE_REFERENCED(funVal))
					funVal = Rf_lazy_duplicate(funVal);
			}
			else
			{
				funVal = Rf_lazy_duplicate(Xi);
			}

			/* recurse further with new list (type 2) if feverywhere == 2 */
			if (args.feverywhere == 2 && Rf_isVectorList(funVal))
			{
				doRecurse = 2;
			}
			else /* otherwise return current value */
			{
				/* update current node info for flat lists */
				if (args.how_C == 4)
					(*xinfo)[countglobal->node][0] = TRUE;

				if (nprotect > 0)
					UNPROTECT(nprotect);
				return funVal;
			}
		}
		else if (args.feverywhere > 0 && Rf_isVectorList(Xi))
		{
			/* recurse further with original list (type 1) */
			doRecurse = 1;
		}
		else
		{
			/* return original list (or default) here if feverywhere == 0 */
			if (args.how_C == 0 || args.how_C > 2)
			{
				return Rf_lazy_duplicate(Xi);
			}
			else /* fill list by default */
			{
				return Rf_lazy_duplicate(deflt);
			}
		}
	}

	if (doRecurse > 0)
	{
		SEXP Xnew, names;
		R_len_t m;

		/* create new object for recursion only if doRecurse != 2 */
		if (doRecurse != 2)
		{
			m = Rf_length(Xi);
			names = PROTECT(Rf_getAttrib(Xi, R_NamesSymbol));

			if (args.how_C == 0 || args.how_C > 2)
			{
				Xnew = PROTECT(Rf_shallow_duplicate(Xi));
			}
			else
			{
				/* VECEXP initializes with R_NilValues */
				Xnew = PROTECT(Rf_allocVector(VECSXP, Rf_length(Xi)));
				C_copyAttrs(Xi, Xnew, names, TRUE);
			}
		}
		else
		{
			m = Rf_length(funVal);
			names = PROTECT(Rf_getAttrib(funVal, R_NamesSymbol));
			Xnew = PROTECT(Rf_shallow_duplicate(funVal));
		}
		nprotect += 2;

		/* update node info for list pruning */
		if (args.how_C > 2)
		{
			countlocal.parent = countglobal->node;

			/* check if names should be included in flat list */
			if (args.how_C == 4 && !(countglobal->anynames) && !Rf_isNull(names))
				countglobal->anynames = TRUE;
		}

		/* descend one level */
		countlocal.depth++;

		for (R_len_t j = 0; j < m; j++)
		{
			/* update current node info */
			if (args.how_C > 2)
			{
				/* reallocate arrays if necessary in this case */
				if (args.feverywhere == 2)
				{
					if (countlocal.depth >= countglobal->depthmax)
					{
						*xloc = (R_len_t *)S_realloc((char *)*xloc, 2 * countglobal->depthmax, countglobal->depthmax, sizeof(R_len_t));
						countglobal->depthmax *= 2;
					}
					if ((countglobal->node + 1) >= countglobal->maxnodes)
					{
						*xinfo = (R_len_t(*)[3])S_realloc((char *)*xinfo, 2 * countglobal->maxnodes, countglobal->maxnodes, sizeof(**xinfo));
						countglobal->maxnodes *= 2;
					}
				}

				countglobal->node += 1;
				(*xinfo)[countglobal->node][1] = countlocal.parent; /* parent node */
				(*xinfo)[countglobal->node][2] = j;					/* child counter */
			}

			/* increment location */
			(*xloc)[countlocal.depth] = j + 1;

			/* evaluate list element */
			if (doRecurse != 2)
			{
				SET_VECTOR_ELT(Xnew, j, C_eval_list(env, VECTOR_ELT(Xi, j), fcall, pcall, classes, deflt, xsym, Rf_isNull(names) ? NA_STRING : STRING_ELT(names, j), args, countglobal, countlocal, xinfo, xloc));
			}
			else
			{
				SET_VECTOR_ELT(Xnew, j, C_eval_list(env, VECTOR_ELT(funVal, j), fcall, pcall, classes, deflt, xsym, Rf_isNull(names) ? NA_STRING : STRING_ELT(names, j), args, countglobal, countlocal, xinfo, xloc));
			}
		}

		UNPROTECT(nprotect);
		return Xnew;
	}
	else
	{
		/* should not normally be reached */
		return R_NilValue;
	}
}

static SEXP C_fill_list(SEXP Xi, R_len_t (*xinfo)[3], R_len_t *buf, R_len_t node, R_len_t maxnodes, R_len_t ibuf)
{
	if (Rf_isVectorList(Xi))
	{
		R_len_t m = 0;
		R_len_t maxparent = node;
		for (R_len_t inode = node + 1; inode < maxnodes; inode++)
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
			for (R_len_t j = 0; j < m; j++)
			{
				SET_VECTOR_ELT(ans, j, C_fill_list(VECTOR_ELT(Xi, xinfo[buf[ibuf + j]][2]), xinfo, buf, buf[ibuf + j], maxnodes, ibuf + m));
			}

			/* add name attribute */
			SEXP names = PROTECT(Rf_getAttrib(Xi, R_NamesSymbol));
			if (!Rf_isNull(names))
			{
				SEXP ansNames = PROTECT(Rf_allocVector(STRSXP, m));

				for (R_len_t j = 0; j < m; j++)
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

/* fill flat list without names */
static void C_fill_flat(SEXP ansNew, SEXP Xi, R_len_t (*xinfo)[3], R_len_t *ix, R_len_t *ians)
{
	if (xinfo[*ix][0])
	{
		SET_VECTOR_ELT(ansNew, *ians, Xi);
		(*ians)++;
	}
	else if (Rf_isVectorList(Xi))
	{
		for (R_len_t i = 0; i < Rf_length(Xi); i++)
		{
			(*ix)++;
			C_fill_flat(ansNew, VECTOR_ELT(Xi, i), xinfo, ix, ians);
		}
	}
}

/* fill flat list with names */
static void C_fill_flat_names(SEXP ansNew, SEXP newNames, SEXP Xi, SEXP name, R_len_t (*xinfo)[3], R_len_t *ix, R_len_t *ians)
{
	if (xinfo[*ix][0])
	{
		SET_VECTOR_ELT(ansNew, *ians, Xi);
		SET_STRING_ELT(newNames, *ians, name);
		(*ians)++;
	}
	else if (Rf_isVectorList(Xi))
	{
		SEXP names = PROTECT(Rf_getAttrib(Xi, R_NamesSymbol));
		for (R_len_t i = 0; i < Rf_length(Xi); i++)
		{
			(*ix)++;
			C_fill_flat_names(ansNew, newNames, VECTOR_ELT(Xi, i), Rf_isNull(names) ? NA_STRING : STRING_ELT(names, i), xinfo, ix, ians);
		}
		UNPROTECT(1);
	}
}
