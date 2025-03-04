# GMOCK-256
Simple encryption algorithm based on SHA-256 implemented in C

# Build
Dependencies only include the standard C library, so it can be built for any platform with a standard C/C++ compiler (i.e. `gcc gmock-256.c`)

# Use
Encryption or decryption can be done with

`./gmock-256 <key> <[e]ncrypt|[d]ecrypt> <in_file> <out_file>`

where `key` is a 32-bit unsigned int, `in_file` is the path to the data to be encrypted/decrypted, and `out_file` is the destination for the result.

# Warning
GMOCK-256 was designed and built by amateur cryptographers (AKA grad students), so use at your own risk!

# References
[SHA-256 C implementation](https://github.com/BareRose/lonesha256) provided by [BareRose](https://github.com/BareRose) (0 dependencies, all standard C library)
