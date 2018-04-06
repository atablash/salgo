#include <gtest/gtest.h>
#include <glog/logging.h>

#include <gflags/gflags.h>


DEFINE_int32(perf, 0, "number of iterations for performance tests");


int main(int argc, char* argv[]) {
	testing::InitGoogleTest(&argc, argv);

	google::ParseCommandLineFlags(&argc, &argv, true); // gflags::

	google::InitGoogleLogging( argv[0] );
	google::InstallFailureSignalHandler();

	return RUN_ALL_TESTS();
}

