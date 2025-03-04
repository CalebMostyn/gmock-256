# GMOCK-256
Simple encryption algorithm based on SHA-256 implemented in C

# Use
Encryption or decryption can be done with

`./gmock-256 <key> <[e]ncrypt|[d]ecrypt> <in_file> <out_file>`

where `key` is a 32-bit unsigned int, `in_file` is the path to the data to be encrypted/decrypted, and `out_file` is the destination for the result.

# References
[SHA-256 C implementation](https://github.com/BareRose/lonesha256) provided by [BareRose](https://github.com/BareRose) (0 dependencies, all standard C library)
