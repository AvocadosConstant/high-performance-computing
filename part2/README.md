# OpenMP

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
