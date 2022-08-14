#include <cstdlib>
#include <gtest/gtest.h>

#include "../src/huebridge.hpp"

TEST(BridgeTests, getTest){
    HueBridge bridge{"192.168.188.27"};
    bridge.setToken("1Vp07bDcWhuBH2d2VYQNA8UCWsk6KUkLVf4cCdr4");
    bool succeeded = false;
    BulbState state = bridge.getState(1, &succeeded);
    ASSERT_TRUE(succeeded);
}

TEST(BridgeTests, changeState){
    HueBridge bridge{"192.168.188.27"};
    bridge.setToken("1Vp07bDcWhuBH2d2VYQNA8UCWsk6KUkLVf4cCdr4");
    bool succeeded = false;
    bool state = bridge.isOn(1, &succeeded);
    ASSERT_TRUE(succeeded);
    usleep(10000);
    succeeded = bridge.setState(1, !state);
    ASSERT_TRUE(succeeded);
    usleep(10000);
    bool secondState = bridge.isOn(1, &succeeded);
    ASSERT_TRUE(succeeded);
    ASSERT_TRUE(secondState != state);
}


TEST(BridgeTests, changeEverything){
    HueBridge bridge{"192.168.188.27"};
    bridge.setToken("1Vp07bDcWhuBH2d2VYQNA8UCWsk6KUkLVf4cCdr4");
    bool succeeded = false;
    auto state = bridge.getState(1, &succeeded);
    ASSERT_TRUE(succeeded);
    usleep(10000);
    auto changedState = state;
    changedState.brightness++;
    changedState.hue++;
    changedState.isOn != changedState.isOn;
    changedState.saturation++;
    succeeded = bridge.setState(1, changedState);
    ASSERT_TRUE(succeeded);
    usleep(10000);
    auto newState = bridge.getState(1, &succeeded);
    ASSERT_TRUE(succeeded);
    ASSERT_TRUE(newState.brightness == changedState.brightness);
    ASSERT_TRUE(newState.hue == changedState.hue);
    ASSERT_TRUE(newState.isOn == changedState.isOn);
    ASSERT_TRUE(newState.saturation == changedState.saturation);
}
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
