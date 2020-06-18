## CRAN package version 1.0.0 (resubmission)

### Comments

* The description text in the DESCRIPTION file has been extended en includes undirected single quotes for package and software names and parenthesis behind function names.
* Included missing options() reset in the vignette.
* The package code has been modified to not assign() to any environment other than the current evaluation environment. 

### Test environments

* ubuntu gcc R-release, R-devel (rhub)
* debian gcc R-release, R-devel, R-patched (rhub)
* debian clang R-devel (rhub)
* fedora clang/gcc R-devel (rhub)
* centos6-epel R-3.5.2 (rhub)
* macos-highsierra R-release (rhub)
* solaris-ods R-release (rhub)
* win-builder R-release, R-devel, R-old-release, 1 note (see below)

### Compiled code checks

* linux-x86_64-rocker-gcc-san (rhub)
* ubuntu-rchk (rhub)
* ubuntu clang R-old-release (local install) --use-valgrind/--use-gct

### Notes

win-builder note:

* checking CRAN incoming feasibility ... NOTE
Maintainer: ‘Joris Chau <joris.chau@openanalytics.eu>’

Possibly mis-spelled words in DESCRIPTION:
  Rapply (3:24)
  rapply (7:45)
  reimplementation (7:16)