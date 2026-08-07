import scapy

# Test cases: 

# First line does not end in /n/r 
    # Bad request
# First line cannot be chunked into three parts 
    # Bad request
# Some valid request type that isn't implemented
    # Not implemented
# Some invalid request type like BLA
    # Bad request 
# Some invalid http version
    # Bad request
# Some request who's path doesn't begin with /
    # Bad request
# Some request that properly gets index.html or another endpoint
    # OK
# A request that includes file traversal
    # Not permitted
# A request that isn't found
    # Not found
# internal error