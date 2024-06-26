#ifndef RRAPPLY_H
#define RRAPPLY_H

#include <R.h>
#include <Rinternals.h>

// fixed arguments between function calls
typedef struct FixedArgs
{
    // 'how' argument
    // 0: 'replace', replace without default and return nested list
    // 1: 'list', replace with default and return nested list
    // 2: 'unlist', replace with default and unlist
    // 3: 'prune', replace without default and return pruned list
    // 4: 'flatten', replace without default and return flat list
    // 5: 'melt', return without default and return melted list
    // 6: 'bind', return without default and return wide list 
    // 7: 'names', return without default and return renamed list
    SEXP ans_ptr;      // pointer to result object (only for flatten, binding and melting)
    SEXP ansnames_ptr; // pointer to result object names (only for flatten, binding and melting)
    SEXP ansnamecols_ptr; // pointer to binding name columns (only for binding)
    int how;
    int dfaslist;
    int feverywhere;
    int depthmax;        // maximum allowed depth
    R_len_t maxnodes;    // maximum allowed node count
    R_len_t maxleafs;    // maximum allowed terminal node count
    Rboolean anynames;   // any names present (only for flatten)
    Rboolean anysymbol;  // any symbols present (only for melting and binding)
    int ans_flags;       // coerce to flagged type (only for flatten and melting), refer to main/bind.c
    int ans_namecols;    // add name-columns (only for binding)
    const char *ans_sep; // name separator (only for flatten, binding)
    int ans_depthmax;    // observed maximum depth (only for melting)
    int ans_depthpivot;  // pivot depth (only for binding)
    R_len_t ans_maxrows;     // max rows (only for binding)
} FixedArgs;

// initialize part of fixed arguments
void C_traverse(FixedArgs *fixedArgs, SEXP X, int depth);
void C_traverse_bind(FixedArgs *fixedArgs, SEXP X, int depth);
void C_count_rows(FixedArgs *fixedArgs, SEXP X, int depth);

// function call info
typedef struct FunCall
{
    SEXP call;     // C call definition
    Rboolean evaluate;  // evaluate call
    int nargs;     // number of arguments
    int xname;     // .xname present
    int xpos;      // .xpos present
    int xparents;  // .xparents present
    int xsiblings; // .xsiblings present
} FunCall;

// variable arguments local to function calls
typedef struct LocalArgs
{
    SEXP xparent_ptr;            // current value .xparents
    SEXP xsiblings_ptr;          // current value .xsiblings
    R_len_t node;                // current node counter (only for pruning)
    int depth;                   // current depth
    R_len_t ans_idx;             // current result index (only used for flatten and melting)
    R_len_t *xpos_vec;           // array with current value .xpos argument
    R_len_t *xinfo_array;        // array with node position information (only for pruning)
    R_len_t ans_row;             // current result row (only used for binding)
    PROTECT_INDEX xparent_ipx;   // xparents protection pointer
    Rboolean *nms_update;        // update names at current layer (only for names)
} LocalArgs;

/* prototypes */

// helper functions
SEXP C_lang7(SEXP s, SEXP t, SEXP u, SEXP v, SEXP w, SEXP x, SEXP y);
SEXP C_int2char(int i, Rboolean prefix);
SEXP C_strcat(SEXP names, int start, int end, const char *sep);
void C_copyAttrs(SEXP obj, SEXP ans, SEXP names, Rboolean copyAttrs);
Rboolean C_matchClass(SEXP obj, SEXP classes);
void C_coerceList(SEXP ans, SEXP newans, R_len_t newlen, SEXPTYPE type);
int C_answerType(SEXP x);

// main recursion functions
SEXP C_recurse_list(SEXP env, SEXP Xi, FunCall f, FunCall condition, FixedArgs *fixedArgs, LocalArgs *localArgs, SEXP classes, SEXP deflt, SEXP xsym);
void C_recurse_flatten(SEXP env, SEXP Xi, FunCall f, FunCall condition, FixedArgs *fixedArgs, LocalArgs *localArgs, SEXP classes, SEXP xsym);
SEXP C_prune_list(SEXP Xi, R_len_t *xinfo, R_len_t *buf, R_len_t node, R_len_t maxnodes, R_len_t newmaxnodes, R_len_t ibuf);
SEXP C_rrapply(SEXP env, SEXP X, SEXP FUN, SEXP argsFun, SEXP PRED, SEXP argsPred, SEXP classes, SEXP how, SEXP deflt, SEXP R_dfaslist, SEXP R_feverywhere, SEXP options);
SEXP C_unmelt(SEXP X);

#endif
