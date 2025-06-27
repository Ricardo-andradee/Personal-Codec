# Block-Based Compression of Event-Based Data Using Huffman and Arithmetic Coding

## Overview
This repository presents a lossless coding solution for JPEG XE-encoded event-based vision data, based on a novel binary transformation of `.xe` files into a block-structured format. The motivation behind this approach is to assess whether reorganizing events into structured blocks enables more efficient compression when combined with standard statistical coding techniques.

The implemented pipeline performs the following:
- Parses and decodes input `.xe` files using the JPEG XE canonical format specification.
- Groups events into fixed-size temporal or spatial blocks to exploit local redundancy.
- Prepares the data for entropy coding using either:
  - **Huffman coding**, or
  - **Arithmetic coding** (both approaches provided as modules or placeholders).

The core hypothesis is that block-based organization enhances the performance of statistical coders by increasing symbol predictability and local frequency distribution, thus improving compression ratios while maintaining bit-exact reconstruction.

This project is part of an academic exploration aligned with JPEG XE standardization activities and is structured for easy experimentation, evaluation, and extension.

## Reference JPEG XE Repository
In the link above is availabe the source code for the JPEG XE standardization activity (raw canonical XE format).

- [JPEG XE Source Code](https://nx51932.your-storageshare.de/s/QgNjbps8dgAaCJ7)


## Reference dataset
In the repository is available some datasets but in the link above is available the full JPEG XE Reference
dataset. The files (in raw canonical XE format) are available here:

- [JPEG XE Reference Dataset](https://nx51932.your-storageshare.de/s/QgNjbps8dgAaCJ7)


## Compiling the software

### Requirements
**C++ Compiler** - you need to install a C++ compiler with support for C++17; depending on your operating system the instructions may be different.
**System compatible with UNIX commands** - Linux, macOS, WSL on Windows.


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

