#' @title Reimplementation of base-R's rapply
#' 
#' @description
#' \code{rrapply} is a reimplemented and extended version of \code{\link{rapply}} to recursively apply a function \code{f} to 
#' a set of elements of a list and deciding \emph{how} the result is structured. 
#' 
#' @section List pruning:
#' In addition to \code{\link{rapply}}'s modes to set \code{how} equal to \code{"replace"}, \code{"list"} or \code{"unlist"}, 
#' three choices \code{"prune"}, \code{"flatten"} and \code{"melt"} are available. \code{how = "prune"} filters all list elements not subject to 
#' application of \code{f} from the list \code{object}. The original list structure is retained, similar to the non-pruned options 
#' \code{how = "replace"} or \code{how = "list"}. \code{how = "flatten"} is an efficient way to return a flattened unnested version 
#' of the pruned list. \code{how = "melt"} returns a melted data.frame of the pruned list, each row contains the path of a single terminal node in 
#' the pruned list at depth layers \code{L1}, \code{L2}, and so on. The list-column \code{"value"} contains the values at the terminal nodes and is 
#' equivalent to the flattened list returned by \code{how = "flatten"}. If no list names are present, the node names in the data.frame default to
#' the indices of the list elements \code{"..1"}, \code{"..2"}, etc. 
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
#' @section Special arguments \code{.xname} and \code{.xpos}:
#' The \code{f} and \code{condition} functions accept two special arguments \code{.xname} and \code{.xpos} in addition to the first principal argument. 
#' The \code{.xname} argument evaluates to the name of the list element. The \code{.xpos} argument evaluates to the position of the element in the nested 
#' list structured as an integer vector. That is, if \code{x = list(list("y", "z"))}, then an \code{.xpos} location of \code{c(1, 2)} corresponds 
#' to the list element \code{x[[c(1, 2)]]}. The names \code{.xname} and \code{.xpos} need to be explicitly included as function arguments in \code{f} and 
#' \code{condition} (in addition to the principal argument). See the package vignette for example uses of the \code{.xname} and 
#' \code{.xpos} variables.  
#' 
#' @section List node aggregation:
#' By default, \code{rrapply} recurses into any \dQuote{list-like} element. If \code{feverywhere = "break"}, this behavior is overridden and the 
#' \code{f} function is applied to any element of \code{object} (e.g. a sublist) that satisfies \code{condition} and \code{classes}. 
#' If the \code{condition} or \code{classes} arguments are not satisfied for a \dQuote{list-like} element, \code{rrapply} will recurse further into the sublist, 
#' apply the \code{f} function to the nodes that satisfy \code{condition} and \code{classes}, and so on. The primary use of \code{feverywhere = "break"} is 
#' to aggregate list nodes or summarize sublists of \code{object}. Additional examples can be found in the package vignette.
#' 
#' @section List node updating:
#' If \code{feverywhere = "recurse"}, \code{rrapply} applies the \code{f} function to any element of \code{object} (e.g. a sublist) that satisfies 
#' \code{condition} and \code{classes} similar to \code{feverywhere = "break"}, but recurses further into the \emph{updated} list-like element 
#' after application of the \code{f} function. The primary use of \code{feverywhere = "recurse"} is to recursively update for instance the names of 
#' all nodes in a nested list. Additional examples are found in the package vignette.
#' 
#' @section Data.frames as lists:
#' If \code{feverywhere = NULL} (default), \code{rrapply} recurses into all \dQuote{list-like} objects equivalent to \code{\link{rapply}}. 
#' Since data.frames are \dQuote{list-like} objects, the \code{f} function will descend into the individual columns of a data.frame. 
#' If \code{dfaslist = FALSE}, data.frames are no longer viewed as lists and the \code{f} and \code{condition} functions are 
#' applied directly to the data.frame itself and not its columns.
#' 
#' @section List attributes:
#' In \code{\link{rapply}} intermediate list attributes (not located at the leafs) are kept when \code{how = "replace"}, but are dropped when 
#' \code{how = "list"}. To avoid unexpected behavior, \code{rrapply} always preserves intermediate list attributes when using \code{how = "replace"}, 
#' \code{how = "list"} or \code{how = "prune"}. If \code{how = "flatten"} or \code{how = "unlist"} intermediate list attributes cannot be preserved as 
#' the result is no longer a nested list. 
#'
#' @return If \code{how = "unlist"}, a vector as in \code{\link{rapply}}. If \code{how = "list"} or \code{how = "replace"}, \dQuote{list-like} of similar 
#' structure as \code{object} as in \code{\link{rapply}}. If \code{how = "prune"}, a pruned \dQuote{list-like} object of similar structure as \code{object}
#' with pruned list elements based on \code{classes} and \code{condition}. If \code{how = "flatten"}, an unnested pruned list with pruned list elements 
#' based on \code{classes} and \code{condition}. If \code{how = "melt"}, a melted data.frame containing the node paths and values of the pruned list 
#' elements based on \code{classes} and \code{condition}.
#'  
#' @note \code{rrapply} allows the \code{f} function argument to be missing, in which case no function is applied to the list 
#' elements.
#' 
#' @examples
#' # Example data
#' 
#' ## Nested list of renewable energy (%) of total energy consumption per country in 2016
#' data("renewable_energy_by_country")
#' ## Subset values for countries and areas in Oceania
#' renewable_oceania <- renewable_energy_by_country[["World"]]["Oceania"]
#'
#' # List pruning
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
#' head(na_drop_oceania3)
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
#' # Special arguments .xname and .xpos
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
#' ## Return position of Sweden in list
#' (xpos_sweden <- rrapply(
#'   renewable_energy_by_country,
#'   condition = function(x, .xname) identical(.xname, "Sweden"),
#'   f = function(x, .xpos) .xpos,
#'   how = "flatten"
#' ))
#' renewable_energy_by_country[[xpos_sweden$Sweden]]
#' 
#' # List node aggregation
#' 
#' ## Calculate mean value of Europe
#' rrapply(
#'   renewable_energy_by_country,  
#'   condition = function(x, .xname) .xname == "Europe",
#'   f = function(x) mean(unlist(x), na.rm = TRUE),
#'   how = "flatten",
#'   feverywhere = "break"
#' )
#'
#' ## Calculate mean value for each continent
#' renewable_continent_summary <- rrapply(
#'   renewable_energy_by_country,  
#'   condition = function(x, .xpos) length(.xpos) == 2,
#'   f = function(x) mean(unlist(x), na.rm = TRUE),
#'   feverywhere = "break"
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
#'   condition = is.list,
#'   f = function(x) {
#'     names(x) <- vapply(x, attr, character(1L), which = "M49-code")
#'     return(x)
#'   },
#'   feverywhere = "recurse"
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
#' # Applying functions to data.frames
#' 
#' ## Scale only Sepal columns in iris dataset
#' head(
#'   rrapply(
#'     iris,
#'     condition = function(x, .xname) grepl("Sepal", .xname),
#'     f = scale
#'   )
#' )
#' 
#' ## Scale and keep only numeric columns
#' head(
#'   rrapply(
#'     iris,
#'     f = scale,
#'     classes = "numeric",
#'     how = "prune"
#'   )
#' )
#' 
#' ## Summarize only numeric columns with how = "flatten"
#' rrapply(
#'   iris,
#'   f = summary,
#'   classes = "numeric",
#'   how = "flatten"
#' )
#' 
#' @inheritParams base::rapply
#' @param f a \code{\link{function}} of one \dQuote{principal} argument and optional special arguments \code{.xname} and/or \code{.xpos} 
#' (see \sQuote{Details}), passing further arguments via \code{\dots}.
#' @param condition a condition \code{\link{function}} of one \dQuote{principal} argument and optional special arguments \code{.xname} and/or 
#' \code{.xpos} (see \sQuote{Details}), passing further arguments via \code{\dots}.
#' @param how character string partially matching the five possibilities given: see \sQuote{Details}.
#' @param deflt the default result (only used if \code{how = "list"} or \code{how = "unlist"}).
#' @param dfaslist logical value to treat data.frames as \dQuote{list-like} object.
#' @param feverywhere character options \code{"break"} or \code{"recurse"} to override default behavior of \code{f}: see \sQuote{Details}.
#' By default \code{NULL}, which applies \code{f} only to non \dQuote{list-like} elements.
#' @param ... additional arguments passed to the call to \code{f} and \code{condition}
#' 
#' @aliases rrapply
#' 
#' @seealso \code{\link{rapply}}
#'
#' @useDynLib rrapply, .registration = TRUE
#' @export 
rrapply <- function(object, condition, f, classes = "ANY", deflt = NULL, 
                    how = c("replace", "list", "unlist", "prune", "flatten", "melt", "unmelt"),
                    feverywhere = NULL, dfaslist = TRUE, ...)
{
  
  ## non-function arguments
  how <- match.arg(how, c("replace", "list", "unlist", "prune", "flatten", "melt", "unmelt"))
  howInt <- match(how, c("replace", "list", "unlist", "prune", "flatten", "melt", "unmelt"))
  dfaslist <- isTRUE(dfaslist)
  feverywhere <- match.arg(feverywhere, c("no", "break", "recurse"))
  feverywhereInt <- match(feverywhere, c("no", "break", "recurse"))
  
  if(!is.list(object) || length(object) < 1) stop("'object' argument should be list-like and of length greater than zero")
  ## unmelt data.frame to nested list
  if(identical(how, "unmelt"))
  {
    if(is.data.frame(object) && is.list(object[[length(object)]]) && all(vapply(object[, -length(object)], is.character, logical(1))))
    {
      object <- .Call(C_unmelt, object)
      how <- "replace"
      howInt <- 1L
    }
    else
      stop("'object' argument must be a data.frame consisting of naming character columns and a value list column")
  }
  ## function arguments  
  if(missing(f)) f <- NULL else f <- match.fun(f)
  if(missing(condition)) condition <- NULL else condition <- match.fun(condition)
  
  if(is.null(f) && (is.null(condition) || identical(how, "replace")) && identical(feverywhereInt, 1L) && howInt < 5L) 
  {  
    ## nothing to be done
    res <- object  
  } else 
  { 
    ## check for .xname and .xpos args
    fArgs <- conditionArgs <- c(0L, 0L)
    if(identical(typeof(f), "closure"))
      fArgs <- match(c(".xname", ".xpos"), names(formals(f)), nomatch = 0L) 
    if(identical(typeof(condition), "closure"))
      conditionArgs <- match(c(".xname", ".xpos"), names(formals(condition)), nomatch = 0L)
    
    ## call main C function
    res <- .Call(C_rrapply, environment(), object, f, fArgs, condition, conditionArgs, classes, howInt, deflt, dfaslist, feverywhereInt)  
  }
  
  if(identical(how, "melt"))
  {
    ## drop NULL name columns
    res <- res[vapply(res, Negate(is.null), logical(1L))]
    ## convert list to data.frame
    if(length(res) > 1)
    {
      res <- structure(
        res,
        names =  c(paste0("L", seq_len(length(res) - 1L)), "value"),
        row.names = seq_len(length(res[[1L]])),
        class = "data.frame"
      )
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


