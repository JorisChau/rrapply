# Pokedex of Pokemon GO

A nested list containing property values of the original 151 Pokemon
present in Pokemon GO. The data is available in JSON format from GitHub
(credits to Gianluca Bonifazi).

## Usage

``` r
pokedex
```

## Format

A nested list containing 151 sublists with up to 17 list elements:

- id:

  integer, Identification number

- num:

  character, Pokemon number in official Pokedex

- name:

  character, Pokemon name

- img:

  character, URL to png image of the Pokemon

- type:

  character, Pokemon type

- height:

  character, Pokemon height

- weight:

  character, Pokemon weight

- candy:

  character, type of candy used to evolve Pokemon or given when
  transfered

- candy_count:

  integer, amount of candies required to evolve

- egg:

  character, travel distance to hatch the egg

- spawn_change:

  numeric, spawn change percentage

- avg_spawns:

  integer, number of spawns per 10.000 spawns

- spawn_time:

  character, local time at which spawns are most active

- multipliers:

  numeric, multiplier of Combat Power (CP) after evolution

- weakness:

  character, types of Pokemon this Pokemon is weak to

- next_evolution:

  list, numbers (`num`) and names (`name`) with successive evolutions

- prev_evolution:

  list, numbers (`num`) and names (`name`) with previous evolutions

## Source

[PokemonGO-Pokedex](https://raw.githubusercontent.com/Biuni/PokemonGO-Pokedex/master/pokedex.json)
