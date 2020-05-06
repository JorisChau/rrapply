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