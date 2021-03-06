#include <gtest/gtest.h>
#include <glog/logging.h>

#include <gflags/gflags.h>


int main(int argc, char* argv[]) {
	testing::InitGoogleTest(&argc, argv);

	//google::ParseCommandLineFlags(&argc, &argv, true); // gflags::

	google::InitGoogleLogging( argv[0] );
	google::InstallFailureSignalHandler();

	return RUN_ALL_TESTS();
}

