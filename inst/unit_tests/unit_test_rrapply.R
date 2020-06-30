require(rrapply)

cat("Running rrapply unit tests...\n")

## ---------------- ##
## Helper functions ##
## ---------------- ##

## check test
dotest <- function(itest, observed, expected) {
  if(!identical(observed, expected)) stop(sprintf("Test %s failed", itest), call. = FALSE) 
}

## create nested list
f <- function(len, d, dmax, expr) {
  x <- vector(mode = "list", length = len)
  for(i in seq_along(x)) {
    if(d + 1 < dmax) {
      x[[i]] <- Recall(len, d + 1, dmax, expr)
    } else {
      x[[i]] <- expr 
    }
  }
  return(x)
}

## ---------- ##
## Unit tests ##
## ---------- ##

## input list
xin <- list(a = 1L, b = list(b1 = 2L, b2 = 3L), c = 4L)

## f argument
xout0.1 <- rapply(xin, f = `-`, classes = "ANY", how = "replace")
xout0.2 <- rapply(xin, f = `+`, e2 = 1L, how = "replace")
xout0.3 <- list(a = 1L, b = list(b1 = c(2L, 1L), b2 = c(2L, 2L)), c = 3L)
xout0.4 <- rapply(xin, f = function(x) 1L, how = "replace")
xout0.5 <- list(a = "a", b = list(b1 = "b1", b2 = "b2"), c = "c")
xout0.6 <- rapply(xin, f = function(x) "a", how = "replace")
xout0.7 <- list(a = c("a", "1"), b = list(b1 = c("b1", "2", "1"), b2 = c("b2", "2", "2")), c = c("c", "3"))
xout0.8 <- xout0.7
xout0.9 <- rapply(xin, f = function(x) c("a", "1"), how = "replace")
xout0.10 <- list(a = c("a", "1"), b = list(b1 = c("a", "1"), b2 = c("a", "1")), c = c("a", "1"))

dotest("0.1", rrapply(xin, f = `-`, classes = "ANY"), xout0.1)
dotest("0.2", rrapply(xin, f = `+`, e2 = 1L), xout0.2)

.xpos <- .xname <- 1L

dotest("0.3", rrapply(xin, f = function(x, .xpos) .xpos), xout0.3)
dotest("0.4", rrapply(xin, f = function(x, xpos) xpos, xpos = 1L), xout0.4)
dotest("0.5", rrapply(xin, f = function(x, .xname) .xname), xout0.5)
dotest("0.6", rrapply(xin, f = function(x, xname) xname, xname = "a"), xout0.6)
dotest("0.7", rrapply(xin, f = function(x, .xname, .xpos) c(.xname, .xpos)), xout0.7)
dotest("0.8", rrapply(xin, f = function(x, .xpos, .xname) c(.xname, .xpos)), xout0.8)
dotest("0.9", rrapply(xin, f = function(x, xname, xpos) c(xname, xpos), xpos = 1L, xname = "a"), xout0.9)
dotest("0.10", rrapply(xin, f = function(x, .xname, .xpos) {.xpos = 1L; .xname = "a"; c(.xname, .xpos)}), xout0.10)
    
## condition argument
xout1.1 <- list(a = -1L, b = list(b1 = 2L, b2 = 3L), c = 4L)
xout1.2 <- list(a = 0L, b = list(b1 = 2L, b2 = 3L), c = 4L)
xout1.3 <- xout1.1
xout1.4 <- xout0.1
xout1.5 <- xout1.1
xout1.6 <- xout0.1
xout1.7 <- xout1.1

dotest("1.1", rrapply(xin, f = `-`, condition = function(x) x == 1L), xout1.1)
dotest("1.2", rrapply(xin, f = `-`, condition = `==`, e2 = 1L), xout1.2)
dotest("1.3", rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos == 1L), xout1.3)
dotest("1.4", rrapply(xin, f = function(x, xpos) -x, condition = function(x, xpos) xpos == 1L, xpos = 1L), xout1.4)
dotest("1.5", rrapply(xin, f = `-`, condition = function(x, .xname) .xname == names(xin)[1]), xout1.5)
dotest("1.6", rrapply(xin, f = function(x, xname) -x, condition = function(x, xname) xname == "a", xname = "a"), xout1.6)
dotest("1.7", rrapply(xin, f = `-`, condition = function(x, .xpos, .xname) .xname == names(xin)[1] & .xpos == 1L), xout1.7)

dotest("1.8", .xpos, 1L)
dotest("1.9", .xname, 1L)
dotest("1.10", exists("X"), FALSE)
rm(.xpos, .xname)

## how argument
xout2.1 <- list(a = -1L, b = list(b1 = 2L, b2 = 3L), c = -4L)
xout2.2 <- list(a = -1L, b = list(b1 = NULL, b2 = NULL), c = -4L)
xout2.3 <- c(a = -1L, c = -4L)
xout2.4 <- as.list(xout2.3)
xout2.5 <- xout2.4
xout2.6 <- structure(list(L1 = c("b", "b"), L2 = c("b1", "b2"), value = list(-2L, -3L)), row.names = 1:2, class = "data.frame")
xout2.7 <- structure(list(L1 = c("a", "c"), value = list(-1L, -4L)), row.names = 1:2, class = "data.frame")
xout2.8 <- structure(list(L1 = c("a", "b"), L2 = c(NA, "b1"), value = list(-1L, -2L)), row.names = 1:2, class = "data.frame")

dotest("2.1", rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "replace"), xout2.1)
dotest("2.2", rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "list"), xout2.2)
dotest("2.3", rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "unlist"), xout2.3)
dotest("2.4", rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "prune"), xout2.4)
dotest("2.5", rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "flatten"), xout2.5)
dotest("2.6", rrapply(xin, f = `-`, condition = function(x, .xpos) length(.xpos) > 1, how = "melt"), xout2.6)
dotest("2.7", rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "melt"), xout2.7)
dotest("2.8", rrapply(xin, f = `-`, condition = function(x, .xname) grepl("a|b1", .xname), how = "melt"), xout2.8)

## check for trailing .xpos and .xname variables
dotest("2.11", exists(".xpos"), FALSE)
dotest("2.12", exists(".xname"), FALSE)

## classes argument
xout3.1 <- list(a = structure(-1L, .Dim = c(1L, 1L)), b = list(b1 = 2L, b2 = 3L), c = 4L)
xout3.2 <- list(a = structure(1L, .Dim = c(1L, 1L)), b = list(b1 = -2L, b2 = -3L), c = -4L)
xout3.3 <- xout0.1
xout3.4 <- list(a = structure(-1L, class = "user-class"), b = list(b1 = 2L, b2 = 3L), c = 4L)
xout3.5 <- list(a = -1, b = list(b1 = 2L, b2 = 3L), c = 4L)
xout3.6 <- list(a = -1, b = list(b1 = -2L, b2 = -3L), c = -4L)

xin[[1]] <- as.matrix(xin[[1]])
dotest("3.1", rrapply(xin, f = `-`, classes = "matrix"), xout3.1)
dotest("3.2", rrapply(xin, f = `-`, classes = "integer"), xout3.2)
xin[[1]] <- as.integer(xin[[1]])
dotest("3.3", rrapply(xin, f = `-`, classes = "integer"), xout3.3)
class(xin[[1]]) <- "user-class"
dotest("3.4", rrapply(xin, f = `-`, classes = "user-class"), xout3.4)
xin[[1]] <- as.numeric(xin[[1]])
dotest("3.5", rrapply(xin, f = `-`, classes = "numeric"), xout3.5)
dotest("3.6", rrapply(xin, f = `-`, classes = c("numeric", "integer")), xout3.6)

## deflt argument
xin <- list(a = 1L, b = list(b1 = 2L, b2 = 3L), c = 4L)
xout4.1 <- list(a = -1L, b = list(b1 = NA_character_, b2 = NA_character_), c = NA_character_)
xout4.2 <- list(a = 1L, b = list(b1 = NA_real_, b2 = NA_real_), c = NA_real_)
xout4.3 <- c(a = 1, b.b1 = NA, b.b2 = NA, c = NA)

dotest("4.1", rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos == 1L, deflt = NA_character_, how = "list"), xout4.1)
dotest("4.2", rrapply(xin, condition = function(x, .xpos) .xpos == 1L, deflt = NA_real_, how = "list"), xout4.2)
dotest("4.3", rrapply(xin, condition = function(x, .xpos) .xpos == 1L, deflt = NA_real_, how = "unlist"), xout4.3)

## dfaslist argument
xin[[1]] <- data.frame(x = 1L, y = 2L)

xout5.1 <- list(a = structure(list(x = 1L, y = 2L), class = "data.frame", row.names = c(NA, -1L)), b = list(b1 = 2L, b2 = 3L), c = 4L)
xout5.2 <- list(a = "a", b = list(b1 = 2L, b2 = 3L), c = 4L)
xout5.3 <- list(a = structure(list(x = "x", y = "y"), class = "data.frame", row.names = c(NA, -1L)), b = list(b1 = 2L, b2 = 3L), c = 4L)
xout5.4 <- xout5.2
    
dotest("5.1", rrapply(xin, f = function(x, .xname) .xname, classes = "data.frame"), xout5.1)
dotest("5.2", rrapply(xin, f = function(x, .xname) .xname, classes = "data.frame", dfaslist = FALSE), xout5.2)
dotest("5.3", rrapply(xin, f = function(x, .xname) .xname, condition = function(x, .xpos) .xpos[1] == 1L), xout5.3)
dotest("5.4", rrapply(xin, f = function(x, .xname) .xname, condition = function(x, .xpos) .xpos[1] == 1L, dfaslist = FALSE), xout5.4)

## feverywhere argument
xin1 <- list(a = 1L, b = list(b1 = list(b11 = 2L), b2 = 3L), c = 4L)
xin2 <- list(a = 1L, b = list(b1 = 2L, b2 = 3L), c = 4L)

xout6.1 <- list(a = "a", b = "b", c = "c")
xout6.2 <- list(a = 1L, b = "b", c = 4L)
xout6.3 <- list(a = 1L, b = list(b1 = "b1", b2 = "b2"), c = 4L)
xout6.4 <- list(a = 1L, b = list(b1 = list(b11 = "b11"), b2 = 3L), c = 4L)

xout6.5 <- list(a = 1L, b = c(b1 = 2L, b2 = 3L), c = 4L)
xout6.6 <- list(a = NULL, b = c(b1 = 2L, b2 = 3L), c = NULL)
xout6.7 <- c(b.b1 = 2L, b.b2 = 3L)
xout6.8 <- list(b = c(b1 = 2L, b2 = 3L))
xout6.9 <- xout6.8
xout6.10 <- structure(list(L1 = "b", value = list(structure(2:3, .Names = c("b1", "b2")))), row.names = 1L, class = "data.frame")
  
xout6.11 <- list(a = 1L, b = list(b1_ = 2L, b2_ = 3L), c = 4L)
xout6.12 <- xout6.11
xout6.13 <- structure(1:4, .Names = c("a", "b.b1_", "b.b2_", "c"))
xout6.14 <- list(b = list(b1_ = 2L, b2_ = 3L))
xout6.15 <- list(a = 1L, b1_ = 2L, b2_ = 3L, c = 4L)
xout6.16 <- structure(list(L1 = c("a", "b", "b", "c"), L2 = c(NA, "b1_", "b2_", NA), value = list(1L, 2L, 3L, 4L)), row.names = c(NA, 4L), class = "data.frame")

xout6.17 <- list(a = list(list(1L)), b = list(b1 = 2L, b2 = 3L), c = 4L)
xout6.18 <- list(a = list(list(1L)), b = list(b1 = 1L, b2 = 1L), c = 1L)
xout6.19 <- c(a = 1L, b.b1 = 1L, b.b2 = 1L, c = 1L)
xout6.20 <- list(a = list(list(1L)))
xout6.21 <- list(a = list(a = 1L))
xout6.22 <- structure(list(L1 = "a", L2 = "..1", value = list(list(1L))), row.names = 1L, class = "data.frame")

dotest("6.1", rrapply(xin1, f = function(x, .xname) .xname, feverywhere = "break"), xout6.1)
dotest("6.2", rrapply(xin1, f = function(x, .xname) .xname, condition = function(x, .xname) .xname == "b", feverywhere = "break"), xout6.2)
dotest("6.3", rrapply(xin1, f = function(x, .xname) .xname, condition = function(x, .xpos) length(.xpos) == 2, feverywhere = "break"), xout6.3)
dotest("6.4", rrapply(xin1, f = function(x, .xname) .xname, condition = function(x, .xpos) length(.xpos) == 3, feverywhere = "break"), xout6.4)

dotest("6.5", rrapply(xin2, f = unlist, condition = function(x, .xname) .xname == "b", how = "replace", feverywhere = "break"), xout6.5)
dotest("6.6", rrapply(xin2, f = unlist, condition = function(x, .xname) .xname == "b", how = "list", feverywhere = "break"), xout6.6)
dotest("6.7", rrapply(xin2, f = unlist, condition = function(x, .xname) .xname == "b", how = "unlist", feverywhere = "break"), xout6.7)
dotest("6.8", rrapply(xin2, f = unlist, condition = function(x, .xname) .xname == "b", how = "prune", feverywhere = "break"), xout6.8)
dotest("6.9", rrapply(xin2, f = unlist, condition = function(x, .xname) .xname == "b", how = "flatten", feverywhere = "break"), xout6.9)
dotest("6.10", rrapply(xin2, f = unlist, condition = function(x, .xname) .xname == "b", how = "melt", feverywhere = "break"), xout6.10)

dotest("6.11", rrapply(xin2, f = function(x){ names(x) <- paste0(names(x), "_"); x }, condition = is.list, how = "replace", feverywhere = "recurse"), xout6.11)
dotest("6.12", rrapply(xin2, f = function(x){ if(is.list(x)) { names(x) <- paste0(names(x), "_") }; x }, how = "list", feverywhere = "recurse"), xout6.12)
dotest("6.13", rrapply(xin2, f = function(x){ if(is.list(x)) { names(x) <- paste0(names(x), "_") }; x }, how = "unlist", feverywhere = "recurse"), xout6.13)
dotest("6.14", rrapply(xin2, f = function(x){ names(x) <- paste0(names(x), "_"); x }, condition = is.list, how = "prune", feverywhere = "recurse"), xout6.14)
dotest("6.15", rrapply(xin2, f = function(x){ if(is.list(x)) { names(x) <- paste0(names(x), "_") }; x }, how = "flatten", feverywhere = "recurse"), xout6.15)
dotest("6.16", rrapply(xin2, f = function(x){ if(is.list(x)) { names(x) <- paste0(names(x), "_") }; x }, how = "melt", feverywhere = "recurse"), xout6.16)

dotest("6.17", rrapply(xin2, f = list, condition = function(x, .xpos, .xname) length(.xpos) < 3 & all(.xpos < 2), how = "replace", feverywhere = "recurse"), xout6.17)
dotest("6.18", rrapply(xin2, f = list, condition = function(x, .xpos, .xname) length(.xpos) < 3 & all(.xpos < 2), how = "list", deflt = 1L, feverywhere = "recurse"), xout6.18)
dotest("6.19", rrapply(xin2, f = list, condition = function(x, .xpos, .xname) length(.xpos) < 3 & all(.xpos < 2), how = "unlist", deflt = 1L, feverywhere = "recurse"), xout6.19)
dotest("6.20", rrapply(xin2, f = list, condition = function(x, .xpos, .xname) length(.xpos) < 3 & all(.xpos < 2), how = "prune", feverywhere = "recurse"), xout6.20)
dotest("6.21", rrapply(xin2, f = function(x) list(a = x), condition = function(x, .xpos, .xname) length(.xpos) < 3 & all(.xpos < 2), how = "flatten", feverywhere = "recurse"), xout6.21)
dotest("6.22", rrapply(xin2, f = list, condition = function(x, .xpos, .xname) length(.xpos) < 3 & all(.xpos < 2), how = "melt", feverywhere = "recurse"), xout6.22)

## named flat list
xin1 <- list(a = 1L, b = 2L, c = 3L)
xin2 <- list(a = 1L, b = NULL)
  
xout7.1 <- list(a = -1L, b = 2L, c = -3L)
xout7.2 <- list(a = -1L, b = NULL, c = -3L)
xout7.3 <- list(a = -1L, c = -3L)
xout7.4 <- xout7.3
xout7.5 <- xout7.1
xout7.6 <- list(a = list(1L), b = 2L, c = 3L)
xout7.7 <- structure(list(L1 = c("a", "c"), value = list(-1L, -3L)), row.names = 1:2, class = "data.frame")
xout7.8 <- list(a = FALSE, b = TRUE)
xout7.9 <- xout7.8
xout7.10 <- xout7.8
xout7.11 <- structure(list(L1 = c("a", "b"), value = list(FALSE, TRUE)), row.names = 1:2, class = "data.frame")

dotest("7.1", rrapply(xin1, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "replace"), xout7.1)
dotest("7.2", rrapply(xin1, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "list"), xout7.2)
dotest("7.3", rrapply(xin1, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "prune"), xout7.3)
dotest("7.4", rrapply(xin1, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "flatten"), xout7.4)
dotest("7.5", rrapply(xin1, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), feverywhere = "break"), xout7.5)
dotest("7.6", rrapply(xin1, f = list, condition = function(x, .xpos) length(.xpos) < 2 & all(.xpos == 1L), feverywhere = "recurse"), xout7.6)
dotest("7.7", rrapply(xin1, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "melt"), xout7.7)
dotest("7.8", rrapply(xin2, f = is.null, how = "replace"), xout7.8)
dotest("7.9", rrapply(xin2, f = is.null, how = "prune"), xout7.9)
dotest("7.10", rrapply(xin2, f = is.null, how = "flatten"), xout7.10)
dotest("7.11", rrapply(xin2, f = is.null, how = "melt"), xout7.11)

## unnamed nested list
xin <- list(1L, 2L, list(3L, 4L))

xout8.1 <- list(-1L, 2L, list(3L, 4L))
xout8.2 <- list(-1L, NULL, list(NULL, NULL))
xout8.3 <- list(-1L)
xout8.4 <- xout8.3
xout8.5 <- structure(list(L1 = "..1", value = list(-1L)), row.names = 1L, class = "data.frame")
xout8.6 <- list(-1L, -2L, list(-3L, -4L))
xout8.7 <- list(-1L, 2L, -3L)
xout8.8 <- list(list(1L), 2L, list(3L, 4L))

dotest("8.1", rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "replace"), xout8.1)
dotest("8.2", rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "list"), xout8.2)
dotest("8.3", rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "prune"), xout8.3)
dotest("8.4", rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "flatten"), xout8.4)
dotest("8.5", rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "melt"), xout8.5)
dotest("8.6", rrapply(xin, f = `-`, condition = function(x, .xname) is.na(.xname)), xout8.6)
dotest("8.7", rrapply(xin, f = function(x, .xpos) -.xpos, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), feverywhere = "break"), xout8.7)
dotest("8.8", rrapply(xin, f = list, condition = function(x, .xpos) length(.xpos) < 2 & all(.xpos == 1L), feverywhere = "recurse"), xout8.8)

## partially named list 1
xin <- list(a = 1L, 2L, list(c1 = 3L, 4L))

xout9.1 <- list(a = -1L, 2L, list(c1 = -3L, 4L))
xout9.2 <- list(a = -1L, NULL, list(c1 = -3L, NULL))
xout9.3 <- list(a = -1L, list(c1 = -3L))
xout9.4 <- list(a = -1L, c1 = -3L)
xout9.5 <- structure(list(L1 = c("a", ""), L2 = c(NA, "c1"), value = list(-1L, -3L)), row.names = 1:2, class = "data.frame")
xout9.6 <- list(a = -1L)
xout9.7 <- list(a = "a", 2L, list(c1 = "c1", 4L))
xout9.8 <- list(a = list(1L), 2L, list(c1 = 3L, 4L))

dotest("9.1", rrapply(xin, f = `-`, condition = function(x, .xname) nzchar(.xname), how = "replace"), xout9.1)
dotest("9.2", rrapply(xin, f = `-`, condition = function(x, .xname) nzchar(.xname), how = "list"), xout9.2)
dotest("9.3", rrapply(xin, f = `-`, condition = function(x, .xname) nzchar(.xname), how = "prune"), xout9.3)
dotest("9.4", rrapply(xin, f = `-`, condition = function(x, .xname) nzchar(.xname), how = "flatten"), xout9.4)
dotest("9.5", rrapply(xin, f = `-`, condition = function(x, .xname) nzchar(.xname), how = "melt"), xout9.5)
dotest("9.6", rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos == 1L, how = "prune"), xout9.6)
dotest("9.7", rrapply(xin, f = function(x, .xname) .xname, condition = function(x, .xname) nzchar(.xname), feverywhere = "break"), xout9.7)
dotest("9.8", rrapply(xin, f = list, condition = function(x, .xpos) length(.xpos) < 2 & all(.xpos == 1L), feverywhere = "recurse"), xout9.8)

## partially named list 2
xin <- list(1L, 2L, list(c1 = 3L, c2 = 4L))

xout10.1 <- list(1L, 2L, list(c1 = -3L, c2 = -4L))
xout10.2 <- list(NULL, NULL, list(c1 = -3L, c2 = -4L))
xout10.3 <- list(list(c1 = -3L, c2 = -4L))  
xout10.4 <- list(c1 = -3L, c2 = -4L) 
xout10.5 <- structure(list(L1 = c("..3", "..3"), L2 = c("c1", "c2"), value = list(-3L, -4L)), row.names = 1:2, class = "data.frame")
xout10.6 <- list(list(c1 = -3L))
xout10.7 <- list(1L, 2L, list(c1 = "c1", c2 = "c2"))
xout10.8 <- list(list(1L), 2L, list(c1 = 3L, c2 = 4L))

dotest("10.1", rrapply(xin, f = `-`, condition = function(x, .xname) !is.na(.xname), how = "replace"), xout10.1)
dotest("10.2", rrapply(xin, f = `-`, condition = function(x, .xname) !is.na(.xname), how = "list"), xout10.2)
dotest("10.3", rrapply(xin, f = `-`, condition = function(x, .xname) !is.na(.xname), how = "prune"), xout10.3)
dotest("10.4", rrapply(xin, f = `-`, condition = function(x, .xname) !is.na(.xname), how = "flatten"), xout10.4)
dotest("10.5", rrapply(xin, f = `-`, condition = function(x, .xname) !is.na(.xname), how = "melt"), xout10.5)
dotest("10.6", rrapply(xin, f = `-`, condition = function(x, .xpos) identical(.xpos, c(3L, 1L)), how = "prune"), xout10.6)
dotest("10.7", rrapply(xin, f = function(x, .xname) .xname, condition = function(x, .xname) !is.na(.xname), feverywhere = "break"), xout10.7)
dotest("10.8", rrapply(xin, f = list, condition = function(x, .xpos) length(.xpos) < 2 & all(.xpos == 1L), feverywhere = "recurse"), xout10.8)

## empty lists
xin1 <- list(a = 1L, b = list(list(2L)))
xin2 <- list(1L)

xout11.1 <- structure(list(), .Names = character(0))
xout11.2 <- list()
xout11.3 <- structure(list(value = list()), row.names = integer(0), class = "data.frame")

dotest("11.1", rrapply(xin1, condition = function(x) FALSE, how = "prune"), xout11.1)
dotest("11.2", rrapply(xin1, condition = function(x) FALSE, how = "flatten"), xout11.1)
dotest("11.3", rrapply(xin1, condition = function(x) FALSE, how = "melt"), xout11.3)
dotest("11.4", rrapply(xin1, f = `-`, classes = "user-class", how = "prune"), xout11.1)
dotest("11.5", rrapply(xin1, f = `-`, classes = "user-class", how = "flatten"), xout11.1)
dotest("11.6", rrapply(xin1, f = `-`, classes = "user-class", how = "melt"), xout11.3)
dotest("11.7", rrapply(xin1, condition = function(x) FALSE, how = "prune", feverywhere = "break"), xout11.1)
dotest("11.8", rrapply(xin1, condition = function(x) FALSE, how = "flatten", feverywhere = "break"), xout11.1)
dotest("11.9", rrapply(xin1, condition = function(x) FALSE, how = "prune", feverywhere = "recurse"), xout11.1)
dotest("11.10", rrapply(xin1, condition = function(x) FALSE, how = "flatten", feverywhere = "recurse"), xout11.1)
dotest("11.11", rrapply(xin2, condition = function(x) FALSE, how = "prune"), xout11.2)
dotest("11.12", rrapply(xin2, condition = function(x) FALSE, how = "flatten"), xout11.2)
dotest("11.13", rrapply(xin2, condition = function(x) FALSE, how = "melt"), xout11.3)
dotest("11.14", rrapply(xin2, f = `-`, classes = "user-class", how = "prune"), xout11.2)
dotest("11.15", rrapply(xin2, f = `-`, classes = "user-class", how = "flatten"), xout11.2)
dotest("11.16", rrapply(xin2, condition = function(x) FALSE, how = "melt"), xout11.3)
dotest("11.17", rrapply(xin2, condition = function(x) FALSE, how = "prune", feverywhere = "break"), xout11.2)
dotest("11.18", rrapply(xin2, condition = function(x) FALSE, how = "flatten", feverywhere = "break"), xout11.2)
dotest("11.19", rrapply(xin2, condition = function(x) FALSE, how = "prune", feverywhere = "recurse"), xout11.2)
dotest("11.20", rrapply(xin2, condition = function(x) FALSE, how = "flatten", feverywhere = "recurse"), xout11.2)

## deeply nested lists

xin1 <- f(len = 1, d = 1, dmax = 17, expr = list(1L, NULL))
xin2 <- f(len = 2, d = 1, dmax = 4, expr = list(1L, NULL, 1L))

xout12.1 <- f(len = 1, d = 1, dmax = 17, expr = list(1L, NA))
xout12.2 <- xout12.1
xout12.3 <- xout12.2
xout12.4 <- f(len = 1, d = 1, dmax = 17, expr = list(2L))
xout12.5 <- list(2L)
xout12.6 <- xout12.5
xout12.7 <- structure(list(L1 = "..1", L2 = "..1", L3 = "..1", L4 = "..1", 
                           L5 = "..1", L6 = "..1", L7 = "..1", L8 = "..1", L9 = "..1", 
                           L10 = "..1", L11 = "..1", L12 = "..1", L13 = "..1", L14 = "..1", 
                           L15 = "..1", L16 = "..1", L17 = "..1", value = list(2L)), row.names = 1L, class = "data.frame")
xout12.8 <- f(len = 2, d = 1, dmax = 4, expr = list(1L, NA, 1L))
xout12.9 <- xout12.8
xout12.10 <- xout12.9
xout12.11 <- f(len = 2, d = 1, dmax = 4, expr = list(2L, 2L))
xout12.12 <- as.list(rep(2L, 16L))
xout12.13 <- as.list(rep(2L, 8L))
xout12.14 <- structure(list(L1 = c("..1", "..1", "..1", "..1", "..1", "..1"), 
                            L2 = c("..1", "..1", "..1", "..1", "..1", "..1"), 
                            L3 = c("..1", "..1", "..1", "..2", "..2", "..2"), 
                            L4 = c("..1", "..2", "..3", "..1", "..2", "..3"), 
                            value = list(2L, 2L, 2L, 2L, 2L, 2L)), row.names = c(NA, 6L), class = "data.frame")

dotest("12.1", rrapply(xin1, condition = is.null, f = function(x) NA, how = "replace"), xout12.1)
dotest("12.2", rrapply(xin1, condition = Negate(is.null), f = function(x) 1L, deflt = NA, how = "list"), xout12.2)
dotest("12.3", rrapply(xin1, condition = function(x, .xpos) identical(.xpos, rep(1L, 17L)), deflt = NA, how = "list"), xout12.3)
dotest("12.4", rrapply(xin1, condition = Negate(is.null), f = function(x) 2L, how = "prune"), xout12.4)
dotest("12.5", rrapply(xin1, condition = Negate(is.null), f = function(x) 2L, how = "flatten"), xout12.5)
dotest("12.6", rrapply(xin1, condition = function(x, .xpos) identical(.xpos, rep(1L, 17L)), f = function(x) 2L, how = "flatten"), xout12.6)
dotest("12.7", rrapply(xin1, condition = function(x, .xpos) identical(.xpos, rep(1L, 17L)), f = function(x) 2L, how = "melt"), xout12.7)
dotest("12.8", rrapply(xin2, condition = is.null, f = function(x) NA, how = "replace"), xout12.8)
dotest("12.9", rrapply(xin2, condition = Negate(is.null), f = function(x) 1L, deflt = NA, how = "list"), xout12.9)
dotest("12.10", rrapply(xin2, condition = function(x, .xpos) .xpos[4] %in% c(1L, 3L), deflt = NA, how = "list"), xout12.10)
dotest("12.11", rrapply(xin2, condition = Negate(is.null), f = function(x) 2L, how = "prune"), xout12.11)
dotest("12.12", rrapply(xin2, condition = Negate(is.null), f = function(x) 2L, how = "flatten"), xout12.12)
dotest("12.13", rrapply(xin2, condition = function(x, .xpos) identical(.xpos[4], 1L), f = function(x) 2L, how = "flatten"), xout12.13)
dotest("12.14", rrapply(xin2, condition = function(x, .xpos) identical(.xpos[c(1, 2)], c(1L, 1L)), f = function(x) 2L, how = "melt"), xout12.14)

cat("Completed rrapply unit tests\n")
