# SIMD

Detailed description about the assignment can be found [here](http://www.cs.binghamton.edu/~kchiu/cs580f/prog/2/).


Use SIMD (AVX specification) to accelerate the computation of the lengths of a sequence of line segments. Each segment is represented by two 4-D points. Thus, for N segments, there would be a total of 2N points. Use Euclidean distance. Use 32-bit floats for the coordinates. Show speedup by implementing both a sequential and a parallel version. You may use your own judgement in deciding how to structure your data for the most speed. A reference for Intel intrinsics is here.

An example that computes the length of a 3-D vector is here. An example that performs matrix-vector multiplies is here.
