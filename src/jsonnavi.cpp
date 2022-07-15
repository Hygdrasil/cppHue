#include "jsonnavi.hpp"
#include <cstring>
#include <cstdlib>
#include <assert.h>



TextFrame::TextFrame(const char* start, size_t size){
    frameStart = start;
    this->size = size;
}
std::string TextFrame::toStdString() const{
    return (frameStart != NULL)?
        std::string(frameStart, frameStart+size):
        "";

}


void TextFrame::moveForward(const char* newStart){
    size = (newStart != NULL && newStart >= frameStart)?
                size - (size_t)(newStart - frameStart) :
                0;
    frameStart = newStart;
}

void TextFrame::moveToNext(char letter){
    const char * newStart = (char*) memchr(frameStart, letter, size);
    moveForward(newStart);
}

JsonNavi::JsonNavi(const std::string& json)
    :json(json.c_str(), json.size())
{
}

JsonNavi::JsonNavi(const TextFrame json)
    :json(json)
{
}

#define OPENER '{'
#define CLOSER '}'
#define HEADER ':'
#define CONTEND_ENDER ','
#define JSON_BUFFER_SIZE = 300;
#define HEADER_FORMAT "\"%s\":"
#define HEADER_FORMAT_TEXT "\"%s\":\"%s"
#define HEADER_FORMAT_LONG "\"%s\":%d"
#define HEADER_FORMAT_DOUBLE "\"%s\":%f"
#define HEADER_FORMAT_BOOL "\"%s\":%s"
#define MAX_NUMBER_LENGTH 20

const char* findString(TextFrame frame, const std::string& string){
    if(frame.size < string.size()){
        return NULL;
    }
    for(unsigned int i = 0; i< frame.size-string.size()-1; i++){
        if(0 == memcmp(frame.frameStart+i,string.c_str(),string.size())){
            return frame.frameStart+i;
        }
    }
    return NULL;

}

TextFrame JsonNavi::findHeader(const std::string& header) const{
    if(json.frameStart == NULL){
        return json;
    }
    TextFrame remaining{json.frameStart+1, json.size-3};
    const char * index;
    do{
        index = findString(remaining, header);
        if(index == NULL){
            return TextFrame(NULL, 0);
        }
        if(*(index-1) == '"' && *(index+header.size())== '"' && *(index+header.size()+1)== ':'){
            return TextFrame(index-1, header.size()+3);
        }
        remaining.moveForward(index+1);

    }while(index!= NULL);
    return TextFrame(NULL,0);
}

bool JsonNavi::hasHeader(const std::string& header) const{
    return findHeader(header).frameStart != NULL;
}

const char* findClosing(TextFrame json){
    const char* lastOpening = NULL;
    const char* lastClosing = NULL;
    long int coursor = -1;
    int openings = 0;
    do{
        const size_t remaining = (size_t)json.size-coursor-1;
        if(remaining < 1){
            return NULL;
        }
        lastOpening = (char*) memchr ( json.frameStart+coursor+1, OPENER, remaining);
        if(lastOpening == NULL){
            return NULL;
        }
        lastOpening += coursor+1;
        lastClosing = (char*) memchr ( json.frameStart+coursor+1, CLOSER, remaining);
        if(lastClosing == NULL){
            return NULL;
        }
        lastClosing += coursor +1;
        if(lastOpening > lastClosing){
            openings--;
            coursor = lastClosing-json.frameStart;
        }else{
            openings ++;
            coursor = lastOpening-json.frameStart;
        }
    }while(openings != 0);
    return lastClosing;
}

int JsonNavi::headerNumber(){
    int headerCount = 1;
    TextFrame coursor = json;
    coursor.moveToNext(OPENER);
    if(coursor.frameStart == NULL){
        return -1;
    }
    do{
        coursor.moveToNext(HEADER);
        if(coursor.frameStart == NULL){
            break;
        }else if(coursor.frameStart[1] == OPENER){
            coursor.moveForward(findClosing(coursor));
            if(coursor.frameStart == NULL){
                return headerCount;
            }
        }
        headerCount++;
    }while(true);
    return headerCount;
}

const char* JsonNavi::getContendStart(const std::string& header) const{
    TextFrame headerStart =  findHeader(header);
    if(headerStart.frameStart==NULL){
        return NULL;
    }
    return headerStart.frameStart+headerStart.size;
}


TextFrame cutComplexContendEnd(TextFrame start, char contendCloser){
    const char* contendEnd = (char*) memchr (start.frameStart, contendCloser, start.size);
    if(contendEnd == NULL){
        return TextFrame(NULL,0);
    }
    return TextFrame(start.frameStart, (size_t)contendEnd-(size_t)start.frameStart);
}

TextFrame cutContendEnd(TextFrame start){
    char endings[] = {CONTEND_ENDER, CLOSER, '\0'};
    for(int i= 0; endings[i] != '\0'; i++){
        TextFrame result = cutComplexContendEnd(start, endings[i]);
        if(result.frameStart != NULL){
            return result;
        }
    }
    return TextFrame(NULL, 0);
}

TextFrame JsonNavi::stringFromHeader(const std::string& header) const{
    const char* contendStart = getContendStart(header);
    if(contendStart == NULL){
        return TextFrame(NULL,0);
    }
    contendStart ++; //strip of the " start
    TextFrame beginning{contendStart, (size_t) json.frameStart+ json.size- (size_t)contendStart};
    TextFrame result =  cutComplexContendEnd(beginning,  '"');
    return result;
}

TextFrame JsonNavi::jsonFromHeader(const std::string& header) const{
    const char* contendStart = getContendStart(header);
    if(contendStart == NULL){
        return TextFrame(NULL, 0);
    }
    TextFrame beginning{contendStart, (size_t) json.frameStart+ json.size- (size_t)contendStart};
    TextFrame result = cutComplexContendEnd(beginning, '}');
    result.size++; //add ending }
    return result;
}

TextFrame JsonNavi::textFromHeader(const std::string& header) const{
    const char* contendStart = getContendStart(header);
    if(contendStart == NULL){
        return TextFrame(NULL, 0);
    }
    TextFrame beginning{contendStart, (size_t) json.frameStart+ json.size- (size_t)contendStart};
    TextFrame result = cutContendEnd(beginning);
    return result;
}

bool JsonNavi::boolFromHeader(const std::string& header, bool* succeeded) const{
     TextFrame buffer = textFromHeader(header);
     if(buffer.frameStart == NULL){
         if(succeeded){
             *succeeded = false;
             return false;
         }
     }
     else{
         if(succeeded){
             *succeeded = true;
         }
     }
    const char* jsonTrue = "true";
    if(memcmp(buffer.frameStart, jsonTrue,  strlen(jsonTrue)) == 0){
        return true;
    }
    const char* jsonFalse = "false";
    if(memcmp(buffer.frameStart, jsonFalse,  strlen(jsonFalse)) == 0){
        return false;
    }
    if(succeeded){
     *succeeded = false;
    }
    return false;

}

long JsonNavi::longFromHeader(const std::string& header, bool* succeeded) const{
    TextFrame buffer = textFromHeader(header);
    if(buffer.frameStart == NULL){
        if(succeeded){
            *succeeded = false;
            return false;
        }
    }
    else{
        if(succeeded){
            *succeeded = true;
        }
    }
    char * parsingError = NULL;
    std::string nullTerminatedCopy = buffer.toStdString();
    long result = strtol(nullTerminatedCopy.c_str(), &parsingError, 10);
    if(result == 0.0 && parsingError == nullTerminatedCopy.c_str()){
        if (succeeded){
            *succeeded = false;
        }
        return 0;
    }
    if(succeeded){
        *succeeded = true;
    }
    return result;
}

double JsonNavi::doubleFromHeader(const std::string& header, bool* succeeded) const{
    TextFrame buffer = textFromHeader(header);
    if(buffer.frameStart == NULL){
        if(succeeded){
            *succeeded = false;
            return false;
        }
    }
    else{
        if(succeeded){
            *succeeded = true;
        }
    }
    char * parsingError = NULL;
    std::string nullTerminatedCopy = buffer.toStdString();
    double result = strtod(nullTerminatedCopy.c_str(), &parsingError);
    if(result == 0.0 && parsingError == nullTerminatedCopy.c_str()){
        if (succeeded){
            *succeeded = false;
        }
        return 0;
    }
    if(succeeded){
        *succeeded = true;
    }
    return result;
}
