#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>
#include <string.h>
#include <stdio.h>

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
		5: melt (prune, return melted list)
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
	int depthmaxobs;   // observed maximum depth (only for melting)
	Rboolean anynames; // any names present (only for flatten)
} CountGlobal;

typedef struct CountLocal
{
	/* counters that are local to function calls */
	int depth;		// current depth
	R_len_t parent; // current parent counter (only for pruning)
} CountLocal;

/* prototypes */

static SEXP C_int2char(int i);
static void C_copyAttrs(SEXP obj, SEXP ans, SEXP names, Rboolean copyAttrs);
static int C_matchClass(SEXP obj, SEXP classes);
static void C_traverse(SEXP X, CountGlobal *count, int depth);
static SEXP C_eval_list(SEXP env, SEXP Xi, SEXP fcall, SEXP pcall, SEXP classes, SEXP deflt, SEXP xsym, SEXP xnameChar, Args args, CountGlobal *countglobal, CountLocal countlocal, R_len_t (**xinfo)[3], R_len_t **xloc, R_len_t **xdepth);
static SEXP C_fill_list(SEXP Xi, R_len_t (*xinfo)[3], R_len_t *buf, R_len_t node, R_len_t maxnodes, R_len_t ibuf);
static void C_fill_flat(SEXP ansNew, SEXP Xi, R_len_t (*xinfo)[3], R_len_t *ix, R_len_t *ians);
static void C_fill_flat_names(SEXP ansNew, SEXP newNames, SEXP Xi, SEXP name, R_len_t (*xinfo)[3], R_len_t *ix, R_len_t *ians);
static void C_fill_melt(SEXP ansFlat, SEXP ansNames, SEXP Xi, SEXP name, R_len_t (*xinfo)[3], R_len_t *ix, R_len_t *ians);
static SEXP C_fill_unmelt(SEXP X, SEXP Xval, R_len_t *namesCount, R_len_t lvl, R_len_t nlvls, R_len_t start, R_len_t end);
SEXP C_rrapply(SEXP env, SEXP X, SEXP FUN, SEXP argsFun, SEXP PRED, SEXP argsPred, SEXP classes, SEXP how, SEXP deflt, SEXP R_dfaslist, SEXP R_feverywhere);
SEXP C_unmelt(SEXP X);
/* ---------------------- */

/* Main functions */

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
	CountGlobal initGlobal = {.depthmax = 1, .maxnodes = 0, .node = -1, .depthmaxobs = 0, .anynames = 0};
	CountLocal initLocal = {.depth = 0, .parent = 0};
	C_traverse(X, &initGlobal, 0);

	/* allocate arrays to store location info */
	R_len_t *xloc = (R_len_t *)S_alloc(initGlobal.depthmax, sizeof(R_len_t));
	R_len_t(*xinfo)[3] = NULL; /* avoid unitialized warning */
	R_len_t *xdepth = NULL;

	if (R_args.how_C > 2)
		xinfo = (R_len_t(*)[3])S_alloc(initGlobal.maxnodes, sizeof(*xinfo));

	if (R_args.how_C == 5)
		xdepth = (R_len_t *)S_alloc(initGlobal.maxnodes, sizeof(R_len_t));

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
	if (R_args.how_C == 1 || R_args.how_C == 2)
	{
		ans = PROTECT(Rf_allocVector((SEXPTYPE)TYPEOF(X), n));
		C_copyAttrs(X, ans, names, TRUE);
	}
	else
	{
		ans = PROTECT(Rf_shallow_duplicate(X));
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
				xinfo = (R_len_t(*)[3])S_realloc((char *)xinfo, 2 * initGlobal.maxnodes, initGlobal.maxnodes, sizeof(*xinfo));
				if (R_args.how_C == 5)
					xdepth = (R_len_t *)S_realloc((char *)xinfo, 2 * initGlobal.maxnodes, initGlobal.maxnodes, sizeof(R_len_t));
				initGlobal.maxnodes *= 2;
			}

			initGlobal.node++;				// increment node counter
			xinfo[initGlobal.node][1] = -1; // parent node counter
			xinfo[initGlobal.node][2] = i;	// child node counter

			if (R_args.how_C == 5)
				xdepth[initGlobal.node] = 0; // current depth counter (only for melting)
		}

		/* main recursion part */
		SET_VECTOR_ELT(ans, i, C_eval_list(env, VECTOR_ELT(X, i), R_fcall, R_pcall, classes, deflt, xsym, Rf_isNull(names) ? NA_STRING : STRING_ELT(names, i), R_args, &initGlobal, initLocal, &xinfo, &xloc, &xdepth));
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
			   otherwise filter evaluated terminal nodes */
			if (R_args.how_C == 3 ? (xinfo[i][0] && xinfo[i][1] == -1) : (xinfo[i][0] == 1))
			{
				buf[m] = i;
				m++;
			}
		}

		/* construct output list based on 'how' argument */
		SEXP ansNew;

		if (R_args.how_C == 3)
		{
			/* return pruned list */
			ansNew = PROTECT(Rf_allocVector(VECSXP, m));
			nprotect++;

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
		else if (R_args.how_C == 4)
		{
			/* return flat list */
			ansNew = PROTECT(Rf_allocVector(VECSXP, m));
			nprotect++;

			/* populate flat list */
			R_len_t ix = 0;
			R_len_t ians = 0;
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
		else
		{
			/* return melted data.frame */
			SEXP namesNew;
			PROTECT_INDEX ipx;
			ansNew = PROTECT(Rf_allocVector(VECSXP, initGlobal.depthmaxobs + 2));
			SEXP ansFlat = PROTECT(Rf_allocVector(VECSXP, m));
			SEXP ansNames = PROTECT(Rf_allocVector(STRSXP, initGlobal.maxnodes));
			PROTECT_WITH_INDEX(namesNew = Rf_getAttrib(ans, R_NamesSymbol), &ipx);
			nprotect += 4;

			/* extract all evaluated parent names + populate flat list */
			Rboolean noNames = Rf_isNull(namesNew);
			if (noNames)
				REPROTECT(namesNew = Rf_allocVector(STRSXP, n), ipx);

			R_len_t ix = 0;
			R_len_t ians = 0;
			for (R_len_t i = 0; i < n; i++)
			{
				if (noNames)
					SET_STRING_ELT(namesNew, i, C_int2char(i + 1));

				C_fill_melt(ansFlat, ansNames, VECTOR_ELT(ans, i), STRING_ELT(namesNew, i), xinfo, &ix, &ians);
				ix++;
			}

			/* add flat list to data.frame */
			SET_VECTOR_ELT(ansNew, initGlobal.depthmaxobs + 1, ansFlat);

			SEXP ansColumn = PROTECT(Rf_allocVector(STRSXP, m));
			nprotect++;

			// fill node columns until root
			Rboolean keep;
			for (int depth = initGlobal.depthmaxobs; depth > -1; depth--)
			{
				keep = FALSE;
				for (R_len_t i = 0; i < m; i++)
				{
					if (xdepth[buf[i]] == depth && buf[i] != -1)
					{
						SET_STRING_ELT(ansColumn, i, STRING_ELT(ansNames, buf[i]));
						buf[i] = xinfo[buf[i]][1]; // update buffer to parent node id
						keep = TRUE;
					}
					else
					{
						SET_STRING_ELT(ansColumn, i, NA_STRING);
					}
				}
				// deep copy of column
				if (keep)
					SET_VECTOR_ELT(ansNew, depth, Rf_duplicate(ansColumn));
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

/* unmelt data.frame to nested list */
SEXP C_unmelt(SEXP X)
{
	R_len_t ncols = (R_len_t)Rf_length(X);
	R_len_t nrows = (R_len_t)Rf_length(VECTOR_ELT(X, ncols - 1));
	R_len_t *namesCount = (R_len_t *)R_alloc(nrows, sizeof(R_len_t));

	return C_fill_unmelt(X, VECTOR_ELT(X, ncols - 1), namesCount, 0, ncols - 2, 0, nrows);
}

/* Helper functions */

/* convert integer to character */
static SEXP C_int2char(int i)
{
	char buf[100]; // fixed buffer size
	snprintf(buf, 100, "..%d", i);
	return Rf_mkChar(buf);
}

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
	R_len_t **xloc,			  // current value .pos argument
	R_len_t **xdepth		  // current depth (only used for melting)
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
			/* update current node info only for pruning and melting */
			if (args.how_C > 2)
			{
				R_len_t i1 = countglobal->node;
				(*xinfo)[i1][0] = TRUE;
				R_len_t i2 = (*xinfo)[i1][1];

				for (int i = countlocal.depth; i > -1; i--)
				{
					if (i2 > -1)
					{
						i1 = i2;
						(*xinfo)[i1][0] = 2;
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

				if (MAYBE_REFERENCED(funVal))
					funVal = Rf_lazy_duplicate(funVal);
			}
			else
			{
				funVal = PROTECT(Rf_lazy_duplicate(Xi));
			}
			nprotect++;

			/* recurse further with new list (type 2) if feverywhere == 2 */
			if (args.feverywhere == 2 && Rf_isVectorList(funVal))
			{
				doRecurse = 2;
			}
			else /* otherwise return current value */
			{
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
				Xnew = PROTECT(Rf_allocVector(VECSXP, m));
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

		if (args.feverywhere == 2)
		{
			if (countlocal.depth > 100) /* stop with error if depth too large */
			{
				Rf_error("a hard limit of maximum 100 nested layers is enforced to avoid infinite recursion");
			}
			if (countlocal.depth >= countglobal->depthmax)
			{
				*xloc = (R_len_t *)S_realloc((char *)*xloc, 2 * countglobal->depthmax, countglobal->depthmax, sizeof(R_len_t));
				countglobal->depthmax *= 2;
			}
		}

		for (R_len_t j = 0; j < m; j++)
		{
			/* update current node info */
			if (args.how_C > 2)
			{
				/* reallocate arrays if necessary in this case */
				if (args.feverywhere == 2)
				{
					if ((countglobal->node + 1) >= countglobal->maxnodes)
					{
						*xinfo = (R_len_t(*)[3])S_realloc((char *)*xinfo, 2 * countglobal->maxnodes, countglobal->maxnodes, sizeof(**xinfo));
						if (args.how_C == 5)
							*xdepth = (R_len_t *)S_realloc((char *)*xdepth, 2 * countglobal->maxnodes, countglobal->maxnodes, sizeof(R_len_t));
						countglobal->maxnodes *= 2;
					}
				}

				countglobal->node += 1;
				(*xinfo)[countglobal->node][1] = countlocal.parent; /* parent node */
				(*xinfo)[countglobal->node][2] = j;					/* child counter */

				if (args.how_C == 5)
				{
					(*xdepth)[countglobal->node] = countlocal.depth;						   /* depth counter */
					countglobal->depthmaxobs += (countlocal.depth > countglobal->depthmaxobs); /* increment maximum observed depth */
				}
			}

			/* increment location */
			(*xloc)[countlocal.depth] = j + 1;

			/* evaluate list element */
			if (doRecurse != 2)
			{
				SET_VECTOR_ELT(Xnew, j, C_eval_list(env, VECTOR_ELT(Xi, j), fcall, pcall, classes, deflt, xsym, Rf_isNull(names) ? NA_STRING : STRING_ELT(names, j), args, countglobal, countlocal, xinfo, xloc, xdepth));
			}
			else
			{
				SET_VECTOR_ELT(Xnew, j, C_eval_list(env, VECTOR_ELT(funVal, j), fcall, pcall, classes, deflt, xsym, Rf_isNull(names) ? NA_STRING : STRING_ELT(names, j), args, countglobal, countlocal, xinfo, xloc, xdepth));
			}
		}

		UNPROTECT(nprotect);
		return Xnew;
	}
	else
	{
		/* should not normally be reached */
		UNPROTECT(nprotect);
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
	if (xinfo[*ix][0] == 1)
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
	if (xinfo[*ix][0] == 1)
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

/* fill melted data.frame */
static void C_fill_melt(SEXP ansFlat, SEXP ansNames, SEXP Xi, SEXP name, R_len_t (*xinfo)[3], R_len_t *ix, R_len_t *ians)
{
	// add name to vector
	SET_STRING_ELT(ansNames, *ix, name);

	if (xinfo[*ix][0] == 1) // terminal nodes only
	{
		SET_VECTOR_ELT(ansFlat, *ians, Xi);
		(*ians)++;
	}
	else if (Rf_isVectorList(Xi))
	{
		SEXP names;
		R_len_t m = Rf_length(Xi);
		PROTECT_INDEX ipx;
		PROTECT_WITH_INDEX(names = Rf_getAttrib(Xi, R_NamesSymbol), &ipx);
		Rboolean noNames = Rf_isNull(names);

		if (noNames)
			REPROTECT(names = Rf_allocVector(STRSXP, m), ipx);

		for (R_len_t i = 0; i < m; i++)
		{
			(*ix)++;
			// use counter for missing names
			if (noNames)
				SET_STRING_ELT(names, i, C_int2char(i + 1));

			// recurse further
			C_fill_melt(ansFlat, ansNames, VECTOR_ELT(Xi, i), STRING_ELT(names, i), xinfo, ix, ians);
		}
		UNPROTECT(1);
	}
}

/* unmelt data.frame to nested list */
static SEXP C_fill_unmelt(SEXP X, SEXP Xval, R_len_t *namesCount, R_len_t lvl, R_len_t nlvls, R_len_t start, R_len_t end)
{
	// get column
	SEXP Xi = VECTOR_ELT(X, lvl);
	SEXP Xi1 = VECTOR_ELT(X, lvl + 1);
	R_len_t m = 0;

	// count names
	for (R_len_t i = start; i < end; i++)
	{
		if (i == start ||
			lvl >= nlvls ||
			STRING_ELT(Xi1, i) == NA_STRING ||
			STRING_ELT(Xi, i - 1) == NA_STRING ||
			STRING_ELT(Xi, i) == NA_STRING ||
			strcmp(CHAR(STRING_ELT(Xi, i - 1)), CHAR(STRING_ELT(Xi, i))) != 0)
		{
			if (i > start)
				m++;
			namesCount[m] = 0;
		}

		(namesCount[m])++;
	}

	SEXP ansNew = PROTECT(Rf_allocVector(VECSXP, m + 1));
	SEXP ansNames = PROTECT(Rf_allocVector(STRSXP, m + 1));
	R_len_t mcum = start;

	for (R_len_t j = 0; j < (m + 1); j++)
	{
		SET_STRING_ELT(ansNames, j, STRING_ELT(Xi, mcum));
		mcum += namesCount[j];
	}

	Rf_setAttrib(ansNew, R_NamesSymbol, ansNames);
	R_len_t jstart = 0;
	R_len_t jend = start;
	R_len_t *jcounts = (R_len_t *)R_alloc((size_t)(m + 1), sizeof(R_len_t));
	memcpy(jcounts, namesCount, (size_t)(m + 1) * sizeof(R_len_t));

	for (R_len_t j = 0; j < (m + 1); j++)
	{
		jstart += (j == 0 ? start : jcounts[j - 1]);
		jend += jcounts[j];

		if ((jend - jstart) > 1 ||
			(lvl < nlvls && STRING_ELT(Xi1, jstart) != NA_STRING))
			SET_VECTOR_ELT(ansNew, j, C_fill_unmelt(X, Xval, namesCount, lvl + 1, nlvls, jstart, jend));
		else
			SET_VECTOR_ELT(ansNew, j, VECTOR_ELT(Xval, jstart));
	}

	UNPROTECT(2);
	return ansNew;
}
