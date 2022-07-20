require(rrapply)

cat("Running rrapply unit tests...\n")

## check test
dotest <- function(itest, observed, expected) {
  if(!identical(observed, expected)) stop(sprintf("Test %s failed", itest), call. = FALSE) 
}

## input objects
xin <- list(a = 1L, b = list(b1 = 2L, b2 = 3L), c = 4L)
xin1 <- quote(f1(a = 1L, b = f2(b1 = 2L, b2 = 3L), c = 4L))
xin2 <- expression(a <- 1L, expression(b))
xin3 <- list(par = pairlist(a = 1L, b = 2L))
xin4 <- list(a = list(a1 = 1L, a2 = 2L), a = list(a2 = 2),  a = list(1L, 2L))

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
xout0.11 <- list(a = "a", b = list(b1 = c("b", "b1"), b2 = c("b", "b2")), c = "c")
xout0.12 <- list(a = c("a", "1", "a"), b = list(b1 = c("b1", "2", "1", "b", "b1"), 
                                                b2 = c("b2", "2", "2", "b", "b2")), c = c("c", "3", "c"))
xout0.13 <- list(c("", "1", ""), a = c("a", "2", "a"), b = list(c("", "3", "1", "b", ""), 
                                                                b1 = c("b1", "3", "2", "b", "b1"), b2 = c("b2", "3", "3", "b", "b2")), c = c("c", "4", "c"))
xout0.14 <- list(list(c(1L, 1L), 1:2, c(1L, 3L)), list(2:1, c(2L, 2L)))
xout0.15 <- list(a = structure(1:4, .Names = c("a", "b.b1", "b.b2", "c")), 
                 b = list(b1 = structure(2:3, .Names = c("b1", "b2")), b2 = structure(2:3, .Names = c("b1", "b2"))), 
                 c = structure(1:4, .Names = c("a", "b.b1", "b.b2", "c")))
xout0.16 <- list(a = c("3", "a", "1", "a"), b = list(b1 = c("2", "b1", "2", "1", "b", "b1"), 
                                                     b2 = c("2", "b2", "2", "2", "b", "b2")), c = c("3", "c", "3", "c"))
xout0.17 <- list(par = list(a = list(a = 1L, b = 2L), b = list(a = 1L, b = 2L)))

dotest("0.1", rrapply(xin, f = `-`, classes = "ANY"), xout0.1)
dotest("0.2", rrapply(xin, f = `+`, e2 = 1L), xout0.2)

.xpos <- .xname <- .xparents <-  1L

dotest("0.3", rrapply(xin, f = function(x, .xpos) .xpos), xout0.3)
dotest("0.4", rrapply(xin, f = function(x, xpos) xpos, xpos = 1L), xout0.4)
dotest("0.5", rrapply(xin, f = function(x, .xname) .xname), xout0.5)
dotest("0.6", rrapply(xin, f = function(x, xname) xname, xname = "a"), xout0.6)
dotest("0.7", rrapply(xin, f = function(x, .xname, .xpos) c(.xname, .xpos)), xout0.7)
dotest("0.8", rrapply(xin, f = function(x, .xpos, .xname) c(.xname, .xpos)), xout0.8)
dotest("0.9", rrapply(xin, f = function(x, xname, xpos) c(xname, xpos), xpos = 1L, xname = "a"), xout0.9)
dotest("0.10", rrapply(xin, f = function(x, .xname, .xpos) {.xpos = 1L; .xname = "a"; c(.xname, .xpos)}), xout0.10)
dotest("0.11", rrapply(xin, f = function(x, .xparents) .xparents), xout0.11)
dotest("0.12", rrapply(xin, f = function(x, .xparents, .xpos, .xname) c(.xname, .xpos, .xparents)), xout0.12)
dotest("0.13", rrapply(xin1, f = function(x, .xparents, .xpos, .xname) c(.xname, .xpos, .xparents), how = "list"), xout0.13)
dotest("0.14", rrapply(xin2, f = function(x, .xpos) .xpos, how = "list"), xout0.14)
dotest("0.15", rrapply(xin, f = function(x, .xsiblings) unlist(.xsiblings)), xout0.15)
dotest("0.16", rrapply(xin, f = function(x, .xsiblings, .xparents, .xpos, .xname) c(length(.xsiblings), .xname, .xpos, .xparents)), xout0.16)
dotest("0.17", rrapply(xin3, f = function(x, .xsiblings) .xsiblings, how = "list"), xout0.17) 

## condition argument
xout1.1 <- list(a = -1L, b = list(b1 = 2L, b2 = 3L), c = 4L)
xout1.2 <- list(a = 0L, b = list(b1 = 2L, b2 = 3L), c = 4L)
xout1.3 <- xout1.1
xout1.4 <- xout0.1
xout1.5 <- xout1.1
xout1.6 <- xout0.1
xout1.7 <- xout1.1
xout1.8 <- list(a = 1L, b = list(b1 = -2L, b2 = -3L), c = 4L)
xout1.9 <- xout0.1
xout1.10 <- xout0.1
xout1.11 <- list(NULL, a = -1L, b = list(NULL, b1 = -2L, b2 = -3L), c = -4L)
xout1.12 <- list(list(quote(`<-`), NULL, NULL), list(quote(expression), NULL))
xout1.13 <- xout0.1

dotest("1.1", rrapply(xin, f = `-`, condition = function(x) x == 1L), xout1.1)
dotest("1.2", rrapply(xin, f = `-`, condition = `==`, e2 = 1L), xout1.2)
dotest("1.3", rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos == 1L), xout1.3)
dotest("1.4", rrapply(xin, f = function(x, xpos) -x, condition = function(x, xpos) xpos == 1L, xpos = 1L), xout1.4)
dotest("1.5", rrapply(xin, f = `-`, condition = function(x, .xname) .xname == names(xin)[1]), xout1.5)
dotest("1.6", rrapply(xin, f = function(x, xname) -x, condition = function(x, xname) xname == "a", xname = "a"), xout1.6)
dotest("1.7", rrapply(xin, f = `-`, condition = function(x, .xpos, .xname) .xname == names(xin)[1] & .xpos == 1L), xout1.7)
dotest("1.8", rrapply(xin, f = `-`, condition = function(x, .xparents) any(.xparents == "b")), xout1.8)
dotest("1.9", rrapply(xin, f = `-`, condition = function(x, .xparents, .xname) .xparents[length(.xparents)] == .xname), xout1.9)
dotest("1.10", rrapply(xin, f = `-`, condition = function(x, .xparents, .xname, .xpos) .xparents[length(.xpos)] == .xname), xout1.10)
dotest("1.11", rrapply(xin1, f = `-`, condition = function(x, .xparents, .xname, .xpos) nzchar(.xparents[length(.xpos)]) & nzchar(.xname), how = "list"), xout1.11)
dotest("1.12", rrapply(xin2, f = identity, condition = function(x, .xpos) .xpos[2] == 1L, how = "list"), xout1.12)
dotest("1.13", rrapply(xin, f = `-`, condition = function(x, .xsiblings, .xparents, .xname, .xpos) names(.xsiblings)[.xpos[length(.xpos)]] == .xparents[length(.xparents)]), xout1.13)

dotest("1.14", .xpos, 1L)
dotest("1.15", .xname, 1L)
dotest("1.16", .xparents, 1L)
dotest("1.17", exists("X"), FALSE)
rm(.xpos, .xname, .xparents)

## how argument
xin5 <- list(l1 = unname(xin4))
xin6 <- list(list(a1 = 1L, a2 = 2L), list(a1 = 1L, a2 = 2L))

xout2.1 <- list(a = -1L, b = list(b1 = 2L, b2 = 3L), c = -4L)
xout2.2 <- list(a = -1L, b = list(b1 = NULL, b2 = NULL), c = -4L)
xout2.3 <- c(a = -1L, c = -4L)
xout2.4 <- as.list(xout2.3)
xout2.5 <- xout2.3
xout2.6 <- structure(list(L1 = c("b", "b"), L2 = c("b1", "b2"), value = c(-2L, -3L)), row.names = 1:2, class = "data.frame")
xout2.7 <- structure(list(L1 = c("a", "c"), value = c(-1L, -4L)), row.names = 1:2, class = "data.frame")
xout2.8 <- structure(list(L1 = c("a", "b"), L2 = c(NA, "b1"), value = c(-1L, -2L)), row.names = 1:2, class = "data.frame")
xout2.9 <- xout2.8
xout2.10 <- list(b = list(b1 = 2L, b2 = 3L))
xout2.11 <- list(a = 1L, c = 4L)
xout2.12 <- list(a = 1L, b = list(b1 = 2L))
xout2.13 <- quote(x(a = 1L, b = x(b1 = 2L, b2 = 3L), c = 4L))
xout2.14 <- list(quote(f1), a = 1L, b = list(quote(f2), b1 = 2L, b2 = 3L), c = 4L)
xout2.15 <- list(a = 1L, b = list(b1 = 2L, b2 = 3L), c = 4L)
xout2.16 <- c(a = 1L, b1 = 2L, b2 = 3L, c = 4L)
xout2.17 <- structure(list(L1 = c("a", "b", "b", "c"), L2 = c(NA, "b1", "b2", NA), value = c(1L, 2L, 3L, 4L)), row.names = c(NA, 4L), class = "data.frame")
xout2.18 <- expression(x(x, 1L), x(x))
xout2.19 <- list(list(quote(`<-`), quote(a), 1L), list(quote(expression), quote(b)))
xout2.20 <- list(list(1L))
xout2.21 <- 1L
xout2.22 <- structure(list(L1 = "1", L2 = "3", value = 1L), row.names = 1L, class = "data.frame")
xout2.23 <- structure(list(L1 = c("a", "b", "b", "c"), L2 = c(NA, "b1", "b2", NA), 
                           value = c(-1L, -2L, -3L, -4L)), row.names = c(NA, 4L), class = "data.frame")
xout2.24 <- structure(list(a1 = c(-1L, NA, NA), a2 = c(-2, -2, NA), `1` = c(NA, NA, -1L),
                           `2` = c(NA, NA, -2L)), row.names = c(NA, 3L), class = "data.frame")
xout2.25 <- structure(list(a1 = c(-1L, NA), a2 = c(-2, -2)), row.names = c(NA, 2L), class = "data.frame")
xout2.26 <- structure(list(b.b1 = -2L), row.names = 1L, class = "data.frame")
xout2.27 <- structure(list(a = 1L, b.b1 = 2L, b.b2 = 3L, c = 4L), row.names = 1L, class = "data.frame")
xout2.28 <- structure(list(a = 1L, b = 2L), row.names = 1L, class = "data.frame")
xout2.29 <- list(a = list(1L, "a", a = 1L, b = list(b1 = 2L, b2 = 3L), c = 4L), b1 = list(2L, 1L, "b", "b1", b1 = 2L, b2 = 3L), 
                 b2 = list(2L, 2L, "b", "b2", b1 = 2L, b2 = 3L), c = list(3L, "c", a = 1L, b = list(b1 = 2L, b2 = 3L), c = 4L))
xout2.30 <- structure(list(a1 = c(1L, NA, NA), a2 = c(2, 2, NA), `1` = c(NA, NA, 1L), `2` = c(NA, NA, 2L)), 
                      row.names = c(NA, 3L), class = "data.frame")
xout2.31 <- structure(list(a1 = c(1L, 1L), a2 = c(2L, 2L)), row.names = 1:2, class = "data.frame")
xout2.32 <- list(a_ = 1L, b_ = list(b1_ = 2L, b2_ = 3L), c_ = 4L)
xout2.33 <- list(`_` = quote(f1), a_ = 1L, b_ = list(`_` = quote(f2), b1_ = 2L, b2_ = 3L), c_ = 4L)
xout2.34 <- list(`1_` = list(`1_` = quote(`<-`), `2_` = quote(a), `3_` = 1L), `2_` = list(`1_` = quote(expression), `2_` = quote(b)))
xout2.35 <- list(par_ = list(a_ = 1L, b_ = 2L))
xout2.36 <- list(a_ = list(a1_ = 1L, a2_ = 2L), a_ = list(a2_ = 2), a_ = list(`1_` = 1L, `2_` = 2L))
xout2.37 <- list(a = 1L, `2_` = list(b1 = 2L, b2 = 3L), c = 4L)
xout2.38 <- list(`1_` = list(a = 1L, b = 2L))
xout2.39 <- list(`1_` = list(a1 = 1L, a2 = 2L), `2_` = list(a2 = 2), `3_` = structure(list(1L, 2L), names = c("", "")))
xout2.40 <- structure(1:4, names = c("a", "b.b1", "b.b2", "c"))
xout2.41 <- list(a = 1L, b1 = 2L, b2 = 3L, c = 4L)
xout2.42 <- list(par.a = 1L, par.b = 2L)
xout2.43 <- list(b.b1 = 2L, b.b2 = 3L)
xout2.44 <- structure(list(L1 = c("b", "b"), L2 = c("b1", "b2"), value = list(2L, 3L)), row.names = 1:2, class = "data.frame")
xout2.45 <- structure(list(L1 = "par", L2 = "a", value = 1L), row.names = 1L, class = "data.frame")
xout2.46 <- structure(list(L1 = c("l1", "l1"), L2 = c("1", "2"), a1 = c(1L, NA), a2 = c(2, 2)), row.names = 1:2, class = "data.frame")
xout2.47 <- structure(list(), names = character(0))
xout2.48 <- structure(list(L1 = NULL, L2 = NULL, L3 = NULL), row.names = integer(0), class = "data.frame")
xout2.49 <- structure(list(l1.1.a1 = 1L, l1.1.a2 = 2L, l1.2.a2 = 2), row.names = 1L, class = "data.frame")
xout2.50 <- structure(list(L1 = "l1", `1_a1` = 1L, `1_a2` = 2L, `2_a2` = 2), row.names = 1L, class = "data.frame")
xout2.51 <- list(a = 1L, `2` = list(b1 = 2L, b2 = 3L), c = 4L)

dotest("2.1", rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "replace"), xout2.1)
dotest("2.2", rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "list"), xout2.2)
dotest("2.3", rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "unlist"), xout2.3)
dotest("2.4", rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "prune"), xout2.4)
dotest("2.5", rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "flatten"), xout2.5)
dotest("2.6", rrapply(xin, f = `-`, condition = function(x, .xpos) length(.xpos) > 1, how = "melt"), xout2.6)
dotest("2.7", rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "melt"), xout2.7)
dotest("2.8", rrapply(xin, f = `-`, condition = function(x, .xname) grepl("a|b1", .xname), how = "melt"), xout2.8)
dotest("2.9", rrapply(xin, f = `-`, condition = function(x, .xparents, .xname) any(grepl("a|b1", .xparents)), how = "melt"), xout2.9)
dotest("2.10", rrapply(xout2.6, f = `-`, how = "unmelt"), xout2.10)
dotest("2.11", rrapply(xout2.7, f = `-`, how = "unmelt"), xout2.11)
dotest("2.12", rrapply(xout2.8, f = `-`, how = "unmelt"), xout2.12)
dotest("2.13", rrapply(xin1, f = function(x) quote(x), condition = is.name, how = "replace"), xout2.13)
dotest("2.14", rrapply(xin1, f = identity, condition = function(x) TRUE, how = "list"), xout2.14)
dotest("2.15", rrapply(xin1, f = identity, condition = is.integer, how = "prune"), xout2.15)
dotest("2.16", rrapply(xin1, f = identity, condition = is.integer, how = "flatten"), xout2.16)
dotest("2.17", rrapply(xin1, f = identity, condition = is.integer, how = "melt"), xout2.17)
dotest("2.18", rrapply(xin2, f = function(x) quote(x), condition = is.name, how = "replace"), xout2.18)
dotest("2.19", rrapply(xin2, f = identity, condition = function(x) TRUE, how = "list"), xout2.19)
dotest("2.20", rrapply(xin2, f = identity, condition = is.integer, how = "prune"), xout2.20)
dotest("2.21", rrapply(xin2, f = identity, condition = is.integer, how = "flatten"), xout2.21)
dotest("2.22", rrapply(xin2, f = identity, condition = is.integer, how = "melt"), xout2.22)
dotest("2.23", rrapply(xin, f = `-`, condition = function(x, .xsiblings, .xname) any(grepl("a|b1", names(.xsiblings))), how = "melt"), xout2.23)
dotest("2.24", rrapply(xin4, f = `-`, condition = function(x, .xpos) length(.xpos) > 1, how = "bind"), xout2.24)
dotest("2.25", rrapply(xin4, f = `-`, condition = function(x, .xname) grepl("a", .xname), how = "bind"), xout2.25)
dotest("2.26", rrapply(xin, f = `-`, condition = function(x, .xparents, .xname) any(grepl("1", .xparents)), how = "bind"), xout2.26)
dotest("2.27", rrapply(xin1, f = identity, condition = is.integer, how = "bind"), xout2.27)
dotest("2.28", rrapply(xin3, f = identity, condition = function(x, .xsiblings) any(grepl("a", names(.xsiblings))), how = "bind"), xout2.28)
dotest("2.29", rrapply(xin, f = function(x, .xpos, .xparents, .xsiblings) c(.xpos, .xparents, .xsiblings), how = "flatten"), xout2.29)
dotest("2.30", rrapply(xin5, how = "bind"), xout2.30)
dotest("2.31", rrapply(xin6, how = "bind"), xout2.31)

dotest("2.32", rrapply(xin, f = function(x, .xname) paste0(.xname, "_"), how = "names"), xout2.32)
dotest("2.33", rrapply(xin1, f = function(x, .xname) paste0(.xname, "_"), how = "names"), xout2.33)
dotest("2.34", rrapply(xin2, f = function(x, .xname) paste0(.xname, "_"), how = "names"), xout2.34)
dotest("2.35", rrapply(xin3, f = function(x, .xname) paste0(.xname, "_"), how = "names"), xout2.35)
dotest("2.36", rrapply(xin4, f = function(x, .xname) paste0(.xname, "_"), how = "names"), xout2.36)
dotest("2.37", rrapply(xin, f = function(x, .xpos) paste0(.xpos[length(.xpos)], "_"), condition = is.list, how = "names"), xout2.37)
dotest("2.38", rrapply(xin3, f = function(x, .xpos) paste0(.xpos[length(.xpos)], "_"), condition = is.list, how = "names"), xout2.38)
dotest("2.39", rrapply(xin4, f = function(x, .xpos) paste0(.xpos[length(.xpos)], "_"), condition = is.list, how = "names"), xout2.39)

dotest("2.40", rrapply(xin, how = "flatten", options = list(namesep = ".")), xout2.40)
dotest("2.41", rrapply(xin, how = "flatten", options = list(simplify = FALSE)), xout2.41)
dotest("2.42", rrapply(xin3, how = "flatten", options = list(namesep = ".", simplify = FALSE)), xout2.42)
dotest("2.43", rrapply(xin, condition = function(x, .xpos) length(.xpos) > 1, how = "flatten", options = list(namesep = ".", simplify = FALSE)), xout2.43)
dotest("2.44", rrapply(xin, condition = function(x, .xpos) length(.xpos) > 1, how = "melt", options = list(simplify = FALSE)), xout2.44)
dotest("2.45", rrapply(xin3, condition = function(x, .xname) .xname == "a", how = "melt", options = list(simplify = TRUE)), xout2.45)
dotest("2.46", rrapply(xin5, condition = function(x, .xname) grepl("a", .xname), how = "bind", options = list(namecols = TRUE)), xout2.46)
dotest("2.47", rrapply(xin5, how = "bind", options = list(coldepth = 4)), xout2.47)
dotest("2.48", rrapply(xin5, how = "bind", options = list(namecols = TRUE, coldepth = 4)), xout2.48)
dotest("2.49", rrapply(xin5, condition = function(x, .xname) grepl("a", .xname), how = "bind", options = list(namecols = TRUE, coldepth = 1)), xout2.49)
dotest("2.50", rrapply(xin5, condition = function(x, .xname) grepl("a", .xname), how = "bind", options = list(namesep = "_", namecols = TRUE, coldepth = 2)), xout2.50)
dotest("2.51", rrapply(xin, f = identity, condition = is.list, how = "names"), xout2.51)

## check for trailing special arguments
dotest("2.32", exists(".xpos"), FALSE)
dotest("2.33", exists(".xname"), FALSE)
dotest("2.34", exists(".xparents"), FALSE)
dotest("2.35", exists(".xsiblings"), FALSE)

## classes argument
xout3.1 <- list(a = structure(-1L, .Dim = c(1L, 1L)), b = list(b1 = 2L, b2 = 3L), c = 4L)
xout3.2 <- list(a = structure(1L, .Dim = c(1L, 1L)), b = list(b1 = -2L, b2 = -3L), c = -4L)
xout3.3 <- list(a = array(-1L, dim = c(1L, 1L, 1L)), b = list(b1 = 2L, b2 = 3L), c = 4L)
xout3.4 <- xout0.1
xout3.5 <- list(a = structure(-1L, class = "user-class"), b = list(b1 = 2L, b2 = 3L), c = 4L)
xout3.6 <- list(a = -1, b = list(b1 = 2L, b2 = 3L), c = 4L)
xout3.7 <- list(a = -1, b = list(b1 = -2L, b2 = -3L), c = -4L)
xout3.8 <- quote(f1(a = 1L, b = f2(b1 = 2L, b2 = 3L), c = 4L))
xout3.8[[2]] <- as.matrix(-xout3.8[[2]])
xout3.9 <- xout3.8
class(xout3.9[[2]]) <- "user-class"
xout3.10 <- expression(a <- 1L, expression(b))
xout3.10[[1]][[3]] <- as.matrix(-xout3.10[[1]][[3]])
xout3.11 <- expression(a <- 1L, expression(b))
xout3.11[[1]][[3]] <- as.matrix(xout3.11[[1]][[3]])
xout3.12 <- xout3.10
class(xout3.12[[1]][[3]]) <- "user-class"
xout3.13 <- list(a = quote(x), b = list(b1 = 2L, b2 = 3L), c = 4L)
xout3.14 <- list(a = list(a1 = "a1"), b = list(b1 = 2L, b2 = 3L), c = 4L)
xout3.15 <- xout3.14
xout3.16 <- c(b1 = "b1", b2 = "b2", c = "c")
xout3.17 <- 4L
xout3.18 <- structure(list(L1 = "c", value = 4L), row.names = 1L, class = "data.frame")
xout3.19 <- quote(f1(a = 1L, b = f2(b1 = 2L, b2 = 3L), c = 4L))
xout3.20 <- expression(a <- 1L, expression(b))
xout3.21 <- structure(list(a.a1 = list(structure(list(x = 1L, y = 2L), class = "data.frame", row.names = c(NA, -1L)))), row.names = 1L, class = "data.frame")
xout3.22 <- c(a1 = 1, a2 = 2, a2 = 2)
xout3.23 <- structure(list(L1 = "a", a1 = list(structure(list(x = 1L, y = 2L), class = "data.frame", row.names = c(NA, -1L)))), row.names = 1L, class = "data.frame")
xout3.24 <- list(a1 = 1L, a2  = 2L, a2 = 2)
xout3.25 <- list(l1.1.a1 = 1L, l1.1.a2 = 2L, l1.2.a2 = 2)
xout3.26 <- structure(list(L1 = "a", L2 = "a1", 
                           value = list(structure(list(x = 1L, y = 2L), class = "data.frame", 
                                                  row.names = c(NA, -1L)))), row.names = 1L, class = "data.frame")
xout3.27 <- list(a = list(a1_ = structure(list(x = 1L, y = 2L), class = "data.frame", row.names = c(NA,-1L))), b = list(b1 = 2L, b2 = 3L), c = 4L)

xin[[1]] <- as.matrix(xin[[1]])
dotest("3.1", rrapply(xin, f = `-`, classes = "matrix"), xout3.1)
dotest("3.2", rrapply(xin, f = `-`, classes = "integer"), xout3.2)
xin[[1]] <- array(xin[[1]], dim = c(1L, 1L, 1L))
dotest("3.3", rrapply(xin, f = `-`, classes = "array"), xout3.3)
xin[[1]] <- as.integer(xin[[1]])
dotest("3.4", rrapply(xin, f = `-`, classes = "integer"), xout3.4)
class(xin[[1]]) <- "user-class"
dotest("3.5", rrapply(xin, f = `-`, classes = "user-class"), xout3.5)
xin[[1]] <- as.numeric(xin[[1]])
dotest("3.6", rrapply(xin, f = `-`, classes = "numeric"), xout3.6)
dotest("3.7", rrapply(xin, f = `-`, classes = c("numeric", "integer")), xout3.7)
xin1[[2]] <- as.matrix(xin1[[2]])
dotest("3.8", rrapply(xin1, f = `-`, classes = "matrix"), xout3.8)
class(xin1[[2]]) <- "user-class"
dotest("3.9", rrapply(xin1, f = `-`, classes = "user-class"), xout3.9)
xin2[[1]][[3]] <- as.matrix(xin2[[1]][[3]])
dotest("3.10", rrapply(xin2, f = `-`, classes = "matrix"), xout3.10)
dotest("3.11", rrapply(xin2, f = `-`, classes = "integer"), xout3.11)
class(xin2[[1]][[3]]) <- "user-class"
dotest("3.12", rrapply(xin2, f = `-`, classes = "user-class"), xout3.12)
xin[[1]] <- function(x) x
dotest("3.13", rrapply(xin, f = body, classes = "function"), xout3.13)
xin[[1]] <- list(a1 = data.frame(x = 1L, y = 2L))
xin1[[2]] <- data.frame(x = 1L, y = 2L)
xin2[[1]][[3]] <- data.frame(x = 1L, y = 2L)
xin3 <- list(data.frame(x = 1L, y = 2L), 4L)
dotest("3.14", rrapply(xin, f = function(x, .xname) .xname, classes = "data.frame"), xout3.14)
dotest("3.15", rrapply(xin, f = function(x, .xname) .xname, condition = function(x, .xpos) .xpos[1] == 1L, classes = "data.frame"), xout3.15)
dotest("3.16", rrapply(xin, condition = Negate(is.data.frame), f = function(x, .xname) .xname, classes = c("data.frame", "ANY"), how = "flatten"), xout3.16)
dotest("3.17", rrapply(xin3, condition = function(x, .xpos) all(.xpos > 1) , classes = c("data.frame", "ANY"), how = "flatten"), xout3.17)
dotest("3.18", rrapply(xin, condition = function(x, .xpos) all(.xpos > 2), classes = c("data.frame", "ANY"), how = "melt"), xout3.18)
dotest("3.19", rrapply(xin1, f = function(x) 1L, classes = "data.frame"), xout3.19)
dotest("3.20", rrapply(xin2, f = function(x) 1L, classes = "data.frame"), xout3.20)
dotest("3.21", rrapply(xin, condition = function(x, .xpos) all(.xpos < 2), classes = c("data.frame", "ANY"), how = "bind"), xout3.21)
dotest("3.22", rrapply(xin5, condition = function(x, .xname) grepl("a", .xname), classes = c("list", "ANY"), how = "flatten"), xout3.22)
dotest("3.23", rrapply(xin, condition = function(x, .xname) grepl("a", .xname), classes = c("data.frame", "ANY"), how = "bind", options = list(namecols = TRUE, coldepth = 2)), xout3.23)
dotest("3.24", rrapply(xin5, condition = function(x, .xname) grepl("a", .xname), classes = c("list", "ANY"), how = "flatten", options = list(simplify = FALSE)), xout3.24)
dotest("3.25", rrapply(xin5, condition = function(x, .xname) grepl("a", .xname), classes = c("list", "ANY"), how = "flatten", options = list(namesep = ".", simplify = FALSE)), xout3.25)
dotest("3.26", rrapply(xin, condition = function(x, .xname) grepl("a", .xname), classes = c("data.frame", "ANY"), how = "melt", options = list(simplify = FALSE)), xout3.26)
dotest("3.27", rrapply(xin, f = function(x, .xname) paste0(.xname, "_"), classes = "data.frame", how = "names"), xout3.27)

## deflt argument
xin <- list(a = 1L, b = list(b1 = 2L, b2 = 3L), c = 4L)
xin1 <- quote(f1(a = 1L, b = f2(b1 = 2L, b2 = 3L), c = 4L))
xin2 <- expression(a <- 1L, expression(b))

xout4.1 <- list(a = -1L, b = list(b1 = NA_character_, b2 = NA_character_), c = NA_character_)
xout4.2 <- list(a = 1L, b = list(b1 = NA_real_, b2 = NA_real_), c = NA_real_)
xout4.3 <- c(a = 1, b.b1 = NA, b.b2 = NA, c = NA)
xout4.4 <- list(quote(f), a = 1L, b = list(quote(f), b1 = 2L, b2 = 3L), c = 4L)
xout4.5 <- list(quote(f), a = 1L, b = quote(f), b.b1 = 2L, b.b2 = 3L, c = 4L)
xout4.6 <- list(list(quote(`<-`), quote(a), NA_real_), list(quote(expression), quote(b)))
xout4.7 <- list(quote(`<-`), quote(a), NA_real_, quote(expression), quote(b))

dotest("4.1", rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos == 1L, deflt = NA_character_, how = "list"), xout4.1)
dotest("4.2", rrapply(xin, condition = function(x, .xpos) .xpos == 1L, deflt = NA_real_, how = "list"), xout4.2)
dotest("4.3", rrapply(xin, condition = function(x, .xpos) .xpos == 1L, deflt = NA_real_, how = "unlist"), xout4.3)
dotest("4.4", rrapply(xin1, f = identity, condition = is.integer, deflt = quote(f), how = "list"), xout4.4)
dotest("4.5", rrapply(xin1, f = identity, condition = is.integer, deflt = quote(f), how = "unlist"), xout4.5)
dotest("4.6", rrapply(xin2, f = identity, condition = is.name, deflt = NA_real_, how = "list"), xout4.6)
dotest("4.7", rrapply(xin2, f = identity, condition = is.name, deflt = NA_real_, how = "unlist"), xout4.7)

## feverywhere argument
xin1 <- list(a = 1L, b = list(b1 = list(b11 = 2L), b2 = 3L), c = 4L)
xin2 <- list(a = 1L, b = list(b1 = 2L, b2 = 3L), c = 4L)
xin3 <- quote(f1(a = 1L, b = f2(b1 = 2L, b2 = 3L), c = 4L))
xin4 <- list(expression(a <- 1L), expression(...))
xin5 <- list(par = pairlist(a = 1L, b = 2L))
xin6 <- list(1L, 2L, 3L)
xin7 <- list(data.frame(a = I(data.frame(b = I(data.frame(c = 1L))))))

xout6.1 <- list(a = 1L, b = "b", c = 4L)
xout6.2 <- list(a = "a", b = "b", c = "c")
xout6.3 <- xout6.1
xout6.4 <- list(a = 1L, b = list(b1 = "b1", b2 = "b2"), c = 4L)
xout6.5 <- list(a = 1L, b = list(b1 = list(b11 = "b11"), b2 = 3L), c = 4L)

xout6.6 <- list(a = 1L, b = c(b1 = 2L, b2 = 3L), c = 4L)
xout6.7 <- list(a = NULL, b = c(b1 = 2L, b2 = 3L), c = NULL)
xout6.8 <- c(b.b1 = 2L, b.b2 = 3L)
xout6.9 <- list(b = c(b1 = 2L, b2 = 3L))
xout6.10 <- xout6.9
xout6.11 <- structure(list(L1 = "b", value = list(structure(2:3, .Names = c("b1", "b2")))), row.names = 1L, class = "data.frame")

xout6.12 <- list(a = 1L, b = list(b1_ = 2L, b2_ = 3L), c = 4L)
xout6.13 <- list(a = list(list(1L)), b = list(b1 = 2L, b2 = 3L), c = 4L)

xout6.14 <- xout6.2
xout6.15 <- list(a = 1L, b = list(b1 = list(b11 = c("b", "b1", "b11")), b2 = 3L), c = 4L)
xout6.16 <- list(a = 1L, b = list(b = list(b = list(b1 = 2L, b2 = 3L))), c = 4L)

xout6.17 <- quote(f1(a = 1L, b = "b", c = 4L))
xout6.18 <- quote(f1(a = 1L, b = f(b1 = 2L, b2 = 3L), c = 4L))
xout6.19 <- list(list(quote(a <- 1L)), list(quote(...)))
xout6.20 <- list(expression(f(a, 1L)), expression(...))
xout6.21 <- list(par = list(a = 1L, b = 2L))
xout6.22 <- list(par = list(a = list(1L), b = list(2L)))

xout6.23 <- list(a = 1L, b = structure(1:4, .Names = c("a", "b.b1.b11", "b.b2", "c")), c = 4L)
xout6.24 <- list(a = 1L, b = list(b1 = c(b11 = 2L), b2 = 3L), c = 4L)
xout6.25 <- list(a = list(b = 1L), b = list(b = list(b1 = 2L, b2 = 3L)), c = list(b = 4L))

xout6.26 <- list(list(1L), list(2L), list(3L))
xout6.27 <- xout6.26

xout6.28 <- structure(list(a = "a", b = "b", c = "c"), row.names = 1L, class = "data.frame")
xout6.29 <- structure(list(b = list(structure(2:3, .Names = c("b1", "b2")))), row.names = 1L, class = "data.frame")
xout6.30 <- list(list(a = list(b = list(c = 1L))))

xout6.31 <- list(a = 1L, b_ = list(b1 = 2L, b2 = 3L), c = 4L)
xout6.32 <- structure(list(b1 = list(list(b11 = 2L)), b2 = 3L), row.names = 1L, class = "data.frame")
xout6.33 <- structure(list(L1 = "b", L2 = "b1", b11 = 2L), row.names = 1L, class = "data.frame")

dotest("6.1", rrapply(xin1, f = function(x, .xname) .xname, classes = "list"), xout6.1)
dotest("6.2", rrapply(xin1, f = function(x, .xname) .xname, classes = c("list", "ANY")), xout6.2)
dotest("6.3", rrapply(xin1, f = function(x, .xname) .xname, condition = function(x, .xname) .xname == "b", classes = "list"), xout6.3)
dotest("6.4", rrapply(xin1, f = function(x, .xname) .xname, condition = function(x, .xpos) length(.xpos) == 2, classes = c("ANY", "list")), xout6.4)
dotest("6.5", rrapply(xin1, f = function(x, .xname) .xname, condition = function(x, .xpos) length(.xpos) == 3, classes = c("ANY", "list")), xout6.5)

dotest("6.6", rrapply(xin2, f = unlist, condition = function(x, .xname) .xname == "b", how = "replace", classes = "list"), xout6.6)
dotest("6.7", rrapply(xin2, f = unlist, condition = function(x, .xname) .xname == "b", how = "list", classes = "list"), xout6.7)
dotest("6.8", rrapply(xin2, f = unlist, condition = function(x, .xname) .xname == "b", how = "unlist", classes = "list"), xout6.8)
dotest("6.9", rrapply(xin2, f = unlist, condition = function(x, .xname) .xname == "b", how = "prune", classes = "list"), xout6.9)
dotest("6.10", rrapply(xin2, f = unlist, condition = function(x, .xname) .xname == "b", how = "flatten", classes = "list"), xout6.10)
dotest("6.11", rrapply(xin2, f = unlist, condition = function(x, .xname) .xname == "b", how = "melt", classes = "list"), xout6.11)

dotest("6.12", rrapply(xin2, f = function(x){ names(x) <- paste0(names(x), "_"); x }, condition = is.list, how = "recurse", classes = "list"), xout6.12)
dotest("6.13", rrapply(xin2, f = list, condition = function(x, .xpos, .xname) length(.xpos) < 3 & all(.xpos < 2), how = "recurse", classes = c("list", "ANY")), xout6.13)

dotest("6.14", rrapply(xin1, f = function(x, .xparents) .xparents, classes = c("list", "ANY")), xout6.14)
dotest("6.15", rrapply(xin1, f = function(x, .xparents) .xparents, condition = function(x, .xparents) length(.xparents) == 3, classes = c("list", "ANY")), xout6.15)
dotest("6.16", rrapply(xin2, f = function(x) list(b = x), condition = function(x, .xparents) length(.xparents) < 3 & "b" %in% .xparents, how = "recurse", classes = c("list", "ANY")), xout6.16)

dotest("6.17", rrapply(xin3, f = function(x, .xname) .xname, classes = "language"), xout6.17)
dotest("6.18", rrapply(xin3, f = function(x) { x[[1L]] <- quote(f); x }, classes = "language", how = "recurse"), xout6.18)
dotest("6.19", rrapply(xin4, f = as.list, classes = "expression"), xout6.19)
dotest("6.20", rrapply(xin4, f = function(x) { x[[1L]] <- quote(f); x }, classes = "language", how = "recurse"), xout6.20)
dotest("6.21", rrapply(xin5, f = as.list, classes = "pairlist"), xout6.21)
dotest("6.22", rrapply(xin5, f = as.list, condition = function(x, .xpos) length(.xpos) < 3, classes = c("ANY", "pairlist"), how = "recurse"), xout6.22)

dotest("6.23", rrapply(xin1, f = function(x, .xsiblings) unlist(.xsiblings), classes = "list"), xout6.23)
dotest("6.24", rrapply(xin1, f = unlist, condition = function(x, .xsiblings) "b2" %in% names(.xsiblings), classes = "list"), xout6.24)
dotest("6.25", rrapply(xin2, f = function(x) list(b = x), condition = function(x, .xsiblings, .xpos) length(.xpos) < 2 & "b" %in% names(.xsiblings),
                       classes = c("list", "ANY"), how = "recurse"), xout6.25)

dotest("6.26", rrapply(xin6, classes = c("list", "integer"), condition = function(x, .xpos) length(.xpos) < 2, f = list, how = "recurse"), xout6.26)
dotest("6.27", rrapply(xin6, classes = c("list", "integer"), f = function(x, .xpos) if(length(.xpos) < 2) list(x) else x, how = "recurse"), xout6.27)

dotest("6.28", rrapply(xin1, f = function(x, .xparents) .xparents, how = "bind", classes = c("list", "ANY")), xout6.28)
dotest("6.29", rrapply(xin2, f = unlist, condition = function(x, .xname) .xname == "b", how = "bind", classes = "list"), xout6.29)
dotest("6.30", rrapply(xin7, classes = "data.frame", f = as.list, how = "recurse"), xout6.30)

dotest("6.31", rrapply(xin2, f = function(x, .xname) paste0(.xname, "_"), classes = "list", how = "names"), xout6.31)
dotest("6.32", rrapply(xin1, how = "bind", classes = c("list", "ANY"), options = list(coldepth = 2)), xout6.32)
dotest("6.33", rrapply(xin1, how = "bind", classes = c("list", "ANY"), options = list(namecols = TRUE, coldepth = 3)), xout6.33)

## errors
tools::assertError(rrapply(list(list(1)), f = list, classes = "list", how = "recurse"))

## named flat list
xin1 <- list(a = 1L, b = 2L, c = 3L)
xin2 <- list(a = 1L, b = NULL)

xout7.1 <- list(a = -1L, b = 2L, c = -3L)
xout7.2 <- list(a = -1L, b = NULL, c = -3L)
xout7.3 <- list(a = -1L, c = -3L)
xout7.4 <- c(a = -1L, c = -3L)
xout7.5 <- xout7.1
xout7.6 <- list(a = list(1L), b = 2L, c = 3L)
xout7.7 <- structure(list(L1 = c("a", "c"), value = c(-1L, -3L)), row.names = 1:2, class = "data.frame")
xout7.8 <- list(a = FALSE, b = TRUE)
xout7.9 <- xout7.8
xout7.10 <- c(a = FALSE, b = TRUE)
xout7.11 <- structure(list(L1 = c("a", "b"), value = c(FALSE, TRUE)), row.names = 1:2, class = "data.frame")
xout7.12 <- list(a = 1L, c = 3L)
xout7.13 <- list(a = 0L, b = -1L)
xout7.14 <- xout7.3
xout7.15 <- list(a = list(list(1L)), b = 2L, c = list(list(3L)))
xout7.16 <- list(a = -1L, b = -2L, c = -3L)
xout7.17 <- list(a = list(1L), b = list(2L), c = list(3L))
xout7.18 <- structure(list(a = -1L, c = -3L), row.names = 1L, class = "data.frame")
xout7.19 <- structure(list(a = FALSE, b = TRUE), row.names = 1L, class = "data.frame")
xout7.21 <- list("1_" = 1L, "2_" = 2L, "3_" = 3L)
xout7.22 <- list("1_" = 1L, "_" = NULL)

dotest("7.1", rrapply(xin1, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "replace"), xout7.1)
dotest("7.2", rrapply(xin1, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "list"), xout7.2)
dotest("7.3", rrapply(xin1, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "prune"), xout7.3)
dotest("7.4", rrapply(xin1, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "flatten"), xout7.4)
dotest("7.5", rrapply(xin1, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), classes = c("list", "ANY")), xout7.5)
dotest("7.6", rrapply(xin1, f = list, condition = function(x, .xpos) length(.xpos) < 2 & all(.xpos == 1L), classes = c("list", "ANY"), how = "recurse"), xout7.6)
dotest("7.7", rrapply(xin1, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "melt"), xout7.7)
dotest("7.8", rrapply(xin2, f = is.null, how = "replace"), xout7.8)
dotest("7.9", rrapply(xin2, f = is.null, how = "prune"), xout7.9)
dotest("7.10", rrapply(xin2, f = is.null, how = "flatten"), xout7.10)
dotest("7.11", rrapply(xin2, f = is.null, how = "melt"), xout7.11)
dotest("7.12", rrapply(xout7.7, f = `-`, how = "unmelt"), xout7.12)
dotest("7.13", rrapply(xout7.11, f = `-`, how = "unmelt"), xout7.13)
dotest("7.14", rrapply(xin1, f = `-`, condition = function(x, .xparents) any(c("a", "c") %in% .xparents), how = "prune"), xout7.14)
dotest("7.15", rrapply(xin1, f = list, condition = function(x, .xparents, .xpos) length(.xpos) < 3 & any(c("a", "c") %in% .xparents), classes = c("list", "ANY"), how = "recurse"), xout7.15)
dotest("7.16", rrapply(xin1, f = `-`, condition = function(x, .xsiblings) "b" %in% names(.xsiblings), how = "prune"), xout7.16)
dotest("7.17", rrapply(xin1, f = list, condition = function(x, .xsiblings, .xpos) length(.xpos) < 2 & "b" %in% names(.xsiblings), classes = c("list", "ANY"), how = "recurse"), xout7.17)
dotest("7.18", rrapply(xin1, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "bind"), xout7.18)
dotest("7.19", rrapply(xin2, f = is.null, how = "bind"), xout7.19)
dotest("7.20", rrapply(xin2, f = is.null, how = "flatten", options = list(simplify = FALSE)), xout7.8)
dotest("7.21", rrapply(xin1, f = paste0, how = "names", sep = "_"), xout7.21)
dotest("7.22", rrapply(xin2, f = paste0, how = "names", sep = "_"), xout7.22)

## unnamed nested list
xin <- list(1L, 2L, list(3L, 4L))

xout8.1 <- list(-1L, 2L, list(3L, 4L))
xout8.2 <- list(-1L, NULL, list(NULL, NULL))
xout8.3 <- list(-1L)
xout8.4 <- -1L
xout8.5 <- structure(list(L1 = "1", value = -1L), row.names = 1L, class = "data.frame")
xout8.6 <- list("1", "2", list("1", "2"))
xout8.7 <- list(-1L, 2L, -3L)
xout8.8 <- list(list(1L), 2L, list(3L, 4L))
xout8.9 <- list("1" = 1L)
xout8.10 <- list(-1L, -2L, list(-3L, -4L))
xout8.11 <-  structure(list(L1 = c("1", "2", "3", "3"),
                            L2 = c(NA,  NA, "1", "2"),
                            value = c(-1L, -2L, -3L, -4L)),
                       row.names = c(NA, 4L), class = "data.frame")
xout8.12 <- list(1L, 2L, list(-3L, -4L))
xout8.13 <- structure(list(L1 = c("3", "3"), L2 = c("1", "2"),
                           value = c(-3L, -4L)), row.names = 1:2, class = "data.frame")
xout8.14 <- structure(list("1" = -1L, "3.1" = -3L, "3.2" = -4L), row.names = 1L, class = "data.frame")
xout8.15 <- structure(list("1" = -1L, "2" = -2L, "3.1" = -3L, "3.2" = -4L), row.names = 1L, class = "data.frame")
xout8.16 <- list(-1L, -2L, -3L, -4L)
xout8.17 <- structure(list(`1` = -1L, `2` = -2L, `3_1` = -3L, `3_2` = -4L), row.names = 1L, class = "data.frame")
xout8.18 <- structure(list(L1 = "3", `1` = -3L, `2` = -4L), row.names = 1L, class = "data.frame")
xout8.19 <- list(`1_` = 1L, `2_` = 2L, `3_` = list(`1_` = 3L, `2_` = 4L))
xout8.20 <- list(`1_` = 1L, `2_` = 2L, `3_` = list(`3_` = 3L, `4_` = 4L))


dotest("8.1", rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "replace"), xout8.1)
dotest("8.2", rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "list"), xout8.2)
dotest("8.3", rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "prune"), xout8.3)
dotest("8.4", rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "flatten"), xout8.4)
dotest("8.5", rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), how = "melt"), xout8.5)
dotest("8.6", rrapply(xin, f = function(x, .xname) .xname), xout8.6)
dotest("8.7", rrapply(xin, f = function(x, .xpos) -.xpos, condition = function(x, .xpos) .xpos %in% c(1L, length(xin)), classes = c("list", "ANY")), xout8.7)
dotest("8.8", rrapply(xin, f = list, condition = function(x, .xpos) length(.xpos) < 2 & all(.xpos == 1L), classes = c("list", "ANY"), how = "recurse"), xout8.8)
dotest("8.9", rrapply(xout8.5, f = `-`, how = "unmelt"), xout8.9)
dotest("8.10", rrapply(xin, f = `-`, condition = function(x, .xparents) all(grepl("^\\d$", .xparents)), how = "replace"), xout8.10)
dotest("8.11", rrapply(xin, f = `-`, condition = function(x, .xparents) all(grepl("^\\d$", .xparents)), how = "melt"), xout8.11)
dotest("8.12", rrapply(xin, f = `-`, condition = function(x, .xsiblings) length(.xsiblings) == 2L, how = "replace"), xout8.12)
dotest("8.13", rrapply(xin, f = `-`, condition = function(x, .xsiblings) length(.xsiblings) == 2L, how = "melt"), xout8.13)
dotest("8.14", rrapply(xin, f = `-`, condition = function(x, .xpos) any(.xpos %in% c(1L, 3L)), how = "bind"), xout8.14)
dotest("8.15", rrapply(xin, f = `-`, condition = function(x, .xparents) all(grepl("^\\d$", .xparents)), how = "bind"), xout8.15)
dotest("8.16", rrapply(xin, f = `-`, how = "flatten", options = list(simplify = FALSE)), xout8.16)
dotest("8.17", rrapply(xin, f = `-`, how = "bind", options = list(namesep = "_")), xout8.17)
dotest("8.18", rrapply(xin, f = `-`, how = "bind", options = list(namecols = TRUE, coldepth = 2)), xout8.18)
dotest("8.19", rrapply(xin, f = function(x, .xname) paste0(.xname, "_"), how = "names"), xout8.19)
dotest("8.20", rrapply(xin, f = paste0, how = "names", sep = "_"), xout8.20)

## partially named list 1
xin <- list(a = 1L, 2L, list(c1 = 3L, 4L))

xout9.1 <- list(a = -1L, 2L, list(c1 = -3L, 4L))
xout9.2 <- list(a = -1L, NULL, list(c1 = -3L, NULL))
xout9.3 <- list(a = -1L, list(c1 = -3L))
xout9.4 <- c(a = -1L, c1 = -3L)
xout9.5 <- structure(list(L1 = c("a", ""), L2 = c(NA, "c1"), value = c(-1L, -3L)), row.names = 1:2, class = "data.frame")
xout9.6 <- list(a = -1L)
xout9.7 <- list(a = "a", 2L, list(c1 = "c1", 4L))
xout9.8 <- list(a = list(1L), 2L, list(c1 = 3L, 4L))
xout9.9 <- list(a = 1L, list(c1 = 3L))
xout9.10 <- list(a = "a", "", list(c1 = c("", "c1"), c("", "")))
xout9.11 <- list(a = -1L, -2L)
xout9.12  <- structure(list(a = -1L, c1 = -3L), row.names = 1L, class = "data.frame")
xout9.13 <- list(a = -1L, -2L, c1 = -3L, -4L)
xout9.14 <- structure(list(c1 = -3L), row.names = 1L, class = "data.frame")
xout9.15 <- list(`1_` = 1L, 2L, list(`3_` = 3L, 4L))
xout9.16 <- list(a_ = 1L, `_` = 2L, `_` = list(c1_ = 3L, `_` = 4L))

dotest("9.1", rrapply(xin, f = `-`, condition = function(x, .xname) nzchar(.xname), how = "replace"), xout9.1)
dotest("9.2", rrapply(xin, f = `-`, condition = function(x, .xname) nzchar(.xname), how = "list"), xout9.2)
dotest("9.3", rrapply(xin, f = `-`, condition = function(x, .xname) nzchar(.xname), how = "prune"), xout9.3)
dotest("9.4", rrapply(xin, f = `-`, condition = function(x, .xname) nzchar(.xname), how = "flatten"), xout9.4)
dotest("9.5", rrapply(xin, f = `-`, condition = function(x, .xname) nzchar(.xname), how = "melt"), xout9.5)
dotest("9.6", rrapply(xin, f = `-`, condition = function(x, .xpos) .xpos == 1L, how = "prune"), xout9.6)
dotest("9.7", rrapply(xin, f = function(x, .xname) .xname, condition = function(x, .xname) nzchar(.xname), classes = c("list", "ANY")), xout9.7)
dotest("9.8", rrapply(xin, f = list, condition = function(x, .xpos) length(.xpos) < 2 & all(.xpos == 1L), classes = c("list", "ANY"), how = "recurse"), xout9.8)
dotest("9.9", rrapply(xout9.5, f = `-`, how = "unmelt"), xout9.9)
dotest("9.10", rrapply(xin, f = function(x, .xparents) .xparents), xout9.10)
dotest("9.11", rrapply(xin, f = `-`, condition = function(x, .xparents) length(.xparents) == 1, how = "prune"), xout9.11)
dotest("9.12", rrapply(xin, f = `-`, condition = function(x, .xname) nzchar(.xname), how = "bind"), xout9.12)
dotest("9.13", rrapply(xin, f = `-`, how = "flatten", options = list(simplify = FALSE)), xout9.13)
dotest("9.14", rrapply(xin, f = `-`, condition = function(x, .xname) nzchar(.xname), how = "bind", options = list(coldepth = 2)), xout9.14)
dotest("9.15", rrapply(xin, f = function(x) paste0(x, "_"), condition = function(x, .xname) nzchar(.xname), how = "names"), xout9.15)
dotest("9.16", rrapply(xin, f = function(x, .xname) paste0(.xname, "_"), how = "names"), xout9.16)

## partially named list 2
xin <- list(1L, 2L, list(c1 = 3L, c2 = 4L))

xout10.1 <- list(1L, 2L, list(c1 = -3L, c2 = -4L))
xout10.2 <- list(NULL, NULL, list(c1 = -3L, c2 = -4L))
xout10.3 <- list(list(c1 = -3L, c2 = -4L))
xout10.4 <- c(c1 = -3L, c2 = -4L)
xout10.5 <- structure(list(L1 = c("3", "3"), L2 = c("c1", "c2"), value = c(-3L, -4L)), row.names = 1:2, class = "data.frame")
xout10.6 <- list(list(c1 = -3L))
xout10.7 <- list(1L, 2L, list(c1 = "c1", c2 = "c2"))
xout10.8 <- list(list(1L), 2L, list(c1 = 3L, c2 = 4L))
xout10.9 <- list("3" = list(c1 = 3L, c2 = 4L))
xout10.10 <- list(1L, 2L, list(c1 = -3L, c2 = -4L))
xout10.11 <- xout10.10
xout10.12 <- xout10.10
xout10.13 <- xout10.10
xout10.14 <- structure(list("1" = -1L, "2" = -2L, "3.c1" = -3L, "3.c2" = -4L), row.names = 1L, class = "data.frame")
xout10.15 <- list(`1` = -1L, `2` = -2L, c1 = -3L, c2 = -4L)
xout10.16 <- structure(list(`1` = -1L, `2` = -2L, `3_c1` = -3L, `3_c2` = -4L), row.names = 1L, class = "data.frame")
xout10.17 <- structure(list(L1 = "3", c1 = -3L, c2 = -4L), row.names = 1L, class = "data.frame")
xout10.18 <- list(`1_` = 1L, `2_` = 2L, `3_` = list(`3_` = 3L, `4_` = 4L))
xout10.19 <- list(`1_` = 1L, `2_` = 2L, `3_` = list(c1_ = 3L, c2_ = 4L))

dotest("10.1", rrapply(xin, f = `-`, condition = function(x, .xname) !grepl("^\\d$", .xname), how = "replace"), xout10.1)
dotest("10.2", rrapply(xin, f = `-`, condition = function(x, .xname) !grepl("^\\d$", .xname), how = "list"), xout10.2)
dotest("10.3", rrapply(xin, f = `-`, condition = function(x, .xname) !grepl("^\\d$", .xname), how = "prune"), xout10.3)
dotest("10.4", rrapply(xin, f = `-`, condition = function(x, .xname) !grepl("^\\d$", .xname), how = "flatten"), xout10.4)
dotest("10.5", rrapply(xin, f = `-`, condition = function(x, .xname) !grepl("^\\d$", .xname), how = "melt"), xout10.5)
dotest("10.6", rrapply(xin, f = `-`, condition = function(x, .xpos) identical(.xpos, c(3L, 1L)), how = "prune"), xout10.6)
dotest("10.7", rrapply(xin, f = function(x, .xname) .xname, condition = function(x, .xname) !grepl("^\\d$", .xname), classes = c("list", "ANY")), xout10.7)
dotest("10.8", rrapply(xin, f = list, condition = function(x, .xpos) length(.xpos) < 2 & all(.xpos == 1L), classes = c("list", "ANY"), how = "recurse"), xout10.8)
dotest("10.9", rrapply(xout10.5, f = `-`, how = "unmelt"), xout10.9)
dotest("10.10", rrapply(xin, f = `-`, condition = function(x, .xparents) any(!grepl("^\\d$", .xparents)), how = "replace"), xout10.10)
dotest("10.11", rrapply(xin, f = `-`, condition = function(x, .xparents) any(!grepl("^\\d$", .xparents)), classes = c("list", "ANY")), xout10.11)
dotest("10.12", rrapply(xin, f = `-`, condition = function(x, .xsiblings) !is.null(names(.xsiblings)), how = "replace"), xout10.12)
dotest("10.13", rrapply(xin, f = `-`, condition = function(x, .xsiblings) !is.null(names(.xsiblings)), classes = c("list", "ANY")), xout10.13)
dotest("10.14", rrapply(xin, f = `-`, how = "bind"), xout10.14)
dotest("10.15", rrapply(xin, f = `-`, how = "flatten", options = list(simplify = FALSE)), xout10.15)
dotest("10.16", rrapply(xin, f = `-`, how = "bind", options = list(namesep = "_")), xout10.16)
dotest("10.17", rrapply(xin, f = `-`, how = "bind", options = list(namecols = TRUE, coldepth = 2)), xout10.17)
dotest("10.18", rrapply(xin, f = function(x) paste0(x, "_"), how = "names"), xout10.18)
dotest("10.19", rrapply(xin, f = function(x, .xname) paste0(.xname, "_"), how = "names"), xout10.19)

## empty lists
xin1 <- list(a = 1L, b = list(list(2L)))
xin2 <- list(1L)
xin3 <- quote(a <- 1L)
xin4 <- expression(1L)
xin5 <- quote(x[, 1])  ## empty symbol

xout11.1 <- structure(list(), .Names = character(0))
xout11.2 <- list()
xout11.3 <- structure(list(L1 = character(0), value = list()), row.names = integer(0), class = "data.frame")

dotest("11.1", rrapply(xin1, condition = function(x) FALSE, how = "prune"), xout11.1)
dotest("11.2", rrapply(xin1, condition = function(x) FALSE, how = "flatten"), xout11.1)
dotest("11.3", rrapply(xin1, condition = function(x) FALSE, how = "melt"), xout11.3)
dotest("11.4", rrapply(xin1, f = `-`, classes = "user-class", how = "prune"), xout11.1)
dotest("11.5", rrapply(xin1, f = `-`, classes = "user-class", how = "flatten"), xout11.1)
dotest("11.6", rrapply(xin1, f = `-`, classes = "user-class", how = "melt"), xout11.3)
dotest("11.7", rrapply(xin1, condition = function(x) FALSE, how = "prune", classes = c("list", "ANY")), xout11.1)
dotest("11.11", rrapply(xin2, condition = function(x) FALSE, how = "prune"), xout11.2)
dotest("11.12", rrapply(xin2, condition = function(x) FALSE, how = "flatten"), xout11.2)
dotest("11.13", rrapply(xin2, condition = function(x) FALSE, how = "melt"), xout11.3)
dotest("11.14", rrapply(xin2, f = `-`, classes = "user-class", how = "prune"), xout11.2)
dotest("11.15", rrapply(xin2, f = `-`, classes = "user-class", how = "flatten"), xout11.2)
dotest("11.16", rrapply(xin2, condition = function(x) FALSE, how = "melt"), xout11.3)
dotest("11.17", rrapply(xin2, condition = function(x) FALSE, how = "prune", classes = c("list", "ANY")), xout11.2)
dotest("11.18", rrapply(xin2, condition = function(x) FALSE, how = "flatten", classes = c("list", "ANY")), xout11.2)
dotest("11.21", rrapply(xin1, condition = function(x, .xparents) "c" %in% .xparents, how = "prune"), xout11.1)
dotest("11.22", rrapply(xin2, condition = function(x, .xparents) any(!grepl("^\\d$", .xparents)), how = "flatten"), xout11.2)
dotest("11.23", rrapply(xin3, condition = function(x) FALSE, how = "prune"), xout11.2)
dotest("11.24", rrapply(xin3, condition = function(x) FALSE, how = "melt", classes = c("language", "ANY")), xout11.3)
dotest("11.26", rrapply(xin4, condition = function(x) FALSE, how = "prune"), xout11.2)
dotest("11.27", rrapply(xin4, condition = function(x) FALSE, how = "melt", classes = c("language", "ANY")), xout11.3)
dotest("11.29", rrapply(xin1, condition = function(x, .xsiblings) length(.xsiblings) > 2, how = "prune"), xout11.1)
dotest("11.30", rrapply(xin2, condition = function(x, .xsiblings) !is.null(names(.xsiblings)), how = "flatten"), xout11.2)
dotest("11.31", rrapply(xin5, f = identity, how = "replace"), xin5)
dotest("11.32", rrapply(xin1, condition = function(x) FALSE, how = "bind"), xout11.1)
dotest("11.33", rrapply(xin2, condition = function(x) FALSE, how = "bind"), xout11.1)
dotest("11.34", rrapply(xin3, condition = function(x) FALSE, how = "bind"), xout11.1)
dotest("11.35", rrapply(xin1, condition = function(x) FALSE, how = "flatten", options = list(namesep = ".", simplify = FALSE)), xout11.1)
dotest("11.36", rrapply(xin2, condition = function(x) FALSE, how = "flatten", options = list(namesep = ".", simplify = FALSE)), xout11.2)
dotest("11.37", rrapply(xin1, condition = function(x) FALSE, how = "bind", options = list(coldepth = 2)), xout11.1)
dotest("11.38", rrapply(xin2, condition = function(x) FALSE, how = "bind", options = list(coldepth = 1)), xout11.1)
dotest("11.39", rrapply(xin3, condition = function(x) FALSE, how = "bind", options = list(namecols = TRUE, coldepth = 1)), xout11.1)
dotest("11.40", rrapply(xin1, condition = function(x) FALSE, how = "names"), xin1)
dotest("11.41", rrapply(xin2, condition = function(x) FALSE, how = "names"), xin2)
dotest("11.42", rrapply(xin5, condition = function(x) FALSE, how = "names"), xin5)
dotest("11.43", rrapply(xin3, how = "bind", options = list(coldepth = 3)), xout11.1)

## check wrong inputs
tools::assertError(rrapply(xin1, f = `-`, how = "unmelt"))
tools::assertError(rrapply(xout11.1, f = `-`, how = "unmelt"))
tools::assertError(rrapply(xout11.3, f = `-`, how = "unmelt"))

## deeply nested lists

## helper function
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

xin1 <- f(len = 1, d = 1, dmax = 17, expr = list(1L, NULL))
xin2 <- f(len = 2, d = 1, dmax = 4, expr = list(1L, NULL, 1L))
xin3 <- rrapply(xin1, condition = Negate(is.null), f = function(x, .xname) .xname, how = "names")

xout12.1 <- f(len = 1, d = 1, dmax = 17, expr = list(1L, NA))
xout12.2 <- xout12.1
xout12.3 <- xout12.2
xout12.4 <- f(len = 1, d = 1, dmax = 17, expr = list(2L))
xout12.5 <- 2L
xout12.6 <- xout12.5
xout12.7 <- structure(list(L1 = "1", L2 = "1", L3 = "1", L4 = "1",
                           L5 = "1", L6 = "1", L7 = "1", L8 = "1", L9 = "1",
                           L10 = "1", L11 = "1", L12 = "1", L13 = "1", L14 = "1",
                           L15 = "1", L16 = "1", L17 = "1", value = 2L), row.names = 1L, class = "data.frame")
xout12.8 <- f(len = 2, d = 1, dmax = 4, expr = list(1L, NA, 1L))
xout12.9 <- xout12.8
xout12.10 <- xout12.9
xout12.11 <- f(len = 2, d = 1, dmax = 4, expr = list(2L, 2L))
xout12.12 <- rep(2L, 16L)
xout12.13 <- rep(2L, 8L)
xout12.14 <- structure(list(L1 = c("1", "1", "1", "1", "1", "1"),
                            L2 = c("1", "1", "1", "1", "1", "1"),
                            L3 = c("1", "1", "1", "2", "2", "2"),
                            L4 = c("1", "2", "3", "1", "2", "3"),
                            value = c(2L, 2L, 2L, 2L, 2L, 2L)), row.names = c(NA, 6L), class = "data.frame")
xout12.15 <- list("1" = list("1" = list("1" = list("1" = list("1" = list("1" = list(
  "1" = list("1" = list("1" = list("1" = list("1" = list("1" = list(
    "1" = list("1" = list("1" = list("1" = list("1" = 2L)))))))))))))))))
xout12.16 <- list("1" = list("1" = list("1" = list("1" = 2L, "2" = 2L, "3" = 2L),
                                        "2" = list("1" = 2L, "2" = 2L, "3" = 2L))))
xout12.17 <- f(len = 1, d = 1, dmax = 17, expr = list(rep("1", 17), c(rep("1", 16), "2")))
xout12.18 <- f(len = 2, d = 1, dmax = 4, expr = replicate(3L, 4L, simplify = FALSE))
xout12.19 <- f(len = 1, d = 1, dmax = 17, expr = list(2L, 2L))
xout12.20 <- f(len = 2, d = 1, dmax = 4, expr = replicate(3L, 3L, simplify = FALSE))
xout12.21 <- list(2L)
xout12.22 <- c(`1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1.1` = 1L)

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
dotest("12.15", rrapply(xout12.7, how = "unmelt"), xout12.15)
dotest("12.16", rrapply(xout12.14, how = "unmelt"), xout12.16)
dotest("12.17", rrapply(xin1, f = function(x, .xparents) .xparents, how = "replace"), xout12.17)
dotest("12.18", rrapply(xin2, f = function(x, .xparents) length(.xparents), how = "replace"), xout12.18)
dotest("12.19", rrapply(xin1, f = function(x, .xsiblings) length(.xsiblings), how = "replace"), xout12.19)
dotest("12.20", rrapply(xin2, f = function(x, .xsiblings) length(.xsiblings), how = "replace"), xout12.20)
dotest("12.21", rrapply(xin1, condition = Negate(is.null), f = function(x) 2L, how = "flatten", options = list(simplify = FALSE)), xout12.21)
dotest("12.22", rrapply(xin3, condition = Negate(is.null), how = "flatten", options = list(namesep = ".")), xout12.22)

## type coercion
xin <- list(
  raw = list(as.raw(TRUE), as.raw(FALSE)),
  lgl = list(TRUE, FALSE),
  int = list(1L, 0L),
  real = list(1, 0),
  cplx = list(1i, 0i),
  str = list("T", "F"),
  name = list(quote(t), quote(f))
)
xin1 <- data.frame(L1 = c("1", "2"), val = as.raw(c(TRUE, FALSE)), stringsAsFactors = FALSE)
xin2 <- data.frame(L1 = c("1", "2"), val = c(TRUE, FALSE), stringsAsFactors = FALSE)
xin3 <- data.frame(L1 = c("1", "2"), val = c(1L, 0L), stringsAsFactors = FALSE)
xin4 <- data.frame(L1 = c("1", "2"), val = c(1, 0), stringsAsFactors = FALSE)
xin5 <- data.frame(L1 = c("1", "2"), val = c(1i, 0i), stringsAsFactors = FALSE)
xin6 <- data.frame(L1 = c("1", "2"), val = c("T", "F"), stringsAsFactors = FALSE)
xin7 <- data.frame(L1 = c("1", "2"), val = I(list(1L, 0L)), stringsAsFactors = FALSE)

xout13.1 <- list("1" = as.raw(TRUE), "2" = as.raw(FALSE))
xout13.2 <- structure(list(raw.1 = list(as.raw(TRUE)), raw.2 = list(as.raw(FALSE))), row.names = 1L, class = "data.frame")
xout13.3 <- c("1" = TRUE, "2" = FALSE)
xout13.4 <- structure(list(lgl.1 = TRUE, lgl.2 = FALSE), row.names = 1L, class = "data.frame")
xout13.5 <- c("1" = 1L, "2" = 0L, "1" = 1L, "2" = 0L)
xout13.6 <- structure(list(lgl.1 = TRUE, lgl.2 = FALSE, int.1 = 1L, int.2 = 0L), row.names = 1L, class = "data.frame")
xout13.7 <- c("1" = 1, "2" = 0, "1" = 1, "2" = 0, "1" = 1, "2" = 0)
xout13.8 <- structure(list(lgl.1 = TRUE, lgl.2 = FALSE, int.1 = 1L, int.2 = 0L, real.1 = 1, real.2 = 0), row.names = 1L, class = "data.frame")
xout13.9 <- c("1" = 1+0i, "2" = 0+0i, "1" = 1+0i, "2" = 0+0i, "1" = 1+0i, "2" = 0+0i, "1" = 0+1i, "2" = 0+0i)
xout13.10 <- structure(list(lgl.1 = TRUE, lgl.2 = FALSE, int.1 = 1L, int.2 = 0L, 
                            real.1 = 1, real.2 = 0, cplx.1 = 0+1i, cplx.2 = 0+0i), row.names = 1L, class = "data.frame")
xout13.11 <- c("1" = "TRUE", "2" = "FALSE", "1" = "1", "2" = "0", "1" = "1", 
               "2" = "0", "1" = "0+1i", "2" = "0+0i", "1" = "T", "2" = "F")
xout13.12 <- structure(list(lgl.1 = TRUE, lgl.2 = FALSE, int.1 = 1L, int.2 = 0L,
                            real.1 = 1, real.2 = 0, cplx.1 = 0+1i, cplx.2 = 0+0i,
                            str.1 = "T", str.2 = "F"), row.names = 1L, class = "data.frame")
xout13.13 <- list("1" = as.raw(TRUE), "2" = as.raw(FALSE), "1" = TRUE, "2" = FALSE, 
                  "1" = 1L, "2" = 0L, "1" = 1, "2" = 0, "1" = 0+1i, "2" = 0+0i, 
                  "1" = "T", "2" = "F", "1" = quote(t), "2" = quote(f))
xout13.14 <- list("1" = as.raw(TRUE), "2" = as.raw(FALSE))
xout13.15 <- list("1" = TRUE, "2" = FALSE)
xout13.16 <- list("1" = 1L, "2" = 0L)
xout13.17 <- list("1" = 1, "2" = 0)
xout13.18 <- list("1" = 0+1i, "2" = 0+0i)
xout13.19 <- list("1" = "T", "2" = "F")
xout13.20 <- xout13.16
xout13.21 <- list(raw.1 = as.raw(TRUE), raw.2 = as.raw(FALSE), lgl.1 = TRUE, 
                  lgl.2 = FALSE, int.1 = 1L, int.2 = 0L, real.1 = 1, real.2 = 0, 
                  cplx.1 = 0+1i, cplx.2 = 0+0i, str.1 = "T", str.2 = "F", name.1 = quote(t), 
                  name.2 = quote(f))
xout13.22 <-  
  
  dotest("13.1", rrapply(xin, classes = "raw", how = "flatten"), xout13.1)
dotest("13.2", rrapply(xin, classes = "raw", how = "bind", options = list(coldepth = 1L)), xout13.2)
dotest("13.3", rrapply(xin, classes = "logical", how = "flatten"), xout13.3)
dotest("13.4", rrapply(xin, classes = "logical", how = "bind", options = list(coldepth = 1L)), xout13.4)
dotest("13.5", rrapply(xin, classes = c("logical", "integer"), how = "flatten"), xout13.5)
dotest("13.6", rrapply(xin, classes = c("logical", "integer"), how = "bind", options = list(coldepth = 1L)), xout13.6)
dotest("13.7", rrapply(xin, classes = c("logical", "integer", "numeric"), how = "flatten"), xout13.7)
dotest("13.8", rrapply(xin, classes = c("logical", "integer", "numeric"), how = "bind", options = list(coldepth = 1L)), xout13.8)
dotest("13.9", rrapply(xin, classes = c("logical", "integer", "numeric", "complex"), how = "flatten"), xout13.9)
dotest("13.10", rrapply(xin, classes = c("logical", "integer", "numeric", "complex"), how = "bind", options = list(coldepth = 1L)), xout13.10)
dotest("13.11", rrapply(xin, classes = c("logical", "integer", "numeric", "complex", "character"), how = "flatten"), xout13.11)
dotest("13.12", rrapply(xin, classes = c("logical", "integer", "numeric", "complex", "character"), how = "bind", options = list(coldepth = 1L)), xout13.12)
dotest("13.13", rrapply(xin, classes = "ANY", how = "flatten"), xout13.13)
dotest("13.14", rrapply(xin1, how = "unmelt"), xout13.14)
dotest("13.15", rrapply(xin2, how = "unmelt"), xout13.15)
dotest("13.16", rrapply(xin3, how = "unmelt"), xout13.16)
dotest("13.17", rrapply(xin4, how = "unmelt"), xout13.17)
dotest("13.18", rrapply(xin5, how = "unmelt"), xout13.18)
dotest("13.19", rrapply(xin6, how = "unmelt"), xout13.19)
dotest("13.20", rrapply(xin7, how = "unmelt"), xout13.20)
dotest("13.21", rrapply(xin, classes = "ANY", how = "flatten", options = list(namesep = ".", simplify = FALSE)), xout13.21)

## miscellaneous
if(getRversion() < "4.0.0") {
  
  xin <- quote(f1(a = 1L, b = f2(b1 = 2L, b2 = 3L), c = 4L))
  xout14.1 <- list(L1 = c("b", "b", "b"), L2 = c("", "b1", "b2"), value = list(quote(f2), 2L, 3L))
  xout14.2 <- list("b." = list(quote(f2)), "b.b1" = 2L, "b.b2" = 3L)
  
  dotest("14.1", rrapply(xin, f = identity, condition = function(x, .xpos) length(.xpos) > 1, how = "melt"), xout14.1)
  dotest("14.2", rrapply(xin, f = identity, condition = function(x, .xpos) length(.xpos) > 1, how = "bind"), xout14.2)
  
} else {
  
  xin1 <- quote(f1(a = 1L, b = f2(b1 = 2L, b2 = 3L), c = 4L))
  xin2 <- expression(a <- 1L, expression(b))
  xout14.3 <- structure(list(L1 = c("", "b"), L2 = c(NA, ""), value = list(quote(f1), quote(f2))), row.names = 1:2, class = "data.frame")
  xout14.4 <- structure(list(`1` = list(quote(`<-`), quote(expression)), `2` = list(quote(a), quote(b))), row.names = 1:2, class = "data.frame")
  
  dotest("14.3", rrapply(xin1, classes = "name", how = "melt"), xout14.3)
  dotest("14.4", rrapply(xin2, classes = "name", how = "bind"), xout14.4)
  
}

cat("Completed rrapply unit tests\n")
