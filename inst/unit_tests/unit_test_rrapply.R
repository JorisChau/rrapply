require(rrapply)

cat("Running rrapply unit tests...\n")

## rrapply unit tests
dotest <- function(itest, observed, expected) {
  if(!identical(observed, expected)) stop(sprintf("Test %.1f failed", itest), call. = FALSE) 
}

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

dotest(0.1, rrapply(xin, f = `-`, classes = "ANY"), xout0.1)
dotest(0.2, rrapply(xin, f = `+`, e2 = 1L), xout0.2)

.xpos <- .xname <- 1L

dotest(0.3, rrapply(xin, f = function(x, .xpos) .xpos), xout0.3)
dotest(0.4, rrapply(xin, f = function(x, xpos) xpos, xpos = 1L), xout0.4)
dotest(0.5, rrapply(xin, f = function(x, .xname) .xname), xout0.5)
dotest(0.6, rrapply(xin, f = function(x, xname) xname, xname = "a"), xout0.6)
dotest(0.7, rrapply(xin, f = function(x, .xname, .xpos) c(.xname, .xpos)), xout0.7)
dotest(0.8, rrapply(xin, f = function(x, .xpos, .xname) c(.xname, .xpos)), xout0.8)
dotest(0.9, rrapply(xin, f = function(x, xname, xpos) c(xname, xpos), xpos = 1L, xname = "a"), xout0.9)
dotest(0.10, rrapply(xin, f = function(x, .xname, .xpos) {.xpos = 1L; .xname = "a"; c(.xname, .xpos)}), xout0.10)
    
## condition argument
xout1.1 <- list(a = -1L, b = list(b1 = 2L, b2 = 3L), c = 4L)
xout1.2 <- list(a = 0L, b = list(b1 = 2L, b2 = 3L), c = 4L)
xout1.3 <- xout1.1
xout1.4 <- xout0.1
xout1.5 <- xout1.1
xout1.6 <- xout0.1
xout1.7 <- xout1.1

dotest(1.1, rrapply(xin, f = `-`, condition = function(x) x == 1L), xout1.1)
dotest(1.2, rrapply(xin, f = `-`, condition = `==`, e2 = 1L), xout1.2)
dotest(1.3, rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos == 1L), xout1.3)
dotest(1.4, rrapply(xin, f = function(x, xpos) -x, condition = function(x, xpos) xpos == 1L, xpos = 1L), xout1.4)
dotest(1.5, rrapply(xin, f = `-`, condition = function(x, .xname) .xname == names(xin)[1]), xout1.5)
dotest(1.6, rrapply(xin, f = function(x, xname) -x, condition = function(x, xname) xname == "a", xname = "a"), xout1.6)
dotest(1.7, rrapply(xin, f = `-`, condition = function(x, .xpos, .xname) .xname == names(xin)[1] & .xpos == 1L), xout1.7)

dotest(1.8, .xpos, 1L)
dotest(1.9, .xname, 1L)
dotest(1.10, exists("X"), FALSE)
rm(.xpos, .xname)

## how argument
xout2.1 <- list(a = -1L, b = list(b1 = 2L, b2 = 3L), c = -4L)
xout2.2 <- list(a = -1L, b = list(b1 = NULL, b2 = NULL), c = -4L)
xout2.3 <- c(a = -1L, c = -4L)
xout2.4 <- as.list(xout2.3)
xout2.5 <- xout2.4
xout2.6 <- list(a = 1L, b = c(b1 = 2L, b2 = 3L), c = 4L)
xout2.7 <- list(a = NULL, b = c(b1 = 2L, b2 = 3L), c = NULL)
xout2.8 <- c(b.b1 = 2L, b.b2 = 3L)
xout2.9 <- list(b = c(b1 = 2L, b2 = 3L))
xout2.10 <- xout2.9
    
dotest(2.1, rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "replace"), xout2.1)
dotest(2.2, rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "list"), xout2.2)
dotest(2.3, rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "unlist"), xout2.3)
dotest(2.4, rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "prune"), xout2.4)
dotest(2.5, rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "flatten"), xout2.5)
dotest(2.6, rrapply(xin, f = unlist, condition = function(x, .xname) .xname == "b", how = "replace", feverywhere = TRUE), xout2.6)
dotest(2.7, rrapply(xin, f = unlist, condition = function(x, .xname) .xname == "b", how = "list", feverywhere = TRUE), xout2.7)
dotest(2.8, rrapply(xin, f = unlist, condition = function(x, .xname) .xname == "b", how = "unlist", feverywhere = TRUE), xout2.8)
dotest(2.9, rrapply(xin, f = unlist, condition = function(x, .xname) .xname == "b", how = "prune", feverywhere = TRUE), xout2.9)
dotest(2.10, rrapply(xin, f = unlist, condition = function(x, .xname) .xname == "b", how = "flatten", feverywhere = TRUE), xout2.10)

## check for trailing .xpos and .xname variables
dotest(2.6, exists(".xpos"), FALSE)
dotest(2.7, exists(".xname"), FALSE)

## classes argument
xout3.1 <- list(a = structure(-1L, .Dim = c(1L, 1L)), b = list(b1 = 2L, b2 = 3L), c = 4L)
xout3.2 <- list(a = structure(1L, .Dim = c(1L, 1L)), b = list(b1 = -2L, b2 = -3L), c = -4L)
xout3.3 <- xout0.1
xout3.4 <- list(a = structure(-1L, class = "user-class"), b = list(b1 = 2L, b2 = 3L), c = 4L)
xout3.5 <- list(a = -1, b = list(b1 = 2L, b2 = 3L), c = 4L)
xout3.6 <- list(a = -1, b = list(b1 = -2L, b2 = -3L), c = -4L)

xin[[1]] <- as.matrix(xin[[1]])
dotest(3.1, rrapply(xin, f = `-`, classes = "matrix"), xout3.1)
dotest(3.2, rrapply(xin, f = `-`, classes = "integer"), xout3.2)
xin[[1]] <- as.integer(xin[[1]])
dotest(3.3, rrapply(xin, f = `-`, classes = "integer"), xout3.3)
class(xin[[1]]) <- "user-class"
dotest(3.4, rrapply(xin, f = `-`, classes = "user-class"), xout3.4)
xin[[1]] <- as.numeric(xin[[1]])
dotest(3.5, rrapply(xin, f = `-`, classes = "numeric"), xout3.5)
dotest(3.6, rrapply(xin, f = `-`, classes = c("numeric", "integer")), xout3.6)

## deflt argument
xin <- list(a = 1L, b = list(b1 = 2L, b2 = 3L), c = 4L)
xout4.1 <- list(a = -1L, b = list(b1 = NA_character_, b2 = NA_character_), c = NA_character_)
xout4.2 <- list(a = 1L, b = list(b1 = NA_real_, b2 = NA_real_), c = NA_real_)
xout4.3 <- c(a = 1, b.b1 = NA, b.b2 = NA, c = NA)

dotest(4.1, rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos == 1L, deflt = NA_character_, how = "list"), xout4.1)
dotest(4.2, rrapply(xin, condition = function(x, .xpos) .xpos == 1L, deflt = NA_real_, how = "list"), xout4.2)
dotest(4.3, rrapply(xin, condition = function(x, .xpos) .xpos == 1L, deflt = NA_real_, how = "unlist"), xout4.3)

## dfaslist argument
xin[[1]] <- data.frame(x = 1L, y = 2L)

xout5.1 <- list(a = structure(list(x = 1L, y = 2L), class = "data.frame", row.names = c(NA, -1L)), b = list(b1 = 2L, b2 = 3L), c = 4L)
xout5.2 <- list(a = "a", b = list(b1 = 2L, b2 = 3L), c = 4L)
xout5.3 <- list(a = structure(list(x = "x", y = "y"), class = "data.frame", row.names = c(NA, -1L)), b = list(b1 = 2L, b2 = 3L), c = 4L)
xout5.4 <- xout5.2
    
dotest(5.1, rrapply(xin, f = function(x, .xname) .xname, classes = "data.frame"), xout5.1)
dotest(5.2, rrapply(xin, f = function(x, .xname) .xname, classes = "data.frame", dfaslist = FALSE), xout5.2)
dotest(5.3, rrapply(xin, f = function(x, .xname) .xname, condition = function(x, .xpos) .xpos[1] == 1L), xout5.3)
dotest(5.4, rrapply(xin, f = function(x, .xname) .xname, condition = function(x, .xpos) .xpos[1] == 1L, dfaslist = FALSE), xout5.4)

## feverywhere argument
xin <- list(a = 1L, b = list(b1 = list(b11 = 2L), b2 = 3L), c = 4L)

xout6.1 <- list(a = "a", b = "b", c = "c")
xout6.2 <- list(a = 1L, b = "b", c = 4L)
xout6.3 <- list(a = 1L, b = list(b1 = "b1", b2 = "b2"), c = 4L)
xout6.4 <- list(a = 1L, b = list(b1 = list(b11 = "b11"), b2 = 3L), c = 4L)

dotest(6.1, rrapply(xin, f = function(x, .xname) .xname, feverywhere = TRUE), xout6.1)
dotest(6.2, rrapply(xin, f = function(x, .xname) .xname, condition = function(x, .xname) .xname == "b", feverywhere = TRUE), xout6.2)
dotest(6.3, rrapply(xin, f = function(x, .xname) .xname, condition = function(x, .xpos) length(.xpos) == 2, feverywhere = TRUE), xout6.3)
dotest(6.4, rrapply(xin, f = function(x, .xname) .xname, condition = function(x, .xpos) length(.xpos) == 3, feverywhere = TRUE), xout6.4)

## named flat list
xin <- list(a = 1L, b = 2L, c = 3L)

xout7.1 <- list(a = -1L, b = 2L, c = -3L)
xout7.2 <- list(a = -1L, b = NULL, c = -3L)
xout7.3 <- list(a = -1L, c = -3L)
xout7.4 <- xout7.3
xout7.5 <- xout7.1

dotest(7.1, rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "replace"), xout7.1)
dotest(7.2, rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "list"), xout7.2)
dotest(7.3, rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "prune"), xout7.3)
dotest(7.4, rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "flatten"), xout7.4)
dotest(7.5, rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), feverywhere = TRUE), xout7.5)

## unnamed nested list
xin <- list(1L, 2L, list(3L, 4L))

xout8.1 <- list(-1L, 2L, list(3L, 4L))
xout8.2 <- list(-1L, NULL, list(NULL, NULL))
xout8.3 <- list(-1L)
xout8.4 <- xout8.3
xout8.5 <- list(-1L, -2L, list(-3L, -4L))
xout8.6 <- list(-1L, 2L, -3L)

dotest(8.1, rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "replace"), xout8.1)
dotest(8.2, rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "list"), xout8.2)
dotest(8.3, rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "prune"), xout8.3)
dotest(8.4, rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "flatten"), xout8.4)
dotest(8.5, rrapply(xin, f = `-`, condition = function(x, .xname) is.na(.xname)), xout8.5)
dotest(8.6, rrapply(xin, f = function(x, .xpos) -.xpos, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), feverywhere = TRUE), xout8.6)

## partially named list 1
xin <- list(a = 1L, 2L, list(c1 = 3L, 4L))

xout9.1 <- list(a = -1L, 2L, list(c1 = -3L, 4L))
xout9.2 <- list(a = -1L, NULL, list(c1 = -3L, NULL))
xout9.3 <- list(a = -1L, list(c1 = -3L))
xout9.4 <- list(a = -1L, c1 = -3L)
xout9.5 <- list(a = -1L)
xout9.6 <- list(a = "a", 2L, list(c1 = "c1", 4L))

dotest(9.1, rrapply(xin, f = `-`, condition = function(x, .xname) nzchar(.xname), how = "replace"), xout9.1)
dotest(9.2, rrapply(xin, f = `-`, condition = function(x, .xname) nzchar(.xname), how = "list"), xout9.2)
dotest(9.3, rrapply(xin, f = `-`, condition = function(x, .xname) nzchar(.xname), how = "prune"), xout9.3)
dotest(9.4, rrapply(xin, f = `-`, condition = function(x, .xname) nzchar(.xname), how = "flatten"), xout9.4)
dotest(9.5, rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos == 1L, how = "prune"), xout9.5)
dotest(9.6, rrapply(xin, f = function(x, .xname) .xname, condition = function(x, .xname) nzchar(.xname), feverywhere = TRUE), xout9.6)

## partially named list 2
xin <- list(1L, 2L, list(c1 = 3L, c2 = 4L))

xout10.1 <- list(1L, 2L, list(c1 = -3L, c2 = -4L))
xout10.2 <- list(NULL, NULL, list(c1 = -3L, c2 = -4L))
xout10.3 <- list(list(-3L, -4L))  ## no names present on L1 so no names returned
xout10.4 <- list(-3L, -4L)  ## no names present on L1 so no names returned
xout10.5 <- list(list(-3L))
xout10.6 <- list(1L, 2L, list(c1 = "c1", c2 = "c2"))

dotest(10.1, rrapply(xin, f = `-`, condition = function(x, .xname) !is.na(.xname), how = "replace"), xout10.1)
dotest(10.2, rrapply(xin, f = `-`, condition = function(x, .xname) !is.na(.xname), how = "list"), xout10.2)
dotest(10.3, rrapply(xin, f = `-`, condition = function(x, .xname) !is.na(.xname), how = "prune"), xout10.3)
dotest(10.4, rrapply(xin, f = `-`, condition = function(x, .xname) !is.na(.xname), how = "flatten"), xout10.4)
dotest(10.5, rrapply(xin, f = `-`, condition = function(x, .xpos) identical(.xpos, c(3L, 1L)), how = "prune"), xout10.5)
dotest(10.6, rrapply(xin, f = function(x, .xname) .xname, condition = function(x, .xname) !is.na(.xname), feverywhere = TRUE), xout10.6)

## empty lists
xin1 <- list(a = 1L, b = list(list(2L)))
xin2 <- list(1L)

xout11.1 <- structure(list(), .Names = character(0))
xout11.2 <- list()

dotest(11.1, rrapply(xin1, condition = function(x) FALSE, how = "prune"), xout11.1)
dotest(11.2, rrapply(xin1, condition = function(x) FALSE, how = "flatten"), xout11.1)
dotest(11.3, rrapply(xin1, f = `-`, classes = "user-class", how = "prune"), xout11.1)
dotest(11.4, rrapply(xin1, f = `-`, classes = "user-class", how = "flatten"), xout11.1)
dotest(11.5, rrapply(xin1, condition = function(x) FALSE, how = "prune", feverywhere = TRUE), xout11.1)
dotest(11.6, rrapply(xin1, condition = function(x) FALSE, how = "flatten", feverywhere = TRUE), xout11.1)
dotest(11.7, rrapply(xin2, condition = function(x) FALSE, how = "prune"), xout11.2)
dotest(11.8, rrapply(xin2, condition = function(x) FALSE, how = "flatten"), xout11.2)
dotest(11.9, rrapply(xin2, f = `-`, classes = "user-class", how = "prune"), xout11.2)
dotest(11.10, rrapply(xin2, f = `-`, classes = "user-class", how = "flatten"), xout11.2)
dotest(11.11, rrapply(xin2, condition = function(x) FALSE, how = "prune", feverywhere = TRUE), xout11.2)
dotest(11.12, rrapply(xin2, condition = function(x) FALSE, how = "flatten", feverywhere = TRUE), xout11.2)

cat("Completed rrapply unit tests\n")
