# NASDAQ ITCH Order Book

This is a very fast implementation of a ITCH order book, when tested on a Macbook Air M1 with 16GB RAM, it is capable of reconstructing a day's worth of ITCH binary data in under 1 minute, averaging about ~130ns per message or about 7.7 million messages per second.

This implementation only uses `std::vector` as the underlying container for all the market data and the implementation aggressively preallocates space for holding market data, this allows `malloc` calls to be kept at a minimum thereby lowering jitter during processing the message stream and allows the L1 cache to be utilized efficiently.


## Running
First, build the binary:

```sh
$ g++ --std=c++14 -Iinclude main.cc -o main
```

Sample NASDAQ ITCH data can be found on <https://emi.nasdaq.com/ITCH> under `NASDAQ ITCH` directory, the filename format is `MMDDYYYY.NASDAQ_ITCH50.gz`

After downloading and unzipping the file, run the binary:
```sh
$ ./main < MMDDYYYY.NASDAQ_ITCH50
```

It should print the stats to `stdout` after reconstructing the order book

```
Total time took: 50s
Average message process time: 136.813ns
```