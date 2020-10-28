#' @title Reimplementation of base-R's rapply
#' 
#' @description
#' \code{rrapply} is a reimplemented and extended version of \code{\link{rapply}} to recursively apply a function \code{f} to 
#' a set of elements of a list and deciding \emph{how} the result is structured. 
#' 
#' @section How to structure result:
#' In addition to \code{\link{rapply}}'s modes to set \code{how} equal to \code{"replace"}, \code{"list"} or \code{"unlist"}, 
#' five choices \code{"prune"}, \code{"flatten"}, \code{"melt"}, \code{"unmelt"} and \code{how = "recurse"} are available. \code{how = "prune"} filters 
#' all list elements not subject to application of \code{f} from the list \code{object}. The original list structure is retained, 
#' similar to the non-pruned options \code{how = "replace"} or \code{how = "list"}. \code{how = "flatten"} is an efficient way to 
#' return a flattened unnested version of the pruned list. \code{how = "melt"} returns a melted data.frame of the pruned list, 
#' each row contains the path of a single terminal node in the pruned list at depth layers \code{L1}, \code{L2}, and so on. The list-column 
#' \code{"value"} contains the values at the terminal nodes and is equivalent to the flattened list returned by \code{how = "flatten"}. 
#' If no list names are present, the node names in the data.frame default to the indices of the list elements \code{"..1"}, \code{"..2"}, etc.
#' \code{how = "unmelt"} is a special case that reconstructs a nested list from a melted data.frame. For this reason, \code{how = "unmelt"} 
#' only applies to data.frames in the same format as returned by \code{how = "melt"}. Internally, \code{how = "unmelt"} first reconstructs 
#' a nested list from the melted data.frame and second uses the same framework as \code{how = "replace"}. \code{how = "recurse"} is a specialized
#' option that is only useful in combination with e.g. \code{how = "list"} to recurse further into updated \dQuote{list-like} elements. 
#' This is explained in more detail below.
#' 
#' @section Condition function:
#' Both \code{\link{rapply}} and \code{rrapply} allow to apply \code{f} to list elements of certain classes via the \code{classes} argument. 
#' \code{rrapply} generalizes this concept via an additional \code{condition} argument, which accepts any function to use as a condition 
#' or predicate to select list elements to which \code{f} is applied. Conceptually, the \code{f} function is applied to all list elements for 
#' which the \code{condition} function exactly evaluates to \code{TRUE} similar to \code{\link{isTRUE}}. If the condition function is missing, 
#' \code{f} is applied to all list elements.
#' Since the \code{condition} function generalizes the \code{classes} argument, it is allowed to use the \code{deflt} argument 
#' together with \code{how = "list"} or \code{how = "unlist"} to set a default value to all list elements for which the \code{condition} does 
#' not evaluate to \code{TRUE}.
#'
#' @section Correct use of \code{...}:
#' The principal argument of the \code{f} and \code{condition} functions evaluates to the content of the list element. Any further arguments to 
#' \code{f} and \code{condition} (besides the special arguments \code{.xname} and \code{.xpos} discussed below) supplied via the dots \code{...} 
#' argument need to be defined as function arguments in \emph{both} the \code{f} and \code{condition} function (if existing), even if they are not
#'  used in the function itself. See also the \sQuote{Examples} section.
#' 
#' @section Special arguments \code{.xname}, \code{.xpos}, \code{.xparents} and \code{.xsiblings}:
#' The \code{f} and \code{condition} functions accept four special arguments \code{.xname}, \code{.xpos}, \code{.xparents} and \code{.xsiblings} in
#' addition to the first principal argument. The \code{.xname} argument evaluates to the name of the list element. The \code{.xpos} argument evaluates 
#' to the position of the element in the nested list structured as an integer vector. That is, if \code{x = list(list("y", "z"))}, then an \code{.xpos} 
#' location of \code{c(1, 2)} corresponds to the list element \code{x[[c(1, 2)]]}. The \code{.xparents} argument evaluates to a vector of all parent 
#' node names in the path to the list element. The \code{.xsiblings} argument evaluates to the complete (sub)list that includes the list element as a direct child.
#' The names \code{.xname}, \code{.xpos}, \code{.xparents} or \code{.xsiblings} need to be explicitly included as function arguments in \code{f} and 
#' \code{condition} (in addition to the principal argument). See the package vignette for example uses of these special variables.
#' 
#' @section Avoid recursing into list nodes:
#' By default, \code{rrapply} recurses into any \dQuote{list-like} element. If \code{classes = "list"}, this behavior is overridden and the 
#' \code{f} function is also applied to any list element of \code{object} that satisfies \code{condition}. For expression objects, use 
#' \code{classes = "language"}, \code{classes = "expression"} or \code{classes = "pairlist"} to avoid recursing into branches of the abstract 
#' syntax tree of \code{object}. If the \code{condition} or \code{classes} arguments are not satisfied for a \dQuote{list-like} element, 
#' \code{rrapply} will recurse further into the sublist, apply the \code{f} function to the nodes that satisfy \code{condition} and \code{classes}, 
#' and so on. Note that this behavior can only be triggered using the \code{classes} argument and not the \code{condition} argument.
#' 
#' @section Recursive list node updating:
#' If \code{classes = "list"} and \code{how = "recurse"}, \code{rrapply} applies the \code{f} function to any list element of \code{object} that satisfies 
#' \code{condition} similar to the previous section using \code{how = "replace"}, but recurses further into the \emph{updated} list-like element 
#' after application of the \code{f} function. The primary use of \code{how = "recurse"} in combination with \code{classes = "list"} is to 
#' recursively update for instance the names of all nodes in a nested list. Additional examples are found in the package vignette.
#' 
#' @section Avoid recursing into data.frames:
#' If \code{classes = "ANY"} (default), \code{rrapply} recurses into all \dQuote{list-like} objects equivalent to \code{\link{rapply}}. 
#' Since data.frames are \dQuote{list-like} objects, the \code{f} function will descend into the individual columns of a data.frame. 
#' To avoid this behavior, set \code{classes = "data.frame"}, in which case the \code{f} and \code{condition} functions are applied directly to 
#' the data.frame and not its columns. Note that this behavior can only be triggered using the \code{classes} argument and not the \code{condition} argument.
#' 
#' @section List attributes:
#' In \code{\link{rapply}} intermediate list attributes (not located at the leafs) are kept when \code{how = "replace"}, but are dropped when 
#' \code{how = "list"}. To avoid unexpected behavior, \code{rrapply} always preserves intermediate list attributes when using \code{how = "replace"}, 
#' \code{how = "list"} or \code{how = "prune"}. If \code{how = "flatten"} or \code{how = "unlist"} intermediate list attributes cannot be preserved as 
#' the result is no longer a nested list. 
#' 
#' @section Expressions:
#' Call objects and expression vectors are also accepted as \code{object} argument, which are treated as nested lists based on their internal abstract
#' syntax trees. As such, all functionality that applies to nested lists extends directly to call objects and expression vectors. If \code{object} is a 
#' call object or expression vector, \code{how = "replace"} always maintains the type of \code{object}, whereas \code{how = "list"} returns the result 
#' structured as a nested list. \code{how = "prune"}, \code{how = "flatten"} and \code{how = "melt"} return the pruned abstract syntax tree as: a nested list, 
#' a flattened list and a melted data.frame respectively. This is identical to application of \code{rrapply} to the abstract syntax tree formatted as a nested list.
#' 
#' @return If \code{how = "unlist"}, a vector as in \code{\link{rapply}}. If \code{how = "list"}, \code{how = "replace"} or \code{how = "recurse"}, 
#' \dQuote{list-like} of similar structure as \code{object} as in \code{\link{rapply}}. If \code{how = "prune"}, a pruned \dQuote{list-like} object 
#' of similar structure as \code{object} with pruned list elements based on \code{classes} and \code{condition}. If \code{how = "flatten"}, an unnested 
#' pruned list with pruned list elements based on \code{classes} and \code{condition}. If \code{how = "melt"}, a melted data.frame containing the node 
#' paths and values of the pruned list elements based on \code{classes} and \code{condition}. If \code{how = "unmelt"}, a nested list with list names 
#' and values defined in the data.frame \code{object}.
#' 
#'  
#' @note \code{rrapply} allows the \code{f} function argument to be missing, in which case no function is applied to the list 
#' elements.
#' @note \code{how = "unmelt"} requires as input a data.frame as returned by \code{how = "melt"} with character columns to name the nested list components
#' and a final list-column containing the values of the nested list elements. 
#' 
#' @examples
#' # Example data
#' 
#' ## Nested list of renewable energy (%) of total energy consumption per country in 2016
#' data("renewable_energy_by_country")
#' ## Subset values for countries and areas in Oceania
#' renewable_oceania <- renewable_energy_by_country[["World"]]["Oceania"]
#'
#' # List pruning and unnesting
#' 
#' ## Drop all logical NA's while preserving list structure 
#' na_drop_oceania <- rrapply(
#'   renewable_oceania,
#'   f = function(x) x,
#'   classes = "numeric",
#'   how = "prune"
#' )
#' str(na_drop_oceania, list.len = 3, give.attr = FALSE)
#' 
#' ## Drop all logical NA's and return unnested list
#' na_drop_oceania2 <- rrapply(
#'   renewable_oceania,
#'   f = function(x) x,
#'   classes = "numeric",
#'   how = "flatten"
#' )
#' str(na_drop_oceania2, list.len = 10, give.attr = FALSE)
#' 
#' ## Drop all logical NA's and return melted data.frame
#' na_drop_oceania3 <- rrapply(
#'   renewable_oceania,
#'   f = identity,
#'   classes = "numeric",
#'   how = "melt"
#' )
#' 
#' head(na_drop_oceania3)
#' 
#' ## Reconstruct nested list from melted data.frame
#' na_drop_oceania4 <- rrapply(
#'   na_drop_oceania3,
#'   how = "unmelt"
#' )
#' 
#' str(na_drop_oceania4, list.len = 3, give.attr = FALSE)
#' 
#' # Condition function
#' 
#' ## Drop all NA elements using condition function
#' na_drop_oceania3 <- rrapply(
#'   renewable_oceania,
#'   condition = Negate(is.na),
#'   f = function(x) x,
#'   how = "prune"
#' )
#' str(na_drop_oceania3, list.len = 3, give.attr = FALSE)
#' 
#' ## Replace NA elements by a new value via the ... argument
#' ## NB: the 'newvalue' argument should be present as function 
#' ## argument in both 'f' and 'condition', even if unused.
#' na_zero_oceania <- rrapply(
#'   renewable_oceania,
#'   condition = function(x, newvalue) is.na(x),
#'   f = function(x, newvalue) newvalue,
#'   newvalue = 0,
#'   how = "replace"
#' )
#' str(na_zero_oceania, list.len = 3, give.attr = FALSE)
#' 
#' ## Filter all countries with values above 85%
#' renewable_energy_above_85 <- rrapply(
#'   renewable_energy_by_country,
#'   condition = function(x) x > 85,
#'   how = "prune"
#' )
#' str(renewable_energy_above_85, give.attr = FALSE)
#' 
#' # Special arguments .xname, .xpos, .xparents and .xsiblings
#' 
#' ## Apply a function using the name of the node
#' renewable_oceania_text <- rrapply(
#'   renewable_oceania,
#'   f = function(x, .xname) sprintf("Renewable energy in %s: %.2f%%", .xname, x),
#'   condition = Negate(is.na),
#'   how = "flatten"
#' )
#' str(renewable_oceania_text, list.len = 10)
#' 
#' ## Extract values based on country names
#' renewable_benelux <- rrapply(
#'   renewable_energy_by_country,
#'   condition = function(x, .xname) .xname %in% c("Belgium", "Netherlands", "Luxembourg"),
#'   how = "prune"
#' )
#' str(renewable_benelux, give.attr = FALSE)
#' 
#' ## Filter European countries with value above 50%
#' renewable_europe_above_50 <- rrapply(
#'   renewable_energy_by_country,
#'   condition = function(x, .xpos) identical(.xpos[c(1, 2)], c(1L, 5L)) & x > 50,
#'   how = "prune"
#' )
#' str(renewable_europe_above_50, give.attr = FALSE)
#' 
#' ## Filter European countries with value above 50%
#' renewable_europe_above_50 <- rrapply(
#'   renewable_energy_by_country,
#'   condition = function(x, .xparents) "Europe" %in% .xparents & x > 50,
#'   how = "prune"
#' )
#' str(renewable_europe_above_50, give.attr = FALSE)
#' 
#' ## Return position of Sweden in list
#' (xpos_sweden <- rrapply(
#'   renewable_energy_by_country,
#'   condition = function(x, .xname) identical(.xname, "Sweden"),
#'   f = function(x, .xpos) .xpos,
#'   how = "flatten"
#' ))
#' renewable_energy_by_country[[xpos_sweden$Sweden]]
#' 
#' ## Return siblings of Sweden in list
#' siblings_sweden <- rrapply(
#'   renewable_energy_by_country,
#'   condition = function(x, .xsiblings) "Sweden" %in% names(.xsiblings),
#'   how = "flatten"
#' )
#' str(siblings_sweden, list.len = 10, give.attr = FALSE)
#' 
#' # Avoid skipping list nodes
#' 
#' ## Calculate mean value of Europe
#' rrapply(
#'   renewable_energy_by_country,  
#'   classes = "list",
#'   condition = function(x, .xname) .xname == "Europe",
#'   f = function(x) mean(unlist(x), na.rm = TRUE),
#'   how = "flatten"
#' )
#'
#' ## Calculate mean value for each continent
#' renewable_continent_summary <- rrapply(
#'   renewable_energy_by_country,  
#'   classes = "list",
#'   condition = function(x, .xpos) length(.xpos) == 2,
#'   f = function(x) mean(unlist(x), na.rm = TRUE)
#' )
#'
#' ## Antarctica's value is missing
#' str(renewable_continent_summary, give.attr = FALSE)
#' 
#' # List node updating
#' 
#' ## replace country names by M-49 codes
#' renewable_M49 <- rrapply(
#'   list(renewable_energy_by_country), 
#'   classes = "list",
#'   f = function(x) {
#'     names(x) <- vapply(x, attr, character(1L), which = "M49-code")
#'     return(x)
#'   },
#'   how = "recurse"
#' )
#' 
#' str(renewable_M49[[1]], max.level = 3, list.len = 3, give.attr = FALSE)
#' 
#' # List attributes
#' 
#' ## how = "list" preserves all list attributes
#' na_drop_oceania_attr <- rrapply(
#'   renewable_oceania,
#'   f = function(x) replace(x, is.na(x), 0),
#'   how = "list"
#' )
#' str(na_drop_oceania_attr, max.level = 2)
#' 
#' ## how = "prune" also preserves list attributes
#' na_drop_oceania_attr2 <- rrapply(
#'   renewable_oceania,
#'   condition = Negate(is.na),
#'   how = "prune"
#' )
#' str(na_drop_oceania_attr2, max.level = 2)
#' 
#' # Expressions
#'
#' ## Replace logicals by integers
#' call_old <- quote(y <- x <- 1 + TRUE)
#' call_new <- rrapply(call_old, 
#'   classes = "logical", 
#'   f = as.numeric, 
#'   how = "replace"
#' )
#' str(call_new)
#' 
#' ## Update and decompose call object
#' call_ast <- rrapply(call_old, 
#'   f = function(x) ifelse(is.logical(x), as.numeric(x), x), 
#'   how = "list"
#' )
#' str(call_ast)
#' 
#' ## Prune and decompose expression
#' expr <- expression(y <- x <- 1, f(g(2 * pi)))
#' is_new_name <- function(x) !exists(as.character(x), envir = baseenv())
#' expr_prune <- rrapply(expr, 
#'   classes = "name", 
#'   condition = is_new_name, 
#'   how = "prune"
#' )
#' str(expr_prune)
#' 
#' ## Prune and flatten expression
#' expr_flatten <- rrapply(expr, 
#'   classes = "name", 
#'   condition = is_new_name, 
#'   how = "flatten"
#' )
#' str(expr_flatten)
#' 
#' ## Prune and melt expression
#' expr_melt <- rrapply(expr, 
#'   classes = "name", 
#'   condition = is_new_name, 
#'   f = as.character,
#'   how = "melt"
#' )
#' expr_melt
#' 
#' @param object a \code{\link{list}}, \code{\link{expression}} vector, or \code{\link{call}} object, i.e., \dQuote{list-like}.
#' @param f a \code{\link{function}} of one \dQuote{principal} argument and optional special arguments \code{.xname}, \code{.xpos}, \code{.xparents} 
#' and/or \code{.xsiblings} (see \sQuote{Details}), passing further arguments via \code{\dots}.
#' @param condition a condition \code{\link{function}} of one \dQuote{principal} argument and optional special arguments \code{.xname}, \code{.xpos}, 
#' \code{.xparents} and/or \code{.xsiblings} (see \sQuote{Details}), passing further arguments via \code{\dots}.
#' @param classes character vector of \code{\link{class}} names, or \code{"ANY"} to match the class of any terminal node. Include \code{"list"} or \code{"data.frame"}
#' to match the class of non-terminal nodes as well.
#' @param how character string partially matching the eight possibilities given: see \sQuote{Details}.
#' @param deflt the default result (only used if \code{how = "list"} or \code{how = "unlist"}).
#' @param feverywhere deprecated use \code{classes = "list"}, \code{classes = "language"} or \code{classes = "expression"} and optionally \code{how = "recurse"} instead.
#' @param dfaslist deprecated use \code{classes = "data.frame"} instead.
#' @param ... additional arguments passed to the call to \code{f} and \code{condition}.
#' 
#' @aliases rrapply
#' 
#' @seealso \code{\link{rapply}}
#'
#' @useDynLib rrapply, .registration = TRUE
#' @export 
rrapply <- function(object, condition, f, classes = "ANY", deflt = NULL, 
    how = c("replace", "list", "unlist", "prune", "flatten", "melt", "recurse", "unmelt"),
    feverywhere, dfaslist, ...)
{
  ## non-function arguments
  if(!(is.list(object) || is.call(object) || is.expression(object)) || length(object) < 1) 
    stop("'object' argument should be list-like and of length greater than zero")
  
  how <- match.arg(how, c("replace", "list", "unlist", "prune", "flatten", "melt", "recurse", "unmelt"))
  howInt <- match(how, c("replace", "list", "unlist", "prune", "flatten", "melt", "recurse", "unmelt"))
  if(identical(how, "recurse")) howInt <- 1L
  
  if(!missing(feverywhere))
  {
	  warning("'feverywhere' is deprecated, use e.g. classes = 'list' or classes = 'call' instead")
    feverywhere <- match.arg(feverywhere, c("no", "break", "recurse"))
    feverywhere <- match(feverywhere, c("no", "break", "recurse")) - 1L
  }
  else 
    feverywhere <- ifelse(isTRUE(any(classes %in% c("list", "language", "pairlist", "expression"))), 1L + identical(how, "recurse"), 0L)

  if(!missing(dfaslist))
  {
    warning("'dfaslist' is deprecated, use classes = 'data.frame' instead")
    dfaslist <- isTRUE(dfaslist)
  }
  else 
    dfaslist <- isFALSE("data.frame" %in% classes)
  if(length(classes) > 1 && isTRUE("ANY" %in% classes)) classes <- "ANY"

  ## unmelt data.frame to nested list
  if(identical(how, "unmelt"))
  {
    if(is.data.frame(object) && is.list(object[[length(object)]]) && all(vapply(object[, -length(object)], is.character, logical(1))))
    {
      if(nrow(object) > 0 && length(object) > 1)
      {
        object <- .Call(C_unmelt, object)
        how <- "replace"
        howInt <- 1L
      }
      else
        stop("'object' argument must be non-empty and contain at least one naming character column and one value list column")
    }
    else
      stop("'object' argument must be a data.frame consisting of naming character columns and a value list column")
  }
  ## function arguments  
  if(missing(f)) f <- NULL else f <- match.fun(f)
  if(missing(condition)) condition <- NULL else condition <- match.fun(condition)
  
  if(is.null(f) && (is.null(condition) || identical(howInt, 1L)) && identical(feverywhere, 0L) && 
      ((is.list(object) && howInt < 5L) || (!is.list(object) && howInt < 2L)))
  {  
    ## nothing to be done
    res <- object  
  } else 
  { 
    ## check for special args
    fArgs <- conditionArgs <- rep(0L, 4L)
    if(identical(typeof(f), "closure"))
      fArgs <- match(c(".xname", ".xpos", ".xparents", ".xsiblings"), names(formals(f)), nomatch = 0L) 
    if(identical(typeof(condition), "closure"))
      conditionArgs <- match(c(".xname", ".xpos", ".xparents", ".xsiblings"), names(formals(condition)), nomatch = 0L)
    
    ## call main C function
    res <- .Call(C_rrapply, environment(), object, f, fArgs, condition, conditionArgs, classes, howInt, deflt, dfaslist, feverywhere)  
  }
  
  if(identical(how, "melt"))
  {
    ## drop NULL name columns
    res <- res[vapply(res, Negate(is.null), logical(1L))]
    ## convert list to data.frame
    if(length(res) > 1)
    {
      if(getRversion() >= "4.0.0" || !any(vapply(res[[length(res)]], is.symbol, logical(1L)))) {
        res <- structure(
            res,
            names =  c(paste0("L", seq_len(length(res) - 1L)), "value"),
            row.names = seq_len(length(res[[1L]])),
            class = "data.frame"
        )
      } else {
        ## no format method for names in R < 4.0.0 
        names(res) <- c(paste0("L", seq_len(length(res) - 1L)), "value")
      }
    }
    else
    {
      res <- structure(
          res,
          names = "value",
          row.names = integer(0L),
          class = "data.frame"
      )
    }
  }
  
  ## unlist result
  if(identical(how, "unlist")) res <- unlist(res, recursive = TRUE)
  
  return(res)
  
}


