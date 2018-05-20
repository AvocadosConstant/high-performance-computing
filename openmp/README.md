# OpenMP

Detailed description about the assignment can be found [here](http://www.cs.binghamton.edu/~kchiu/cs580f/prog/2/).

High-dimensional space is non-intuive in a number of ways. In particular, it turns out that most of the volume of a sphere lies near the surface of the sphere. For this part, submit a program, parallelized with OpenMP, that confirms this.

To do this, sample uniformly distributed, random points within the volume of a unit sphere (radius 1). Then compute a histogram of distance from the surface. Make the histogram with 100 intervals, i.e., from 0 to 1 in steps of 0.01. Show output for dimensionality from 2 to 16. You can just print numbers for your histogram, giving the relative fraction in each interval.

Your submission should include both a sequential and parallelized implementation. If you start with a sequential implementation, the nature of OpenMP is such that your parallel version can be almost identical (or even 100% identical).

Extra Credit, 15 points: Use something like the inverse transform technique so that you can efficiently sample space within the unit sphere. Show the histogram for dimensionality up to 50.


## Implementation

My solution uses the results defined in [this paper](http://compneuro.uwaterloo.ca/files/publications/voelker.2017.pdf), 
namely section 3.2 on Uniformly sampling coordinates from the n-ball.

## Usage

```
./exec.out print_bool n_points max_dim num_threads hist_buckets

  print_bool    Prints histograms if print_bool is "true"
  n_points      Number of points to sample
  max_dim       Dimensional upper bound. Minimum dim is always 2.
  num_threads   Number of threads
  hist_buckets  Number of buckets for the histograms
```

For testing, I recommend using the following to test for the requirements.
```
./exec.out false 100000 16 20 100
```
This will sample 100k points from 2 to 16 dimensions with 20 threads and 100 histogram buckets (intervals of size 0.01).

You can also just run the following to try this example.
```
make test
```


To print the histograms, just change the print flag to true.
```
./exec.out true 100000 16 20 100
```


To show that the solution can handle very high dimensions for the extra credit, run with 50 dimensions.
```
./exec.out false 100000 50 20 100
```


Another small example to try for fun to see the different histogram bucket sizes is the following.
```
./exec.out true 100000 5 20 4
```
