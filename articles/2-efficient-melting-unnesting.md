# Efficient list melting and unnesting with rrapply

## Introduction

The previous article `vignette("1-when-to-use-rrapply")` describes the
principal use of the
[`rrapply()`](https://jorischau.github.io/rrapply/reference/rrapply.md)
function as a revised and extended version of base
[`rapply()`](https://rdrr.io/r/base/rapply.html) in the context of
nested list recursion. For quick data exploration of a nested list it
can make sense to keep the list in its original nested format to reduce
the number of processing steps and minimize code complexity. As part of
a more elaborate data analysis, if there is no specific reason to keep
the nested data structure, it is often more practical to transform the
nested list into a more convenient rectangular format and work with the
unnested object (e.g. a data.frame) instead. In this follow-up article,
we review the available (`how`) options in
[`rrapply()`](https://jorischau.github.io/rrapply/reference/rrapply.md)
to unnest or melt nested lists into a rectangular format in more detail
and highlight the similarities and differences with respect to several
common alternatives in R.

## Nested list to data.frame

### Melt to long data.frame

The option `how = "melt"` in
[`rrapply()`](https://jorischau.github.io/rrapply/reference/rrapply.md)
unnests a nested list to a *long* or melted data.frame similar in format
to the retired
[`reshape2::melt()`](https://rdrr.io/pkg/reshape2/man/melt.html)
function applied to a nested list. The rows of the melted data.frame
contain the individual node paths of the elements in the nested list
after pruning (based on the `condition` and/or `classes` arguments). The
`"value"` column is a vector- or list-column containing the values of
the leaf elements identical to the object returned by `how = "flatten"`.

To demonstrate, we use the `renewable_energy_by_country` dataset
included in the `rrapply`-package, a nested list containing the
renewable energy shares per country (% of total energy consumption) in
2016[¹](#fn1). The 249 countries and areas are structured based on their
geographical locations according to the [United Nations M49
standard](https://unstats.un.org/unsd/methodology/m49/). The numeric
values listed for each country are percentages, if no data is available
the value of the country is `NA`.

``` r
library(rrapply)

## melt all data to long data.frame
rrapply(
  renewable_energy_by_country, 
  how = "melt"
) |>
  head(n = 10)
#>       L1     L2                 L3             L4                             L5 value
#> 1  World Africa    Northern Africa        Algeria                           <NA>  0.08
#> 2  World Africa    Northern Africa          Egypt                           <NA>  5.69
#> 3  World Africa    Northern Africa          Libya                           <NA>  1.64
#> 4  World Africa    Northern Africa        Morocco                           <NA> 11.02
#> 5  World Africa    Northern Africa          Sudan                           <NA> 61.64
#> 6  World Africa    Northern Africa        Tunisia                           <NA> 12.47
#> 7  World Africa    Northern Africa Western Sahara                           <NA>    NA
#> 8  World Africa Sub-Saharan Africa Eastern Africa British Indian Ocean Territory    NA
#> 9  World Africa Sub-Saharan Africa Eastern Africa                        Burundi 89.22
#> 10 World Africa Sub-Saharan Africa Eastern Africa                        Comoros 41.92
```

``` r
## drop logical NA's and melt to data.frame
rrapply(
  renewable_energy_by_country,
  classes = "numeric",
  how = "melt"
) |>
  head(n = 10)
#>       L1     L2                 L3             L4       L5 value
#> 1  World Africa    Northern Africa        Algeria     <NA>  0.08
#> 2  World Africa    Northern Africa          Egypt     <NA>  5.69
#> 3  World Africa    Northern Africa          Libya     <NA>  1.64
#> 4  World Africa    Northern Africa        Morocco     <NA> 11.02
#> 5  World Africa    Northern Africa          Sudan     <NA> 61.64
#> 6  World Africa    Northern Africa        Tunisia     <NA> 12.47
#> 7  World Africa Sub-Saharan Africa Eastern Africa  Burundi 89.22
#> 8  World Africa Sub-Saharan Africa Eastern Africa  Comoros 41.92
#> 9  World Africa Sub-Saharan Africa Eastern Africa Djibouti 28.50
#> 10 World Africa Sub-Saharan Africa Eastern Africa  Eritrea 80.14
```

``` r
## apply condition and melt to data.frame
rrapply(
  renewable_energy_by_country,
  condition = \(x, .xparents) "Western Europe" %in% .xparents,
  how = "melt"
) |>
  head(n = 10)
#>      L1     L2             L3            L4 value
#> 1 World Europe Western Europe       Austria 34.67
#> 2 World Europe Western Europe       Belgium  9.14
#> 3 World Europe Western Europe        France 14.74
#> 4 World Europe Western Europe       Germany 14.17
#> 5 World Europe Western Europe Liechtenstein 62.93
#> 6 World Europe Western Europe    Luxembourg 13.54
#> 7 World Europe Western Europe        Monaco    NA
#> 8 World Europe Western Europe   Netherlands  5.78
#> 9 World Europe Western Europe   Switzerland 25.49
```

As shown in the above examples, in comparison to
[`reshape2::melt()`](https://rdrr.io/pkg/reshape2/man/melt.html),
[`rrapply()`](https://jorischau.github.io/rrapply/reference/rrapply.md)
allows to filter or transform list elements before melting the nested
list through the `f`, `classes` and `condition` arguments[²](#fn2). More
importantly,
[`rrapply()`](https://jorischau.github.io/rrapply/reference/rrapply.md)
is optimized specifically for handling nested lists, whereas
[`reshape2::melt()`](https://rdrr.io/pkg/reshape2/man/melt.html) was
aimed primarily at melting data.frames before being superseded by
[`tidyr::gather()`](https://tidyr.tidyverse.org/reference/gather.html)
and more recently
[`tidyr::pivot_longer()`](https://tidyr.tidyverse.org/reference/pivot_longer.html).
For this reason,
[`reshape2::melt()`](https://rdrr.io/pkg/reshape2/man/melt.html) can be
quite slow when applied to large nested lists:

``` r
## melt to long data.frame (reshape2)
reshape2::melt(renewable_energy_by_country) |>
  head(10)
#>    value             L4                             L5                 L3     L2    L1
#> 1   0.08        Algeria                           <NA>    Northern Africa Africa World
#> 2   5.69          Egypt                           <NA>    Northern Africa Africa World
#> 3   1.64          Libya                           <NA>    Northern Africa Africa World
#> 4  11.02        Morocco                           <NA>    Northern Africa Africa World
#> 5  61.64          Sudan                           <NA>    Northern Africa Africa World
#> 6  12.47        Tunisia                           <NA>    Northern Africa Africa World
#> 7     NA Western Sahara                           <NA>    Northern Africa Africa World
#> 8     NA Eastern Africa British Indian Ocean Territory Sub-Saharan Africa Africa World
#> 9  89.22 Eastern Africa                        Burundi Sub-Saharan Africa Africa World
#> 10 41.92 Eastern Africa                        Comoros Sub-Saharan Africa Africa World

## computation times
bench::mark(
  rrapply(renewable_energy_by_country),
  reshape2::melt(renewable_energy_by_country),
  check = FALSE
)
#> # A tibble: 2 × 6
#>   expression                                       min   median `itr/sec` mem_alloc `gc/sec`
#>   <bch:expr>                                  <bch:tm> <bch:tm>     <dbl> <bch:byt>    <dbl>
#> 1 rrapply(renewable_energy_by_country)          10.6µs   11.7µs   82890.         0B     41.5
#> 2 reshape2::melt(renewable_energy_by_country)   38.4ms   38.6ms      25.9      74KB    129.
```

For a medium-sized list as used in this example, the computation time of
[`reshape2::melt()`](https://rdrr.io/pkg/reshape2/man/melt.html) is not
a bottleneck for practical usage. However, the computational effort
quickly increases when melting larger or more deeply nested lists:

``` r
## helper function to generate large nested list
new_list <- function(n, d) {
  v <- vector(mode = "list", length = n)
  rrapply(
    object = v,
    classes = c("list", "NULL"),
    condition = \(x, .xpos) length(.xpos) <= d,
    f = \(x, .xpos) if(length(.xpos) < d) v else runif(1),
    how = "recurse"
  )
}

## random seed
set.seed(1)

## generate large shallow list (10^6 elements)
shallow_list <- new_list(n = 100, d = 3)
str(shallow_list, list.len = 2)
#> List of 100
#>  $ :List of 100
#>   ..$ :List of 100
#>   .. ..$ : num 0.266
#>   .. ..$ : num 0.372
#>   .. .. [list output truncated]
#>   ..$ :List of 100
#>   .. ..$ : num 0.655
#>   .. ..$ : num 0.353
#>   .. .. [list output truncated]
#>   .. [list output truncated]
#>  $ :List of 100
#>   ..$ :List of 100
#>   .. ..$ : num 0.0647
#>   .. ..$ : num 0.677
#>   .. .. [list output truncated]
#>   ..$ :List of 100
#>   .. ..$ : num 0.266
#>   .. ..$ : num 0.66
#>   .. .. [list output truncated]
#>   .. [list output truncated]
#>   [list output truncated]

## benchmark timing with rrapply
system.time(shallow_melt <- rrapply(shallow_list, how = "melt")) 
#>    user  system elapsed 
#>   0.760   0.036   0.796
head(shallow_melt)
#>   L1 L2 L3     value
#> 1  1  1  1 0.2655087
#> 2  1  1  2 0.3721239
#> 3  1  1  3 0.5728534
#> 4  1  1  4 0.9082078
#> 5  1  1  5 0.2016819
#> 6  1  1  6 0.8983897

## benchmark timing with reshape2::melt
system.time(shallow_melt_reshape2 <- reshape2::melt(shallow_list))
#>    user  system elapsed 
#> 128.458   0.063 128.537
head(shallow_melt_reshape2)
#>       value L3 L2 L1
#> 1 0.2655087  1  1  1
#> 2 0.3721239  2  1  1
#> 3 0.5728534  3  1  1
#> 4 0.9082078  4  1  1
#> 5 0.2016819  5  1  1
#> 6 0.8983897  6  1  1
```

``` r
## generate large deeply nested list (2^18 elements)
deep_list <- new_list(n = 2, d = 18)

## benchmark timing with rrapply
system.time(deep_melt <- rrapply(deep_list, how = "melt")) 
#>    user  system elapsed 
#>   0.468   0.027   0.495
head(deep_melt)
#>   L1 L2 L3 L4 L5 L6 L7 L8 L9 L10 L11 L12 L13 L14 L15 L16 L17 L18      value
#> 1  1  1  1  1  1  1  1  1  1   1   1   1   1   1   1   1   1   1 0.14011775
#> 2  1  1  1  1  1  1  1  1  1   1   1   1   1   1   1   1   1   2 0.69562066
#> 3  1  1  1  1  1  1  1  1  1   1   1   1   1   1   1   1   2   1 0.72888445
#> 4  1  1  1  1  1  1  1  1  1   1   1   1   1   1   1   1   2   2 0.09164734
#> 5  1  1  1  1  1  1  1  1  1   1   1   1   1   1   1   2   1   1 0.06661200
#> 6  1  1  1  1  1  1  1  1  1   1   1   1   1   1   1   2   1   2 0.61285721

## benchmark timing with reshape2::melt
system.time(deep_melt_reshape2 <- reshape2::melt(deep_list))
#>    user  system elapsed 
#>  95.664   0.023  95.711
head(deep_melt_reshape2)
#>        value L18 L17 L16 L15 L14 L13 L12 L11 L10 L9 L8 L7 L6 L5 L4 L3 L2 L1
#> 1 0.14011775   1   1   1   1   1   1   1   1   1  1  1  1  1  1  1  1  1  1
#> 2 0.69562066   2   1   1   1   1   1   1   1   1  1  1  1  1  1  1  1  1  1
#> 3 0.72888445   1   2   1   1   1   1   1   1   1  1  1  1  1  1  1  1  1  1
#> 4 0.09164734   2   2   1   1   1   1   1   1   1  1  1  1  1  1  1  1  1  1
#> 5 0.06661200   1   1   2   1   1   1   1   1   1  1  1  1  1  1  1  1  1  1
#> 6 0.61285721   2   1   2   1   1   1   1   1   1  1  1  1  1  1  1  1  1  1
```

Although unlikely to encounter such large or deeply nested lists in
practice, these artificial examples serve to illustrate that
[`reshape2::melt()`](https://rdrr.io/pkg/reshape2/man/melt.html) is not
particularly efficient in unnesting large nested lists to data.frames.

### Bind to wide data.frame

The option `how = "bind"` unnests a nested list to a *wide* data.frame
and is used to unnest nested lists containing repeated entries of the
same variables. To illustrate, we consider the `pokedex` dataset
included in the `rrapply`-package, a nested list containing various
property values for each of the 151 original Pokémon available (in
.json) from <https://github.com/Biuni/PokemonGO-Pokedex>.

``` r
## all 151 Pokemon
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

## single Pokemon entry
str(pokedex[["pokemon"]][[1]])
#> List of 16
#>  $ id            : int 1
#>  $ num           : chr "001"
#>  $ name          : chr "Bulbasaur"
#>  $ img           : chr "http://www.serebii.net/pokemongo/pokemon/001.png"
#>  $ type          : chr [1:2] "Grass" "Poison"
#>  $ height        : chr "0.71 m"
#>  $ weight        : chr "6.9 kg"
#>  $ candy         : chr "Bulbasaur Candy"
#>  $ candy_count   : int 25
#>  $ egg           : chr "2 km"
#>  $ spawn_chance  : num 0.69
#>  $ avg_spawns    : int 69
#>  $ spawn_time    : chr "20:00"
#>  $ multipliers   : num 1.58
#>  $ weaknesses    : chr [1:4] "Fire" "Ice" "Flying" "Psychic"
#>  $ next_evolution:List of 2
#>   ..$ :List of 2
#>   .. ..$ num : chr "002"
#>   .. ..$ name: chr "Ivysaur"
#>   ..$ :List of 2
#>   .. ..$ num : chr "003"
#>   .. ..$ name: chr "Venusaur"
```

Calling
[`rrapply()`](https://jorischau.github.io/rrapply/reference/rrapply.md)
with `how = "bind` expands each Pokémon sublist as a single row in a
wide data.frame. The 151 rows are stacked and aligned by matching
variable names, with missing entries replaced by `NA`’s (similar to
`data.table::rbindlist(..., fill = TRUE)`). Note that any nested
variables, such as `next_evolution` and `prev_evolution`, are unnested
as wide as possible into individual data.frame columns similar to
repeated application of
[`tidyr::unnest_wider()`](https://tidyr.tidyverse.org/reference/unnest_wider.html)
to a data.frame with nested list-columns.

``` r
rrapply(pokedex, how = "bind")[, 1:9] |>
  head()
#>   id num       name                                              img          type height   weight            candy candy_count
#> 1  1 001  Bulbasaur http://www.serebii.net/pokemongo/pokemon/001.png Grass, Poison 0.71 m   6.9 kg  Bulbasaur Candy          25
#> 2  2 002    Ivysaur http://www.serebii.net/pokemongo/pokemon/002.png Grass, Poison 0.99 m  13.0 kg  Bulbasaur Candy         100
#> 3  3 003   Venusaur http://www.serebii.net/pokemongo/pokemon/003.png Grass, Poison 2.01 m 100.0 kg  Bulbasaur Candy          NA
#> 4  4 004 Charmander http://www.serebii.net/pokemongo/pokemon/004.png          Fire 0.61 m   8.5 kg Charmander Candy          25
#> 5  5 005 Charmeleon http://www.serebii.net/pokemongo/pokemon/005.png          Fire 1.09 m  19.0 kg Charmander Candy         100
#> 6  6 006  Charizard http://www.serebii.net/pokemongo/pokemon/006.png  Fire, Flying 1.70 m  90.5 kg Charmander Candy          NA
```

By default, the list layer containing the repeated observations is
identified by the minimal depth detected across leaf elements. This
option can be overridden by the `coldepth` parameter in the `options`
argument, which can be useful to unnest nested sublists, such as
`next_evolution` or `prev_evolution`. In addition, setting
`namecols = TRUE` in the `options` argument includes the parent list
names associated to each row in the wide data.frame as individual
columns `L1`, `L2`, etc.

``` r
## bind prev/next evolution columns
rrapply(
  pokedex, 
  how = "bind",
  options = list(coldepth = 5, namecols = TRUE)
) |>
  head(n = 10)
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

#### Common alternatives

Several common alternatives used to unnest lists containing repeated
entries include
[`data.table::rbindlist()`](https://rdatatable.gitlab.io/data.table/reference/rbindlist.html),
[`dplyr::bind_rows()`](https://dplyr.tidyverse.org/reference/bind_rows.html),
and `tidyr`’s dedicated rectangling functions
[`unnest_longer()`](https://tidyr.tidyverse.org/reference/unnest_longer.html),
[`unnest_wider()`](https://tidyr.tidyverse.org/reference/unnest_wider.html)
and [`hoist()`](https://tidyr.tidyverse.org/reference/hoist.html).

The first two functions are primarily aimed at binding lists of
data.frames or lists of lists, but are not meant for nested lists
containing multiple levels of nesting, such as `pokedex`:

``` r
library(dplyr)

## simple list of lists
lapply(pokedex[["pokemon"]], `[`, 1:4) |>
  bind_rows() |> 
  head()
#> # A tibble: 6 × 4
#>      id num   name       img                                             
#>   <int> <chr> <chr>      <chr>                                           
#> 1     1 001   Bulbasaur  http://www.serebii.net/pokemongo/pokemon/001.png
#> 2     2 002   Ivysaur    http://www.serebii.net/pokemongo/pokemon/002.png
#> 3     3 003   Venusaur   http://www.serebii.net/pokemongo/pokemon/003.png
#> 4     4 004   Charmander http://www.serebii.net/pokemongo/pokemon/004.png
#> 5     5 005   Charmeleon http://www.serebii.net/pokemongo/pokemon/005.png
#> 6     6 006   Charizard  http://www.serebii.net/pokemongo/pokemon/006.png

## complex nested list (error)
bind_rows(pokedex[["pokemon"]])
#> Error in `vctrs::data_frame()`:
#> ! Can't recycle `id` (size 2) to match `weaknesses` (size 4).

## simple list of lists
lapply(pokedex[["pokemon"]], `[`, 1:4) |>
  data.table::rbindlist() |>
  head()
#>       id    num       name                                              img
#>    <int> <char>     <char>                                           <char>
#> 1:     1    001  Bulbasaur http://www.serebii.net/pokemongo/pokemon/001.png
#> 2:     2    002    Ivysaur http://www.serebii.net/pokemongo/pokemon/002.png
#> 3:     3    003   Venusaur http://www.serebii.net/pokemongo/pokemon/003.png
#> 4:     4    004 Charmander http://www.serebii.net/pokemongo/pokemon/004.png
#> 5:     5    005 Charmeleon http://www.serebii.net/pokemongo/pokemon/005.png
#> 6:     6    006  Charizard http://www.serebii.net/pokemongo/pokemon/006.png

## complex nested list (error)
data.table::rbindlist(pokedex[["pokemon"]])
#> Error in data.table::rbindlist(pokedex[["pokemon"]]): Column 5 of item 1 is length 2 inconsistent with column 15 which is length 4. Only length-1 columns are recycled.
```

The rectangling functions in the `tidyr`-package offer a lot more
flexibility. A similar data.frame as returned by
`rrapply(pokedex, how = "bind")` can be obtained by repeated application
of
[`tidyr::unnest_wider()`](https://tidyr.tidyverse.org/reference/unnest_wider.html):

``` r
library(tidyr)
library(tibble)

as_tibble(pokedex) |>
  unnest_wider(pokemon) |>
  unnest_wider(next_evolution, names_sep = ".") |>
  unnest_wider(prev_evolution, names_sep = ".") |>
  unnest_wider(next_evolution.1, names_sep = ".") |>
  unnest_wider(next_evolution.2, names_sep = ".") |>
  unnest_wider(next_evolution.3, names_sep = ".") |>
  unnest_wider(prev_evolution.1, names_sep = ".") |>
  unnest_wider(prev_evolution.2, names_sep = ".") |>
  head()
#> # A tibble: 6 × 25
#>      id num   name    img   type  height weight candy candy_count egg   spawn_chance avg_spawns spawn_time multipliers weaknesses next_evolution.1.num
#>   <int> <chr> <chr>   <chr> <lis> <chr>  <chr>  <chr>       <int> <chr>        <dbl>      <dbl> <chr>      <list>      <list>     <chr>               
#> 1     1 001   Bulbas… http… <chr> 0.71 m 6.9 kg Bulb…          25 2 km        0.69        69    20:00      <dbl [1]>   <chr [4]>  002                 
#> 2     2 002   Ivysaur http… <chr> 0.99 m 13.0 … Bulb…         100 Not …       0.042        4.2  07:00      <dbl [2]>   <chr [4]>  003                 
#> 3     3 003   Venusa… http… <chr> 2.01 m 100.0… Bulb…          NA Not …       0.017        1.7  11:30      <dbl [1]>   <chr [4]>  NA                  
#> 4     4 004   Charma… http… <chr> 0.61 m 8.5 kg Char…          25 2 km        0.253       25.3  08:45      <dbl [1]>   <chr [3]>  005                 
#> 5     5 005   Charme… http… <chr> 1.09 m 19.0 … Char…         100 Not …       0.012        1.2  19:00      <dbl [1]>   <chr [3]>  006                 
#> 6     6 006   Chariz… http… <chr> 1.70 m 90.5 … Char…          NA Not …       0.0031       0.31 13:34      <dbl [1]>   <chr [3]>  NA                  
#> # ℹ 9 more variables: next_evolution.1.name <chr>, next_evolution.2.num <chr>, next_evolution.2.name <chr>, next_evolution.3.num <chr>,
#> #   next_evolution.3.name <chr>, prev_evolution.1.num <chr>, prev_evolution.1.name <chr>, prev_evolution.2.num <chr>, prev_evolution.2.name <chr>
```

The option `how = "bind"` in
[`rrapply()`](https://jorischau.github.io/rrapply/reference/rrapply.md)
is less flexible as it always expands the nested list to a data.frame
that is *as wide as possible*. On the other hand, the flexibility and
interpretability in `tidyr`’s rectangling functions come at the cost of
increased computational effort, which can become a bottleneck when
unnesting large nested lists:

``` r
## large replicated pokedex list 
pokedex_large <- list(pokemon = do.call(c, replicate(1500, pokedex[["pokemon"]], simplify = FALSE)))

system.time({
  rrapply(pokedex_large, how = "bind")
})
#>    user  system elapsed 
#>   2.709   0.067   2.776

## unnest first layers prev_evolution and next_evolution
system.time({
  as_tibble(pokedex_large) |>
    unnest_wider(pokemon) |>
    unnest_wider(next_evolution, names_sep = ".") |>
    unnest_wider(prev_evolution, names_sep = ".") 
})
#>    user  system elapsed 
#>  32.825   0.404  33.240
```

**Remark**: in the chained calls to
[`unnest_wider()`](https://tidyr.tidyverse.org/reference/unnest_wider.html)
above, we only unnest the first layer of the `next_evolution` and
`prev_evolution` list-columns, and not any of the resulting children
list-columns, which would only further increase computation time.

To extract and unnest sublists at deeper levels of nesting in the list,
such as `next_evolution`, we manually set the `coldepth` parameter in
the `options` argument, as also demonstrated above:

``` r
system.time({
  ev1 <- rrapply(
    pokedex_large, 
    condition = \(x, .xparents) "next_evolution" %in% .xparents,
    how = "bind",
    options = list(namecols = TRUE, coldepth = 5)
  )
})
#>    user  system elapsed 
#>   1.119   0.002   1.121
head(ev1)
#>        L1 L2             L3 L4 num       name
#> 1 pokemon  1 next_evolution  1 002    Ivysaur
#> 2 pokemon  1 next_evolution  2 003   Venusaur
#> 3 pokemon  2 next_evolution  1 003   Venusaur
#> 4 pokemon  4 next_evolution  1 005 Charmeleon
#> 5 pokemon  4 next_evolution  2 006  Charizard
#> 6 pokemon  5 next_evolution  1 006  Charizard
```

The same unnested version of the `next_evolution` sublists can be
obtained by mixing several calls to
[`unnest_wider()`](https://tidyr.tidyverse.org/reference/unnest_wider.html)
and
[`unnest_longer()`](https://tidyr.tidyverse.org/reference/unnest_longer.html):

``` r
system.time({
  ev2 <- as_tibble(pokedex_large) |>
    unnest_wider(pokemon) |>
    unnest_longer(next_evolution) |>
    unnest_wider(next_evolution, names_sep = "_") |>
    select(id, next_evolution_num, next_evolution_name)
})
#>    user  system elapsed 
#>  23.490   0.003  23.496
head(ev2)
#> # A tibble: 6 × 3
#>      id next_evolution_num next_evolution_name
#>   <int> <chr>              <chr>              
#> 1     1 002                Ivysaur            
#> 2     1 003                Venusaur           
#> 3     2 003                Venusaur           
#> 4     4 005                Charmeleon         
#> 5     4 006                Charizard          
#> 6     5 006                Charizard
```

In the context of the current example, a more efficient approach is to
combine
[`unnest_wider()`](https://tidyr.tidyverse.org/reference/unnest_wider.html)
with [`hoist()`](https://tidyr.tidyverse.org/reference/hoist.html). The
disadvantage is that we need to manually specify the exact locations of
the elements that we wish to hoist from the nested list:

``` r
system.time({
  ev3 <- as_tibble(pokedex_large) |>
    unnest_wider(pokemon) |>
    hoist(next_evolution, 
          name.1 = list(1, "name"),
          name.2 = list(2, "name"),
          name.3 = list(3, "name")
    ) |>
    select(id, name.1, name.2, name.3)
})
#>    user  system elapsed 
#>  28.175   0.001  28.181
head(ev3)
#> # A tibble: 6 × 4
#>      id name.1     name.2    name.3
#>   <int> <chr>      <chr>     <chr> 
#> 1     1 Ivysaur    Venusaur  NA    
#> 2     2 Venusaur   NA        NA    
#> 3     3 NA         NA        NA    
#> 4     4 Charmeleon Charizard NA    
#> 5     5 Charizard  NA        NA    
#> 6     6 NA         NA        NA
```

Using
[`rrapply()`](https://jorischau.github.io/rrapply/reference/rrapply.md),
the same result can be obtained by adding a call to
[`reshape()`](https://rdrr.io/r/stats/reshape.html) (or alternatively
e.g. [`tidyr::pivot_wider()`](https://tidyr.tidyverse.org/reference/pivot_wider.html)
or
[`data.table::dcast()`](https://rdatatable.gitlab.io/data.table/reference/dcast.data.table.html))
by converting from a long to a wide data.frame:

``` r
system.time({
  ev4 <- rrapply(
    pokedex_large, 
    condition = \(x, .xparents) "next_evolution" %in% .xparents,
    how = "bind", 
    options = list(namecols = TRUE, coldepth = 5)
  ) 
  ev5 <- reshape(
    ev4[, c("L2", "L4", "name")],
    idvar = "L2",
    timevar = "L4",
    v.names = "name",
    direction = "wide"
  )
})
#>    user  system elapsed 
#>   1.527   0.001   1.529
head(ev5)
#>   L2     name.1    name.2 name.3
#> 1  1    Ivysaur  Venusaur   <NA>
#> 3  2   Venusaur      <NA>   <NA>
#> 4  4 Charmeleon Charizard   <NA>
#> 6  5  Charizard      <NA>   <NA>
#> 7  7  Wartortle Blastoise   <NA>
#> 9  8  Blastoise      <NA>   <NA>
```

#### Additional examples

We conclude this section by replicating some of the data rectangling
examples presented in the `tidyr` vignette:
<https://tidyr.tidyverse.org/articles/rectangle.html>. The example
nested lists are all conveniently included in the
[repurrrsive](https://CRAN.R-project.org/package=repurrrsive)-package.

##### GitHub Users

``` r
library(repurrrsive)

## nested data
str(gh_users, list.len = 3)
#> List of 6
#>  $ :List of 30
#>   ..$ login              : chr "gaborcsardi"
#>   ..$ id                 : int 660288
#>   ..$ avatar_url         : chr "https://avatars.githubusercontent.com/u/660288?v=3"
#>   .. [list output truncated]
#>  $ :List of 30
#>   ..$ login              : chr "jennybc"
#>   ..$ id                 : int 599454
#>   ..$ avatar_url         : chr "https://avatars.githubusercontent.com/u/599454?v=3"
#>   .. [list output truncated]
#>  $ :List of 30
#>   ..$ login              : chr "jtleek"
#>   ..$ id                 : int 1571674
#>   ..$ avatar_url         : chr "https://avatars.githubusercontent.com/u/1571674?v=3"
#>   .. [list output truncated]
#>   [list output truncated]

## unnested version
rrapply(gh_users, how = "bind") |>
  as_tibble()
#> # A tibble: 6 × 30
#>   login       id avatar_url gravatar_id url   html_url followers_url following_url gists_url starred_url subscriptions_url organizations_url repos_url
#>   <chr>    <int> <chr>      <chr>       <chr> <chr>    <chr>         <chr>         <chr>     <chr>       <chr>             <chr>             <chr>    
#> 1 gaborc… 6.60e5 https://a… ""          http… https:/… https://api.… https://api.… https://… https://ap… https://api.gith… https://api.gith… https://…
#> 2 jennybc 5.99e5 https://a… ""          http… https:/… https://api.… https://api.… https://… https://ap… https://api.gith… https://api.gith… https://…
#> 3 jtleek  1.57e6 https://a… ""          http… https:/… https://api.… https://api.… https://… https://ap… https://api.gith… https://api.gith… https://…
#> 4 julias… 1.25e7 https://a… ""          http… https:/… https://api.… https://api.… https://… https://ap… https://api.gith… https://api.gith… https://…
#> 5 leeper  3.51e6 https://a… ""          http… https:/… https://api.… https://api.… https://… https://ap… https://api.gith… https://api.gith… https://…
#> 6 masalm… 8.36e6 https://a… ""          http… https:/… https://api.… https://api.… https://… https://ap… https://api.gith… https://api.gith… https://…
#> # ℹ 17 more variables: events_url <chr>, received_events_url <chr>, type <chr>, site_admin <lgl>, name <chr>, company <list>, blog <chr>,
#> #   location <chr>, email <list>, hireable <list>, bio <list>, public_repos <int>, public_gists <int>, followers <int>, following <int>,
#> #   created_at <chr>, updated_at <chr>
```

##### GitHub repos

``` r
## nested data
str(gh_repos, list.len = 2)
#> List of 6
#>  $ :List of 30
#>   ..$ :List of 68
#>   .. ..$ id               : int 61160198
#>   .. ..$ name             : chr "after"
#>   .. .. [list output truncated]
#>   ..$ :List of 68
#>   .. ..$ id               : int 40500181
#>   .. ..$ name             : chr "argufy"
#>   .. .. [list output truncated]
#>   .. [list output truncated]
#>  $ :List of 30
#>   ..$ :List of 68
#>   .. ..$ id               : int 14756210
#>   .. ..$ name             : chr "2013-11_sfu"
#>   .. .. [list output truncated]
#>   ..$ :List of 68
#>   .. ..$ id               : int 14152301
#>   .. ..$ name             : chr "2014-01-27-miami"
#>   .. .. [list output truncated]
#>   .. [list output truncated]
#>   [list output truncated]

## unnested version
rrapply(gh_repos, how = "bind") |>
  as_tibble()
#> # A tibble: 176 × 84
#>          id name    full_name owner.login owner.id owner.avatar_url owner.gravatar_id owner.url owner.html_url owner.followers_url owner.following_url
#>       <int> <chr>   <chr>     <chr>          <int> <chr>            <chr>             <chr>     <chr>          <chr>               <chr>              
#>  1 61160198 after   gaborcsa… gaborcsardi   660288 https://avatars… ""                https://… https://githu… https://api.github… https://api.github…
#>  2 40500181 argufy  gaborcsa… gaborcsardi   660288 https://avatars… ""                https://… https://githu… https://api.github… https://api.github…
#>  3 36442442 ask     gaborcsa… gaborcsardi   660288 https://avatars… ""                https://… https://githu… https://api.github… https://api.github…
#>  4 34924886 baseim… gaborcsa… gaborcsardi   660288 https://avatars… ""                https://… https://githu… https://api.github… https://api.github…
#>  5 61620661 citest  gaborcsa… gaborcsardi   660288 https://avatars… ""                https://… https://githu… https://api.github… https://api.github…
#>  6 33907457 clisym… gaborcsa… gaborcsardi   660288 https://avatars… ""                https://… https://githu… https://api.github… https://api.github…
#>  7 37236467 cmaker  gaborcsa… gaborcsardi   660288 https://avatars… ""                https://… https://githu… https://api.github… https://api.github…
#>  8 67959624 cmark   gaborcsa… gaborcsardi   660288 https://avatars… ""                https://… https://githu… https://api.github… https://api.github…
#>  9 63152619 condit… gaborcsa… gaborcsardi   660288 https://avatars… ""                https://… https://githu… https://api.github… https://api.github…
#> 10 24343686 crayon  gaborcsa… gaborcsardi   660288 https://avatars… ""                https://… https://githu… https://api.github… https://api.github…
#> # ℹ 166 more rows
#> # ℹ 73 more variables: owner.gists_url <chr>, owner.starred_url <chr>, owner.subscriptions_url <chr>, owner.organizations_url <chr>,
#> #   owner.repos_url <chr>, owner.events_url <chr>, owner.received_events_url <chr>, owner.type <chr>, owner.site_admin <lgl>, private <lgl>,
#> #   html_url <chr>, description <list>, fork <lgl>, url <chr>, forks_url <chr>, keys_url <chr>, collaborators_url <chr>, teams_url <chr>,
#> #   hooks_url <chr>, issue_events_url <chr>, events_url <chr>, assignees_url <chr>, branches_url <chr>, tags_url <chr>, blobs_url <chr>,
#> #   git_tags_url <chr>, git_refs_url <chr>, trees_url <chr>, statuses_url <chr>, languages_url <chr>, stargazers_url <chr>, contributors_url <chr>,
#> #   subscribers_url <chr>, subscription_url <chr>, commits_url <chr>, git_commits_url <chr>, comments_url <chr>, issue_comment_url <chr>, …
```

##### Game of Thrones characters

``` r
## nested data
str(got_chars, list.len = 3)
#> List of 30
#>  $ :List of 18
#>   ..$ url        : chr "https://www.anapioficeandfire.com/api/characters/1022"
#>   ..$ id         : int 1022
#>   ..$ name       : chr "Theon Greyjoy"
#>   .. [list output truncated]
#>  $ :List of 18
#>   ..$ url        : chr "https://www.anapioficeandfire.com/api/characters/1052"
#>   ..$ id         : int 1052
#>   ..$ name       : chr "Tyrion Lannister"
#>   .. [list output truncated]
#>  $ :List of 18
#>   ..$ url        : chr "https://www.anapioficeandfire.com/api/characters/1074"
#>   ..$ id         : int 1074
#>   ..$ name       : chr "Victarion Greyjoy"
#>   .. [list output truncated]
#>   [list output truncated]

## unnested version
rrapply(got_chars, how = "bind") |>
  as_tibble()
#> # A tibble: 30 × 18
#>    url                      id name  gender culture born  died  alive titles aliases father mother spouse allegiances books povBooks tvSeries playedBy
#>    <chr>                 <int> <chr> <chr>  <chr>   <chr> <chr> <lgl> <list> <list>  <chr>  <chr>  <chr>  <list>      <lis> <list>   <list>   <list>  
#>  1 https://www.anapiofi…  1022 Theo… Male   "Ironb… "In … ""    TRUE  <chr>  <chr>   ""     ""     ""     <chr [1]>   <chr> <chr>    <chr>    <chr>   
#>  2 https://www.anapiofi…  1052 Tyri… Male   ""      "In … ""    TRUE  <chr>  <chr>   ""     ""     "http… <chr [1]>   <chr> <chr>    <chr>    <chr>   
#>  3 https://www.anapiofi…  1074 Vict… Male   "Ironb… "In … ""    TRUE  <chr>  <chr>   ""     ""     ""     <chr [1]>   <chr> <chr>    <chr>    <chr>   
#>  4 https://www.anapiofi…  1109 Will  Male   ""      ""    "In … FALSE <chr>  <chr>   ""     ""     ""     <lgl [1]>   <chr> <chr>    <chr>    <chr>   
#>  5 https://www.anapiofi…  1166 Areo… Male   "Norvo… "In … ""    TRUE  <chr>  <chr>   ""     ""     ""     <chr [1]>   <chr> <chr>    <chr>    <chr>   
#>  6 https://www.anapiofi…  1267 Chett Male   ""      "At … "In … FALSE <chr>  <chr>   ""     ""     ""     <lgl [1]>   <chr> <chr>    <chr>    <chr>   
#>  7 https://www.anapiofi…  1295 Cres… Male   ""      "In … "In … FALSE <chr>  <chr>   ""     ""     ""     <lgl [1]>   <chr> <chr>    <chr>    <chr>   
#>  8 https://www.anapiofi…   130 Aria… Female "Dorni… "In … ""    TRUE  <chr>  <chr>   ""     ""     ""     <chr [1]>   <chr> <chr>    <chr>    <chr>   
#>  9 https://www.anapiofi…  1303 Daen… Female "Valyr… "In … ""    TRUE  <chr>  <chr>   ""     ""     "http… <chr [1]>   <chr> <chr>    <chr>    <chr>   
#> 10 https://www.anapiofi…  1319 Davo… Male   "Weste… "In … ""    TRUE  <chr>  <chr>   ""     ""     "http… <chr [2]>   <chr> <chr>    <chr>    <chr>   
#> # ℹ 20 more rows
```

##### Sharla Gelfand’s discography

``` r
## nested data (first element)
str(discog[1], list.len = 3)
#> List of 1
#>  $ :List of 5
#>   ..$ instance_id      : int 354823933
#>   ..$ date_added       : chr "2019-02-16T17:48:59-08:00"
#>   ..$ basic_information:List of 11
#>   .. ..$ labels      :List of 1
#>   .. .. ..$ :List of 6
#>   .. .. .. ..$ name            : chr "Tobi Records (2)"
#>   .. .. .. ..$ entity_type     : chr "1"
#>   .. .. .. ..$ catno           : chr "TOB-013"
#>   .. .. .. .. [list output truncated]
#>   .. ..$ year        : int 2015
#>   .. ..$ master_url  : NULL
#>   .. .. [list output truncated]
#>   .. [list output truncated]

## unnested version (excluding deeply nested sublists)
discs <- rrapply(
  discog,
  condition = \(x, .xpos) length(.xpos) < 5,
  f = \(x) ifelse(is.null(x), NA, x),  ## replace NULLs
  how = "bind"
)
as_tibble(discs)
#> # A tibble: 155 × 12
#>    instance_id date_added             basic_information.year basic_information.ma…¹ basic_information.id basic_information.th…² basic_information.ti…³
#>          <int> <chr>                                   <int> <chr>                                 <int> <chr>                  <chr>                 
#>  1   354823933 2019-02-16T17:48:59-0…                   2015 NA                                  7496378 "https://img.discogs.… Demo                  
#>  2   354092601 2019-02-13T14:13:11-0…                   2013 https://api.discogs.c…              4490852 "https://img.discogs.… Observant Com El Mon …
#>  3   354091476 2019-02-13T14:07:23-0…                   2017 https://api.discogs.c…              9827276 "https://img.discogs.… I                     
#>  4   351244906 2019-02-02T11:39:58-0…                   2017 https://api.discogs.c…              9769203 "https://img.discogs.… Oído Absoluto         
#>  5   351244801 2019-02-02T11:39:37-0…                   2015 https://api.discogs.c…              7237138 "https://img.discogs.… A Cat's Cause, No Dog…
#>  6   351052065 2019-02-01T20:40:53-0…                   2019 https://api.discogs.c…             13117042 "https://img.discogs.… Tashme                
#>  7   350315345 2019-01-29T15:48:37-0…                   2014 https://api.discogs.c…              7113575 "https://img.discogs.… Demo                  
#>  8   350315103 2019-01-29T15:47:22-0…                   2015 https://api.discogs.c…             10540713 "https://img.discogs.… Let The Miracles Begin
#>  9   350314507 2019-01-29T15:44:08-0…                   2017 https://api.discogs.c…             11260950 ""                     Sub Space             
#> 10   350314047 2019-01-29T15:41:35-0…                   2017 NA                                 11726853 "https://img.discogs.… Demo                  
#> # ℹ 145 more rows
#> # ℹ abbreviated names: ¹​basic_information.master_url, ²​basic_information.thumb, ³​basic_information.title
#> # ℹ 5 more variables: basic_information.cover_image <chr>, basic_information.resource_url <chr>, basic_information.master_id <int>, id <int>,
#> #   rating <int>

## unnest labels sublists 
labels <- rrapply(
  discog,
  condition = \(x, .xparents) "labels" %in% .xparents,
  how = "bind",
  options = list(coldepth = 5, namecols = TRUE)
)
as_tibble(labels)
#> # A tibble: 182 × 10
#>    L1    L2                L3     L4    name                                      entity_type catno   resource_url                 id entity_type_name
#>    <chr> <chr>             <chr>  <chr> <chr>                                     <chr>       <chr>   <chr>                     <int> <chr>           
#>  1 1     basic_information labels 1     Tobi Records (2)                          1           TOB-013 https://api.discogs.com… 633407 Label           
#>  2 2     basic_information labels 1     La Vida Es Un Mus                         1           Mus70   https://api.discogs.com…  38322 Label           
#>  3 3     basic_information labels 1     La Vida Es Un Mus                         1           MUS118  https://api.discogs.com…  38322 Label           
#>  4 4     basic_information labels 1     La Vida Es Un Mus                         1           MUS132  https://api.discogs.com…  38322 Label           
#>  5 4     basic_information labels 2     Beat Generation                           1           BEAT64  https://api.discogs.com…  88198 Label           
#>  6 4     basic_information labels 3     Beat Generation                           1           BEAT 64 https://api.discogs.com…  88198 Label           
#>  7 5     basic_information labels 1     Katorga Works                             1           KW-043  https://api.discogs.com… 205895 Label           
#>  8 6     basic_information labels 1     High Fashion Industries                   1           HFI017  https://api.discogs.com… 637837 Label           
#>  9 7     basic_information labels 1     Mind Control Records (6)                  1           none    https://api.discogs.com… 763103 Label           
#> 10 8     basic_information labels 1     Not On Label (Phantom Head Self-released) 1           none    https://api.discogs.com… 879916 Label           
#> # ℹ 172 more rows

## merge disc id's with labels
merge(
  x = data.frame(L1 = rownames(discs), disc_id = discs[, "id"]),
  y = labels, 
  by = "L1", 
  sort = FALSE
) |>
  as_tibble()
#> # A tibble: 182 × 11
#>    L1     disc_id L2                L3     L4    name                                      entity_type catno   resource_url        id entity_type_name
#>    <chr>    <int> <chr>             <chr>  <chr> <chr>                                     <chr>       <chr>   <chr>            <int> <chr>           
#>  1 1      7496378 basic_information labels 1     Tobi Records (2)                          1           TOB-013 https://api.di… 633407 Label           
#>  2 2      4490852 basic_information labels 1     La Vida Es Un Mus                         1           Mus70   https://api.di…  38322 Label           
#>  3 3      9827276 basic_information labels 1     La Vida Es Un Mus                         1           MUS118  https://api.di…  38322 Label           
#>  4 4      9769203 basic_information labels 1     La Vida Es Un Mus                         1           MUS132  https://api.di…  38322 Label           
#>  5 4      9769203 basic_information labels 2     Beat Generation                           1           BEAT64  https://api.di…  88198 Label           
#>  6 4      9769203 basic_information labels 3     Beat Generation                           1           BEAT 64 https://api.di…  88198 Label           
#>  7 5      7237138 basic_information labels 1     Katorga Works                             1           KW-043  https://api.di… 205895 Label           
#>  8 6     13117042 basic_information labels 1     High Fashion Industries                   1           HFI017  https://api.di… 637837 Label           
#>  9 7      7113575 basic_information labels 1     Mind Control Records (6)                  1           none    https://api.di… 763103 Label           
#> 10 8     10540713 basic_information labels 1     Not On Label (Phantom Head Self-released) 1           none    https://api.di… 879916 Label           
#> # ℹ 172 more rows
```

## Data.frame to nested list

As a demonstrating example, we reconsider the long data.frame from the
first section obtained after melting the renewable energy shares of all
Western European countries:

``` r
renewable_energy_melt_west_eu <- rrapply(
  renewable_energy_by_country,
  condition = \(x, .xparents) "Western Europe" %in% .xparents,
  how = "melt"
) 
head(renewable_energy_melt_west_eu, n = 10)
#>      L1     L2             L3            L4 value
#> 1 World Europe Western Europe       Austria 34.67
#> 2 World Europe Western Europe       Belgium  9.14
#> 3 World Europe Western Europe        France 14.74
#> 4 World Europe Western Europe       Germany 14.17
#> 5 World Europe Western Europe Liechtenstein 62.93
#> 6 World Europe Western Europe    Luxembourg 13.54
#> 7 World Europe Western Europe        Monaco    NA
#> 8 World Europe Western Europe   Netherlands  5.78
#> 9 World Europe Western Europe   Switzerland 25.49
```

For certain tasks, it may be necessary to convert this data.frame back
to a nested list object, e.g. to write the data to a JSON- or XML-object
or for some tree visualization purpose. Writing a recursive function to
reconstruct the nested list can prove to be quite time-consuming and
error-prone.

In this context, the [`unlist()`](https://rdrr.io/r/base/unlist.html)
function has an inverse counterpart
[`relist()`](https://rdrr.io/r/utils/relist.html) that reconstructs a
nested list from the unlisted vector. The
[`relist()`](https://rdrr.io/r/utils/relist.html) function always
requires a `skeleton` nested list to repopulate, which can make it
difficult to use in practice, as such a `skeleton` object is for
instance unavailable for the current example. In particular, the melted
data.frame contains only a subset of the original list elements, so we
can not use the original list as a template object without filtering
nodes from the original list as well.

### Unmelt to nested list

To address this difficulty,
[`rrapply()`](https://jorischau.github.io/rrapply/reference/rrapply.md)
includes the dedicated option `how = "unmelt"` that performs the inverse
operation of `how = "melt"`. No skeleton object is needed in this case,
only a plain data.frame in the format returned by `how = "melt"`. To
illustrate, we can convert the melted data.frame above to a nested list
as follows:

``` r
rrapply(
  renewable_energy_melt_west_eu, 
  how = "unmelt"
) |>
  str(give.attr = FALSE)
#> List of 1
#>  $ World:List of 1
#>   ..$ Europe:List of 1
#>   .. ..$ Western Europe:List of 9
#>   .. .. ..$ Austria      : num 34.7
#>   .. .. ..$ Belgium      : num 9.14
#>   .. .. ..$ France       : num 14.7
#>   .. .. ..$ Germany      : num 14.2
#>   .. .. ..$ Liechtenstein: num 62.9
#>   .. .. ..$ Luxembourg   : num 13.5
#>   .. .. ..$ Monaco       : num NA
#>   .. .. ..$ Netherlands  : num 5.78
#>   .. .. ..$ Switzerland  : num 25.5
```

**Remark 1:** `how = "unmelt"` is based on a *greedy* approach parsing
data.frame rows as list elements starting from the top of the
data.frame. That is,
[`rrapply()`](https://jorischau.github.io/rrapply/reference/rrapply.md)
continues collecting children nodes as long as the parent node name
remains unchanged. If, for instance, the goal is to create two separate
nodes (on the same level) with the name `"Western Europe"`, these nodes
should not be listed directly after one another in the melted data.frame
as
[`rrapply()`](https://jorischau.github.io/rrapply/reference/rrapply.md)
will group all children under a single `"Western Europe"` list element.

**Remark 2:** Internally, `how = "unmelt"` reconstructs a nested list
from the melted data.frame and subsequently follows the same conceptual
framework as `how = "replace"`. Any other function arguments, such as
`f` and `condition` can be used in exactly the same way as when applying
`how = "replace"` to a nested list object.

**Remark 3:** `how = "unmelt"` does (currently) not restore the
attributes of intermediate list nodes and is therefore not an exact
inverse of `how = "melt"`. The other way around always produces the same
result:

``` r
renewable_energy_unmelt <- rrapply(renewable_energy_melt_west_eu, how = "unmelt")
renewable_energy_remelt <- rrapply(renewable_energy_unmelt, how = "melt")

identical(renewable_energy_melt_west_eu, renewable_energy_remelt)
#> [1] TRUE
```

In terms of computational effort,
[`rrapply()`](https://jorischau.github.io/rrapply/reference/rrapply.md)’s
`how = "unmelt"` can be equally or more efficient than
[`relist()`](https://rdrr.io/r/utils/relist.html) even though there is
no template list object that can be repopulated. This is highlighted
using the large list objects generated previously:

``` r
## large deeply nested list (2^18 elements)
##  benchmark timing with rrapply
system.time(deep_unmelt <- rrapply(deep_melt, how = "unmelt")) 
#>    user  system elapsed 
#>   0.101   0.000   0.101

## benchmark timing with relist
deep_unlist <- unlist(as.relistable(deep_list))
system.time(deep_relist <- relist(deep_unlist))
#>    user  system elapsed 
#>   2.640   0.000   2.641
```

``` r
## large shallow list (10^6 elements)
## benchmark timing with rrapply 
system.time(shallow_unmelt <- rrapply(shallow_melt, how = "unmelt")) 
#>    user  system elapsed 
#>   0.061   0.000   0.061

## benchmark timing with relist
shallow_unlist <- unlist(as.relistable(shallow_list))
system.time(shallow_relist <- relist(shallow_unlist))
#>    user  system elapsed 
#>   4.683   0.000   4.683
```

**Note**: the unmelted lists are not exactly identical to the original
nested lists, since `how = "unmelt"` uses the placeholder names `1`,
`2`, `3`, etc. in the melted data.frames to name the nodes in the newly
constructed lists, whereas the name attributes in the original lists are
all empty. By removing all names from the unmelted lists, they become
identical to their original counterparts:

``` r
## remove all list names
deep_unmelt_unnamed <- rrapply(
  deep_unmelt,
  f = unname,
  classes = "list",
  how = "recurse"
)
## check if identical
identical(unname(deep_unmelt_unnamed), deep_list)
#> [1] TRUE
```

``` r
## remove all list names
shallow_unmelt_unnamed <- rrapply(
  shallow_unmelt,
  f = unname,
  classes = "list",
  how = "recurse"
)
## check if identical
identical(unname(shallow_unmelt_unnamed), shallow_list)
#> [1] TRUE
```

------------------------------------------------------------------------

1.  The `renewable_energy_by_country` dataset is publicly available at
    the [United Nations Open SDG Data
    Hub](https://unstats-undesa.opendata.arcgis.com/datasets/)

2.  Note that
    [`rrapply()`](https://jorischau.github.io/rrapply/reference/rrapply.md)
    imposes a different column order than
    [`reshape2::melt()`](https://rdrr.io/pkg/reshape2/man/melt.html) and
    the `"value"` column may follow slightly different coercion rules,
    but other than that the melted data.frames are the same.
