# Block-Based Compression of Event-Based Data Using Huffman and Arithmetic Coding

## Overview
This repository presents a lossless coding solution for event-based vision data stored in JPEG XE’s `.xe` format. The proposed method introduces a binary transformation that restructures the `.xe` file into a block-based format (`.bxe`), where events are grouped in fixed-size blocks.

The main goal is to evaluate whether this block-based organization enables more efficient statistical compression when combined with standard lossless coders.

The implementation includes:
- A C++ encoder that converts `.xe` files into `.bxe` format, segmenting the event stream into blocks with a configurable number of events (default: 1024).
- Python modules that apply global lossless compression using:
  - **Huffman coding**, or
  - **Arithmetic coding**, both using concatenated block data.

Although the block-based structure allows for independent compression per block, initial tests showed that generating a separate Huffman table for each block led to high overhead and reduced overall efficiency. Therefore, a global compression strategy was adopted, where all block contents are merged before coding. Still, the `.bxe` format preserves block boundaries, supporting potential block-wise compression and parallelism in future implementations.

This work is part of an academic study related to JPEG XE standardization and is intended to support further experimentation and extension.


## Reference JPEG XE Repository
In the link above is availabe the source code for the JPEG XE standardization activity (raw canonical XE format).

- [JPEG XE Source Code](https://nx51932.your-storageshare.de/s/QgNjbps8dgAaCJ7)


## Reference dataset
In the repository is available some datasets but in the link above is available the full JPEG XE Reference
dataset. The files (in raw canonical XE format) are available here:

- [JPEG XE Reference Dataset](https://nx51932.your-storageshare.de/s/QgNjbps8dgAaCJ7)

## Reference Arithmetic Coding Repository  
The arithmetic encoder and decoder used in this project are based on publicly available educational implementations of static arithmetic coding.

- [Reference Arithmetic Coding Source (Project Nayuki)](https://eli.thegreenplace.net/2015/adaptive-entropy-coding-with-static-models/))




## Compiling the software

### Requirements
**C++ Compiler** - you need to install a C++ compiler with support for C++17; depending on your operating system the instructions may be different.
**System compatible with UNIX commands** - Linux, macOS, WSL on Windows.
**Python 3.6+**  
The compression and decompression scripts are written in Python.

**Required Python packages:**
- `dahuffman` — for static Huffman coding
- `pickle`, `os`, `sys` — standard Python libraries

You can install `dahuffman` via pip:

```bash
pip install dahuffman


## Running test scripts

### On Linux

Run the following commands in the project directory:

```sh
cd Encoder
g++ -std=c++17 -O2 xe_to_blockxe.cpp ../Codec/xe_format.cpp -o xe_to_blockxe
```

The executable files:
```
./xe_to_blockxe ../../Datasets/"dataset_name".xe 0 
```
0 to run the full file or a determinated number (corresponds to the number of events to be readed).

```sh
cd Scripts
python3 compress_block_huff.py
python3 decompress_block_huff.py
```
for Huffman coding and decoding.

```sh
cd Scripts
python3 compress_block_arit.py
python3 decompress_block_arit.py
```
for Huffman coding and decoding.

