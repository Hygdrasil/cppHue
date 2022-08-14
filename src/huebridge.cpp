#include "huebridge.hpp"
#include <sstream>

#include "json_serialicer.hpp"
#include "jsonnavi.hpp"

HueBridge::HueBridge(const std::string& ip)
{
    this->ip = ip.c_str();
    client.connectToIp(ip, 80);
    client.close();
}

std::string HueBridge::createGetToken()const{
    return HueBridge::BASE_PATH;
}

std::string HueBridge::createGetAll()const{
    std::string path = createGetToken();
    path += '/';
    path += accessToken + '/';
    path += HueBridge::LIGHTS;
    return path;
}

std::string HueBridge::createGet(unsigned int bulbIndex)const{
    std::string path = createGetAll();
    return path + '/' + std::to_string(bulbIndex);
}

std::string HueBridge::createPut(unsigned int bulbIndex)const{
    std::string path = createGet(bulbIndex);
    return path + "/state";
}

HttpResponse HueBridge::getSomeThing(const std::string& path){
    client.connectToIp(ip, 80);
    HttpResponse response = client.get(path ,"");
    client.close();
    return response;
}

std::string HueBridge::getAccessToken(){
    return accessToken;
}

void HueBridge::setToken(const std::string& token){
    accessToken = token;
}

bool HueBridge::setTokenFromBridge(){
    JsonSerializer json;
    json = json.join("devicetype", "HUE_CLIENT");
    HttpResponse response = client.post(createGetToken(), json.toString());
    if(!response.succeeded()){
        return false;
    }
    JsonNavi responseParser{response.message};
    const std::string_view token = responseParser.stringFromHeader("username");
    if(token == ""){
        return false;
    }
    accessToken = token;
    return true;
}

int HueBridge::getBulbCount(){
    HttpResponse response = getSomeThing(createGetAll());
    if(response.succeeded()){
        JsonNavi parser{response.message};
        return parser.headerNumber();
    }
    return -1;
}

bool HueBridge::isSomeThing(int bulbId, const std::string& someThing, bool* succeeded){
    BulbData raw{getBulbJson(bulbId)};
    return JsonNavi(raw.json).boolFromHeader(someThing, succeeded);
}

bool HueBridge::isOn(int bulbId, bool* succeeded){
    return isSomeThing(bulbId, HueBridge::ON_KEY, succeeded);
}

bool HueBridge::isReachable(int bulbId, bool* succeeded){
    return isSomeThing(bulbId, HueBridge::REACHABLE_KEY, succeeded);
}


int HueBridge::getBrightness(int bulbId, bool* succeeded){
    BulbData raw{getBulbJson(bulbId)};
    return JsonNavi(raw.json).longFromHeader(HueBridge::BRIGHTNESS_KEY, succeeded);
}

BulbState HueBridge::getState(int bulbId, bool* succeeded){
    BulbState state;
    BulbData raw = getBulbJson(bulbId);
    JsonNavi parser{raw.json};
    bool working = false;
    do{
        state.brightness = parser.longFromHeader( HueBridge::BRIGHTNESS_KEY, &working);
        if(!working){
            break;
        }
        state.hue =  parser.longFromHeader(HueBridge::HUE_KEY, &working);
        if(!working){
            break;
        }
        state.isOn = parser.boolFromHeader(HueBridge::ON_KEY, &working);
        if(!working){
            break;
        }
        state.isReachable = parser.boolFromHeader(HueBridge::REACHABLE_KEY, &working);
        if(!working){
            break;
        }
        state.saturation = parser.longFromHeader(HueBridge::SATURATION_KEY, &working);
        if(!working){
            break;
        }
    }while(false);
    if(succeeded){
        *succeeded = working;
    }
    return state;
}

bool HueBridge::setSomeThing(int bulbId, const std::string& json){    
    client.connectToIp(ip, 80);
    HttpResponse response = client.put(createPut(bulbId), json);
    client.close();
    if(!response.succeeded()){
        logHue("request faild with: %s\n", response.message.c_str());
        return false;
    }

    JsonNavi parser{response.message};
    return parser.hasHeader( "success");
}

bool HueBridge::setState(int bulbId, bool state){
    JsonSerializer json = JsonSerializer().join(HueBridge::ON_KEY, state);
    return setSomeThing(bulbId, json.toString());

}

std::string HueBridge::stateToJson(const BulbState& state){
    JsonSerializer json = JsonSerializer()
        .join(HueBridge::ON_KEY, state.isOn)
        .join(HueBridge::BRIGHTNESS_KEY, state.brightness)
        .join(HueBridge::SATURATION_KEY, state.saturation)
        .join(HueBridge::HUE_KEY, state.hue);
    return json.toString();
}
bool HueBridge::setState(int bulbId, const BulbState& state){
    //{"on":true, "sat":254, "bri":254,"hue":10000}
    JsonSerializer json;
    return setSomeThing(bulbId, stateToJson(state));
}

bool HueBridge::setBrightness(int bulbId, int brightness){
    JsonSerializer json = JsonSerializer().join(HueBridge::BRIGHTNESS_KEY, brightness);
    return setSomeThing(bulbId, json.toString());
}
