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
	int fxparents;	 // .xparents present in f
	int fxsiblings;	 // .xsiblings present in f
	int pArgs;		 // number of arguments condition
	int pxname;		 // .xname present in condition
	int pxpos;		 // .xpos present in condition
	int pxparents;	 // .xparents present in condition
	int pxsiblings;	 // .xsiblings present in condition
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
static SEXP C_lang7(SEXP s, SEXP t, SEXP u, SEXP v, SEXP w, SEXP x, SEXP y);
static SEXP C_int2char(int i);
static void C_copyAttrs(SEXP obj, SEXP ans, SEXP names, Rboolean copyAttrs);
static int C_matchClass(SEXP obj, SEXP classes);
static void C_traverse(SEXP X, CountGlobal *count, int depth);
static SEXP C_eval_list(SEXP env, SEXP Xi, SEXP fcall, SEXP pcall, SEXP classes, SEXP deflt, SEXP xsym, SEXP *xparents, SEXP xsiblings, PROTECT_INDEX ipx, Args args, CountGlobal *countglobal, CountLocal countlocal, R_len_t (**xinfo)[3], R_len_t **xloc, R_len_t **xdepth);
static SEXP C_fill_list(SEXP Xi, R_len_t (*xinfo)[3], R_len_t *buf, R_len_t node, R_len_t maxnodes, R_len_t ibuf);
static void C_fill_flat(SEXP ansNew, SEXP Xi, R_len_t (*xinfo)[3], R_len_t *ix, R_len_t *ians, R_len_t maxnodes, int feverywhere);
static void C_fill_flat_names(SEXP ansNew, SEXP newNames, SEXP Xi, SEXP name, R_len_t (*xinfo)[3], R_len_t *ix, R_len_t *ians, R_len_t maxnodes, int feverywhere);
static void C_fill_melt(SEXP ansFlat, SEXP ansNames, SEXP Xi, SEXP name, R_len_t (*xinfo)[3], R_len_t *ix, R_len_t *ians, R_len_t maxnodes, int feverywhere);
SEXP C_rrapply(SEXP env, SEXP X, SEXP FUN, SEXP argsFun, SEXP PRED, SEXP argsPred, SEXP classes, SEXP how, SEXP deflt, SEXP R_dfaslist, SEXP R_feverywhere);
/* ---------------------- */

/* Main function */

SEXP C_rrapply(SEXP env, SEXP X, SEXP FUN, SEXP argsFun, SEXP PRED, SEXP argsPred, SEXP classes, SEXP R_how, SEXP deflt, SEXP R_dfaslist, SEXP R_feverywhere)
{
	SEXP ans, ansptr, xptr, names, xsym, xname, xpos, xparents, xsiblings, R_xparents, R_fcall, R_pcall;

	/* protect calls */
	int nprotect = 0;

	/* integer arguments */
	Args R_args;
	R_args.how_C = INTEGER_ELT(R_how, 0) - 1;
	R_args.fArgs = 0;
	R_args.fxname = INTEGER_ELT(argsFun, 0) > 0;
	R_args.fxpos = INTEGER_ELT(argsFun, 1) > 0;
	R_args.fxparents = INTEGER_ELT(argsFun, 2) > 0;
	R_args.fxsiblings = INTEGER_ELT(argsFun, 3) > 0;
	R_args.pArgs = 0;
	R_args.pxname = INTEGER_ELT(argsPred, 0) > 0;
	R_args.pxpos = INTEGER_ELT(argsPred, 1) > 0;
	R_args.pxparents = INTEGER_ELT(argsPred, 2) > 0;
	R_args.pxsiblings = INTEGER_ELT(argsPred, 3) > 0;
	R_args.dfaslist = LOGICAL_ELT(R_dfaslist, 0);
	R_args.feverywhere = INTEGER_ELT(R_feverywhere, 0);

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

	PROTECT_INDEX parentipx;
	if (R_args.fxparents || R_args.pxparents)
		PROTECT_WITH_INDEX(R_xparents = Rf_allocVector(STRSXP, initGlobal.depthmax), &parentipx);
	else
		PROTECT_WITH_INDEX(R_xparents = Rf_ScalarString(NA_STRING), &parentipx);
	nprotect++;

	/* install arguments and initialize call objects */
	xsym = Rf_install("X");
	xname = Rf_install(".xname");
	xpos = Rf_install(".xpos");
	xparents = Rf_install(".xparents");
	xsiblings = Rf_install(".xsiblings");

	if (Rf_isFunction(FUN))
	{
		/* call definitions depend on presence of special arguments */
		R_args.fArgs = 1 + R_args.fxname + R_args.fxpos + R_args.fxparents + R_args.fxsiblings;

		switch (R_args.fArgs)
		{
		case 1:
			R_fcall = PROTECT(Rf_lang3(FUN, xsym, R_DotsSymbol));
			break;
		case 2:
			R_fcall = PROTECT(Rf_lang4(FUN, xsym, R_NilValue, R_DotsSymbol));
			break;
		case 3:
			R_fcall = PROTECT(Rf_lang5(FUN, xsym, R_NilValue, R_NilValue, R_DotsSymbol));
			break;
		case 4:
			R_fcall = PROTECT(Rf_lang6(FUN, xsym, R_NilValue, R_NilValue, R_NilValue, R_DotsSymbol));
			break;
		case 5:
			R_fcall = PROTECT(C_lang7(FUN, xsym, R_NilValue, R_NilValue, R_NilValue, R_NilValue, R_DotsSymbol));
			break;
		default:
			R_fcall = PROTECT(Rf_lang3(FUN, xsym, R_DotsSymbol)); // not reached
		}
		nprotect++;

		SEXP fCDR = CDR(R_fcall);

		if (R_args.fxname)
		{
			fCDR = CDR(fCDR);
			SET_TAG(fCDR, xname);
		}
		if (R_args.fxpos)
		{
			fCDR = CDR(fCDR);
			SET_TAG(fCDR, xpos);
		}
		if (R_args.fxparents)
		{
			fCDR = CDR(fCDR);
			SET_TAG(fCDR, xparents);
		}
		if (R_args.fxsiblings)
		{
			SET_TAG(CDR(fCDR), xsiblings);
		}
	}
	else
	{
		R_fcall = FUN;
	}

	if (Rf_isFunction(PRED))
	{
		/* call definitions depend on presence of special arguments */
		R_args.pArgs = 1 + R_args.pxname + R_args.pxpos + R_args.pxparents + R_args.pxsiblings;

		switch (R_args.pArgs)
		{
		case 1:
			R_pcall = PROTECT(Rf_lang3(PRED, xsym, R_DotsSymbol));
			break;
		case 2:
			R_pcall = PROTECT(Rf_lang4(PRED, xsym, R_NilValue, R_DotsSymbol));
			break;
		case 3:
			R_pcall = PROTECT(Rf_lang5(PRED, xsym, R_NilValue, R_NilValue, R_DotsSymbol));
			break;
		case 4:
			R_pcall = PROTECT(Rf_lang6(PRED, xsym, R_NilValue, R_NilValue, R_NilValue, R_DotsSymbol));
			break;
		case 5:
			R_pcall = PROTECT(C_lang7(PRED, xsym, R_NilValue, R_NilValue, R_NilValue, R_NilValue, R_DotsSymbol));
			break;
		default:
			R_pcall = PROTECT(Rf_lang3(PRED, xsym, R_DotsSymbol)); // not reached
		}
		nprotect++;

		SEXP pCDR = CDR(R_pcall);

		if (R_args.pxname)
		{
			pCDR = CDR(pCDR);
			SET_TAG(pCDR, xname);
		}
		if (R_args.pxpos)
		{
			pCDR = CDR(pCDR);
			SET_TAG(pCDR, xpos);
		}
		if (R_args.pxparents)
		{
			pCDR = CDR(pCDR);
			SET_TAG(pCDR, xparents);
		}
		if (R_args.pxsiblings)
		{
			SET_TAG(CDR(pCDR), xsiblings);
		}
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
	ansptr = NULL; /* avoid unitialized warning */
	xptr = NULL;
	if (Rf_isPairList(X))
		xptr = X;

	if ((Rf_isVectorList(X) && (R_args.how_C == 1 || R_args.how_C == 2)) || (Rf_isPairList(X) && R_args.how_C > 0))
	{
		ans = PROTECT(Rf_allocVector(VECSXP, n));
		C_copyAttrs(X, ans, names, !Rf_isPairList(X));
		if (Rf_isPairList(X))
			Rf_copyMostAttrib(X, ans);
	}
	else
	{
		ans = PROTECT(Rf_shallow_duplicate(X));
		if (Rf_isPairList(X))
			ansptr = ans;
	}
	nprotect += 2;

	/* traverse list to evaluate function calls */
	for (R_len_t i = 0; i < n; i++)
	{
		/* increment location counter */
		xloc[0] += 1;
		/* update parent names */
		SET_STRING_ELT(R_xparents, 0, Rf_isNull(names) ? NA_STRING : STRING_ELT(names, i));

		/* update current node info for list pruning */
		if (R_args.how_C > 2)
		{
			/* reallocate array if necessary in this case */
			if (R_args.feverywhere == 2 && (initGlobal.node + 1) >= initGlobal.maxnodes)
			{
				xinfo = (R_len_t(*)[3])S_realloc((char *)xinfo, 2 * initGlobal.maxnodes, initGlobal.maxnodes, sizeof(*xinfo));
				// if (R_args.how_C == 5)
				// 	xdepth = (R_len_t *)S_realloc((char *)xdepth, 2 * initGlobal.maxnodes, initGlobal.maxnodes, sizeof(R_len_t));
				initGlobal.maxnodes *= 2;
			}

			initGlobal.node++;				// increment node counter
			xinfo[initGlobal.node][1] = -1; // parent node counter
			xinfo[initGlobal.node][2] = i;	// child node counter

			if (R_args.how_C == 5)
				xdepth[initGlobal.node] = 0; // current depth counter (only for melting)
		}

		/* main recursion part */
		if (Rf_isVectorList(X))
		{
			SET_VECTOR_ELT(ans, i, C_eval_list(env, VECTOR_ELT(X, i), R_fcall, R_pcall, classes, deflt, xsym, &R_xparents, (R_args.fxsiblings || R_args.pxsiblings) ? X : R_NilValue, parentipx, R_args, &initGlobal, initLocal, &xinfo, &xloc, &xdepth));
		}
		else if (Rf_isPairList(X))
		{
			if (R_args.how_C > 0)
			{
				SET_VECTOR_ELT(ans, i, C_eval_list(env, CAR(xptr), R_fcall, R_pcall, classes, deflt, xsym, &R_xparents, (R_args.fxsiblings || R_args.pxsiblings) ? X : R_NilValue, parentipx, R_args, &initGlobal, initLocal, &xinfo, &xloc, &xdepth));
			}
			else
			{
				SETCAR(ansptr, C_eval_list(env, CAR(xptr), R_fcall, R_pcall, classes, deflt, xsym, &R_xparents, (R_args.fxsiblings || R_args.pxsiblings) ? X : R_NilValue, parentipx, R_args, &initGlobal, initLocal, &xinfo, &xloc, &xdepth));
				ansptr = CDR(ansptr);
			}
			xptr = CDR(xptr);
		}
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
					C_fill_flat(ansNew, VECTOR_ELT(ans, i), xinfo, &ix, &ians, initGlobal.maxnodes, R_args.feverywhere);
			}
			else
			{
				SEXP newNames = PROTECT(Rf_allocVector(STRSXP, m));
				for (R_len_t i = 0; i < Rf_length(ans); i++)
					C_fill_flat_names(ansNew, newNames, VECTOR_ELT(ans, i), Rf_isNull(names) ? NA_STRING : STRING_ELT(names, i), xinfo, &ix, &ians, initGlobal.maxnodes, R_args.feverywhere);

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

				C_fill_melt(ansFlat, ansNames, VECTOR_ELT(ans, i), STRING_ELT(namesNew, i), xinfo, &ix, &ians, initGlobal.maxnodes, R_args.feverywhere);
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

/* Helper functions */

/* create long language object */
static SEXP C_lang7(SEXP s, SEXP t, SEXP u, SEXP v, SEXP w, SEXP x, SEXP y)
{
	PROTECT(s);
	s = Rf_lcons(s, Rf_list6(t, u, v, w, x, y));
	UNPROTECT(1);
	return s;
}

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

static void C_traverse(SEXP X, CountGlobal *count, int depth)
{
	SEXP Xi;
	SEXP xptr = X;
	/* increment max depth if current depth is higher than max depth */
	R_len_t n = Rf_length(X);
	depth++;
	count->maxnodes += n;
	count->depthmax += (depth > count->depthmax);

	for (R_len_t i = 0; i < n; i++)
	{
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
	SEXP *xparents,			  // current value .xname and .xparents arguments
	SEXP xsiblings,			  // current value .xsiblings argument
	PROTECT_INDEX ipx,		  // protection index xparents
	Args args,				  // integer arguments
	CountGlobal *countglobal, // global node counters
	CountLocal countlocal,	  // local node counters
	R_len_t (**xinfo)[3],	  // array with node position information
	R_len_t **xloc,			  // current value .pos argument
	R_len_t **xdepth		  // current depth (only used for melting)
)
{
	/* initialize function value */
	SEXP funVal;
	PROTECT_INDEX childipx;
	PROTECT_WITH_INDEX(funVal = Rf_lazy_duplicate(Xi), &childipx);
	int nprotect = 1;

	/* if Xi is list (and data.frame is treated as list if !dfaslist)
	   and !feverywhere recurse, otherwise evaluate functions */
	int doRecurse = 0;

	if (args.feverywhere < 1 && ((Rf_isVectorList(Xi) || Rf_isPairList(Xi)) && TYPEOF(Xi) != NILSXP))
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
		SEXP xname_val = NULL; /* avoid uninitialized warnings */
		SEXP xpos_val = NULL;
		SEXP xparents_val = NULL;
		SEXP xsiblings_val = NULL;
		int nargprotect = 0;

		/* define X argument */
		Rf_defineVar(xsym, Xi, env);
		INCREMENT_NAMED(Xi);

		/* update current .xname value */
		if (args.fxname || args.pxname)
		{
			if (args.fxparents || args.pxparents)
				xname_val = PROTECT(Rf_ScalarString(STRING_ELT(*xparents, countlocal.depth)));
			else
				xname_val = PROTECT(Rf_duplicate(*xparents));
			nargprotect++;
		}

		/* update current .xpos value */
		if (args.fxpos || args.pxpos)
		{
			xpos_val = PROTECT(Rf_allocVector(INTSXP, countlocal.depth + 1));
			for (R_len_t k = 0; k < (countlocal.depth + 1); k++)
				SET_INTEGER_ELT(xpos_val, k, (int)((*xloc)[k]));
			nargprotect++;
		}

		/* update current .xparents value */
		if (args.fxparents || args.pxparents)
		{
			xparents_val = PROTECT(Rf_allocVector(STRSXP, countlocal.depth + 1));
			for (R_len_t k = 0; k < (countlocal.depth + 1); k++)
				SET_STRING_ELT(xparents_val, k, STRING_ELT(*xparents, k));
			nargprotect++;
		}

		/* update current .xsiblings value */
		if (args.fxsiblings || args.pxsiblings)
		{
			if (Rf_isPairList(xsiblings))
			{
				R_len_t n = Rf_length(xsiblings);
				xsiblings_val = PROTECT(Rf_allocVector(VECSXP, n));
				SEXP xsiblingsptr = xsiblings;
				for (R_len_t i = 0; i < n; i++)
				{
					SET_VECTOR_ELT(xsiblings_val, i, CAR(xsiblingsptr));
					xsiblingsptr = CDR(xsiblingsptr);
				}
				Rf_copyMostAttrib(xsiblings, xsiblings_val);
				Rf_setAttrib(xsiblings_val, R_NamesSymbol, PROTECT(Rf_getAttrib(xsiblings, R_NamesSymbol)));
				UNPROTECT(1);
			}
			else
			{
				xsiblings_val = PROTECT(Rf_duplicate(xsiblings));
			}
			nargprotect++;
		}

		/* define f special arguments */
		if (args.fArgs > 1)
		{
			SEXP fCDR = CDDR(fcall);

			if (args.fxname)
			{
				SETCAR(fCDR, xname_val);
				fCDR = CDR(fCDR);
			}
			if (args.fxpos)
			{
				SETCAR(fCDR, xpos_val);
				fCDR = CDR(fCDR);
			}
			if (args.fxparents)
			{
				SETCAR(fCDR, xparents_val);
				fCDR = CDR(fCDR);
			}
			if (args.fxsiblings)
			{
				SETCAR(fCDR, xsiblings_val);
			}
		}

		/* define condition special arguments */
		if (args.pArgs > 1)
		{
			SEXP pCDR = CDDR(pcall);

			if (args.pxname)
			{
				SETCAR(pCDR, xname_val);
				pCDR = CDR(pCDR);
			}
			if (args.pxpos)
			{
				SETCAR(pCDR, xpos_val);
				pCDR = CDR(pCDR);
			}
			if (args.pxparents)
			{
				SETCAR(pCDR, xparents_val);
				pCDR = CDR(pCDR);
			}
			if (args.pxsiblings)
			{
				SETCAR(pCDR, xsiblings_val);
			}
		}

		UNPROTECT(nargprotect);

		/* evaluate predicate */
		int doEval = TRUE;
		int matched = FALSE;
		int skip = FALSE;

		/* skip empty symbols */
		if (Rf_isSymbol(Xi))
		{
			const char *str = CHAR(PRINTNAME(Xi));
			if (strlen(str) < 1)
				skip = TRUE;
		}

		/* match classes argument */
		if (strcmp(CHAR(STRING_ELT(classes, 0)), "ANY") == 0) /* ASCII */
			matched = TRUE;
		else
			matched = C_matchClass(Xi, classes);

		if (matched && !skip && args.pArgs > 0)
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
		if (matched && !skip && doEval)
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
						(*xinfo)[i1][0] = 2; // non-terminal node
						i2 = (*xinfo)[i1][1];
					}
					else
						break;
				}
			}

			/* evaluate f */
			if (args.fArgs > 0)
			{
				REPROTECT(funVal = R_forceAndCall(fcall, args.fArgs, env), childipx);
			}
			/* recurse further with new list (type 2) if feverywhere == 2 */
			if (args.feverywhere == 2 && ((Rf_isVectorList(funVal) || Rf_isPairList(funVal)) && TYPEOF(funVal) != NILSXP))
			{
				doRecurse = 2;
			}
			else /* otherwise return current value */
			{
				UNPROTECT(nprotect);
				return funVal;
			}
		}
		else if (args.feverywhere > 0 && !skip && ((Rf_isVectorList(Xi) || Rf_isPairList(Xi)) && TYPEOF(Xi) != NILSXP))
		{
			/* recurse further with original list (type 1) */
			doRecurse = 1;
		}
		else
		{
			/* return original list (or default) here if feverywhere == 0 */
			if (args.how_C == 1 || args.how_C == 2)
			{
				UNPROTECT(nprotect);
				return Rf_lazy_duplicate(deflt);
			}
			else
			{
				UNPROTECT(nprotect);
				return funVal;
			}
		}
	}

	if (doRecurse > 0)
	{
		/* create new object for recursion */
		SEXP Xnew;
		SEXP xnewptr = NULL; /* avoid unitialized warning */
		SEXP funptr = NULL;
		if (Rf_isPairList(funVal))
			funptr = funVal;

		R_len_t m = Rf_length(funVal);
		SEXP names = PROTECT(Rf_getAttrib(funVal, R_NamesSymbol));
		if ((Rf_isVectorList(funVal) && (args.how_C == 1 || args.how_C == 2)) || (Rf_isPairList(funVal) && args.how_C > 0))
		{
			/* VECEXP initializes with R_NilValues */
			Xnew = PROTECT(Rf_allocVector(VECSXP, m));
			C_copyAttrs(funVal, Xnew, names, !Rf_isPairList(funVal));
			if (Rf_isPairList(funVal))
				Rf_copyMostAttrib(funVal, Xnew);
		}
		else
		{
			Xnew = PROTECT(Rf_shallow_duplicate(funVal));
			if (Rf_isPairList(funVal))
				xnewptr = Xnew;
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
				/* reallocate location vectors */
				*xloc = (R_len_t *)S_realloc((char *)*xloc, 2 * countglobal->depthmax, countglobal->depthmax, sizeof(R_len_t));

				if (args.fxparents || args.pxparents)
				{
					SEXP xparentsNew = PROTECT(Rf_allocVector(STRSXP, 2 * countglobal->depthmax));
					for (R_len_t i = 0; i < countglobal->depthmax; i++)
						SET_STRING_ELT(xparentsNew, i, STRING_ELT(*xparents, i));

					REPROTECT(*xparents = xparentsNew, ipx);
					UNPROTECT(1);
				}
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
						// if (args.how_C == 5)
						// 	*xdepth = (R_len_t *)S_realloc((char *)*xdepth, 2 * countglobal->maxnodes, countglobal->maxnodes, sizeof(R_len_t));
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
			/* update parent names */
			SET_STRING_ELT(*xparents, (args.fxparents || args.pxparents) ? countlocal.depth : 0, Rf_isNull(names) ? NA_STRING : STRING_ELT(names, j));

			/* evaluate list element */
			if (Rf_isVectorList(funVal))
			{
				SET_VECTOR_ELT(Xnew, j, C_eval_list(env, VECTOR_ELT(funVal, j), fcall, pcall, classes, deflt, xsym, xparents, (args.fxsiblings || args.pxsiblings) ? funVal : R_NilValue, ipx, args, countglobal, countlocal, xinfo, xloc, xdepth));
			}
			else if (Rf_isPairList(funVal))
			{
				if (args.how_C > 0)
				{
					SET_VECTOR_ELT(Xnew, j, C_eval_list(env, CAR(funptr), fcall, pcall, classes, deflt, xsym, xparents, (args.fxsiblings || args.pxsiblings) ? funVal : R_NilValue, ipx, args, countglobal, countlocal, xinfo, xloc, xdepth));
				}
				else
				{
					SETCAR(xnewptr, C_eval_list(env, CAR(funptr), fcall, pcall, classes, deflt, xsym, xparents, (args.fxsiblings || args.pxsiblings) ? funVal : R_NilValue, ipx, args, countglobal, countlocal, xinfo, xloc, xdepth));
					xnewptr = CDR(xnewptr);
				}
				funptr = CDR(funptr);
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
			maxparent++;
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
static void C_fill_flat(SEXP ansNew, SEXP Xi, R_len_t (*xinfo)[3], R_len_t *ix, R_len_t *ians, R_len_t maxnodes, int feverywhere)
{
	(*ix)++;					// increment counter
	if (xinfo[*ix - 1][0] == 1) // terminal node
	{
		SET_VECTOR_ELT(ansNew, *ians, Xi);
		(*ians)++;

		/* stop incrementing if no longer an (indirect) child node */
		if (feverywhere == 2 && Rf_isVectorList(Xi))
		{
			R_len_t ixstart = *ix - 1;
			while ((*ix) < maxnodes && xinfo[*ix][1] >= ixstart && xinfo[*ix][1] <= (*ix))
				(*ix)++;
		}
	}
	else if (Rf_isVectorList(Xi))
	{
		for (R_len_t i = 0; i < Rf_length(Xi); i++)
			C_fill_flat(ansNew, VECTOR_ELT(Xi, i), xinfo, ix, ians, maxnodes, feverywhere);
	}
}

/* fill flat list with names */
static void C_fill_flat_names(SEXP ansNew, SEXP newNames, SEXP Xi, SEXP name, R_len_t (*xinfo)[3], R_len_t *ix, R_len_t *ians, R_len_t maxnodes, int feverywhere)
{
	(*ix)++;					// increment counter
	if (xinfo[*ix - 1][0] == 1) // terminal node
	{
		SET_VECTOR_ELT(ansNew, *ians, Xi);
		SET_STRING_ELT(newNames, *ians, name);
		(*ians)++;

		/* stop incrementing if no longer an (indirect) child node */
		if (feverywhere == 2 && Rf_isVectorList(Xi))
		{
			R_len_t ixstart = *ix - 1;
			while ((*ix) < maxnodes && xinfo[*ix][1] >= ixstart && xinfo[*ix][1] <= (*ix))
				(*ix)++;
		}
	}
	else if (Rf_isVectorList(Xi))
	{
		SEXP names = PROTECT(Rf_getAttrib(Xi, R_NamesSymbol));
		for (R_len_t i = 0; i < Rf_length(Xi); i++)
			C_fill_flat_names(ansNew, newNames, VECTOR_ELT(Xi, i), Rf_isNull(names) ? NA_STRING : STRING_ELT(names, i), xinfo, ix, ians, maxnodes, feverywhere);

		UNPROTECT(1);
	}
}

/* fill melted data.frame */
static void C_fill_melt(SEXP ansFlat, SEXP ansNames, SEXP Xi, SEXP name, R_len_t (*xinfo)[3], R_len_t *ix, R_len_t *ians, R_len_t maxnodes, int feverywhere)
{
	// add name to vector
	SET_STRING_ELT(ansNames, *ix, name);
	(*ix)++; // increment counter

	if (xinfo[*ix - 1][0] == 1) // terminal nodes only
	{
		SET_VECTOR_ELT(ansFlat, *ians, Xi);
		(*ians)++;

		/* stop incrementing if no longer an (indirect) child node */
		if (feverywhere == 2 && Rf_isVectorList(Xi))
		{
			R_len_t ixstart = *ix - 1;
			while ((*ix) < maxnodes && xinfo[*ix][1] >= ixstart && xinfo[*ix][1] <= (*ix))
			{
				SET_STRING_ELT(ansNames, *ix, NA_STRING);
				(*ix)++;
			}
		}
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
			// use counter for missing names
			if (noNames)
				SET_STRING_ELT(names, i, C_int2char(i + 1));

			// recurse further
			C_fill_melt(ansFlat, ansNames, VECTOR_ELT(Xi, i), STRING_ELT(names, i), xinfo, ix, ians, maxnodes, feverywhere);
		}
		UNPROTECT(1);
	}
}
