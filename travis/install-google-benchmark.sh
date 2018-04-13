#!/bin/bash
git clone https://github.com/google/benchmark.git --depth 1
cd benchmark && cmake . -DBENCHMARK_ENABLE_GTEST_TESTS=OFF && make -j8 && sudo make install
rm -rf benchmark
