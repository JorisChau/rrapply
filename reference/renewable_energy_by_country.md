# UNSD renewable energy share by country in 2016

A nested list containing renewable energy shares as a percentage in the
total energy consumption per country in 2016. The dataset is publicly
available at the United Nations Open SDG Data Hub.

## Usage

``` r
renewable_energy_by_country
```

## Format

The 249 countries and areas are structured as a nested list based on
their geographical location according to the United Nations M49 Standard
([UNSD-M49](https://unstats.un.org/unsd/methodology/m49/)). The numeric
values listed for each country or area are percentages, if no data is
available the value is `NA`. Each list element contains an `"M49-code"`
attribute with the UN Standard Country or Area Codes for Statistical Use
(Series M, No. 49).

## Source

[UNSD_SDG07](https://unstats-undesa.opendata.arcgis.com/datasets/indicator-7-2-1-renewable-energy-share-in-the-total-final-energy-consumption-percent-3)
