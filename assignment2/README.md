# SIMD and OpenMP

Detailed description about the assignment can be found [here](http://www.cs.binghamton.edu/~kchiu/cs580f/prog/2/).


## Part 1

Use SIMD (AVX specification) to accelerate the computation of the lengths of a sequence of line segments. Each segment is represented by two 4-D points. Thus, for N segments, there would be a total of 2N points. Use Euclidean distance. Use 32-bit floats for the coordinates. Show speedup by implementing both a sequential and a parallel version. You may use your own judgement in deciding how to structure your data for the most speed. A reference for Intel intrinsics is here.

An example that computes the length of a 3-D vector is here. An example that performs matrix-vector multiplies is here.


## Part 2
High-dimensional space is non-intuive in a number of ways. In particular, it turns out that most of the volume of a sphere lies near the surface of the sphere. For this part, submit a program, parallelized with OpenMP, that confirms this.

To do this, sample uniformly distributed, random points within the volume of a unit sphere (radius 1). Then compute a histogram of distance from the surface. Make the histogram with 100 intervals, i.e., from 0 to 1 in steps of 0.01. Show output for dimensionality from 2 to 16. You can just print numbers for your histogram, giving the relative fraction in each interval.

Your submission should include both a sequential and parallelized implementation. If you start with a sequential implementation, the nature of OpenMP is such that your parallel version can be almost identical (or even 100% identical).

Extra Credit, 10 points: Use a package such at matplotlib to visualize this. I suggest a 3-D surface plot. If you do something difference, check with me first.

Extra Credit, 15 points: Use something like the inverse transform technique so that you can efficiently sample space within the unit sphere. Show the histogram for dimensionality up to 50.
