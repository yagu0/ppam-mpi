getLabel = function(fileName)
{
    return (as.integer(as.matrix(read.table(fileName))))
}

getData = function(fileName)
{
    return (as.matrix(read.table(fileName,sep=',')))
}

plotCurves = function(data, cols=rep(1,nrow(data)), ylim=range(data), ...)
{
    for (i in 1:nrow(data))
    {
        if (dev.cur() > 1)
			par(new=TRUE)
		par(mar=c(5,5,2,2))
        plot(data[i,],type="l",col=cols[i],ylim=ylim, ...)
    }
}

getDistor = function(data, ctrs)
{
	distor = 0.0
	for (i in 1:nrow(data))
		distor = distor + min(sqrt(colSums((t(ctrs) - data[i,])^2)))
	return (distor)
}

getPartition = function(data, ctrs)
{
	partition = c()
	for (i in 1:nrow(data))
		partition = c(partition, which.min(sqrt(colSums((t(ctrs) - data[i,])^2))))
	return (partition)
}

#~ system("R CMD SHLIB hungarian.c")
#~ dyn.load("hungarian.so")
comparePartitions = function(partition1, partition2)
{
	result = 1.0
	n = length(partition1)
	maxInd = max(partition1)
	return ( .C("computeCoefSimil", P1=as.integer(partition1), P2=as.integer(partition2), 
		maxInd=as.integer(maxInd), n=as.integer(n), coefSimil=as.double(result))$coefSimil )
}
