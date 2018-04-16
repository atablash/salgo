git clone https://github.com/google/benchmark.git google-benchmark --depth 1
cd google-benchmark

eval cmake . -DBENCHMARK_ENABLE_GTEST_TESTS=OFF -DCMAKE_BUILD_TYPE=RELEASE $1
make -j2 VERBOSE=1
make install
