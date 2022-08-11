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

void HueBridge::createGetToken(){
    pathBuffer.str("");
    pathBuffer.clear();
    pathBuffer << HueBridge::BASE_PATH;
}

void HueBridge::createGetAll(){
    createGetToken();
    pathBuffer <<'/';
    pathBuffer << accessToken << '/';
    pathBuffer << HueBridge::LIGHTS;
}

void HueBridge::createGet(unsigned int bulbIndex){
    createGetAll();
    pathBuffer << '/' << bulbIndex;
}

void HueBridge::createPut(unsigned int bulbIndex){
    createGet(bulbIndex);
    pathBuffer << "/state";
}

HttpResponse HueBridge::getSomeThing(){
    client.connectToIp(ip, 80);
    HttpResponse response = client.get(pathBuffer.str() ,"");
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
    json.addMember("devicetype", "HUE_CLIENT");
    createGetToken();
    HttpResponse response = client.post(pathBuffer.str(), json.asClass());
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
    createGetAll();
    HttpResponse response = getSomeThing();
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
        state.brightness = (int) parser.longFromHeader( HueBridge::BRIGHTNESS_KEY, &working);
        if(!working){
            break;
        }
        state.hue = (int) parser.longFromHeader(HueBridge::HUE_KEY, &working);
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
    }while(false);
    if(succeeded){
        *succeeded = working;
    }
    return state;
}

bool HueBridge::setSomeThing(int bulbId, const std::string& json){
    createPut(bulbId);
    client.connectToIp(ip, 80);
    HttpResponse response = client.put(pathBuffer.str(), json);
    client.close();
    if(!response.succeeded()){
        logHue("request faild with: %s\n", response.message.c_str());
        return false;
    }

    JsonNavi parser{response.message};
    return parser.hasHeader( "success");
}

bool HueBridge::setState(int bulbId, bool state){
    JsonSerializer json;
    json.addBoolMember(HueBridge::ON_KEY, state);
    return setSomeThing(bulbId, json.asClass());

}

std::string HueBridge::stateToJson(const BulbState& state){
    JsonSerializer json;
    json.addBoolMember(HueBridge::ON_KEY, state.isOn);
    json.addMember(HueBridge::BRIGHTNESS_KEY, state.brightness);
    json.addMember(HueBridge::SATURATION_KEY, state.saturation);
    json.addMember(HueBridge::HUE_KEY, state.hue);
    return json.asClass();
}
bool HueBridge::setState(int bulbId, const BulbState& state){
    //{"on":true, "sat":254, "bri":254,"hue":10000}
    JsonSerializer json;
    return setSomeThing(bulbId, stateToJson(state));
}

bool HueBridge::setBrightness(int bulbId, int brightness){
    JsonSerializer json;
    json.addMember(HueBridge::BRIGHTNESS_KEY, brightness);
    return setSomeThing(bulbId, json.asClass());
}
