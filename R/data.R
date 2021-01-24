#' UNSD renewable energy share by country in 2016
#'
#' A nested list containing renewable energy shares as a percentage in the total energy consumption
#' per country in 2016. The dataset is publicly available at the United Nations Open SDG Data Hub. 
#' 
#' @format The 249 countries and areas are structured as a nested list based on their geographical location 
#' according to the United Nations M49 Standard (\href{https://unstats.un.org/unsd/methodology/m49/}{UNSD-M49}). 
#' The numeric values listed for each country or area are percentages, if no data is available the value is \code{NA}.
#' Each list element contains an \code{"M49-code"} attribute with the UN Standard Country or Area Codes for Statistical Use (Series M, No. 49).
#' 
#' @source \href{https://unstats-undesa.opendata.arcgis.com/datasets/indicator-7-2-1-renewable-energy-share-in-the-total-final-energy-consumption-percent-3}{UNSD_SDG07}
"renewable_energy_by_country"

#' Pokedex of Pokemon GO
#' 
#' A nested list containing property values of the original 151 Pokemon present in Pokemon GO. The data is available in JSON format from GitHub (credits to Gianluca Bonifazi).
#' 
#' @format A nested list containing 151 sublists with up to 17 list elements:
#' \describe{
#'   \item{id}{integer, Identification number}
#'   \item{num}{character, Pokemon number in official Pokedex}
#'   \item{name}{character, Pokemon name}
#'   \item{img}{character, URL to png image of the Pokemon}
#'   \item{type}{character, Pokemon type}
#'   \item{height}{character, Pokemon height}
#'   \item{weight}{character, Pokemon weight}
#'   \item{candy}{character, type of candy used to evolve Pokemon or given when transfered}
#'   \item{candy_count}{integer, amount of candies required to evolve}
#'   \item{egg}{character, travel distance to hatch the egg}
#'   \item{spawn_change}{numeric, spawn change percentage}
#'   \item{avg_spawns}{integer, number of spawns per 10.000 spawns}
#'   \item{spawn_time}{character, local time at which spawns are most active}
#'   \item{multipliers}{numeric, multiplier of Combat Power (CP) after evolution}
#'   \item{weakness}{character, types of Pokemon this Pokemon is weak to}
#'   \item{next_evolution}{list, numbers (\code{num}) and names (\code{name}) with successive evolutions}
#'   \item{prev_evolution}{list, numbers (\code{num}) and names (\code{name}) with previous evolutions}
#' }
#' 
#' @source \href{https://raw.githubusercontent.com/Biuni/PokemonGO-Pokedex/master/pokedex.json}{PokemonGO-Pokedex}
"pokedex"