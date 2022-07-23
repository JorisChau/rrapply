
<!-- README.md is generated from README.Rmd. Please edit that file -->

# {rrapply}: Revisiting R-base rapply() <img style="height: 139px;" src='man/figures/sticker.svg' align="right" />

<!-- badges: start -->

[![CRAN
version](http://www.r-pkg.org/badges/version/rrapply)](https://cran.r-project.org/package=rrapply)
[![R-CMD-check](https://github.com/JorisChau/rrapply/workflows/R-CMD-check/badge.svg)](https://github.com/JorisChau/rrapply/actions)
[![codecov](https://codecov.io/gh/JorisChau/rrapply/branch/master/graph/badge.svg)](https://app.codecov.io/gh/JorisChau/rrapply)
[![status](https://tinyverse.netlify.com/badge/rrapply)](https://CRAN.R-project.org/package=rrapply)
[![Total
Downloads](https://cranlogs.r-pkg.org/badges/grand-total/rrapply)](https://cran.r-project.org/package=rrapply)
<!-- badges: end -->

The minimal {rrapply}-package contains a single function `rrapply()`,
providing an extended implementation of R-base’s `rapply()` function,
which applies a function `f` to all elements of a nested list
recursively and controls how to structure the returned result.
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

## Cheat sheet

<div>

<a href='https://github.com/JorisChau/rrapply/blob/master/vignettes/cheatsheet.pdf'>
<img src='man/figures/cheatsheet.svg' align="center" height="600" />
</a>

</div>

## When to use `rrapply()`

### List pruning and unnesting

#### • `how = "prune"`

With base `rapply()`, there is no convenient way to prune or filter list
elements from the input list object. The `rrapply()` function adds an
option `how = "prune"` to prune all list elements not subject to
application of `f` from a nested list,

``` r
library(rrapply)

## data: renewable energy per country in 2016 (% of total energy consumption)
data("renewable_energy_by_country")

## subset countries and areas in Oceania
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
```

``` r
## drop all logical NA's while preserving list structure 
rrapply(
  renewable_oceania,
  f = \(x) x,
  classes = "numeric",
  how = "prune"
) |>
  str(list.len = 3, give.attr = FALSE)

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

#### • `how = "flatten"`

Instead, use `how = "flatten"` to return a flattened unnested version of
the pruned list,

``` r
## drop all logical NA's and return unnested list
rrapply(
  renewable_oceania,
  f = \(x) x,
  classes = "numeric",
  how = "flatten"
) |>
  head(n = 10)

#>        Australia      New Zealand             Fiji    New Caledonia 
#>             9.32            32.76            24.36             4.03 
#> Papua New Guinea  Solomon Islands          Vanuatu             Guam 
#>            50.34            65.73            33.67             3.03 
#>         Kiribati Marshall Islands 
#>            45.43            11.75
```

**Hint**: the `options` argument allows to avoid coercion of the
flattened list to a vector and/or to include all parent list names in
the result.

``` r
## flatten to simple list with full names
rrapply(
  renewable_oceania,
  f = \(x) x,
  classes = "numeric",
  how = "flatten",
  options = list(namesep = ".", simplify = FALSE)
) |>
  str(list.len = 5, give.attr = FALSE)

#> List of 22
#>  $ Oceania.Australia and New Zealand.Australia        : num 9.32
#>  $ Oceania.Australia and New Zealand.New Zealand      : num 32.8
#>  $ Oceania.Melanesia.Fiji                             : num 24.4
#>  $ Oceania.Melanesia.New Caledonia                    : num 4.03
#>  $ Oceania.Melanesia.Papua New Guinea                 : num 50.3
#>   [list output truncated]
```

#### • `how = "melt"`

Or, use `how = "melt"` to return a melted data.frame of the pruned list
similar in format to `reshape2::melt()` applied to a nested list.

``` r
## drop all logical NA's and return melted data.frame
oceania_melt <- rrapply(
  renewable_oceania,
  f = \(x) x,
  classes = "numeric",
  how = "melt"
)
head(oceania_melt)

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
## reconstruct nested list from melted data.frame
rrapply(oceania_melt, how = "unmelt") |>
  str(list.len = 3, give.attr = FALSE)

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

#### • `how = "bind"`

Nested lists containing repeated observations can be unnested with
`how = "bind"`, which returns a wide data.frame similar in format to
`dplyr::bind_rows()` applied to a list of data.frames or repeated
application of `tidyr::unnest_wider()` to a nested data.frame.

``` r
## data: nested list of Pokemon properties in Pokemon GO
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
```

``` r
## unnest list to wide data.frame
rrapply(pokedex, how = "bind")[, c(1:3, 5:8)] |>
  head(n = 9)

#>   id num       name          type height   weight            candy
#> 1  1 001  Bulbasaur Grass, Poison 0.71 m   6.9 kg  Bulbasaur Candy
#> 2  2 002    Ivysaur Grass, Poison 0.99 m  13.0 kg  Bulbasaur Candy
#> 3  3 003   Venusaur Grass, Poison 2.01 m 100.0 kg  Bulbasaur Candy
#> 4  4 004 Charmander          Fire 0.61 m   8.5 kg Charmander Candy
#> 5  5 005 Charmeleon          Fire 1.09 m  19.0 kg Charmander Candy
#> 6  6 006  Charizard  Fire, Flying 1.70 m  90.5 kg Charmander Candy
#> 7  7 007   Squirtle         Water 0.51 m   9.0 kg   Squirtle Candy
#> 8  8 008  Wartortle         Water 0.99 m  22.5 kg   Squirtle Candy
#> 9  9 009  Blastoise         Water 1.60 m  85.5 kg   Squirtle Candy
```

**Hint**: set `options = list(namecols = TRUE)` to include the parent
list names associated to each row in the wide data.frame as individual
columns `L1`, `L2`, etc.

``` r
## bind to data.frame including parent columns
rrapply(pokedex, how = "bind", options = list(namecols = TRUE))[, c(1:5, 7:10)] |>
  head(n = 6)

#>        L1 L2 id num       name          type height   weight            candy
#> 1 pokemon  1  1 001  Bulbasaur Grass, Poison 0.71 m   6.9 kg  Bulbasaur Candy
#> 2 pokemon  2  2 002    Ivysaur Grass, Poison 0.99 m  13.0 kg  Bulbasaur Candy
#> 3 pokemon  3  3 003   Venusaur Grass, Poison 2.01 m 100.0 kg  Bulbasaur Candy
#> 4 pokemon  4  4 004 Charmander          Fire 0.61 m   8.5 kg Charmander Candy
#> 5 pokemon  5  5 005 Charmeleon          Fire 1.09 m  19.0 kg Charmander Candy
#> 6 pokemon  6  6 006  Charizard  Fire, Flying 1.70 m  90.5 kg Charmander Candy
```

### Condition function

Base `rapply()` allows to apply a function `f` to list elements of
certain types or classes via the `classes` argument. `rrapply()`
generalizes this option via an additional `condition` argument, which
accepts any function to use as a condition or predicate to apply `f` to
a subset of list elements.

``` r
## drop all NA elements using condition 
rrapply(
  renewable_oceania,
  condition = \(x) !is.na(x),
  f = \(x) x,
  how = "prune"
) |>
  str(list.len = 3, give.attr = FALSE)

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

``` r
## filter all countries with values > 85%
rrapply(
  renewable_energy_by_country,
  condition = \(x) x > 85,
  how = "prune"
) |>
  str(give.attr = FALSE)

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

With base `rapply()`, the `f` function only has access to the *content*
of the list element under evaluation, and there is no convenient way to
access its name or location in the nested list from inside `f`.
`rrapply()` defines the special arguments `.xname`, `.xpos`,
`.xparents`, `.xsiblings` inside the `f` and `condition` functions (in
addition to the principal function argument):

-   `.xname` evaluates to the name of the list element;
-   `.xpos` evaluates to the position of the element in the nested list
    structured as an integer vector;
-   `.xparents` evaluates to a vector of parent list names in the path
    to the current list element;
-   `.xsiblings` evaluates to the parent list containing the current
    list element and its direct siblings.

``` r
## apply f based on element's name
rrapply(
  renewable_oceania,
  condition = \(x) !is.na(x),
  f = \(x, .xname) sprintf("Renewable energy in %s: %.2f%%", .xname, x),
  how = "flatten"
) |>
  head(n = 5)

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

## filter elements by name 
rrapply(
  renewable_energy_by_country,
  condition = \(x, .xname) .xname %in% c("Belgium", "Netherlands", "Luxembourg"),
  how = "prune"
) |>
  str(give.attr = FALSE)

#> List of 1
#>  $ World:List of 1
#>   ..$ Europe:List of 1
#>   .. ..$ Western Europe:List of 3
#>   .. .. ..$ Belgium    : num 9.14
#>   .. .. ..$ Luxembourg : num 13.5
#>   .. .. ..$ Netherlands: num 5.78

## filter European countries > 50% using .xpos
rrapply(
  renewable_energy_by_country,
  condition = \(x, .xpos) identical(.xpos[1:2], c(1L, 5L)) && x > 50,
  how = "prune"
) |>
  str(give.attr = FALSE)

#> List of 1
#>  $ World:List of 1
#>   ..$ Europe:List of 2
#>   .. ..$ Northern Europe:List of 3
#>   .. .. ..$ Iceland: num 78.1
#>   .. .. ..$ Norway : num 59.5
#>   .. .. ..$ Sweden : num 51.4
#>   .. ..$ Western Europe :List of 1
#>   .. .. ..$ Liechtenstein: num 62.9

## filter European countries > 50% using .xparents
rrapply(
  renewable_energy_by_country, 
  condition = \(x, .xparents) "Europe" %in% .xparents && x > 50,
  how = "prune"
) |>
  str(give.attr = FALSE)

#> List of 1
#>  $ World:List of 1
#>   ..$ Europe:List of 2
#>   .. ..$ Northern Europe:List of 3
#>   .. .. ..$ Iceland: num 78.1
#>   .. .. ..$ Norway : num 59.5
#>   .. .. ..$ Sweden : num 51.4
#>   .. ..$ Western Europe :List of 1
#>   .. .. ..$ Liechtenstein: num 62.9

## return position of element in list
rrapply(
  renewable_energy_by_country,
  condition = \(x, .xname) .xname == "Sweden",
  f = \(x, .xpos) .xpos,
  how = "flatten"
)

#> $Sweden
#> [1]  1  5  2 14

## return siblings of element in list
rrapply(
  renewable_energy_by_country,
  condition = \(x, .xsiblings) "Sweden" %in% names(.xsiblings),
  how = "flatten"
) |>
  head(n = 5)

#> Aland Islands       Denmark       Estonia Faroe Islands       Finland 
#>            NA         33.06         26.55          4.24         42.03

## filter elements and unnest list  
rrapply(
  pokedex,
  condition = \(x, .xpos, .xname) length(.xpos) < 4 & .xname %in% c("num", "name", "type"),
  how = "bind"
) |>
  head()

#>   num       name          type
#> 1 001  Bulbasaur Grass, Poison
#> 2 002    Ivysaur Grass, Poison
#> 3 003   Venusaur Grass, Poison
#> 4 004 Charmander          Fire
#> 5 005 Charmeleon          Fire
#> 6 006  Charizard  Fire, Flying
```

### Modifying list elements

By default, both base `rapply()` and `rrapply()` recurse into any
“list-like” element. Set `classes = "list"` in `rrapply()` to override
and apply `f` to any list element (i.e. a sublist) that satisfies the
`condition` argument. This can be useful to e.g. collapse sublists or
calculate summary statistics across elements in a nested list:

``` r
## calculate mean value of Europe
rrapply(
  renewable_energy_by_country,  
  condition = \(x, .xname) .xname == "Europe",
  f = \(x) mean(unlist(x), na.rm = TRUE),
  classes = "list",
  how = "flatten"
)

#>   Europe 
#> 22.36565

## calculate mean value for each continent
## (Antartica's value is missing)
rrapply(
  renewable_energy_by_country, 
  condition = \(x, .xpos) length(.xpos) == 2,
  f = \(x) mean(unlist(x), na.rm = TRUE),
  classes = "list"
) |>
  str(give.attr = FALSE)

#> List of 1
#>  $ World:List of 6
#>   ..$ Africa    : num 54.3
#>   ..$ Americas  : num 18.2
#>   ..$ Antarctica: logi NA
#>   ..$ Asia      : num 17.9
#>   ..$ Europe    : num 22.4
#>   ..$ Oceania   : num 17.8

## simplify pokemon evolutions to character vectors 
rrapply(
  pokedex,
  condition = \(x, .xname) .xname %in% c("name", "next_evolution", "prev_evolution"), 
  f = \(x) if(is.list(x)) sapply(x, `[[`, "name") else x,
  classes = c("character", "list"),
  how = "bind"
) |>
  head(n = 9)

#>         name        next_evolution         prev_evolution
#> 1  Bulbasaur     Ivysaur, Venusaur                     NA
#> 2    Ivysaur              Venusaur              Bulbasaur
#> 3   Venusaur                    NA     Bulbasaur, Ivysaur
#> 4 Charmander Charmeleon, Charizard                     NA
#> 5 Charmeleon             Charizard             Charmander
#> 6  Charizard                    NA Charmander, Charmeleon
#> 7   Squirtle  Wartortle, Blastoise                     NA
#> 8  Wartortle             Blastoise               Squirtle
#> 9  Blastoise                    NA    Squirtle, Wartortle
```

**Hint**: as data.frames are also list-like objects, `rrapply()` applies
`f` to individual data.frame columns by default. Set
`classes = "data.frame"` to avoid this behavior and apply the `f` and
`condition` functions to complete data.frame objects instead of
individual data.frame columns.

### Recursive list updating

#### • `how = "recurse"`

If `classes = "list"` and `how = "recurse"`, `rrapply()` applies the `f`
function to any list element that satisfies the `condition` argument
similar to the previous section, but recurses further into any *updated*
list-like element after application of `f`. This can be useful to
e.g. recursively update the class or other attributes of all elements in
a nested list:

``` r
## recursively remove all list attributes
rrapply(
  renewable_oceania,
  f = \(x) c(x),
  classes = c("list", "ANY"),
  how = "recurse"
) |>
  str(list.len = 3, give.attr = TRUE)

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
```

#### • `how = "names"`

The option `how = "names"` is a special case of `how = "recurse"`, where
the value of `f` is used to replace the *name* of the evaluated list
element instead of its *content* (as with all other `how` options). By
default, `how = "names"` uses `classes = c("list", "ANY")` in order to
allow updating of all names in the nested list.

``` r
## recursively replace all names by M49-codes
rrapply(
  renewable_oceania,
  f = \(x) attr(x, "M49-code"),
  how = "names"
) |>
  str(list.len = 3, give.attr = FALSE)

#> List of 1
#>  $ 009:List of 4
#>   ..$ 053:List of 6
#>   .. ..$ 036: num 9.32
#>   .. ..$ 162: logi NA
#>   .. ..$ 166: logi NA
#>   .. .. [list output truncated]
#>   ..$ 054:List of 5
#>   .. ..$ 242: num 24.4
#>   .. ..$ 540: num 4.03
#>   .. ..$ 598: num 50.3
#>   .. .. [list output truncated]
#>   ..$ 057:List of 8
#>   .. ..$ 316: num 3.03
#>   .. ..$ 296: num 45.4
#>   .. ..$ 584: num 11.8
#>   .. .. [list output truncated]
#>   .. [list output truncated]
```

### Expression objects

Base `rapply()` does not include recursion for expression objects. In
contrast `rrapply()` supports recursion of call objects and expression
vectors, which are treated as nested lists based on their abstract
syntax trees. As such, all functionality that applies to nested lists
extends directly to call objects and expression vectors.

To update the abstract syntax tree of a call object, use
`how = "replace"`:

``` r
## language object
(lang <- quote(y <- x <- 1 + TRUE))

#> y <- x <- 1 + TRUE

## replace logicals by integers 
rrapply(lang, classes = "logical", f = as.numeric, how = "replace")

#> y <- x <- 1 + 1
```

To update the abstract syntax tree and return it as a nested list, use
`how = "list"`:

``` r
## update and decompose call object
rrapply(lang, f = function(x) ifelse(is.logical(x), as.numeric(x), x), how = "list") |>
  str()

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

The modes `how = "prune"`, `how = "flatten"` and `how = "melt"` return
the pruned abstract syntax tree as: a nested list, a flattened list and
a melted data.frame respectively. This is identical to application of
`rrapply()` to the abstract syntax tree formatted as a nested list.

To illustrate, we return all names (i.e. symbols) in the abstract syntax
tree that not part of base R:

``` r
## expression vector
expr <- expression(y <- x <- 1, f(g(2 * pi)))
is_new_name <- function(x) !exists(as.character(x), envir = baseenv())

## prune and decompose expression
rrapply(expr, classes = "name", condition = is_new_name, how = "prune") |>
  str()

#> List of 2
#>  $ :List of 2
#>   ..$ : symbol y
#>   ..$ :List of 1
#>   .. ..$ : symbol x
#>  $ :List of 2
#>   ..$ : symbol f
#>   ..$ :List of 1
#>   .. ..$ : symbol g

## prune and flatten expression
rrapply(expr, classes = "name", condition = is_new_name, how = "flatten") |>
  str()

#> List of 4
#>  $ : symbol y
#>  $ : symbol x
#>  $ : symbol f
#>  $ : symbol g

## prune and melt expression
rrapply(expr, classes = "name", condition = is_new_name, f = as.character, how = "melt")

#>   L1 L2   L3 value
#> 1  1  2 <NA>     y
#> 2  1  3    2     x
#> 3  2  1 <NA>     f
#> 4  2  2    1     g
```

For more details and examples on how to use the `rrapply()` function see
the accompanying package vignette in the vignettes folder or the
Articles section at <https://jorischau.github.io/rrapply/>.
