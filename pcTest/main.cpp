#include <iostream>
#include "simplehttpclient.h"
using namespace std;

#include "huebridge.h"
int main()
{
    HueBridge bridge{"192.168.188.27"};
    bridge.setToken("1Vp07bDcWhuBH2d2VYQNA8UCWsk6KUkLVf4cCdr4");
    BuilbState state = bridge.getState(1);
    cout << "Hello World!" << endl;
    return 0;
}
