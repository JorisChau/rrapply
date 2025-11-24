# Efficient list recursion with rrapply

## List recursion in R

The nested list below shows a small extract from the [Mathematics
Genealogy Project](https://www.genealogy.math.ndsu.nodak.edu/)
highlighting the advisor/student genealogy of several famous
mathematicians. The mathematician’s given names are present in the
`"given"` attribute of each list element. The numeric values at the leaf
elements are the total number of student descendants according to the
website as of June 2022. If no descendants are available there is a
missing value present at the leaf element.

``` r
students <- list(
  Bernoulli = structure(list(
    Bernoulli = structure(list(
      Bernoulli = structure(1L, given = "Daniel"),
      Euler = structure(list(
        Euler = structure(NA, given = "Johann"),
        Lagrange = structure(list(
          Fourier = structure(73788L, given = "Jean-Baptiste"), 
          Plana = structure(NA, given = "Giovanni"),
          Poisson = structure(128235L, given = "Simeon")
        ), given = "Joseph")
      ), given = "Leonhard")
    ), given = "Johann"),
    Bernoulli = structure(NA, given = "Nikolaus")
  ), given = "Jacob")
)

str(students, give.attr = FALSE)
#> List of 1
#>  $ Bernoulli:List of 2
#>   ..$ Bernoulli:List of 2
#>   .. ..$ Bernoulli: int 1
#>   .. ..$ Euler    :List of 2
#>   .. .. ..$ Euler   : logi NA
#>   .. .. ..$ Lagrange:List of 3
#>   .. .. .. ..$ Fourier: int 73788
#>   .. .. .. ..$ Plana  : logi NA
#>   .. .. .. ..$ Poisson: int 128235
#>   ..$ Bernoulli: logi NA
```

As an exercise in list recursion, consider the following simple data
exploration question:

> Filter all descendants of ‘Leonhard Euler’ and replace all missing
> values by zero while maintaining the list structure.

Here is a possible (not so efficient) base R solution using recursion
with the [`Recall()`](https://rdrr.io/r/base/Recall.html) function:

``` r
filter_desc_euler <- function(x) {
  i <- 1
  while(i <= length(x)) {
    if(identical(names(x)[i], "Euler") & identical(attr(x[[i]], "given"), "Leonhard")) {
      x[[i]] <- rapply(x[[i]], f = \(x) replace(x, is.na(x), 0), how = "replace")
      i <- i + 1
    } else {
      if(is.list(x[[i]])) {
        val <- Recall(x[[i]])
        x[[i]] <- val
        i <- i + !is.null(val)
      } else {
        x[[i]] <- NULL
      }
      if(all(sapply(x, is.null))) {
        x <- NULL
      }
    }
  }
  return(x)
}

str(filter_desc_euler(students), give.attr = FALSE)
#> List of 1
#>  $ Bernoulli:List of 1
#>   ..$ Bernoulli:List of 1
#>   .. ..$ Euler:List of 2
#>   .. .. ..$ Euler   : num 0
#>   .. .. ..$ Lagrange:List of 3
#>   .. .. .. ..$ Fourier: num 73788
#>   .. .. .. ..$ Plana  : num 0
#>   .. .. .. ..$ Poisson: num 128235
```

This works, but is hardly the kind of convoluted code we would like to
write for such a seemingly simple question. Moreover, this code is not
very easy to follow, which can make updating or modifying it quite a
time-consuming and error-prone task.

An alternative approach would be to unnest the list into a more
manageable (e.g. rectangular) format or use specialized packages, such
as [igraph](https://igraph.org/r/) or
[data.tree](https://CRAN.R-project.org/package=data.tree), to make
pruning or modifying node entries more straightforward. Note that
attention must be paid to correctly include the node attributes in the
transformed object as the node names themselves are not unique in this
example. This is a sensible approach and usually the way to go when
cleaning or tidying up the data, but for fast prototyping and data
exploration tasks we may want to keep the list in its original format to
reduce the number of processing steps and minimize the code complexity.
Sometimes it is also required to maintain a nested data structure as
this can be for instance the expected input format for some data
visualization or data exporting function.

The recursive function above makes use of base
[`rapply()`](https://rdrr.io/r/base/rapply.html), a member of the
[apply](https://rdrr.io/r/base/lapply.html)-family of functions in R,
that allows to apply a function recursively to the elements of a nested
list and decide how the returned result is structured. Although
sometimes useful, the existing
[`rapply()`](https://rdrr.io/r/base/rapply.html) function is often not
sufficiently flexible for list recursion tasks in practice, for instance
for pruning elements of a nested list (as demonstrated above). In this
context, the
[`rrapply()`](https://jorischau.github.io/rrapply/reference/rrapply.md)
function in the minimal `rrapply`-package attempts to enhance and update
base [`rapply()`](https://rdrr.io/r/base/rapply.html) to make it more
generally applicable for list recursion in the wild. The
[`rrapply()`](https://jorischau.github.io/rrapply/reference/rrapply.md)
function builds upon R’s native C implementation
of[`rapply()`](https://rdrr.io/r/base/rapply.html) and for this reason
requires no other external dependencies.

## When to use `rrapply()`

For demonstrational purposes, we make use of the two datasets
`renewable_energy_by_country` and `pokedex` included in the
`rrapply`-package. `renewable_energy_by_country` is a nested list
containing the renewable energy shares per country (% of total energy
consumption) in 2016. The data is publicly available at the [United
Nations Open SDG Data
Hub](https://unstats-undesa.opendata.arcgis.com/datasets/). The 249
countries and areas are structured based on their geographical locations
according to the [United Nations M49
standard](https://unstats.un.org/unsd/methodology/m49/). The numeric
values listed for each country are percentages, if no data is available
the value of the country is `NA`. `pokedex` is a nested list containing
various property values for each of the 151 original Pokémon available
(in .json) from <https://github.com/Biuni/PokemonGO-Pokedex>.

``` r
library(rrapply)
data("renewable_energy_by_country")
```

For convenience, we subset only the values for countries and areas in
Oceania from `renewable_energy_by_country`,

``` r
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

### List pruning and unnesting

#### `how = "prune"`

With base [`rapply()`](https://rdrr.io/r/base/rapply.html), there is no
convenient way to prune or filter elements from the input list. The
[`rrapply()`](https://jorischau.github.io/rrapply/reference/rrapply.md)
function adds an option `how = "prune"` to prune all list elements not
subject to application of the function `f` from a nested list. The
original list structure is retained, similar to the non-pruned versions
`how = "replace"` and `how = "list"`. Using `how = "prune"` and the same
syntax as in [`rapply()`](https://rdrr.io/r/base/rapply.html), we can
easily drop all missing values from the list while preserving the nested
list structure:

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

**Remark**: if the `f` function is missing, it defaults to the identity
function. That is, the `f` argument can be dropped when no (non-trivial)
function is applied to the list elements.

#### `how = "flatten"`

Instead, we can set `how = "flatten"` to return a *flattened* unnested
version of the pruned list. This is more efficient than first returning
the pruned list with `how = "prune"` and unlisting or flattening the
list in a subsequent step.

``` r
## drop all logical NA's and return unnested list
rrapply(
  renewable_oceania,
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

**Hint**: the `options` argument allows to tune several options specific
to certain choices of `how`. With `how = "flatten"`, we can choose to
not coerce the flattened list to a vector and/or to include all parent
list names in the result similar to `how = "unlist"` but then with a
custom name separator.

``` r
## flatten to simple list with full names
rrapply(
  renewable_oceania,
  classes = "numeric",
  how = "flatten",
  options = list(namesep = ".", simplify = FALSE)
) |>
  str(list.len = 10, give.attr = FALSE)
#> List of 22
#>  $ Oceania.Australia and New Zealand.Australia        : num 9.32
#>  $ Oceania.Australia and New Zealand.New Zealand      : num 32.8
#>  $ Oceania.Melanesia.Fiji                             : num 24.4
#>  $ Oceania.Melanesia.New Caledonia                    : num 4.03
#>  $ Oceania.Melanesia.Papua New Guinea                 : num 50.3
#>  $ Oceania.Melanesia.Solomon Islands                  : num 65.7
#>  $ Oceania.Melanesia.Vanuatu                          : num 33.7
#>  $ Oceania.Micronesia.Guam                            : num 3.03
#>  $ Oceania.Micronesia.Kiribati                        : num 45.4
#>  $ Oceania.Micronesia.Marshall Islands                : num 11.8
#>   [list output truncated]
```

#### `how = "melt"`

Using `how = "melt"`, we can return a melted data.frame of the pruned
list similar in format to
[`reshape2::melt()`](https://rdrr.io/pkg/reshape2/man/melt.html) applied
to a nested list. The rows of the melted data.frame contain the parent
node paths of the elements in the pruned list. The `"value"` column
contains the values of the terminal or leaf nodes analogous to the
flattened list returned by `how = "flatten"`.

``` r
## drop all logical NA's and return melted data.frame
oceania_melt <- rrapply(
  renewable_oceania,
  classes = "numeric",
  how = "melt"
) 
head(oceania_melt, n = 10)
#>         L1                        L2               L3 value
#> 1  Oceania Australia and New Zealand        Australia  9.32
#> 2  Oceania Australia and New Zealand      New Zealand 32.76
#> 3  Oceania                 Melanesia             Fiji 24.36
#> 4  Oceania                 Melanesia    New Caledonia  4.03
#> 5  Oceania                 Melanesia Papua New Guinea 50.34
#> 6  Oceania                 Melanesia  Solomon Islands 65.73
#> 7  Oceania                 Melanesia          Vanuatu 33.67
#> 8  Oceania                Micronesia             Guam  3.03
#> 9  Oceania                Micronesia         Kiribati 45.43
#> 10 Oceania                Micronesia Marshall Islands 11.75
```

**Remark**: if no names are present in a certain sublist of the input
list, `how = "melt"` replaces the names in the melted data.frame by list
element indices `"1"`, `"2"`, etc.

``` r
## drop some area names 
renewable_oceania1 <- renewable_oceania
renewable_oceania1[[1]] <- unname(renewable_oceania[[1]])

## drop all logical NA's and return melted data.frame
rrapply(
  renewable_oceania1,
  classes = "numeric",
  how = "melt"
) |>
  head(n = 10)
#>         L1 L2               L3 value
#> 1  Oceania  1        Australia  9.32
#> 2  Oceania  1      New Zealand 32.76
#> 3  Oceania  2             Fiji 24.36
#> 4  Oceania  2    New Caledonia  4.03
#> 5  Oceania  2 Papua New Guinea 50.34
#> 6  Oceania  2  Solomon Islands 65.73
#> 7  Oceania  2          Vanuatu 33.67
#> 8  Oceania  3             Guam  3.03
#> 9  Oceania  3         Kiribati 45.43
#> 10 Oceania  3 Marshall Islands 11.75
```

A melted data.frame can be used to reconstruct a nested list with
`how = "unmelt"`. No skeleton object as e.g. required by
[`relist()`](https://rdrr.io/r/utils/relist.html) is needed, only an
ordinary data.frame in the format returned by `how = "melt"`. This
option can be convenient to construct nested lists from a rectangular
data.frame format without having to resort to recursive function
definitions.

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

#### `how = "bind"`

Nested lists containing repeated observations can be unnested with
`how = "bind"`. Each repeated sublist is expanded as a single row in a
wide data.frame and identical sublist component names are aligned as
individual columns. By default, the list layer containing the repeated
observations is identified by the minimal depth detected across leaf
elements, but this can also be overridden using the `coldepth` option in
the `options` argument. Note that the returned data.frame is similar in
format to repeated application of
[`tidyr::unnest_wider()`](https://tidyr.tidyverse.org/reference/unnest_wider.html)
to a nested data.frame, with the same coercion rules applied to the
individual columns as \`how = “unlist”.

``` r
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
  head(n = 10)
#>    id num       name          type height   weight            candy
#> 1   1 001  Bulbasaur Grass, Poison 0.71 m   6.9 kg  Bulbasaur Candy
#> 2   2 002    Ivysaur Grass, Poison 0.99 m  13.0 kg  Bulbasaur Candy
#> 3   3 003   Venusaur Grass, Poison 2.01 m 100.0 kg  Bulbasaur Candy
#> 4   4 004 Charmander          Fire 0.61 m   8.5 kg Charmander Candy
#> 5   5 005 Charmeleon          Fire 1.09 m  19.0 kg Charmander Candy
#> 6   6 006  Charizard  Fire, Flying 1.70 m  90.5 kg Charmander Candy
#> 7   7 007   Squirtle         Water 0.51 m   9.0 kg   Squirtle Candy
#> 8   8 008  Wartortle         Water 0.99 m  22.5 kg   Squirtle Candy
#> 9   9 009  Blastoise         Water 1.60 m  85.5 kg   Squirtle Candy
#> 10 10 010   Caterpie           Bug 0.30 m   2.9 kg   Caterpie Candy
```

**Hint**: setting `namecols = TRUE` in the `options` argument includes
the parent list names associated to each row in the wide data.frame as
individual columns `L1`, `L2`, etc.

``` r
## bind to data.frame including parent columns
pokemon_evolutions <- rrapply(
  pokedex, 
  how = "bind", 
  options = list(namecols = TRUE, coldepth = 5)
) 
head(pokemon_evolutions, n = 10)
#>         L1 L2             L3 L4 num       name
#> 1  pokemon  1 next_evolution  1 002    Ivysaur
#> 2  pokemon  1 next_evolution  2 003   Venusaur
#> 3  pokemon  2 prev_evolution  1 001  Bulbasaur
#> 4  pokemon  2 next_evolution  1 003   Venusaur
#> 5  pokemon  3 prev_evolution  1 001  Bulbasaur
#> 6  pokemon  3 prev_evolution  2 002    Ivysaur
#> 7  pokemon  4 next_evolution  1 005 Charmeleon
#> 8  pokemon  4 next_evolution  2 006  Charizard
#> 9  pokemon  5 prev_evolution  1 004 Charmander
#> 10 pokemon  5 next_evolution  1 006  Charizard
```

This can be useful to unnest repeated list elements at multiple nested
list levels and join the results into a single data.frame:

``` r
## merge pokemon evolutions with pokemon names
rrapply(
  pokedex,
  how = "bind",
  options = list(namecols = TRUE)
)[, c("L1", "L2", "name")] |>
  merge(
    pokemon_evolutions[, c("L1", "L2", "L3", "name")],
    by = c("L1", "L2"),
    suffixes = c("", ".evolution")
  ) |>
  head(n = 10)
#>         L1  L2      name             L3 name.evolution
#> 1  pokemon   1 Bulbasaur next_evolution        Ivysaur
#> 2  pokemon   1 Bulbasaur next_evolution       Venusaur
#> 3  pokemon  10  Caterpie next_evolution        Metapod
#> 4  pokemon  10  Caterpie next_evolution     Butterfree
#> 5  pokemon 100   Voltorb next_evolution      Electrode
#> 6  pokemon 101 Electrode prev_evolution        Voltorb
#> 7  pokemon 102 Exeggcute next_evolution      Exeggutor
#> 8  pokemon 103 Exeggutor prev_evolution      Exeggcute
#> 9  pokemon 104    Cubone next_evolution        Marowak
#> 10 pokemon 105   Marowak prev_evolution         Cubone
```

### Condition function

Base [`rapply()`](https://rdrr.io/r/base/rapply.html) allows to apply a
function `f` to list elements of certain types or classes via the
`classes` argument.
[`rrapply()`](https://jorischau.github.io/rrapply/reference/rrapply.md)
generalizes this concept via an additional `condition` argument, which
accepts any function to use as a condition or predicate to apply `f` to
a subset of list elements. Conceptually, the `f` function is applied to
all leaf elements for which the `condition` function exactly evaluates
to `TRUE` similar to [`isTRUE()`](https://rdrr.io/r/base/Logic.html). If
the `condition` argument is missing, `f` is applied to all leaf
elements. In combination with `how = "prune"`, the `condition` function
provides additional flexibility in selecting and filtering elements from
a nested list,

``` r
## drop all NA's using condition function
rrapply(
  renewable_oceania,
  condition = \(x) !is.na(x),
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

More interesting is to consider a `condition` that cannot also be
defined using the `classes` argument. For instance, we can filter all
countries with values that satisfy a certain numeric condition:

``` r
## filter all countries with values above 85%
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

## or by passing arguments to condition via ...
rrapply(
  renewable_energy_by_country, 
  condition = "==", 
  e2 = 0, 
  how = "prune"
) |>
  str(give.attr = FALSE)
#> List of 1
#>  $ World:List of 4
#>   ..$ Americas:List of 1
#>   .. ..$ Latin America and the Caribbean:List of 1
#>   .. .. ..$ Caribbean:List of 1
#>   .. .. .. ..$ Antigua and Barbuda: num 0
#>   ..$ Asia    :List of 1
#>   .. ..$ Western Asia:List of 4
#>   .. .. ..$ Bahrain: num 0
#>   .. .. ..$ Kuwait : num 0
#>   .. .. ..$ Oman   : num 0
#>   .. .. ..$ Qatar  : num 0
#>   ..$ Europe  :List of 2
#>   .. ..$ Northern Europe:List of 1
#>   .. .. ..$ Channel Islands:List of 1
#>   .. .. .. ..$ Guernsey: num 0
#>   .. ..$ Southern Europe:List of 1
#>   .. .. ..$ Gibraltar: num 0
#>   ..$ Oceania :List of 2
#>   .. ..$ Micronesia:List of 1
#>   .. .. ..$ Northern Mariana Islands: num 0
#>   .. ..$ Polynesia :List of 1
#>   .. .. ..$ Wallis and Futuna Islands: num 0
```

Note that the `NA` elements are not returned, as the `condition`
function does not evaluate to `TRUE` for `NA` values.

As the `condition` function is a generalization of the `classes`
argument, it remains possible to use `deflt` together with
`how = "list"` or `how = "unlist"` to set a default value to all leaf
elements for which the `condition` is not `TRUE`:

``` r
## replace all NA elements by zero
rrapply(
  renewable_oceania, 
  condition = Negate(is.na), 
  deflt = 0, 
  how = "list"
) |>
  str(list.len = 3, give.attr = FALSE)
#> List of 1
#>  $ Oceania:List of 4
#>   ..$ Australia and New Zealand:List of 6
#>   .. ..$ Australia                        : num 9.32
#>   .. ..$ Christmas Island                 : num 0
#>   .. ..$ Cocos (Keeling) Islands          : num 0
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

To be consistent with base
[`rapply()`](https://rdrr.io/r/base/rapply.html), the `deflt` argument
can still only be used in combination with `how = "list"` or
`how = "unlist"`.

#### Using the `...` argument

The first argument to `f` always evaluates to the content of the list
element to which `f` is applied. Any further arguments, (besides the
special arguments `.xname`, `.xpos`, `.xparents` and `.xsiblings`
discussed below), that are independent of the list content can be
supplied via the `...` argument. Since
[`rrapply()`](https://jorischau.github.io/rrapply/reference/rrapply.md)
accepts a function in two of its arguments `f` and `condition`, any
arguments defined via the `...` need to be defined as function arguments
in *both* the `f` and `condition` functions (if existing), even if they
are not used in the function itself.

To clarify, consider the following example which replaces all missing
values by a value defined in a separate argument `newvalue`:

``` r
## this is not ok!
tryCatch({
  rrapply(
    renewable_oceania, 
    condition = is.na, 
    f = \(x, newvalue) newvalue, 
    newvalue = 0, 
    how = "replace"
  )
}, error = function(error) error$message)
#> [1] "2 arguments passed to 'is.na' which requires 1"

## this is ok
rrapply(
  renewable_oceania, 
  condition = \(x, newvalue) is.na(x), 
  f = \(x, newvalue) newvalue, 
  newvalue = 0, 
  how = "replace"
) |>
  str(list.len = 3, give.attr = FALSE)
#> List of 1
#>  $ Oceania:List of 4
#>   ..$ Australia and New Zealand:List of 6
#>   .. ..$ Australia                        : num 9.32
#>   .. ..$ Christmas Island                 : num 0
#>   .. ..$ Cocos (Keeling) Islands          : num 0
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

### Special arguments `.xname`, `.xpos`, `.xparents`, `.xsiblings`

With base [`rapply()`](https://rdrr.io/r/base/rapply.html), the `f`
function only has access to the *content* of the list element under
evaluation, and there is no convenient way to access its name or
location in the nested list from inside the `f` function. To overcome
this limitation,
[`rrapply()`](https://jorischau.github.io/rrapply/reference/rrapply.md)
defines the special arguments `.xname`, `.xpos`, `.xparents` and
`.xsiblings` inside the `f` and `condition` functions (in addition to
the principal function argument):

- `.xname` evaluates to the name of the list element;
- `.xpos` evaluates to the position of the element in the nested list
  structured as an integer vector;
- `.xparents` evaluates to a vector of parent list names in the path to
  the current list element;
- `.xsiblings` evaluates to the parent list containing the current list
  element and its direct siblings.

Using the `.xname` and `.xpos` arguments, we can transform or filter
list elements based on their names and/or positions in the nested list:

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
```

Knowing that Europe is located at
`renewable_energy_by_country[[c(1, 5)]]`, we can filter all European
countries with a renewable energy share above 50% using the `.xpos`
argument as follows,

``` r
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
```

This can be done more conveniently using the `.xparents` argument, which
this does not require looking up the location of Europe in the nested
list,

``` r
## filter European countries > 50% using .xparents
rrapply(
  renewable_energy_by_country,
  condition = function(x, .xparents) "Europe" %in% .xparents && x > 50,
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
```

Using the `.xpos` argument, we can quickly look up the position of a
specific element in the nested list,

``` r
## return position of Sweden in list
rrapply(
  renewable_energy_by_country,
  condition = \(x, .xname) .xname == "Sweden",
  f = \(x, .xpos) .xpos,
  how = "flatten"
)
#> $Sweden
#> [1]  1  5  2 14
```

Using the `.xsiblings` argument, we can look up the direct neighbors of
an element in the nested list,

``` r
## look up neighbors of Sweden in list
rrapply(
  renewable_energy_by_country,
  condition = \(x, .xsiblings) "Sweden" %in% names(.xsiblings),
  how = "flatten"
) |>
  head(n = 10)
#> Aland Islands       Denmark       Estonia Faroe Islands       Finland 
#>            NA         33.06         26.55          4.24         42.03 
#>       Iceland       Ireland   Isle of Man        Latvia     Lithuania 
#>         78.07          8.65          4.30         38.48         31.42
```

We can also use the `.xpos` argument to determine the maximum depth of
the list or the length of the longest sublist as follows,

``` r
## maximum list depth
rrapply(
  renewable_energy_by_country, 
  f = \(x, .xpos) length(.xpos), 
  how = "unlist"
) |>
  max()
#> [1] 5

## longest sublist length
rrapply(
  renewable_energy_by_country, 
  f = \(x, .xpos) max(.xpos), 
  how = "unlist"
) |>
  max()
#> [1] 28
```

When unnesting nested lists with `how = "bind"`, the `.xname`, `.xpos`
or `.xparents` arguments can be useful to decide which list elements to
include in the unnested data.frame:

``` r
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

By default, both base [`rapply()`](https://rdrr.io/r/base/rapply.html)
and
[`rrapply()`](https://jorischau.github.io/rrapply/reference/rrapply.md)
recurse into any *list-like* element. Setting `classes = "list"` in
[`rrapply()`](https://jorischau.github.io/rrapply/reference/rrapply.md)
overrides this behavior and applies the `f` function to any list element
(i.e. a sublist) that satisfies the `condition` argument. If the
`condition` is not satisfied for a list element,
[`rrapply()`](https://jorischau.github.io/rrapply/reference/rrapply.md)
recurses further into the sublist, applies `f` to the elements that
satisfy `condition` and so on. The use of `classes = "list"` signals the
[`rrapply()`](https://jorischau.github.io/rrapply/reference/rrapply.md)
function not to descend into list objects by default. For this reason
this behavior can only be triggered via the `classes` argument and *not*
through the use of e.g. `condition = is.list`.

The mode `classes = "list"` can be useful to e.g. collapse sublists or
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
```

Note that the principal argument in the `f` function now evaluates to a
list. For this reason, we first have to `unlist` the sublist before
calculating the mean.

To calculate the mean renewable energy shares for each continent, we can
make use of the fact that the `.xpos` vector of each continent has
length (i.e. depth) 2:

``` r
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
```

**Remark**: if `classes = "list"`, the `f` function is only applied to
the (non-terminal) list elements. To apply `f` to both terminal and
non-terminal elements in the nested list, we can include additional
classes, such as `classes = c("list", "numeric", "character")`. To apply
`f` to *any* terminal and non-terminal element in the nested list, we
can even combine `classes = c("list", "ANY")`. To illustrate, we search
across all list elements for the country or region with M49-code
`"155"`:

``` r
## filter country or region by M49-code
rrapply(
  renewable_energy_by_country,
  condition = \(x) attr(x, "M49-code") == "155",
  f = \(x, .xname) .xname,
  classes = c("list", "ANY"), 
  how = "unlist"
)
#> World.Europe.Western Europe 
#>            "Western Europe"
```

As a more complex example, we unnest the Pokémon evolutions in `pokedex`
into a wide data.frame by returning the sublists with Pokémon evolutions
as character vectors:

``` r
## simplify pokemon evolutions to character vectors 
rrapply(
  pokedex,
  condition = \(x, .xname) .xname %in% c("name", "next_evolution", "prev_evolution"), 
  f = \(x) if(is.list(x)) sapply(x, `[[`, "name") else x,
  classes = c("list", "character"),
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

**Hint:** as data.frames are also list-like objects,
[`rrapply()`](https://jorischau.github.io/rrapply/reference/rrapply.md)
applies `f` to individual data.frame columns by default. Set
`classes = "data.frame"` to avoid this behavior and apply the `f` and
`condition` functions to complete data.frame objects instead of
individual data.frame columns.

``` r
## create a nested list of data.frames
oceania_df <- rrapply(
  renewable_oceania,
  condition = \(x, .xpos) length(.xpos) == 2,
  f = \(x) data.frame(name = names(x), value = unlist(x)),
  classes = "list",
  how = "replace"
)

## this does not work!
tryCatch({
  rrapply(
    oceania_df,
    f = function(x) subset(x, !is.na(value)), ## filter NA-rows of data.frame
    how = "replace"
  )
}, error = function(error) error$message)
#> [1] "object 'value' not found"

## this does work
rrapply(
  oceania_df,
  f = function(x) subset(x, !is.na(value)),
  classes = "data.frame",
  how = "replace"
)[[1]][1:2]
#> $`Australia and New Zealand`
#>                    name value
#> Australia     Australia  9.32
#> New Zealand New Zealand 32.76
#> 
#> $Melanesia
#>                              name value
#> Fiji                         Fiji 24.36
#> New Caledonia       New Caledonia  4.03
#> Papua New Guinea Papua New Guinea 50.34
#> Solomon Islands   Solomon Islands 65.73
#> Vanuatu                   Vanuatu 33.67
```

### Recursive list updating

#### `how = "recurse"`

If `classes = "list"` and `how = "recurse"`,
[`rrapply()`](https://jorischau.github.io/rrapply/reference/rrapply.md)
applies the `f` function to any list element that satisfies the
`condition` argument, but recurses further into any *updated* list
element after application of `f`. This can be useful to e.g. recursively
update the class or other attributes of all elements in a nested list:

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

#### `how = "names"`

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

## Conclusion

To conclude, we return to the list recursion exercise in the first
section. Using
[`rrapply()`](https://jorischau.github.io/rrapply/reference/rrapply.md),
one possible solution is to split the question into two steps as
follows:

``` r
## look up position of Euler (Leonhard)
euler <- rrapply(
  students,
  condition = \(x, .xname) .xname == "Euler" && attr(x, "given") == "Leonhard",
  f = \(x, .xpos) .xpos,
  classes = "list",
  how = "flatten"
)[["Euler"]]

## filter descendants of Euler (Leonhard) and replace missing values by zero
rrapply(
  students,
  condition = \(x, .xpos) identical(.xpos[seq_along(euler)], euler), 
  f = \(x) replace(x, is.na(x), 0),
  how = "prune"
) |>
  str(give.attr = FALSE)
#> List of 1
#>  $ Bernoulli:List of 1
#>   ..$ Bernoulli:List of 1
#>   .. ..$ Euler:List of 2
#>   .. .. ..$ Euler   : num 0
#>   .. .. ..$ Lagrange:List of 3
#>   .. .. .. ..$ Fourier: num 73788
#>   .. .. .. ..$ Plana  : num 0
#>   .. .. .. ..$ Poisson: num 128235
```

Knowing that Johann Euler is a descendant of Leonhard Euler, we can
further simplify this into a single function call using the `.xparents`
argument:

``` r
## filter descendants of Euler (Leonhard) and replace missing values by zero
rrapply(
  students,
  condition = \(x, .xparents) "Euler" %in% .xparents,
  f = \(x) replace(x, is.na(x), 0),
  how = "prune"
) |>
  str(give.attr = FALSE)
#> List of 1
#>  $ Bernoulli:List of 1
#>   ..$ Bernoulli:List of 1
#>   .. ..$ Euler:List of 2
#>   .. .. ..$ Euler   : num 0
#>   .. .. ..$ Lagrange:List of 3
#>   .. .. .. ..$ Fourier: num 73788
#>   .. .. .. ..$ Plana  : num 0
#>   .. .. .. ..$ Poisson: num 128235
```

Or alternatively, we could first update the names of the elements in the
nested list to include both first and last names and then prune the list
in a second step:

``` r
## include first names in list element names
students_fullnames <- rrapply(
  students, 
  f = \(x, .xname) paste(attr(x, "given"), .xname),
  how = "names"
)

## filter descendants of Euler (Leonhard) and replace missing values by zero
rrapply(
  students_fullnames,
  condition = \(x, .xparents) "Leonhard Euler" %in% .xparents,
  f = \(x) replace(x, is.na(x), 0),
  how = "prune"
) |>
  str(give.attr = FALSE)
#> List of 1
#>  $ Jacob Bernoulli:List of 1
#>   ..$ Johann Bernoulli:List of 1
#>   .. ..$ Leonhard Euler:List of 2
#>   .. .. ..$ Johann Euler   : num 0
#>   .. .. ..$ Joseph Lagrange:List of 3
#>   .. .. .. ..$ Jean-Baptiste Fourier: num 73788
#>   .. .. .. ..$ Giovanni Plana       : num 0
#>   .. .. .. ..$ Simeon Poisson       : num 128235
```
