#ifndef HUEBRIDGE_H
#define HUEBRIDGE_H
#include <string>
#include <sstream>

#include "jsonnavi.hpp"
#include "simplehttpclient.hpp"


struct BulbState{
    int hue;
    int saturation;
    int brightness;
    bool isOn;
    bool isReachable;
};

class HueBridge
{
public:
    HueBridge(const std::string& ip);

    std::string getAccessToken();
    void setToken(const std::string& token);
    bool setTokenFromBridge();
    int getBulbCount();

    bool isOn(int bulbId, bool* succeeded = NULL);
    bool isReachable(int bulbId, bool* succeeded = NULL);
    int getBrightness(int bulbId, bool* succeeded = NULL);
    BulbState getState(int bulbId, bool* succeeded = NULL);

    bool setState(int bulbId, bool state);
    bool setBrightness(int bulbId, int brightness);
    bool setState(int bulbId, const BulbState& state);
    std::string stateToJson(const BulbState& state);

protected:
    struct BulbData{
        const std::string raw;
        std::string_view json;
    };
    void createGetToken();
    void createGetAll();
    void createGet(unsigned int bulbIndex);
    void createPut(unsigned int bulbIndex);
    HttpResponse getSomeThing();
    BulbData getBulbJson(int bulbId){
        createGet(bulbId);
        HttpResponse response = getSomeThing();
        if(response.succeeded()){
            BulbData data{response.message,""};
            JsonNavi parser{data.raw};
            data.json = parser.jsonFromHeader("state");
            return data;
        }
        return BulbData{"",""};
    }
    bool isSomeThing(int bulbId, const std::string& someThing, bool* succeeded);
    bool setSomeThing(int bulbId, const std::string& json);
    std::stringstream pathBuffer;

    SimpleHttpClient client{};
    std::string ip;
    std::string accessToken ="";

    static constexpr const char* BASE_PATH = "/api";
    static constexpr const char* LIGHTS = "lights";
    static constexpr const char* ON_KEY = "on";
    static constexpr const char* BRIGHTNESS_KEY = "bri";
    static constexpr const char* SATURATION_KEY = "sat";
    static constexpr const char* HUE_KEY = "hue";
    static constexpr const char* REACHABLE_KEY = "reachable";

    

};

#endif // HUEBRIDGE_H
