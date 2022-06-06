#ifndef JSONNAVI_H
#define JSONNAVI_H
#include <string>

struct TextFrame{
    TextFrame(const char* start, size_t size);
    const char* frameStart;
    size_t size;
    std::string toStdString() const;
    void moveForward(const char* newStart);
    void moveToNext(char letter);
};

class JsonNavi
{
public:
    static JsonNavi fromHttp(const std::string& response);
    JsonNavi(const std::string& json);
    JsonNavi(const TextFrame json);
    int headerNumber();
    bool hasHeader(const std::string& header) const;
    TextFrame stringFromHeader(const std::string& header) const;
    TextFrame jsonFromHeader(const std::string& header) const;
    TextFrame textFromHeader(const std::string& header) const;

    bool boolFromHeader(const std::string& header, bool* succeded) const;
    long longFromHeader(const std::string& header, bool* succeded) const;
    double doubleFromHeader(const std::string& header, bool* succeded) const;

    TextFrame findHeader(const std::string& header)const;
    const char* getContendStart(const std::string& header) const;
protected:
    const TextFrame json;
};

#endif // JSONNAVI_H
