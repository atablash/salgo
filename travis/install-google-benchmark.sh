#!/bin/bash
git clone https://github.com/google/benchmark.git google-benchmark --depth 1
cd google-benchmark && cmake . -DBENCHMARK_ENABLE_GTEST_TESTS=OFF -DCMAKE_BUILD_TYPE=RELEASE && make -j8 && sudo make install
rm -rf google-benchmark
