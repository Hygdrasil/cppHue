#include <gtest/gtest.h>

#include "../src/huebridge.hpp"

TEST(BridgeTests, getTest){
    HueBridge bridge{"192.168.188.27"};
    //bridge.setToken("1Vp07bDcWhuBH2d2VYQNA8UCWsk6KUkLVf4cCdr4")
    bool succeeded = false;
    BulbState state = bridge.getState(1, &succeeded);
    ASSERT_TRUE(succeeded);
}
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
