#include "huebridge.h"

#include <sstream>

#include "json_serialicer.h"
#include "jsonnavi.h"

HueBridge::HueBridge(const std::string ip)
{
    client.connectToIp(ip, 80);
}

void HueBridge::createGetToken(){
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

HttpResponse HueBridge::getSomeThing() const{
    printf("connecting to bulbServer");

    HttpResponse response = client.get(pathBuffer.str() ,"");
    printf("http Response: %s\n", response.message.c_str());
    return response;
}

std::string HueBridge::getAccessToken()const{
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
    if(!response.succeded()){
        return false;
    }
    JsonNavi responseParser{response.message};
    TextFrame token = responseParser.stringFromHeader("username");
    if(token.frameStart == NULL){
        return false;
    }
    accessToken = token.toStdString();
    return true;
}

int HueBridge::getBulbCount(){
    createGetAll();
    HttpResponse response = getSomeThing();
    if(response.succeded()){
        JsonNavi parser{response.message};
        return parser.headerNumber();
    }
    return -1;

}

TextFrame HueBridge::getBulbJson(int bulbId){
    createGet(bulbId);
    HttpResponse response = getSomeThing();
    printf("bulbResponse: %s\n", response.message.c_str());
    printf("was bulb Response errorCode %d\n", response.statusCode());
    if(response.succeded()){
        JsonNavi parser{response.message};
        printf("getting Json");
        return parser.jsonFromHeader("state");
    }
    return TextFrame(NULL, 0);
}

bool HueBridge::isSomeThing(int bulbId, const std::string& someThing, bool* succeded){
    JsonNavi parser{getBulbJson(bulbId)};
    return parser.boolFromHeader(someThing, succeded);
}

bool HueBridge::isOn(int bulbId, bool* succeded){
    return isSomeThing(bulbId, HueBridge::ON_KEY, succeded);
}

bool HueBridge::isReachable(int bulbId, bool* succeded){
    return isSomeThing(bulbId, HueBridge::REACHABLE_KEY, succeded);
}


int HueBridge::getBrightness(int bulbId, bool* succeded){
    JsonNavi parser{getBulbJson(bulbId)};
    return parser.longFromHeader(HueBridge::BRIGHTNESS_KEY, succeded);
}

BuilbState HueBridge::getState(int bulbId, bool* succeded){
    BuilbState state;
    JsonNavi parser{getBulbJson(bulbId)};
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
    if(succeded){
        *succeded = working;
    }
    return state;
}

bool HueBridge::setSomeThing(int bulbId, const std::string& json){
    createPut(bulbId);
    HttpResponse response = client.put(pathBuffer.str(), json);
    if(!response.succeded()){
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

bool HueBridge::setState(int bulbId, const BuilbState& state){
    //{"on":true, "sat":254, "bri":254,"hue":10000}
    JsonSerializer json;
    json.addBoolMember(HueBridge::ON_KEY, state.isOn);
    json.addMember(HueBridge::BRIGHTNESS_KEY, state.brightness);
    json.addMember(HueBridge::SATURATION_KEY, state.saturation);
    json.addMember(HueBridge::HUE_KEY, state.hue);
    return setSomeThing(bulbId, json.asClass());
}

bool HueBridge::setBrightness(int bulbId, int brightness){
    JsonSerializer json;
    json.addMember(HueBridge::BRIGHTNESS_KEY, brightness);
    return setSomeThing(bulbId, json.asClass());
}
