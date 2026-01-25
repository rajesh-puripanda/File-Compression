# zstd-file-compression-tool

This repository contains a C based file compression and decompression tool built using the Zstandard compression library.  
The project is written as a learning exercise focused on understanding how real world compression tools work internally rather than just using high level wrappers.

The implementation supports streaming compression for large files, automatic mode detection, multi threaded compression, and cryptographic hash based verification.

---

## Motivation

I am a college student interested in systems programming, file formats, and performance oriented software.  
Most compression tutorials stop at compressing small buffers in memory, which does not reflect how production tools operate.

This project was created to learn:

- How modern compression libraries like Zstandard are actually used
- How large files are processed safely using streaming APIs
- How integrity verification is done using cryptographic hashes
- How multithreading is enabled in compression pipelines
- How command line tools are structured at a low level in C

---

## Features

Streaming compression and decompression using constant memory  
Automatic detection of compress or decompress mode based on file extension  
Multi threaded compression using Zstandard worker threads  
SHA 256 hash computation for integrity verification  
Works with large files without loading them entirely into memory

---

## How it works

If the input file does not have a `.zst` extension, the program compresses it using Zstandard and produces a compressed output file.

If the input file has a `.zst` extension, the program automatically switches to decompression mode and restores the original file.

All compression and decompression is done using streaming APIs, meaning the file is processed in fixed size chunks rather than as a single buffer.

During decompression, the output file can be verified using SHA 256 hashing to ensure the data was restored correctly.

---

## Dependencies

Zstandard compression library  
OpenSSL for SHA 256 hashing  
A C compiler such as GCC or Clang

On MSYS2 these can be installed using:

```
pacman -S mingw-w64-x86_64-zstd mingw-w64-x86_64-openssl
```

## Build

Compile using GCC:

```
gcc zstd_tool.c -lzstd -lcrypto -o zstd_tool
```

## Usage

To compress a file:

```
./zstd_tool input.txt
```

This produces a compressed file:

`output.zst`

To decompress:

```
./zstd_tool output.zst
```

This produces:

`output.dec`

## Learning Outcomes

This project helped me understand:

- Zstandard streaming APIs and frame handling
- Why compression overhead exists for small files
- How multi threading improves compression throughput
- How integrity verification is implemented using hashing
- How production grade command line tools are structured in C

---

## Limitations and Future Work

Currently output file names are fixed  
The tool operates on single files only  
There is no progress indicator for large files

Planned improvements include preserving original filenames, recursive directory compression, progress reporting, and embedding hashes directly into compressed metadata.

---

## Disclaimer

This project is intended for learning and experimentation.  
It is not meant to replace mature compression utilities.

---

## License

MIT License
