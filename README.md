
<!-- README.md is generated from README.Rmd. Please edit that file -->

# rrapply: revisiting R-base rapply

The rrapply-package contains a single function `rrapply`, providing an
extended implementation of R-base’s `rapply` function, which applies a
function `f` to all elements of a nested list recursively. rrapply is
implemented using R’s native C API and for this reason requires no
external R-package dependencies.

## Installation

``` r
# Install the development version from GitHub:
# install.packages("devtools")
devtools::install_github("JorisChau/rrapply")
```

## Example usage

### List pruning

With base `rapply`, there is no convenient way to prune or filter list
elements from the input list. The `rrapply` function adds an option `how
= "prune"` to prune all list elements not subject to application of `f`
from a nested
list,

``` r
# Nested list of renewable energy as a percentage of total energy consumption per country in 2016
data("renewable_energy_by_country")
# Subset values for countries and areas in Oceania
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

# Drop all logical NA's while preserving list structure 
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
```

Instead, use `how = "flatten"` to return a flattened unnested version of
the pruned list,

``` r
## Drop all logical NA's and return unnested list
na_drop_oceania2 <- rrapply(
  renewable_oceania,
  f = function(x) x,
  classes = "numeric",
  how = "flatten"
)
str(na_drop_oceania2, list.len = 10, give.attr = FALSE)
#> List of 22
#>  $ Australia                       : num 9.32
#>  $ New Zealand                     : num 32.8
#>  $ Fiji                            : num 24.4
#>  $ New Caledonia                   : num 4.03
#>  $ Papua New Guinea                : num 50.3
#>  $ Solomon Islands                 : num 65.7
#>  $ Vanuatu                         : num 33.7
#>  $ Guam                            : num 3.03
#>  $ Kiribati                        : num 45.4
#>  $ Marshall Islands                : num 11.8
#>   [list output truncated]
```

### Condition function

Base `rapply` allows to apply a function `f` to list elements of certain
types or classes via the `classes` argument. `rrapply` generalizes this
option via an additional `condition` argument, which accepts any
function to use as a condition or predicate to apply `f` to a subset of
list elements.

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

### Special symbols `.xname` and `.xpos`

In base `rapply`, the `f` function only has access to the content of the
list element under evaluation, and there is no convenient way to access
its name or location in the nested list from inside the `f` function.
`rrapply` allows the use the special symbols `.xname` and `.xpos` inside
the `f` and `condition` function. `.xname` evaluates to the name of list
element, and `.xpos` evaluates to the position of the element in the
nested list structured as an integer vector.

``` r
## Apply a function using the name of the node
renewable_oceania4 <- rrapply(
  renewable_oceania,
  f = function(x) sprintf("Renewable energy in %s: %.2f%%", .xname, x),
  condition = Negate(is.na),
  how = "flatten"
)
str(renewable_oceania4, list.len = 10)
#> List of 22
#>  $ Australia                       : chr "Renewable energy in Australia: 9.32%"
#>  $ New Zealand                     : chr "Renewable energy in New Zealand: 32.76%"
#>  $ Fiji                            : chr "Renewable energy in Fiji: 24.36%"
#>  $ New Caledonia                   : chr "Renewable energy in New Caledonia: 4.03%"
#>  $ Papua New Guinea                : chr "Renewable energy in Papua New Guinea: 50.34%"
#>  $ Solomon Islands                 : chr "Renewable energy in Solomon Islands: 65.73%"
#>  $ Vanuatu                         : chr "Renewable energy in Vanuatu: 33.67%"
#>  $ Guam                            : chr "Renewable energy in Guam: 3.03%"
#>  $ Kiribati                        : chr "Renewable energy in Kiribati: 45.43%"
#>  $ Marshall Islands                : chr "Renewable energy in Marshall Islands: 11.75%"
#>   [list output truncated]

## Extract values based on country names
renewable_benelux <- rrapply(
  renewable_energy_by_country,
  condition = function(x) .xname %in% c("Belgium", "Netherlands", "Luxembourg"),
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
  condition = function(x) identical(head(.xpos, 2), c(1L, 5L)) & x > 50,
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
  condition = function(x) identical(.xname, "Sweden"),
  f = function(x) .xpos,
  how = "flatten"
))
#> $Sweden
#> [1]  1  5  2 14
renewable_energy_by_country[[xpos_sweden$Sweden]]
#> [1] 51.35
#> attr(,"M49-code")
#> [1] "752"
```

### Using `rrapply` on data.frames

Base `rapply` recurses into all list-like objects. Since data.frames are
list-like objects, `rapply` always descends into the individual columns
of a data.frame. `rrapply` includes an additional `dfAsList` argument,
which if `FALSE` does not treat a data.frame as a list and applies the
`f` function to the data.frame as a whole instead of its individual
columns.

However, it can also be useful to exploit the property that a data.frame
is a list-like object and use base `rapply` to apply a function `f` to
data.frame columns of certain classes via the `classes` argument. Using
the `condition` argument in `rrapply`, we can apply a function `f` to a
subset of data.frame columns using a more general predicate function,

``` r
## Scale only Sepal columns in iris dataset
iris_standard_sepal <- rrapply(
  iris,
  condition = function(x) grepl("Sepal", .xname),
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

For more details and examples on how to use the `rrapply` function see
the accompanying vignette in the vignettes folder.