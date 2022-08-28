#pragma once
#include <array>
#include <cmath>
#include <string>
#include <string_view>

class JsonNavi
{
public:
    static JsonNavi fromHttp(const std::string_view response);
    constexpr JsonNavi(const std::string_view json)
    :json(json)
    {}
    /**
     * @brief return number of headers aka items from the top json entry
     * 
     * @return int negative number on failure (invalid json)
     */
    constexpr int headerNumber() const{
        int headerCount = 0;
        std::size_t startPoint = json.find(OPENER, 0);
        if(startPoint == json.npos){
            return -1;
        }
        for(size_t courser = json.find(HEADER, startPoint); 
            courser != json.npos; 
            courser = json.find(HEADER, courser))
        {
            courser = json.find_first_of(",{}",courser);
            if(json[courser]== OPENER){
                courser = findClosing(courser);
                if(courser == json.npos){
                    return -1;
                }
            }
            headerCount++;
        }
        return headerCount;
    }

    constexpr bool hasHeader(const std::string_view header) const{
        return !findHeader(header).empty();
    }

    constexpr std::string_view stringFromHeader(const std::string_view header) const{
        const std::string_view contend = getContendStart(header);
        if(contend.empty()){
            return contend;
        }
        std::size_t prefixIndex = contend.find('"');
        if(prefixIndex == contend.npos || contend.size() <= prefixIndex+2){
            return "";
        }
        prefixIndex++;
        std::size_t suffixIndex = contend.find('"',prefixIndex);
        if(suffixIndex == contend.npos){
            return "";
        }
        suffixIndex--;
        return contend.substr(prefixIndex,suffixIndex);
    }

    constexpr std::string_view jsonFromHeader(const std::string_view header) const{
        const std::string_view contend= getContendStart(header);
        if(contend.empty()){
            return "";
        }
        std::size_t openerIndex = contend.find('{');
        if(openerIndex == contend.npos){
            return "";
        }
        std::size_t closerIndex = findClosing(openerIndex, contend);
        if(closerIndex == contend.npos){
            return "";
        }
        return contend.substr(openerIndex, closerIndex-openerIndex+1);
    }

    constexpr std::string_view textFromHeader(const std::string_view header) const{
        const std::string_view contend = getContendStart(header);
        if(contend.empty()){
            return "";
        }
        const std::size_t endIndex = contend.find_first_of(END_POSSIBILITIES);
        if(endIndex == contend.npos){
            return "";
        }
        return contend.substr(0,endIndex);
    }

    constexpr bool boolFromHeader(const std::string_view header, bool* succeeded) const{
        const std::string_view buffer = textFromHeader(header);
        if(buffer.empty()){
            if(succeeded){
                *succeeded = false;
                return false;
            }
        }else{
            if(succeeded){
                *succeeded = true;
            }
        }
        const char* jsonTrue = "true";
        if(buffer.find(jsonTrue) != buffer.npos){
            return true;
        }
        const char* jsonFalse = "false";
        if(buffer.find(jsonFalse) != buffer.npos){
            return false;
        }
        if(succeeded){
            *succeeded = false;
        }
        return false;
    }

    constexpr long longFromHeader(const std::string_view header, bool* succeeded) const{
        return fromHeader<long>(header, succeeded, [this](const std::string_view text, bool* succeeded){return toLong(text, succeeded);});
    }
    double doubleFromHeader(const std::string_view header, bool* succeeded) const;

    constexpr std::string_view findHeader(const std::string_view header) const{
        std::size_t headerStart = 0;
        do{
            headerStart = json.find(header, headerStart);
            if(headerStart == json.npos){
                return {""};
            }
            if(headerStart > 0 && json[headerStart-1] == '"' 
                && headerStart+header.size()+1 < json.size() && json[headerStart+header.size()] == '"'){
                
                std::size_t headerEnd = json.find_first_of(":{},",headerStart+header.size()+1);
                if(json[headerEnd] == ':'){
                    return json.substr(headerStart-1, headerEnd-headerStart+2);
                }
            }
            headerStart++;
        }while(headerStart != json.npos);
        return {""};
    }

    constexpr std::string_view getContendStart(const std::string_view header) const{
        const std::string_view headerStart =  findHeader(header);
        if(headerStart.empty()){
            return headerStart;
        }
        return  json.substr(headerStart.data()-json.data()+headerStart.size(), json.npos);
    }

protected:
    const std::string_view json;
    constexpr static char OPENER = '{';
    constexpr static char CLOSER = '}';
    constexpr static char HEADER = ':';
    constexpr static char CONTEND_ENDER = ',';
    constexpr static const char* END_POSSIBILITIES = ",}";

    /**
     * @brief get the end of nested json objects
     * 
     * @param searchStart start position to search for based on this json object. Can be the first '{' or the direct space behind it.
     * @return constexpr std::size_t npos or the position of the last '}' for the subsequence of nested json objects
     */
    constexpr std::size_t findClosing(const std::size_t searchStart) const{
        return findClosing(searchStart, json);
    }
    constexpr std::size_t findClosing(const std::size_t searchStart, const std::string_view json) const{
        unsigned int openings = 1;
        std::size_t courser = searchStart;
        if(json[courser]==OPENER){
            courser++;
        }
        while(openings != 0){
            courser = json.find_first_of("{}",courser);
            if(courser == json.npos){
                return json.npos;
            }
            if(json[courser] == '}'){
                openings--;
            }else{
                openings++;
            }
            courser++;
        }
        return courser -1;
    }
    constexpr long toLong(const std::string_view text, bool* succeeded) const{
        long value = 0;
        const uint16_t base = 10;
        for(const char c : text){
            if(c == ' '){
                continue;
            }
            if(c >= '0' && c<= '9'){
                value *= base;
                value += static_cast<long>(c - '0');
            }else{
                if(succeeded){
                    *succeeded = false;
                    return 0;
                }
            }
        }
        if(succeeded){
            *succeeded = true;
        }
        return value;
    }

    constexpr double toDouble(const std::string_view text, bool* succeeded) const{
        std::array<long,2> parts = {0,0};
        long subSpaces = 0;
        unsigned int step = 0;
        const uint16_t base = 10;
        for(const char c : text){
            if(c == ' '){
                continue;
            }
            if(c >= '0' && c<= '9'){
                parts[step] *= base;
                parts[step] += static_cast<double>(c - '0');
                if(step == 1){
                    subSpaces++;
                }
            }else if(c == '.'){
                if(step>1){
                    if(succeeded){
                        *succeeded = false;
                    }
                    return 0;
                }
                step++;
            }else{
                if(succeeded){
                    *succeeded = false;
                   
                }
                return 0;
            }
        }
        if(succeeded){
            *succeeded = true;
            return 0;
        }
        auto value = static_cast<double>(parts[1]);
        value /= pow(base, subSpaces);
        value += parts[0];
        return value;
    }



    template<typename ValueType, typename ConverterType>
    constexpr ValueType fromHeader(const std::string_view header , bool* succeeded, ConverterType converter) const{
        const std::string_view buffer = textFromHeader(header);
        if(buffer.empty()){
            if(succeeded){
                *succeeded = false;
                return false;
            }
        }else{
            if(succeeded){
                *succeeded = true;
            }
        }
        ValueType value = converter(buffer, succeeded);
        return value;
    }
};
