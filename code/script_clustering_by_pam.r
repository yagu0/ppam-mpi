# Original R script (not required by the C program)


##################################################################
## File: script_clustering_by_pam.r
##
## Description:   Using PAM to clustering conso using pam
## Last modified: jan 2012 by JC
##
##################################################################

## Function: Converts a matrix of curves to the discret wavelet domain
toDWT <- function(x, filter.number = 6, family = "DaubLeAsymm"){ 
  x2   <- spline(x, n = 2^ceiling( log(length(x), 2) ),
                 method = 'natural')$y
  Dx2 <- wd(x2, family = family, filter.number = filter.number)$D
  return(Dx2)
}

## Function: Computes the absolute contribution of the wavelet's scale 
##           to the total total of the curve.
contrib <- function(x) { 
  J   <- log( length(x)+1, 2)
  nrj <- numeric(J)
  t0  <- 1
  t1  <- 0
  for( j in 1:J ) {
    t1     <- t1 + 2^(J-j)
    nrj[j] <- sqrt( sum( x[t0:t1]^2 ) )
    t0     <- t1 + 1
  }
  return(nrj)
}

## 1. Load libraries & data ####
library(wavethresh)
library(cluster)


# powerload is a matrix that contains on each line one observation
#           of length delta

## 2. DWT  ##################
delta <- ncol(powerload)
n     <- nrow(powerload)
Xdwt  <- t(apply(powerload, 1, toDWT)) # DWT over the lines of powerload.
Xnrj  <- t(apply(Xdwt, 1, contrib))    # Absolute contribution to the energy.

## 3. Cluster  ##############
K <- 8 # Number of clusters
Xnrj_dist <- dist(Xnrj)
Xnrj_pam  <- pam(as.dist(Xnrj_dist), K) 
