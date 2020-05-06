#' @title Reimplementation of base-R's rapply
#' 
#' @description
#' \code{rrapply} is a reimplemented and extended version of \code{\link{rapply}} to recursively apply a function \code{f} to 
#' a set of elements of a list and deciding \emph{how} the result is structured. 
#' 
#' @section List pruning:
#' In addition to \code{\link{rapply}}'s modes to set \code{how} equal to \code{"replace"}, \code{"list"} or \code{"unlist"}, 
#' two choices \code{"prune"} and \code{"flatten"} are available. \code{how = "prune"} filters all list elements not subject to 
#' application of \code{f} from the list \code{object}. The original list structure is retained, similar to the non-pruned options 
#' \code{how = "replace"} or \code{how = "list"}. \code{how = "flatten"} is an efficient way to return a flattened unnested version 
#' of the pruned list.
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
#' \code{f} and \code{condition} supplied via the dots \code{...} argument need to be defined as function arguments in \emph{both} the \code{f}
#' and \code{condition} function (if existing), even if they are not used in the function itself. See also the \sQuote{Examples} section.
#' 
#' @section Special symbols \code{.xname} and \code{.xpos}:
#' Two special symbols \code{.xname} and \code{.xpos} are available inside the \code{f} and \code{condition} functions. The \code{.xname} symbol 
#' evaluates to the name of the list element. The \code{.xpos} symbol evaluates to the position of the element in the nested list structured as an 
#' integer vector. For instance, if \code{x = list(list("y", "z"))}, then an \code{.xpos} location of \code{c(1, 2)} corresponds to the 
#' list element \code{x[[c(1, 2)]]}. The names \code{.xname} and \code{.xpos} do not need to be included as function arguments in \code{f} and 
#' \code{condition}, and can be thought of as pre-defined variables in the current function environment. See the package vignette for additional
#' details on overriding the \code{.xname} and \code{.xpos} variables.  
#' 
#' @section Data.frames as lists:
#' By default, \code{rrapply} recurses into all list-like objects equivalent to \code{\link{rapply}}. Since data.frames are list-like objects, the
#' \code{f} function always descends into the individual columns of a data.frame. If \code{dfAsList = FALSE}, data.frames are no longer viewed as lists 
#' and the \code{f} and \code{condition} functions are applied directly to the data.frame itself and not its columns.
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
#' based on \code{classes} and \code{condition}. 
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
#' # Special symbols .xname and .xpos
#' 
#' ## Apply a function using the name of the node
#' renewable_oceania_text <- rrapply(
#'   renewable_oceania,
#'   f = function(x) sprintf("Renewable energy in %s: %.2f%%", .xname, x),
#'   condition = Negate(is.na),
#'   how = "flatten"
#' )
#' str(renewable_oceania_text, list.len = 10)
#' 
#' ## Extract values based on country names
#' renewable_benelux <- rrapply(
#'   renewable_energy_by_country,
#'   condition = function(x) .xname %in% c("Belgium", "Netherlands", "Luxembourg"),
#'   how = "prune"
#' )
#' str(renewable_benelux, give.attr = FALSE)
#' 
#' ## Filter European countries with value above 50%
#' renewable_europe_above_50 <- rrapply(
#'   renewable_energy_by_country,
#'   condition = function(x) identical(.xpos[c(1, 2)], c(1L, 5L)) & x > 50,
#'   how = "prune"
#' )
#' str(renewable_europe_above_50, give.attr = FALSE)
#' 
#' ## Return position of Sweden in list
#' (xpos_sweden <- rrapply(
#'   renewable_energy_by_country,
#'   condition = function(x) identical(.xname, "Sweden"),
#'   f = function(x) .xpos,
#'   how = "flatten"
#' ))
#' renewable_energy_by_country[[xpos_sweden$Sweden]]
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
#' rrapply(
#'   iris,
#'   condition = function(x) grepl("Sepal", .xname),
#'   f = scale
#' )
#' 
#' ## Scale and keep only numeric columns
#' rrapply(
#'   iris,
#'   f = scale,
#'   classes = "numeric",
#'   how = "prune"
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
#' @param condition a condition \code{\link{function}} of one \dQuote{principal} argument, passing further arguments via \code{\dots}.
#' @param how character string partially matching the five possibilities given: see \sQuote{Details}.
#' @param deflt the default result (only used if \code{how = "list"} or \code{how = "unlist"}).
#' @param dfAsList logical value to treat data.frames as \dQuote{list-like} object.
#' @param ... additional arguments passed to the call to \code{f} and \code{condition}
#' 
#' @aliases rrapply
#' 
#' @seealso \code{\link{rapply}}
#'
#' @useDynLib rrapply, .registration = TRUE
#' @export 
rrapply <- function(object, condition, f, classes = "ANY", deflt = NULL, 
                    how = c("replace", "list", "unlist", "prune", "flatten"), dfAsList = TRUE, ...) 
{
  
  ## check arguments
  if(!is.list(object) || length(object) < 1) stop("'object' argument should be list-like and of length greater than zero")
  how <- match.arg(how, c("replace", "list", "unlist", "prune", "flatten"))
  howInt <- match(how, c("replace", "list", "unlist", "prune", "flatten")) - 1L
  dfAsList <- isTRUE(dfAsList)
  if(missing(f)) f <- NULL
  if(missing(condition)) condition <- NULL
  if(!is.null(f) && !is.function(f)) {
    warning("'f' argument is not a function, defaults to identity") 
    f <- NULL
  }
  if(!is.null(condition) && !is.function(condition)) {
    warning("'condition' argument is not a function, no condition is used")
    condition <- NULL
  }
  if((is.null(f) && is.null(condition) && howInt != 4L) || (is.null(f) && howInt == 0L)) return(object)
  
  ## save existing values .xname and/or .xpos
  if(exists(".xname", envir = parent.frame())) xname <- get(".xname", envir = parent.frame())
  if(exists(".xpos", envir = parent.frame())) xpos <- get(".xpos", envir = parent.frame())
  
  ## call C function
  res <- .Call(do_rrapply, environment(), parent.frame(), object, f, condition, classes, howInt, deflt, dfAsList) 
  
  ## restore values .xname and/or .xpos
  if(exists("xname", envir = environment())) assign(".xname", xname, envir = parent.frame())
  else remove(".xname", envir = parent.frame())
  if(exists("xpos", envir = environment())) assign(".xpos", xpos, envir = parent.frame())
  else remove(".xpos", envir = parent.frame())
  
  if (how == "unlist") res <- unlist(res, recursive = TRUE)
  
  return(res)
  
}


