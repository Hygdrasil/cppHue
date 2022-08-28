#pragma once
#include <string>

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
    
    bool isOn(int bulbId, bool* succeeded = nullptr);
    bool isReachable(int bulbId, bool* succeeded = nullptr);
    int getBrightness(int bulbId, bool* succeeded = nullptr);
    BulbState getState(int bulbId, bool* succeeded = nullptr);

    bool setState(int bulbId, bool state);
    bool setBrightness(int bulbId, int brightness);
    bool setState(int bulbId, const BulbState& state);
    std::string stateToJson(const BulbState& state);

protected:
    struct BulbData{
        const std::string raw;
        std::string_view json;
    };
    std::string createGetToken()const;
    std::string createGetAll()const;
    std::string createGet(const unsigned int bulbIndex)const;
    std::string createPut(const unsigned int bulbIndex)const;
    HttpResponse getSomeThing(const std::string& path);
    BulbData getBulbJson(int bulbId){   
        HttpResponse response = getSomeThing(createGet(bulbId));
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

    void connect();

    static constexpr const char* BASE_PATH = "/api";
    static constexpr const char* LIGHTS = "lights";
    static constexpr const char* ON_KEY = "on";
    static constexpr const char* BRIGHTNESS_KEY = "bri";
    static constexpr const char* SATURATION_KEY = "sat";
    static constexpr const char* HUE_KEY = "hue";
    static constexpr const char* REACHABLE_KEY = "reachable";

private:
    SimpleHttpClient client{};
    std::string ip;
    std::string accessToken ="";
};
