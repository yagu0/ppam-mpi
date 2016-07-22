# Parallel clustering with a k-medoids algorithm

Joint work with [Jairo Cugliari](http://eric.univ-lyon2.fr/~jcugliari/)

---

This C program runs the k-medoid algorithm on several subsets of one (presumably big) dataset. 
The computed medoids are then merged iteratively, until we get a final set of k centers.

---

The folder "communication/" contains latex sources (and generated pdf files) of a short paper submitted 
to the Journ&eacute;es de Statistique in Rennes, France (2014), and also the slides presented at this event.

The other folder contains all the C code; but not the EDF (french electricity company) datasets, because they 
are not public. Since the (de)serialization process in code/src/TimeSeries/ is tailored for these data, 
it is necessary to adapt this small part of the code to use any other custom time-series files.
