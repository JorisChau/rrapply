## CRAN package version 1.2.8

* Removed non-API calls to `Rf_lazy_duplicate` and `Rf_isValidString`

## Test environments

* ubuntu gcc R-oldrel, R-release, R-next, R-devel (rhub, gh-actions)
* debian gcc/clang R-devel (rhub)
* fedora gcc/clang R-devel (rhub)
* macos-monterey clang R-release, R-next (gh-actions)
* windows gcc R-oldrel, R-release, R-next, R-devel (r-winbuilder, gh-actions)

## Compiled code checks

* ubuntu-rchk R-devel (docker)
* fedora gcc R-devel --use-valgrind (rhub)
* ubuntu gcc R-4.4 --use-gct (local install)
