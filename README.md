
<!-- README.md is generated from README.Rmd. Please edit that file -->

# rrapply: revisiting R-base rapply

<!-- badges: start -->

[![CRAN
version](http://www.r-pkg.org/badges/version/rrapply)](https://cran.r-project.org/package=rrapply)
[![R-CMD-check](https://github.com/JorisChau/rrapply/workflows/R-CMD-check/badge.svg)](https://github.com/JorisChau/rrapply/actions)
[![codecov](https://codecov.io/gh/JorisChau/rrapply/branch/master/graph/badge.svg)](https://codecov.io/gh/JorisChau/rrapply)
[![status](https://tinyverse.netlify.com/badge/rrapply)](https://CRAN.R-project.org/package=rrapply)
[![Total
Downloads](https://cranlogs.r-pkg.org/badges/grand-total/rrapply)](https://cran.r-project.org/package=rrapply)
<!-- badges: end -->

The minimal rrapply-package contains a single function `rrapply()`,
providing an extended implementation of R-base’s `rapply()` function,
which applies a function `f` to all elements of a nested list
recursively and provides control in structuring the returned result.
`rrapply()` builds upon `rapply()`’s native C implementation and for
this reason requires no external R-package dependencies.

## Installation

``` r
# Install latest release from CRAN:
install.packages("rrapply")

# Install the development version from GitHub:
# install.packages("devtools")
devtools::install_github("JorisChau/rrapply")
```

## Example usage

### List pruning and unnesting

With base `rapply()`, there is no convenient way to prune or filter list
elements from the input list. The `rrapply()` function adds an option
`how = "prune"` to prune all list elements not subject to application of
`f` from a nested list,

``` r
library(rrapply)
## Nested list of renewable energy as a percentage of total energy consumption per country in 2016
data("renewable_energy_by_country")

## Subset values for countries and areas in Oceania
renewable_oceania <- renewable_energy_by_country[["World"]]["Oceania"]
str(renewable_oceania, list.len = 3, give.attr = FALSE)
#> List of 1
#>  $ Oceania:List of 4
#>   ..$ Australia and New Zealand:List of 6
#>   .. ..$ Australia                        : num 9.32
#>   .. ..$ Christmas Island                 : logi NA
#>   .. ..$ Cocos (Keeling) Islands          : logi NA
#>   .. .. [list output truncated]
#>   ..$ Melanesia                :List of 5
#>   .. ..$ Fiji            : num 24.4
#>   .. ..$ New Caledonia   : num 4.03
#>   .. ..$ Papua New Guinea: num 50.3
#>   .. .. [list output truncated]
#>   ..$ Micronesia               :List of 8
#>   .. ..$ Guam                                : num 3.03
#>   .. ..$ Kiribati                            : num 45.4
#>   .. ..$ Marshall Islands                    : num 11.8
#>   .. .. [list output truncated]
#>   .. [list output truncated]

## Drop all logical NA's while preserving list structure 
na_drop_oceania <- rrapply(
  renewable_oceania,
  f = identity,
  classes = "numeric",
  how = "prune"
)
str(na_drop_oceania, list.len = 3, give.attr = FALSE)
#> List of 1
#>  $ Oceania:List of 4
#>   ..$ Australia and New Zealand:List of 2
#>   .. ..$ Australia  : num 9.32
#>   .. ..$ New Zealand: num 32.8
#>   ..$ Melanesia                :List of 5
#>   .. ..$ Fiji            : num 24.4
#>   .. ..$ New Caledonia   : num 4.03
#>   .. ..$ Papua New Guinea: num 50.3
#>   .. .. [list output truncated]
#>   ..$ Micronesia               :List of 7
#>   .. ..$ Guam                            : num 3.03
#>   .. ..$ Kiribati                        : num 45.4
#>   .. ..$ Marshall Islands                : num 11.8
#>   .. .. [list output truncated]
#>   .. [list output truncated]
```

Instead, use `how = "flatten"` to return a flattened unnested version of
the pruned list,

``` r
## Drop all logical NA's and return unnested list
na_drop_oceania2 <- rrapply(
  renewable_oceania,
  f = identity,
  classes = "numeric",
  how = "flatten"
)
head(na_drop_oceania2, n = 10)
#>        Australia      New Zealand             Fiji    New Caledonia 
#>             9.32            32.76            24.36             4.03 
#> Papua New Guinea  Solomon Islands          Vanuatu             Guam 
#>            50.34            65.73            33.67             3.03 
#>         Kiribati Marshall Islands 
#>            45.43            11.75
```

Or, use `how = "melt"` to return a melted data.frame of the pruned list
similar in format to `reshape2::melt()` applied to a nested list.

``` r
## Drop all logical NA's and return melted data.frame
na_drop_oceania3 <- rrapply(
  renewable_oceania,
  f = identity,
  classes = "numeric",
  how = "melt"
)
head(na_drop_oceania3)
#>        L1                        L2               L3 value
#> 1 Oceania Australia and New Zealand        Australia  9.32
#> 2 Oceania Australia and New Zealand      New Zealand 32.76
#> 3 Oceania                 Melanesia             Fiji 24.36
#> 4 Oceania                 Melanesia    New Caledonia  4.03
#> 5 Oceania                 Melanesia Papua New Guinea 50.34
#> 6 Oceania                 Melanesia  Solomon Islands 65.73
```

A melted data.frame can be used to reconstruct a nested list with
`how = "unmelt"`,

``` r
## Reconstruct nested list from melted data.frame
na_drop_oceania4 <- rrapply(
  na_drop_oceania3, 
  how = "unmelt"
)
str(na_drop_oceania4, list.len = 3, give.attr = FALSE)
#> List of 1
#>  $ Oceania:List of 4
#>   ..$ Australia and New Zealand:List of 2
#>   .. ..$ Australia  : num 9.32
#>   .. ..$ New Zealand: num 32.8
#>   ..$ Melanesia                :List of 5
#>   .. ..$ Fiji            : num 24.4
#>   .. ..$ New Caledonia   : num 4.03
#>   .. ..$ Papua New Guinea: num 50.3
#>   .. .. [list output truncated]
#>   ..$ Micronesia               :List of 7
#>   .. ..$ Guam                            : num 3.03
#>   .. ..$ Kiribati                        : num 45.4
#>   .. ..$ Marshall Islands                : num 11.8
#>   .. .. [list output truncated]
#>   .. [list output truncated]
```

Nested lists containing repeated observations can be unnested with
`how = "bind"`, which returns a wide data.frame similar in format to
`dplyr::bind_rows()` applied to a list of data.frames or repeated
application of `tidyr::unnest_wider()` to a nested data.frame.

``` r
## Nested list of Pokemon properties in Pokemon GO
data("pokedex")

str(pokedex, list.len = 3)
#> List of 1
#>  $ pokemon:List of 151
#>   ..$ :List of 16
#>   .. ..$ id            : int 1
#>   .. ..$ num           : chr "001"
#>   .. ..$ name          : chr "Bulbasaur"
#>   .. .. [list output truncated]
#>   ..$ :List of 17
#>   .. ..$ id            : int 2
#>   .. ..$ num           : chr "002"
#>   .. ..$ name          : chr "Ivysaur"
#>   .. .. [list output truncated]
#>   ..$ :List of 15
#>   .. ..$ id            : int 3
#>   .. ..$ num           : chr "003"
#>   .. ..$ name          : chr "Venusaur"
#>   .. .. [list output truncated]
#>   .. [list output truncated]

## Unnest list as a wide data.frame
pokedex_wide <- rrapply(pokedex, how = "bind")

head(pokedex_wide[, c(1:3, 5:10)], n = 5)
#>   pokemon.id pokemon.num pokemon.name  pokemon.type pokemon.height
#> 1          1         001    Bulbasaur Grass, Poison         0.71 m
#> 2          2         002      Ivysaur Grass, Poison         0.99 m
#> 3          3         003     Venusaur Grass, Poison         2.01 m
#> 4          4         004   Charmander          Fire         0.61 m
#> 5          5         005   Charmeleon          Fire         1.09 m
#>   pokemon.weight    pokemon.candy pokemon.candy_count pokemon.egg
#> 1         6.9 kg  Bulbasaur Candy                  25        2 km
#> 2        13.0 kg  Bulbasaur Candy                 100 Not in Eggs
#> 3       100.0 kg  Bulbasaur Candy                  NA Not in Eggs
#> 4         8.5 kg Charmander Candy                  25        2 km
#> 5        19.0 kg Charmander Candy                 100 Not in Eggs
```

### Condition function

Base `rapply()` allows to apply a function `f` to list elements of
certain types or classes via the `classes` argument. `rrapply()`
generalizes this option via an additional `condition` argument, which
accepts any function to use as a condition or predicate to apply `f` to
a subset of list elements.

``` r
## Drop all NA elements using condition function
na_drop_oceania3 <- rrapply(
  renewable_oceania,
  condition = Negate(is.na),
  f = function(x) x,
  how = "prune"
)
str(na_drop_oceania3, list.len = 3, give.attr = FALSE)
#> List of 1
#>  $ Oceania:List of 4
#>   ..$ Australia and New Zealand:List of 2
#>   .. ..$ Australia  : num 9.32
#>   .. ..$ New Zealand: num 32.8
#>   ..$ Melanesia                :List of 5
#>   .. ..$ Fiji            : num 24.4
#>   .. ..$ New Caledonia   : num 4.03
#>   .. ..$ Papua New Guinea: num 50.3
#>   .. .. [list output truncated]
#>   ..$ Micronesia               :List of 7
#>   .. ..$ Guam                            : num 3.03
#>   .. ..$ Kiribati                        : num 45.4
#>   .. ..$ Marshall Islands                : num 11.8
#>   .. .. [list output truncated]
#>   .. [list output truncated]

## Filter all countries with values above 85%
renewable_energy_above_85 <- rrapply(
  renewable_energy_by_country,
  condition = function(x) x > 85,
  how = "prune"
)
str(renewable_energy_above_85, give.attr = FALSE)
#> List of 1
#>  $ World:List of 1
#>   ..$ Africa:List of 1
#>   .. ..$ Sub-Saharan Africa:List of 3
#>   .. .. ..$ Eastern Africa:List of 7
#>   .. .. .. ..$ Burundi                    : num 89.2
#>   .. .. .. ..$ Ethiopia                   : num 91.9
#>   .. .. .. ..$ Rwanda                     : num 86
#>   .. .. .. ..$ Somalia                    : num 94.7
#>   .. .. .. ..$ Uganda                     : num 88.6
#>   .. .. .. ..$ United Republic of Tanzania: num 86.1
#>   .. .. .. ..$ Zambia                     : num 88.5
#>   .. .. ..$ Middle Africa :List of 2
#>   .. .. .. ..$ Chad                            : num 85.3
#>   .. .. .. ..$ Democratic Republic of the Congo: num 97
#>   .. .. ..$ Western Africa:List of 1
#>   .. .. .. ..$ Guinea-Bissau: num 86.5
```

### Special arguments `.xname`, `.xpos`, `.xparents` and `.xsiblings`

In base `rapply()`, the `f` function only has access to the content of
the list element under evaluation, and there is no convenient way to
access its name or location in the nested list from inside the `f`
function. `rrapply()` allows the use of the special arguments `.xname`,
`.xpos`,`.xparents` and `.xsiblings` inside the `f` and `condition`
functions (in addition to the principal function argument). `.xname`
evaluates to the name of the list element, `.xpos` evaluates to the
position of the element in the nested list structured as an integer
vector, `.xparents` evaluates to a vector of parent node names in the
path to the current list element, and `.xsiblings` evaluates to the
parent list containing the current list element and all of its direct
siblings.

``` r
## Apply a function using the name of the node
renewable_oceania4 <- rrapply(
  renewable_oceania,
  f = function(x, .xname) sprintf("Renewable energy in %s: %.2f%%", .xname, x),
  condition = Negate(is.na),
  how = "flatten"
)
head(renewable_oceania4, n = 5)
#>                                      Australia 
#>         "Renewable energy in Australia: 9.32%" 
#>                                    New Zealand 
#>      "Renewable energy in New Zealand: 32.76%" 
#>                                           Fiji 
#>             "Renewable energy in Fiji: 24.36%" 
#>                                  New Caledonia 
#>     "Renewable energy in New Caledonia: 4.03%" 
#>                               Papua New Guinea 
#> "Renewable energy in Papua New Guinea: 50.34%"

## Extract values based on country names
renewable_benelux <- rrapply(
  renewable_energy_by_country,
  condition = function(x, .xname) .xname %in% c("Belgium", "Netherlands", "Luxembourg"),
  how = "prune"
)
str(renewable_benelux, give.attr = FALSE)
#> List of 1
#>  $ World:List of 1
#>   ..$ Europe:List of 1
#>   .. ..$ Western Europe:List of 3
#>   .. .. ..$ Belgium    : num 9.14
#>   .. .. ..$ Luxembourg : num 13.5
#>   .. .. ..$ Netherlands: num 5.78

## Filter European countries > 50% using .xpos
renewable_europe_above_50 <- rrapply(
  renewable_energy_by_country,
  condition = function(x, .xpos) identical(head(.xpos, 2), c(1L, 5L)) & x > 50,
  how = "prune"
)
str(renewable_europe_above_50, give.attr = FALSE)
#> List of 1
#>  $ World:List of 1
#>   ..$ Europe:List of 2
#>   .. ..$ Northern Europe:List of 3
#>   .. .. ..$ Iceland: num 78.1
#>   .. .. ..$ Norway : num 59.5
#>   .. .. ..$ Sweden : num 51.4
#>   .. ..$ Western Europe :List of 1
#>   .. .. ..$ Liechtenstein: num 62.9

## Filter European countries > 50% using .xparents
renewable_europe_above_50 <- rrapply(
  renewable_energy_by_country, 
  condition = function(x, .xparents) "Europe" %in% .xparents & x > 50,
  how = "prune"
)
str(renewable_europe_above_50, give.attr = FALSE)
#> List of 1
#>  $ World:List of 1
#>   ..$ Europe:List of 2
#>   .. ..$ Northern Europe:List of 3
#>   .. .. ..$ Iceland: num 78.1
#>   .. .. ..$ Norway : num 59.5
#>   .. .. ..$ Sweden : num 51.4
#>   .. ..$ Western Europe :List of 1
#>   .. .. ..$ Liechtenstein: num 62.9

## Return position of Sweden in list
(xpos_sweden <- rrapply(
  renewable_energy_by_country,
  condition = function(x, .xname) identical(.xname, "Sweden"),
  f = function(x, .xpos) .xpos,
  how = "flatten"
))
#> $Sweden
#> [1]  1  5  2 14
renewable_energy_by_country[[xpos_sweden$Sweden]]
#> [1] 51.35
#> attr(,"M49-code")
#> [1] "752"

## Return siblings of Sweden in list
siblings_sweden <- rrapply(
  renewable_energy_by_country,
  condition = function(x, .xsiblings) "Sweden" %in% names(.xsiblings),
  how = "flatten"
)
head(siblings_sweden, n = 5)
#> Aland Islands       Denmark       Estonia Faroe Islands       Finland 
#>            NA         33.06         26.55          4.24         42.03

## Parse only Pokemon number, name and type columns 
pokedex_small <- rrapply(
  pokedex,
  condition = function(x, .xpos, .xname) length(.xpos) < 4 & .xname %in% c("num", "name", "type"),
  how = "bind"
)

head(pokedex_small)
#>   pokemon.num pokemon.name  pokemon.type
#> 1         001    Bulbasaur Grass, Poison
#> 2         002      Ivysaur Grass, Poison
#> 3         003     Venusaur Grass, Poison
#> 4         004   Charmander          Fire
#> 5         005   Charmeleon          Fire
#> 6         006    Charizard  Fire, Flying
```

### Modifying list elements

By default, both base `rapply()` and `rrapply()` recurse into any
list-like element. Using `rrapply()`, we can set `classes = "list"` to
override this behavior and apply `f` to any list element (i.e. a
sublist) that satisfies the `condition` argument. This is useful to
collapse sublists or calculate summary statistics of sublists of a
nested list. Together with e.g. the `.xname` and `.xpos` arguments, we
have a flexible way in deciding which sublists to summarize through the
`condition` function.

``` r
## Calculate mean value of Europe
rrapply(
  renewable_energy_by_country,  
  classes = "list",
  condition = function(x, .xname) .xname == "Europe",
  f = function(x) mean(unlist(x), na.rm = TRUE),
  how = "flatten"
)
#>   Europe 
#> 22.36565

## Calculate mean value for each continent
renewable_continent_summary <- rrapply(
  renewable_energy_by_country, 
  classes = "list",
  condition = function(x, .xpos) length(.xpos) == 2,
  f = function(x) mean(unlist(x), na.rm = TRUE)
)

## Antarctica's value is missing
str(renewable_continent_summary, give.attr = FALSE)
#> List of 1
#>  $ World:List of 6
#>   ..$ Africa    : num 54.3
#>   ..$ Americas  : num 18.2
#>   ..$ Antarctica: logi NA
#>   ..$ Asia      : num 17.9
#>   ..$ Europe    : num 22.4
#>   ..$ Oceania   : num 17.8

## Simplify Pokemon evolution sublists to character vectors 
pokedex_wide2 <- rrapply(
  pokedex,
  classes = c("character", "list"),
  condition = function(x, .xname) .xname %in% c("name", "next_evolution", "prev_evolution"), 
  f = function(x) if(is.list(x)) sapply(x, `[[`, "name") else x,
  how = "bind"
)
    
head(pokedex_wide2, n = 9)
#>   pokemon.name pokemon.next_evolution pokemon.prev_evolution
#> 1    Bulbasaur      Ivysaur, Venusaur                     NA
#> 2      Ivysaur               Venusaur              Bulbasaur
#> 3     Venusaur                     NA     Bulbasaur, Ivysaur
#> 4   Charmander  Charmeleon, Charizard                     NA
#> 5   Charmeleon              Charizard             Charmander
#> 6    Charizard                     NA Charmander, Charmeleon
#> 7     Squirtle   Wartortle, Blastoise                     NA
#> 8    Wartortle              Blastoise               Squirtle
#> 9    Blastoise                     NA    Squirtle, Wartortle
```

### Recursive list updating

If `classes = "list"` and `how = "recurse"`, `rrapply()` applies the `f`
function to any list element that satisfies the `condition` argument
similar to the previous section, but recurses further into any *updated*
list-like element after application of the `f` function. Using
`how = "recurse"`, we can for instance recursively update all node names
in a nested list:

``` r
## Replace country names by M-49 attributes
renewable_M49 <- rrapply(
  list(renewable_energy_by_country), 
  classes = "list",
  f = function(x) {
    names(x) <- vapply(x, attr, character(1L), which = "M49-code")
    return(x)
  },
  how = "recurse"
)

str(renewable_M49[[1]], max.level = 3, list.len = 3, give.attr = FALSE)
#> List of 1
#>  $ 001:List of 6
#>   ..$ 002:List of 2
#>   .. ..$ 015:List of 7
#>   .. ..$ 202:List of 4
#>   ..$ 019:List of 2
#>   .. ..$ 419:List of 3
#>   .. ..$ 021:List of 5
#>   ..$ 010: logi NA
#>   .. [list output truncated]
```

### Using `rrapply()` on data.frames

Since base `rapply()` recurses into all list-like objects, and
data.frames are list-like objects, `rapply()` always descends into the
individual columns of a data.frame. To avoid this behavior using
`rrapply()`, set `classes = "data.frame"`, in which case the `f` and
`condition` functions are applied to complete data.frame objects instead
of individual data.frame columns.

However, it can also be useful to exploit the property that a data.frame
is a list-like object and use base `rapply()` to apply a function `f` to
data.frame columns of certain classes via the `classes` argument. Using
the `condition` argument in `rrapply()`, we can apply a function `f` to
a subset of data.frame columns using a general predicate function,

``` r
## Scale only Sepal columns in iris dataset
iris_standard_sepal <- rrapply(
  iris,
  condition = function(x, .xname) grepl("Sepal", .xname),
  f = scale
)
head(iris_standard_sepal)
#>   Sepal.Length Sepal.Width Petal.Length Petal.Width Species
#> 1   -0.8976739  1.01560199          1.4         0.2  setosa
#> 2   -1.1392005 -0.13153881          1.4         0.2  setosa
#> 3   -1.3807271  0.32731751          1.3         0.2  setosa
#> 4   -1.5014904  0.09788935          1.5         0.2  setosa
#> 5   -1.0184372  1.24503015          1.4         0.2  setosa
#> 6   -0.5353840  1.93331463          1.7         0.4  setosa

## Scale and keep only numeric columns
iris_standard_transmute <- rrapply(
  iris,
  f = scale,
  classes = "numeric",
  how = "prune"
)
head(iris_standard_transmute)
#>   Sepal.Length Sepal.Width Petal.Length Petal.Width
#> 1   -0.8976739  1.01560199    -1.335752   -1.311052
#> 2   -1.1392005 -0.13153881    -1.335752   -1.311052
#> 3   -1.3807271  0.32731751    -1.392399   -1.311052
#> 4   -1.5014904  0.09788935    -1.279104   -1.311052
#> 5   -1.0184372  1.24503015    -1.335752   -1.311052
#> 6   -0.5353840  1.93331463    -1.165809   -1.048667

## Summarize only numeric columns with how = "flatten"
iris_standard_summarize <- rrapply(
  iris,
  f = summary,
  classes = "numeric",
  how = "flatten"
)
iris_standard_summarize
#> $Sepal.Length
#>    Min. 1st Qu.  Median    Mean 3rd Qu.    Max. 
#>   4.300   5.100   5.800   5.843   6.400   7.900 
#> 
#> $Sepal.Width
#>    Min. 1st Qu.  Median    Mean 3rd Qu.    Max. 
#>   2.000   2.800   3.000   3.057   3.300   4.400 
#> 
#> $Petal.Length
#>    Min. 1st Qu.  Median    Mean 3rd Qu.    Max. 
#>   1.000   1.600   4.350   3.758   5.100   6.900 
#> 
#> $Petal.Width
#>    Min. 1st Qu.  Median    Mean 3rd Qu.    Max. 
#>   0.100   0.300   1.300   1.199   1.800   2.500
```

### Using `rrapply()` on expressions

In contrast to base `rapply()`, `rrapply()` supports recursion of call
objects and expression vectors, which are treated as nested lists based
on their internal abstract syntax trees. As such, all functionality that
applies to nested lists extends directly to call objects and expression
vectors.

To update the abstract syntax tree of a call object, use
`how = "replace"`:

``` r
call_old <- quote(y <- x <- 1 + TRUE)
str(call_old)
#>  language y <- x <- 1 + TRUE

## Replace logicals by integers 
call_new <- rrapply(call_old, classes = "logical", f = as.numeric, how = "replace")
str(call_new)
#>  language y <- x <- 1 + 1
```

To update the abstract syntax tree and return it as a nested list, use
`how = "list"`:

``` r
## Update and decompose call object
call_ast <- rrapply(call_old, f = function(x) ifelse(is.logical(x), as.numeric(x), x), how = "list")
str(call_ast)
#> List of 3
#>  $ : symbol <-
#>  $ : symbol y
#>  $ :List of 3
#>   ..$ : symbol <-
#>   ..$ : symbol x
#>   ..$ :List of 3
#>   .. ..$ : symbol +
#>   .. ..$ : num 1
#>   .. ..$ : num 1
```

The choices `how = "prune"`, `how = "flatten"` and `how = "melt"` return
the pruned abstract syntax tree as: a nested list, a flattened list and
a melted data.frame respectively. This is identical to application of
`rrapply()` to the abstract syntax tree formatted as a nested list. To
illustrate, we return all names (i.e. symbols) in the abstract syntax
tree that are not part of base R:

``` r
expr <- expression(y <- x <- 1, f(g(2 * pi)))
is_new_name <- function(x) !exists(as.character(x), envir = baseenv())

## Prune and decompose expression
expr_prune <- rrapply(expr, classes = "name", condition = is_new_name, how = "prune")
str(expr_prune)
#> List of 2
#>  $ :List of 2
#>   ..$ : symbol y
#>   ..$ :List of 1
#>   .. ..$ : symbol x
#>  $ :List of 2
#>   ..$ : symbol f
#>   ..$ :List of 1
#>   .. ..$ : symbol g

## Prune and flatten expression
expr_flatten <- rrapply(expr, classes = "name", condition = is_new_name, how = "flatten")
str(expr_flatten)
#> List of 4
#>  $ : symbol y
#>  $ : symbol x
#>  $ : symbol f
#>  $ : symbol g

## Prune and melt expression
expr_melt <- rrapply(expr, classes = "name", condition = is_new_name, f = as.character, how = "melt")
expr_melt
#>   L1 L2   L3 value
#> 1  1  2 <NA>     y
#> 2  1  3    2     x
#> 3  2  1 <NA>     f
#> 4  2  2    1     g
```

Additional demonstrating examples from
<https://jorischau.github.io/rrapply/articles/articles/3-calls-and-expressions.html>
include replacing any logical abbreviations in an expression by their
non-abbreviated counterparts:

``` r
## Expand logical abbreviation
logical_abbr_expand <- function(x) {
  if(identical(x, quote(T))) {
    TRUE
  } else if(identical(x, quote(F))) {
    FALSE
  } else {
    x
  }
}

rrapply(expression(f(x = c(T, F)), any(T, FALSE)), f = logical_abbr_expand, how = "replace")
#> expression(f(x = c(TRUE, FALSE)), any(TRUE, FALSE))
```

Or, collecting all names created by assignment and returning them as a
character vector:

``` r
## Check if variable is created by assignment
is_assign <- function(x, .xpos, .xsiblings) {
    identical(.xpos[length(.xpos)], 2L) &&
    as.character(.xsiblings[[1]]) %in% c("<-", "=", "for", "assign", "delayedAssign")
}

unique(rrapply(body(lm), condition = is_assign, f = as.character, how = "unlist"))
#>  [1] "ret.x"  "ret.y"  "cl"     "mf"     "m"      "mt"     "y"      "w"     
#>  [9] "offset" "mlm"    "ny"     "x"      "z"
```

For more details and examples on how to use the `rrapply()` function see
the accompanying package vignette in the vignettes folder or the
Articles section at <https://jorischau.github.io/rrapply/>.
