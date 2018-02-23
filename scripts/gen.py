import ctypes

# input file
with open('input', 'wb+') as f:
    header = [
        1234,   # set ID
        3,      # num points
        2       # num dims
    ]
    for i in header:
        f.write(ctypes.c_ulonglong(i))


# queries file
with open('queries', 'wb+') as f:
    header = [
        1234,   # input id
        4321,   # query id
        3,      # num queries
        2,      # num dims
        1       # num neighbors
    ]
    for i in header:
        f.write(ctypes.c_ulonglong(i))
