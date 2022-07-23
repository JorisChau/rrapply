#' @title Reimplementation of base-R's rapply
#' 
#' @description
#' \code{rrapply} is a reimplemented and extended version of \code{\link{rapply}} to recursively apply a function \code{f} to 
#' a set of elements of a list and deciding \emph{how} the result is structured. 
#' 
#' @section How to structure result:
#' In addition to \code{\link{rapply}}'s modes to set \code{how} equal to \code{"replace"}, \code{"list"} or \code{"unlist"}, 
#' seven choices \code{"prune"}, \code{"flatten"}, \code{"melt"}, \code{"bind"}, \code{"unmelt"}, \code{"recurse"}  and \code{"names"} are available:
#' \itemize{
#' \item \code{how = "prune"} filters all list elements not subject to application of \code{f} from the list \code{object}. The original 
#' list structure is retained, similar to the non-pruned options \code{how = "replace"} or \code{how = "list"}.
#' \item \code{how = "flatten"} is an efficient way to return a flattened unnested version of the pruned list. By default \code{how = "flatten"} 
#' uses similar coercion rules as \code{how = "unlist"}, this can be disabled with \code{simplify = FALSE} in the \code{options} argument.
#' \item \code{how = "melt"} returns a melted data.frame of the pruned list, each row contains the path of a single 
#' terminal node in the pruned list at depth layers \code{L1}, \code{L2}, and so on. The column \code{"value"} contains the 
#' possibly coerced values at the terminal nodes and is equivalent to the result of \code{how = "flatten"}. If no list names are present, 
#' the node names in the data.frame default to the indices of the list elements \code{"1"}, \code{"2"}, etc.
#' \item \code{how = "bind"} is used to unnest a nested list containing repeated sublists into a wide data.frame. Each repeated sublist is expanded 
#' as a single row in the data.frame and identical sublist component names are aligned as individual columns. By default, the list layer 
#' containing repeated sublists is identified based on the minimal depth detected across leaf nodes, this can be set manually with \code{coldepth} 
#' in the \code{options} argument.
#' \item \code{how = "unmelt"} is a special case that reconstructs a nested list from a melted data.frame. For this reason, \code{how = "unmelt"} 
#' only applies to data.frames in the same format as returned by \code{how = "melt"}. Internally, \code{how = "unmelt"} first reconstructs 
#' a nested list from the melted data.frame and second uses the same functional framework as \code{how = "replace"}.
#' \item \code{how = "recurse"} is a specialized option that is only useful in combination with e.g. \code{classes = "list"} to recurse further 
#' into updated \dQuote{list-like} elements. This is explained in more detail below.
#' \item \code{how = "names"} modifies the \emph{names} of the nested list elements instead of the list content. \code{how = "names"} internally works  
#' similar to \code{how = "list"}, except that the value of \code{f} is used to replace the name of the list element under evaluation 
#' instead of its content. 
#' }
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
#' \code{f} and \code{condition} (besides the special arguments \code{.xname}, \code{.xpos}, etc. discussed below) supplied via the dots \code{...} 
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
#' \code{condition} (in addition to the principal argument). See also the \sQuote{Examples} section.
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
#' after application of the \code{f} function. A primary use of \code{how = "recurse"} in combination with \code{classes = "list"} is to 
#' recursively update for instance the class or other attributes of all nodes in a nested list.
#' 
#' @section Avoid recursing into data.frames:
#' If \code{classes = "ANY"} (default), \code{rrapply} recurses into all \dQuote{list-like} objects equivalent to \code{\link{rapply}}. 
#' Since data.frames are \dQuote{list-like} objects, the \code{f} function will descend into the individual columns of a data.frame. 
#' To avoid this behavior, set \code{classes = "data.frame"}, in which case the \code{f} and \code{condition} functions are applied directly to 
#' the data.frame and not its columns. Note that this behavior can only be triggered using the \code{classes} argument and not the \code{condition} argument.
#' 
#' @section List attributes:
#' In \code{\link{rapply}} intermediate list attributes (not located at terminal nodes) are kept when \code{how = "replace"}, but are dropped when 
#' \code{how = "list"}. To avoid unexpected behavior, \code{rrapply} always preserves intermediate list attributes when using \code{how = "replace"}, 
#' \code{how = "list"}, \code{how = "prune"} or \code{how = "names"}. If \code{how = "unlist"}, \code{how = "flatten"}, \code{how = "melt"} or \code{how = "bind"} 
#' intermediate list attributes cannot be preserved as the result is no longer a nested list. 
#' 
#' @section Expressions:
#' Call objects and expression vectors are also accepted as \code{object} argument, which are treated as nested lists based on their internal abstract
#' syntax trees. As such, all functionality that applies to nested lists extends directly to call objects and expression vectors. If \code{object} is a 
#' call object or expression vector, \code{how = "replace"} always maintains the type of \code{object}, whereas \code{how = "list"} returns the result 
#' structured as a nested list. \code{how = "prune"}, \code{how = "flatten"} and \code{how = "melt"} return the pruned abstract syntax tree as: a nested list, 
#' a flattened list and a melted data.frame respectively. This is identical to application of \code{rrapply} to the abstract syntax tree formatted as a nested list.
#' 
#' @section Additional options:
#' The \code{options} argument accepts a named list to configure several default options that only apply to certain choices of \code{how}. The \code{options} 
#' list can contain (any of) the named components \code{namesep}, \code{simplify}, \code{namecols} and/or \code{coldepth}:
#' \itemize{
#' \item \code{namesep}, a character separator used to combine parent and child list names in \code{how = "flatten"} and \code{how = "bind"}. If \code{namesep = NA} (default), 
#' no parent names are included in \code{how = "flatten"} and the default separator \code{"."} is used in \code{how = "bind"}. Note that \code{namesep} cannot be used with 
#' \code{how = "unlist"} for which the name separator always defaults to \code{"."}. 
#' \item \code{simplify}, a logical value indicating whether the flattened unnested list in \code{how = "flatten"} and \code{how = "melt"} is simplified 
#' according to standard coercion rules similar to \code{how = "unlist"}. The default is \code{simplify = TRUE}. If \code{simplify = FALSE}, 
#' \code{object} is flattened to a single-layer list and returned as is.
#' \item \code{namecols}, a logical value that only applies to \code{how = "bind"} indicating whether the parent node names associated to the each expanded sublist 
#' should be included as columns \code{L1}, \code{L2}, etc. in the wide data.frame returned by \code{how = "bind"}. 
#' \item \code{coldepth}, an integer value indicating the depth (starting from depth 1) at which list elements should be mapped to individual columns 
#' in the wide data.frame returned by \code{how = "bind"}. If \code{coldepth = 0} (default), this depth layer is identified automatically based on the 
#' minimal depth detected across all leaf nodes. This option only applies to \code{how = "bind"}.
#' }
#' 
#' @return If \code{how = "unlist"}, a vector as in \code{\link{rapply}}. If \code{how = "list"}, \code{how = "replace"}, \code{how = "recurse"} or \code{how = "names"}, 
#' \dQuote{list-like} of similar structure as \code{object} as in \code{\link{rapply}}. If \code{how = "prune"}, a pruned \dQuote{list-like} object 
#' of similar structure as \code{object} with pruned list elements based on \code{classes} and \code{condition}. If \code{how = "flatten"}, a flattened
#' pruned vector or list with pruned elements based on \code{classes} and \code{condition}. If \code{how = "melt"}, a melted data.frame containing the node paths 
#' and values of the pruned list elements based on \code{classes} and \code{condition}. If \code{how = "bind"}, a wide data.frame with repeated list elements
#' expanded as single data.frame rows and aligned by identical list names using the same coercion rules as \code{how = "unlist"}. The repeated list elements 
#' are subject to pruning based on \code{classes} and \code{condition}. If \code{how = "unmelt"}, a nested list with list names and values defined 
#' in the data.frame \code{object}.
#' 
#' @note \code{rrapply} allows the \code{f} function argument to be missing, in which case no function is applied to the list 
#' elements.
#' @note \code{how = "unmelt"} requires as input a data.frame as returned by \code{how = "melt"} with character columns to name the nested list components
#' and a final list- or vector-column containing the values of the nested list elements. 
#' 
#' @examples
#' # Example data
#' 
#' ## Renewable energy shares per country (% of total consumption) in 2016
#' data("renewable_energy_by_country")
#' 
#' ## Renewable energy shares in Oceania
#' renewable_oceania <- renewable_energy_by_country[["World"]]["Oceania"]
#'
#' ## Pokemon properties in Pokemon GO
#' data("pokedex")
#'
#' # List pruning and unnesting
#' 
#' ## Drop logical NA's while preserving list structure 
#' na_drop_oceania <- rrapply(
#'   renewable_oceania,
#'   f = function(x) x,
#'   classes = "numeric",
#'   how = "prune"
#' )
#' str(na_drop_oceania, list.len = 3, give.attr = FALSE)
#' 
#' ## Drop logical NA's and return unnested list
#' na_drop_oceania2 <- rrapply(
#'   renewable_oceania,
#'   classes = "numeric",
#'   how = "flatten"
#' )
#' head(na_drop_oceania2, n = 10)
#' 
#' ## Flatten to simple list with full names
#' na_drop_oceania3 <- rrapply(
#'   renewable_oceania,
#'   classes = "numeric",
#'   how = "flatten",
#'   options = list(namesep = ".", simplify = FALSE)
#' ) 
#' str(na_drop_oceania3, list.len = 10, give.attr = FALSE)
#' 
#' ## Drop logical NA's and return melted data.frame
#' na_drop_oceania4 <- rrapply(
#'   renewable_oceania,
#'   classes = "numeric",
#'   how = "melt"
#' )
#' head(na_drop_oceania4)
#' 
#' ## Reconstruct nested list from melted data.frame
#' na_drop_oceania5 <- rrapply(
#'   na_drop_oceania4,
#'   how = "unmelt"
#' )
#' str(na_drop_oceania5, list.len = 3, give.attr = FALSE)
#' 
#' ## Unnest list to wide data.frame
#' pokedex_wide <- rrapply(pokedex, how = "bind")
#' head(pokedex_wide)
#' 
#' ## Unnest to data.frame including parent columns
#' pokemon_evolutions <- rrapply(
#'   pokedex, 
#'   how = "bind", 
#'   options = list(namecols = TRUE, coldepth = 5)
#' ) 
#' head(pokemon_evolutions, n = 10)
#'
#' # Condition function
#' 
#' ## Drop all NA elements using condition function
#' na_drop_oceania6 <- rrapply(
#'   renewable_oceania,
#'   condition = Negate(is.na),
#'   how = "prune"
#' )
#' str(na_drop_oceania6, list.len = 3, give.attr = FALSE)
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
#'   condition = Negate(is.na),
#'   f = function(x, .xname) sprintf("Renewable energy in %s: %.2f%%", .xname, x),
#'   how = "flatten"
#' )
#' head(renewable_oceania_text, n = 10)
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
#' ## Return neighbors of Sweden in list
#' siblings_sweden <- rrapply(
#'   renewable_energy_by_country,
#'   condition = function(x, .xsiblings) "Sweden" %in% names(.xsiblings),
#'   how = "flatten"
#' )
#' head(siblings_sweden, n = 10)
#' 
#' ## Unnest selected columns in Pokedex list 
#' pokedex_small <- rrapply(
#'    pokedex,
#'    condition = function(x, .xpos, .xname) length(.xpos) < 4 & .xname %in% c("num", "name", "type"),
#'    how = "bind"
#' )  
#' head(pokedex_small)
#' 
#' # Modifying list elements
#' 
#' ## Calculate mean value of Europe
#' rrapply(
#'   renewable_energy_by_country,  
#'   condition = function(x, .xname) .xname == "Europe",
#'   f = function(x) mean(unlist(x), na.rm = TRUE),
#'   classes = "list",
#'   how = "flatten"
#' )
#'
#' ## Calculate mean value for each continent
#' ## (Antarctica's value is missing)
#' renewable_continent_summary <- rrapply(
#'   renewable_energy_by_country,  
#'   condition = function(x, .xpos) length(.xpos) == 2,
#'   f = function(x) mean(unlist(x), na.rm = TRUE),
#'   classes = "list"
#' )
#' str(renewable_continent_summary, give.attr = FALSE)
#' 
#' ## Filter country or region by M49-code
#' rrapply(
#'   renewable_energy_by_country,
#'   condition = function(x) attr(x, "M49-code") == "155",
#'   f = function(x, .xname) .xname,
#'   classes = c("list", "ANY"), 
#'   how = "unlist"
#' )
#' 
#' # Recursive list updating
#' 
#' ## Recursively remove list attributes
#' renewable_no_attrs <- rrapply(
#'   renewable_oceania,
#'   f = function(x) c(x),
#'   classes = c("list", "ANY"),
#'   how = "recurse"
#' ) 
#' str(renewable_no_attrs, list.len = 3, give.attr = TRUE)
#' 
#' ## recursively replace all names by M49-codes
#' renewable_m49_names <- rrapply(
#'   renewable_oceania,
#'   f = function(x) attr(x, "M49-code"),
#'   how = "names"
#' ) 
#' str(renewable_m49_names, list.len = 3, give.attr = FALSE)
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
#'   f = as.numeric, 
#'   how = "replace",
#'   classes = "logical"
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
#' rrapply(expr, 
#'   classes = "name", 
#'   condition = is_new_name, 
#'   f = as.character,
#'   how = "melt"
#' )
#' 
#' ## Avoid recursing into call objects
#' rrapply(
#'   expr, 
#'   classes = "language", 
#'   condition = function(x) !any(sapply(x, is.call)),
#'   how = "flatten"
#' )
#' 
#' @param object a \code{\link{list}}, \code{\link{expression}} vector, or \code{\link{call}} object, i.e., \dQuote{list-like}.
#' @param f a \code{\link{function}} of one \dQuote{principal} argument and optional special arguments \code{.xname}, \code{.xpos}, \code{.xparents} 
#' and/or \code{.xsiblings} (see \sQuote{Details}), passing further arguments via \code{\dots}.
#' @param condition a condition \code{\link{function}} of one \dQuote{principal} argument and optional special arguments \code{.xname}, \code{.xpos}, 
#' \code{.xparents} and/or \code{.xsiblings} (see \sQuote{Details}), passing further arguments via \code{\dots}.
#' @param classes character vector of \code{\link{class}} names, or \code{"ANY"} to match the class of any terminal node. Include \code{"list"} or \code{"data.frame"}
#' to match the class of non-terminal nodes as well.
#' @param how character string partially matching the ten possibilities given: see \sQuote{Details}.
#' @param deflt the default result (only used if \code{how = "list"} or \code{how = "unlist"}).
#' @param options a named \code{\link{list}} with additional options \code{namesep}, \code{simplify}, \code{namecols} and/or \code{coldepth} 
#' that only apply to certain choices of \code{how}: see \sQuote{Details}. 
#' @param ... additional arguments passed to the call to \code{f} and \code{condition}.
#' 
#' @aliases rrapply
#' 
#' @seealso \code{\link{rapply}}
#'
#' @useDynLib rrapply, .registration = TRUE
#' @export 
rrapply <- function(object, condition, f, classes = "ANY", deflt = NULL, 
                    how = c("replace", "list", "unlist", "prune", "flatten", "melt", "bind", "recurse", "unmelt", "names"), 
                    options, ...)
{
  ## non-function arguments
  if(!(is.list(object) || is.call(object) || is.expression(object)) || length(object) < 1) 
    stop("'object' argument should be list-like and of length greater than zero")
  
  how <- match.arg(how, c("replace", "list", "unlist", "prune", "flatten", "melt", "bind", "recurse", "unmelt", "names"))
  howInt <- match(how, c("replace", "list", "unlist", "prune", "flatten", "melt", "bind", "recurse", "unmelt", "names"))
  if(identical(how, "recurse")) howInt <- 1L
  
  feverywhere <- ifelse(isTRUE(any(classes %in% c("list", "language", "pairlist", "expression"))) || identical(how, "names"), 
                        1L + how %in% c("recurse", "names"), 0L) 
  dfaslist <- ifelse(isFALSE("data.frame" %in% classes), 1L, -identical(how, "recurse"))
  
  if(length(classes) > 1 && isTRUE("ANY" %in% classes)) classes <- "ANY"
  
  ## additional options
  .options <- list(namesep = NA_character_, simplify = TRUE, namecols = FALSE, coldepth = 0L)
  if(!missing(options)) 
  {
    options <- as.list(options)
    .options[names(options)] <- options
    if(how %in% c("flatten", "bind"))
    {
      .options$namesep <- as.character(.options$namesep)
      stopifnot(length(.options$namesep) == 1L)
    }
    if(how %in% c("flatten", "melt")) 
      stopifnot(is.logical(.options$simplify))
    if(how %in% c("bind"))
    {
      .options$coldepth <- as.integer(.options$coldepth)
      stopifnot(
        is.logical(.options$namecols),
        !is.na(.options$coldepth),
        length(.options$coldepth) == 1L
      )
    }
  }
  
  ## unmelt data.frame to nested list
  if(identical(how, "unmelt"))
  {
    if(is.data.frame(object) && all(vapply(object[, -length(object)], is.character, logical(1))))
    {
      if(nrow(object) > 0 && length(object) > 1)
      {
        object <- .Call(C_unmelt, object)
        how <- "replace"
        howInt <- 1L
      }
      else
        stop("'object' argument must be non-empty and contain at least one naming character column followed by a value column")
    }
    else
      stop("'object' argument must be a data.frame consisting of naming character columns followed by a value column")
  }
  ## function arguments  
  if(missing(f)) f <- NULL else f <- match.fun(f)
  if(missing(condition)) condition <- NULL else condition <- match.fun(condition)
  
  if(is.null(f) && (((is.null(condition) || identical(howInt, 1L)) && identical(feverywhere, 0L) && 
     identical(classes, "ANY") && ((is.list(object) && howInt < 5L) || (!is.list(object) && howInt < 2L))) || identical(howInt, 10L)))
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
    res <- .Call(C_rrapply, environment(), object, f, fArgs, condition, conditionArgs, classes, howInt, deflt, dfaslist, feverywhere, .options)  
  }
  
  if(identical(how, "melt"))
  {
    anysymbol <- attr(res, "anysymbol")
    attr(res, "anysymbol") <- NULL
    
    ## convert list to data.frame
    if(getRversion() >= "4.0.0" || !anysymbol)
    {
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
  
  if(identical(how, "bind"))
  {
    ## convert list to data.frame (no format method for names in R < 4.0.0)
    if(length(res) > 0 && (getRversion() >= "4.0.0" || !attr(res, "anysymbol")))
    {
      res <- structure(
        res,
        names = names(res),
        row.names = seq_len(length(res[[1L]])),
        class = "data.frame"
      )
    }
    attr(res, "anysymbol") <- NULL
  }
  
  ## unlist result
  if(identical(how, "unlist")) res <- unlist(res, recursive = TRUE)
  
  return(res)
  
}


