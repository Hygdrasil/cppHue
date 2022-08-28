#pragma once
#include <string>
#include <string_view>

/**
 * @brief helper to generate a json-document from primary c-types
 * It separates each member with ',' but douse no string escaping.
 */

class JsonSerializer{
public:
    JsonSerializer() = default;
    JsonSerializer(const std::string_view contend)
        :buffer(contend){};

    std::string toString() const{
        return buffer+'}';
    }

    JsonSerializer join(const std::string_view memberName, const std::string_view text){
        std::string textWrap = "\"";
        textWrap += text;
        textWrap += '"';
        return addSomeThing(memberName, textWrap);
    }

    JsonSerializer join(const std::string_view memberName, const bool state){
        std::string value = state?std::string("true"):std::string("false");
        return addSomeThing(memberName, value);
    }

    JsonSerializer join(const std::string_view memberName,long long number){
        std::string value = std::to_string(number);
        return addSomeThing(memberName, value);
    }

    JsonSerializer join(const std::string_view memberName, unsigned long long number){
        std::string value = std::to_string(number);
        return addSomeThing(memberName, value);
    }

    JsonSerializer join(const std::string_view memberName, long number){
        std::string value = std::to_string(number);
        return addSomeThing(memberName, value);
    }

    JsonSerializer join(const std::string_view memberName, unsigned long number){
        std::string value = std::to_string(number);
        return addSomeThing(memberName, value);
    }

    JsonSerializer join(const std::string_view memberName, int number){
        std::string value = std::to_string(number);
        return addSomeThing(memberName, value);
    }

    JsonSerializer join(const std::string_view memberName, unsigned int number){
        std::string value = std::to_string(number);
        return addSomeThing(memberName, value);
    }

private:
    std::string buffer ="{";
    
    std::string setMemberName(const std::string_view name){
        std::string member;
        if(buffer.size() == 1){
            member = "\"";
        }else{
            member = ",\"";
        }
        member += name;
        member += "\":";
        return member;  
    }

    JsonSerializer addSomeThing(const std::string_view memberName, const std::string_view value){
        std::string data = buffer;
        data += setMemberName(memberName);
        data += value;
        return {data};
    }
};