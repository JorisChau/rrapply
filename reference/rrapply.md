# Reimplementation of base-R's rapply

`rrapply` is a reimplemented and extended version of
[`rapply`](https://rdrr.io/r/base/rapply.html) to recursively apply a
function `f` to a set of elements of a list and deciding *how* the
result is structured.

## Usage

``` r
rrapply(
  object,
  condition,
  f,
  classes = "ANY",
  deflt = NULL,
  how = c("replace", "list", "unlist", "prune", "flatten", "melt", "bind", "recurse",
    "unmelt", "names"),
  options,
  ...
)
```

## Arguments

- object:

  a [`list`](https://rdrr.io/r/base/list.html),
  [`expression`](https://rdrr.io/r/base/expression.html) vector, or
  [`call`](https://rdrr.io/r/base/call.html) object, i.e., “list-like”.

- condition:

  a condition [`function`](https://rdrr.io/r/base/function.html) of one
  “principal” argument and optional special arguments `.xname`, `.xpos`,
  `.xparents` and/or `.xsiblings` (see ‘Details’), passing further
  arguments via `...`.

- f:

  a [`function`](https://rdrr.io/r/base/function.html) of one
  “principal” argument and optional special arguments `.xname`, `.xpos`,
  `.xparents` and/or `.xsiblings` (see ‘Details’), passing further
  arguments via `...`.

- classes:

  character vector of [`class`](https://rdrr.io/r/base/class.html)
  names, or `"ANY"` to match the class of any terminal node. Include
  `"list"` or `"data.frame"` to match the class of non-terminal nodes as
  well.

- deflt:

  the default result (only used if `how = "list"` or `how = "unlist"`).

- how:

  character string partially matching the ten possibilities given: see
  ‘Details’.

- options:

  a named [`list`](https://rdrr.io/r/base/list.html) with additional
  options `namesep`, `simplify`, `namecols` and/or `coldepth` that only
  apply to certain choices of `how`: see ‘Details’.

- ...:

  additional arguments passed to the call to `f` and `condition`.

## Value

If `how = "unlist"`, a vector as in
[`rapply`](https://rdrr.io/r/base/rapply.html). If `how = "list"`,
`how = "replace"`, `how = "recurse"` or `how = "names"`, “list-like” of
similar structure as `object` as in
[`rapply`](https://rdrr.io/r/base/rapply.html). If `how = "prune"`, a
pruned “list-like” object of similar structure as `object` with pruned
list elements based on `classes` and `condition`. If `how = "flatten"`,
a flattened pruned vector or list with pruned elements based on
`classes` and `condition`. If `how = "melt"`, a melted data.frame
containing the node paths and values of the pruned list elements based
on `classes` and `condition`. If `how = "bind"`, a wide data.frame with
repeated list elements expanded as single data.frame rows and aligned by
identical list names using the same coercion rules as `how = "unlist"`.
The repeated list elements are subject to pruning based on `classes` and
`condition`. If `how = "unmelt"`, a nested list with list names and
values defined in the data.frame `object`.

## Note

`rrapply` allows the `f` function argument to be missing, in which case
no function is applied to the list elements.

`how = "unmelt"` requires as input a data.frame as returned by
`how = "melt"` with character columns to name the nested list components
and a final list- or vector-column containing the values of the nested
list elements.

## How to structure result

In addition to [`rapply`](https://rdrr.io/r/base/rapply.html)'s modes to
set `how` equal to `"replace"`, `"list"` or `"unlist"`, seven choices
`"prune"`, `"flatten"`, `"melt"`, `"bind"`, `"unmelt"`, `"recurse"` and
`"names"` are available:

- `how = "prune"` filters all list elements not subject to application
  of `f` from the list `object`. The original list structure is
  retained, similar to the non-pruned options `how = "replace"` or
  `how = "list"`.

- `how = "flatten"` is an efficient way to return a flattened unnested
  version of the pruned list. By default `how = "flatten"` uses similar
  coercion rules as `how = "unlist"`, this can be disabled with
  `simplify = FALSE` in the `options` argument.

- `how = "melt"` returns a melted data.frame of the pruned list, each
  row contains the path of a single terminal node in the pruned list at
  depth layers `L1`, `L2`, and so on. The column `"value"` contains the
  possibly coerced values at the terminal nodes and is equivalent to the
  result of `how = "flatten"`. If no list names are present, the node
  names in the data.frame default to the indices of the list elements
  `"1"`, `"2"`, etc.

- `how = "bind"` is used to unnest a nested list containing repeated
  sublists into a wide data.frame. Each repeated sublist is expanded as
  a single row in the data.frame and identical sublist component names
  are aligned as individual columns. By default, the list layer
  containing repeated sublists is identified based on the minimal depth
  detected across leaf nodes, this can be set manually with `coldepth`
  in the `options` argument.

- `how = "unmelt"` is a special case that reconstructs a nested list
  from a melted data.frame. For this reason, `how = "unmelt"` only
  applies to data.frames in the same format as returned by
  `how = "melt"`. Internally, `how = "unmelt"` first reconstructs a
  nested list from the melted data.frame and second uses the same
  functional framework as `how = "replace"`.

- `how = "recurse"` is a specialized option that is only useful in
  combination with e.g. `classes = "list"` to recurse further into
  updated “list-like” elements. This is explained in more detail below.

- `how = "names"` modifies the *names* of the nested list elements
  instead of the list content. `how = "names"` internally works similar
  to `how = "list"`, except that the value of `f` is used to replace the
  name of the list element under evaluation instead of its content.

## Condition function

Both [`rapply`](https://rdrr.io/r/base/rapply.html) and `rrapply` allow
to apply `f` to list elements of certain classes via the `classes`
argument. `rrapply` generalizes this concept via an additional
`condition` argument, which accepts any function to use as a condition
or predicate to select list elements to which `f` is applied.
Conceptually, the `f` function is applied to all list elements for which
the `condition` function exactly evaluates to `TRUE` similar to
[`isTRUE`](https://rdrr.io/r/base/Logic.html). If the condition function
is missing, `f` is applied to all list elements. Since the `condition`
function generalizes the `classes` argument, it is allowed to use the
`deflt` argument together with `how = "list"` or `how = "unlist"` to set
a default value to all list elements for which the `condition` does not
evaluate to `TRUE`.

## Correct use of `...`

The principal argument of the `f` and `condition` functions evaluates to
the content of the list element. Any further arguments to `f` and
`condition` (besides the special arguments `.xname`, `.xpos`, etc.
discussed below) supplied via the dots `...` argument need to be defined
as function arguments in *both* the `f` and `condition` function (if
existing), even if they are not used in the function itself. See also
the ‘Examples’ section.

## Special arguments `.xname`, `.xpos`, `.xparents` and `.xsiblings`

The `f` and `condition` functions accept four special arguments
`.xname`, `.xpos`, `.xparents` and `.xsiblings` in addition to the first
principal argument. The `.xname` argument evaluates to the name of the
list element. The `.xpos` argument evaluates to the position of the
element in the nested list structured as an integer vector. That is, if
`x = list(list("y", "z"))`, then an `.xpos` location of `c(1, 2)`
corresponds to the list element `x[[c(1, 2)]]`. The `.xparents` argument
evaluates to a vector of all parent node names in the path to the list
element. The `.xsiblings` argument evaluates to the complete (sub)list
that includes the list element as a direct child. The names `.xname`,
`.xpos`, `.xparents` or `.xsiblings` need to be explicitly included as
function arguments in `f` and `condition` (in addition to the principal
argument). See also the ‘Examples’ section.

## Avoid recursing into list nodes

By default, `rrapply` recurses into any “list-like” element. If
`classes = "list"`, this behavior is overridden and the `f` function is
also applied to any list element of `object` that satisfies `condition`.
For expression objects, use `classes = "language"`,
`classes = "expression"` or `classes = "pairlist"` to avoid recursing
into branches of the abstract syntax tree of `object`. If the
`condition` or `classes` arguments are not satisfied for a “list-like”
element, `rrapply` will recurse further into the sublist, apply the `f`
function to the nodes that satisfy `condition` and `classes`, and so on.
Note that this behavior can only be triggered using the `classes`
argument and not the `condition` argument.

## Recursive list node updating

If `classes = "list"` and `how = "recurse"`, `rrapply` applies the `f`
function to any list element of `object` that satisfies `condition`
similar to the previous section using `how = "replace"`, but recurses
further into the *updated* list-like element after application of the
`f` function. A primary use of `how = "recurse"` in combination with
`classes = "list"` is to recursively update for instance the class or
other attributes of all nodes in a nested list.

## Avoid recursing into data.frames

If `classes = "ANY"` (default), `rrapply` recurses into all “list-like”
objects equivalent to [`rapply`](https://rdrr.io/r/base/rapply.html).
Since data.frames are “list-like” objects, the `f` function will descend
into the individual columns of a data.frame. To avoid this behavior, set
`classes = "data.frame"`, in which case the `f` and `condition`
functions are applied directly to the data.frame and not its columns.
Note that this behavior can only be triggered using the `classes`
argument and not the `condition` argument.

## List attributes

In [`rapply`](https://rdrr.io/r/base/rapply.html) intermediate list
attributes (not located at terminal nodes) are kept when
`how = "replace"`, but are dropped when `how = "list"`. To avoid
unexpected behavior, `rrapply` always preserves intermediate list
attributes when using `how = "replace"`, `how = "list"`, `how = "prune"`
or `how = "names"`. If `how = "unlist"`, `how = "flatten"`,
`how = "melt"` or `how = "bind"` intermediate list attributes cannot be
preserved as the result is no longer a nested list.

## Expressions

Call objects and expression vectors are also accepted as `object`
argument, which are treated as nested lists based on their internal
abstract syntax trees. As such, all functionality that applies to nested
lists extends directly to call objects and expression vectors. If
`object` is a call object or expression vector, `how = "replace"` always
maintains the type of `object`, whereas `how = "list"` returns the
result structured as a nested list. `how = "prune"`, `how = "flatten"`
and `how = "melt"` return the pruned abstract syntax tree as: a nested
list, a flattened list and a melted data.frame respectively. This is
identical to application of `rrapply` to the abstract syntax tree
formatted as a nested list.

## Additional options

The `options` argument accepts a named list to configure several default
options that only apply to certain choices of `how`. The `options` list
can contain (any of) the named components `namesep`, `simplify`,
`namecols` and/or `coldepth`:

- `namesep`, a character separator used to combine parent and child list
  names in `how = "flatten"` and `how = "bind"`. If `namesep = NA`
  (default), no parent names are included in `how = "flatten"` and the
  default separator `"."` is used in `how = "bind"`. Note that `namesep`
  cannot be used with `how = "unlist"` for which the name separator
  always defaults to `"."`.

- `simplify`, a logical value indicating whether the flattened unnested
  list in `how = "flatten"` and `how = "melt"` is simplified according
  to standard coercion rules similar to `how = "unlist"`. The default is
  `simplify = TRUE`. If `simplify = FALSE`, `object` is flattened to a
  single-layer list and returned as is.

- `namecols`, a logical value that only applies to `how = "bind"`
  indicating whether the parent node names associated to the each
  expanded sublist should be included as columns `L1`, `L2`, etc. in the
  wide data.frame returned by `how = "bind"`.

- `coldepth`, an integer value indicating the depth (starting from
  depth 1) at which list elements should be mapped to individual columns
  in the wide data.frame returned by `how = "bind"`. If `coldepth = 0`
  (default), this depth layer is identified automatically based on the
  minimal depth detected across all leaf nodes. This option only applies
  to `how = "bind"`.

## See also

[`rapply`](https://rdrr.io/r/base/rapply.html)

## Examples

``` r
# Example data

## Renewable energy shares per country (% of total consumption) in 2016
data("renewable_energy_by_country")

## Renewable energy shares in Oceania
renewable_oceania <- renewable_energy_by_country[["World"]]["Oceania"]

## Pokemon properties in Pokemon GO
data("pokedex")

# List pruning and unnesting

## Drop logical NA's while preserving list structure 
na_drop_oceania <- rrapply(
  renewable_oceania,
  f = function(x) x,
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

## Drop logical NA's and return unnested list
na_drop_oceania2 <- rrapply(
  renewable_oceania,
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

## Flatten to simple list with full names
na_drop_oceania3 <- rrapply(
  renewable_oceania,
  classes = "numeric",
  how = "flatten",
  options = list(namesep = ".", simplify = FALSE)
) 
str(na_drop_oceania3, list.len = 10, give.attr = FALSE)
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

## Drop logical NA's and return melted data.frame
na_drop_oceania4 <- rrapply(
  renewable_oceania,
  classes = "numeric",
  how = "melt"
)
head(na_drop_oceania4)
#>        L1                        L2               L3 value
#> 1 Oceania Australia and New Zealand        Australia  9.32
#> 2 Oceania Australia and New Zealand      New Zealand 32.76
#> 3 Oceania                 Melanesia             Fiji 24.36
#> 4 Oceania                 Melanesia    New Caledonia  4.03
#> 5 Oceania                 Melanesia Papua New Guinea 50.34
#> 6 Oceania                 Melanesia  Solomon Islands 65.73

## Reconstruct nested list from melted data.frame
na_drop_oceania5 <- rrapply(
  na_drop_oceania4,
  how = "unmelt"
)
str(na_drop_oceania5, list.len = 3, give.attr = FALSE)
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

## Unnest list to wide data.frame
pokedex_wide <- rrapply(pokedex, how = "bind")
head(pokedex_wide)
#>   id num       name                                              img
#> 1  1 001  Bulbasaur http://www.serebii.net/pokemongo/pokemon/001.png
#> 2  2 002    Ivysaur http://www.serebii.net/pokemongo/pokemon/002.png
#> 3  3 003   Venusaur http://www.serebii.net/pokemongo/pokemon/003.png
#> 4  4 004 Charmander http://www.serebii.net/pokemongo/pokemon/004.png
#> 5  5 005 Charmeleon http://www.serebii.net/pokemongo/pokemon/005.png
#> 6  6 006  Charizard http://www.serebii.net/pokemongo/pokemon/006.png
#>            type height   weight            candy candy_count         egg
#> 1 Grass, Poison 0.71 m   6.9 kg  Bulbasaur Candy          25        2 km
#> 2 Grass, Poison 0.99 m  13.0 kg  Bulbasaur Candy         100 Not in Eggs
#> 3 Grass, Poison 2.01 m 100.0 kg  Bulbasaur Candy          NA Not in Eggs
#> 4          Fire 0.61 m   8.5 kg Charmander Candy          25        2 km
#> 5          Fire 1.09 m  19.0 kg Charmander Candy         100 Not in Eggs
#> 6  Fire, Flying 1.70 m  90.5 kg Charmander Candy          NA Not in Eggs
#>   spawn_chance avg_spawns spawn_time multipliers                 weaknesses
#> 1       0.6900      69.00      20:00        1.58 Fire, Ice, Flying, Psychic
#> 2       0.0420       4.20      07:00    1.2, 1.6 Fire, Ice, Flying, Psychic
#> 3       0.0170       1.70      11:30          NA Fire, Ice, Flying, Psychic
#> 4       0.2530      25.30      08:45        1.65        Water, Ground, Rock
#> 5       0.0120       1.20      19:00        1.79        Water, Ground, Rock
#> 6       0.0031       0.31      13:34          NA      Water, Electric, Rock
#>   next_evolution.1.num next_evolution.1.name next_evolution.2.num
#> 1                  002               Ivysaur                  003
#> 2                  003              Venusaur                 <NA>
#> 3                 <NA>                  <NA>                 <NA>
#> 4                  005            Charmeleon                  006
#> 5                  006             Charizard                 <NA>
#> 6                 <NA>                  <NA>                 <NA>
#>   next_evolution.2.name prev_evolution.1.num prev_evolution.1.name
#> 1              Venusaur                 <NA>                  <NA>
#> 2                  <NA>                  001             Bulbasaur
#> 3                  <NA>                  001             Bulbasaur
#> 4             Charizard                 <NA>                  <NA>
#> 5                  <NA>                  004            Charmander
#> 6                  <NA>                  004            Charmander
#>   prev_evolution.2.num prev_evolution.2.name next_evolution.3.num
#> 1                 <NA>                  <NA>                 <NA>
#> 2                 <NA>                  <NA>                 <NA>
#> 3                  002               Ivysaur                 <NA>
#> 4                 <NA>                  <NA>                 <NA>
#> 5                 <NA>                  <NA>                 <NA>
#> 6                  005            Charmeleon                 <NA>
#>   next_evolution.3.name
#> 1                  <NA>
#> 2                  <NA>
#> 3                  <NA>
#> 4                  <NA>
#> 5                  <NA>
#> 6                  <NA>

## Unnest to data.frame including parent columns
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

# Condition function

## Drop all NA elements using condition function
na_drop_oceania6 <- rrapply(
  renewable_oceania,
  condition = Negate(is.na),
  how = "prune"
)
str(na_drop_oceania6, list.len = 3, give.attr = FALSE)
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

## Replace NA elements by a new value via the ... argument
## NB: the 'newvalue' argument should be present as function 
## argument in both 'f' and 'condition', even if unused.
na_zero_oceania <- rrapply(
  renewable_oceania,
  condition = function(x, newvalue) is.na(x),
  f = function(x, newvalue) newvalue,
  newvalue = 0,
  how = "replace"
)
str(na_zero_oceania, list.len = 3, give.attr = FALSE)
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

# Special arguments .xname, .xpos, .xparents and .xsiblings

## Apply a function using the name of the node
renewable_oceania_text <- rrapply(
  renewable_oceania,
  condition = Negate(is.na),
  f = function(x, .xname) sprintf("Renewable energy in %s: %.2f%%", .xname, x),
  how = "flatten"
)
head(renewable_oceania_text, n = 10)
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
#>                                Solomon Islands 
#>  "Renewable energy in Solomon Islands: 65.73%" 
#>                                        Vanuatu 
#>          "Renewable energy in Vanuatu: 33.67%" 
#>                                           Guam 
#>              "Renewable energy in Guam: 3.03%" 
#>                                       Kiribati 
#>         "Renewable energy in Kiribati: 45.43%" 
#>                               Marshall Islands 
#> "Renewable energy in Marshall Islands: 11.75%" 

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

## Filter European countries with value above 50%
renewable_europe_above_50 <- rrapply(
  renewable_energy_by_country,
  condition = function(x, .xpos) identical(.xpos[c(1, 2)], c(1L, 5L)) & x > 50,
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

## Filter European countries with value above 50%
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
#> 
renewable_energy_by_country[[xpos_sweden$Sweden]]
#> [1] 51.35
#> attr(,"M49-code")
#> [1] "752"

## Return neighbors of Sweden in list
siblings_sweden <- rrapply(
  renewable_energy_by_country,
  condition = function(x, .xsiblings) "Sweden" %in% names(.xsiblings),
  how = "flatten"
)
head(siblings_sweden, n = 10)
#> Aland Islands       Denmark       Estonia Faroe Islands       Finland 
#>            NA         33.06         26.55          4.24         42.03 
#>       Iceland       Ireland   Isle of Man        Latvia     Lithuania 
#>         78.07          8.65          4.30         38.48         31.42 

## Unnest selected columns in Pokedex list 
pokedex_small <- rrapply(
   pokedex,
   condition = function(x, .xpos, .xname) length(.xpos) < 4 & .xname %in% c("num", "name", "type"),
   how = "bind"
)  
head(pokedex_small)
#>   num       name          type
#> 1 001  Bulbasaur Grass, Poison
#> 2 002    Ivysaur Grass, Poison
#> 3 003   Venusaur Grass, Poison
#> 4 004 Charmander          Fire
#> 5 005 Charmeleon          Fire
#> 6 006  Charizard  Fire, Flying

# Modifying list elements

## Calculate mean value of Europe
rrapply(
  renewable_energy_by_country,  
  condition = function(x, .xname) .xname == "Europe",
  f = function(x) mean(unlist(x), na.rm = TRUE),
  classes = "list",
  how = "flatten"
)
#>   Europe 
#> 22.36565 

## Calculate mean value for each continent
## (Antarctica's value is missing)
renewable_continent_summary <- rrapply(
  renewable_energy_by_country,  
  condition = function(x, .xpos) length(.xpos) == 2,
  f = function(x) mean(unlist(x), na.rm = TRUE),
  classes = "list"
)
str(renewable_continent_summary, give.attr = FALSE)
#> List of 1
#>  $ World:List of 6
#>   ..$ Africa    : num 54.3
#>   ..$ Americas  : num 18.2
#>   ..$ Antarctica: logi NA
#>   ..$ Asia      : num 17.9
#>   ..$ Europe    : num 22.4
#>   ..$ Oceania   : num 17.8

## Filter country or region by M49-code
rrapply(
  renewable_energy_by_country,
  condition = function(x) attr(x, "M49-code") == "155",
  f = function(x, .xname) .xname,
  classes = c("list", "ANY"), 
  how = "unlist"
)
#> World.Europe.Western Europe 
#>            "Western Europe" 

# Recursive list updating

## Recursively remove list attributes
renewable_no_attrs <- rrapply(
  renewable_oceania,
  f = function(x) c(x),
  classes = c("list", "ANY"),
  how = "recurse"
) 
str(renewable_no_attrs, list.len = 3, give.attr = TRUE)
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

## recursively replace all names by M49-codes
renewable_m49_names <- rrapply(
  renewable_oceania,
  f = function(x) attr(x, "M49-code"),
  how = "names"
) 
str(renewable_m49_names, list.len = 3, give.attr = FALSE)
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

# List attributes

## how = "list" preserves all list attributes
na_drop_oceania_attr <- rrapply(
  renewable_oceania,
  f = function(x) replace(x, is.na(x), 0),
  how = "list"
)
str(na_drop_oceania_attr, max.level = 2)
#> List of 1
#>  $ Oceania:List of 4
#>   ..$ Australia and New Zealand:List of 6
#>   .. ..- attr(*, "M49-code")= chr "053"
#>   ..$ Melanesia                :List of 5
#>   .. ..- attr(*, "M49-code")= chr "054"
#>   ..$ Micronesia               :List of 8
#>   .. ..- attr(*, "M49-code")= chr "057"
#>   ..$ Polynesia                :List of 10
#>   .. ..- attr(*, "M49-code")= chr "061"
#>   ..- attr(*, "M49-code")= chr "009"

## how = "prune" also preserves list attributes
na_drop_oceania_attr2 <- rrapply(
  renewable_oceania,
  condition = Negate(is.na),
  how = "prune"
)
str(na_drop_oceania_attr2, max.level = 2)
#> List of 1
#>  $ Oceania:List of 4
#>   ..$ Australia and New Zealand:List of 2
#>   .. ..- attr(*, "M49-code")= chr "053"
#>   ..$ Melanesia                :List of 5
#>   .. ..- attr(*, "M49-code")= chr "054"
#>   ..$ Micronesia               :List of 7
#>   .. ..- attr(*, "M49-code")= chr "057"
#>   ..$ Polynesia                :List of 8
#>   .. ..- attr(*, "M49-code")= chr "061"
#>   ..- attr(*, "M49-code")= chr "009"

# Expressions

## Replace logicals by integers
call_old <- quote(y <- x <- 1 + TRUE)
call_new <- rrapply(call_old, 
  f = as.numeric, 
  how = "replace",
  classes = "logical"
)
str(call_new)
#>  language y <- x <- 1 + 1

## Update and decompose call object
call_ast <- rrapply(call_old, 
  f = function(x) ifelse(is.logical(x), as.numeric(x), x), 
  how = "list"
)
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

## Prune and decompose expression
expr <- expression(y <- x <- 1, f(g(2 * pi)))
is_new_name <- function(x) !exists(as.character(x), envir = baseenv())
expr_prune <- rrapply(expr, 
  classes = "name", 
  condition = is_new_name, 
  how = "prune"
)
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
expr_flatten <- rrapply(expr, 
  classes = "name", 
  condition = is_new_name, 
  how = "flatten"
)
str(expr_flatten)
#> List of 4
#>  $ : symbol y
#>  $ : symbol x
#>  $ : symbol f
#>  $ : symbol g

## Prune and melt expression
rrapply(expr, 
  classes = "name", 
  condition = is_new_name, 
  f = as.character,
  how = "melt"
)
#>   L1 L2   L3 value
#> 1  1  2 <NA>     y
#> 2  1  3    2     x
#> 3  2  1 <NA>     f
#> 4  2  2    1     g

## Avoid recursing into call objects
rrapply(
  expr, 
  classes = "language", 
  condition = function(x) !any(sapply(x, is.call)),
  how = "flatten"
)
#> [[1]]
#> x <- 1
#> 
#> [[2]]
#> 2 * pi
#> 
```
