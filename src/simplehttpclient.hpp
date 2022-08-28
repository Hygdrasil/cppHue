#pragma once

#include <string>
#include <string_view>
struct HttpResponse{
    enum ConnectionState {OK, EMPTY, NO_CONNECTION};
    ConnectionState state;
    std::string message;

    bool succeeded() const;
    unsigned int statusCode() const;

};

void logHue(const char * fmt, ...);

class SimpleHttpClient
{
public:
    enum RequestTyp {GET, POST, PUT};
    ~SimpleHttpClient();
    HttpResponse::ConnectionState connectToIp(const std::string& ip, int port);
    void close();
    HttpResponse get(const std::string& path, const std::string& message = "") const;
    HttpResponse post(const std::string& path, const std::string& message = "") const;
    HttpResponse put(const std::string& path, const std::string& message = "") const;


    static const std::string protocol;
    static const std::string newLine;
    static const std::string acceptTypes;
    static const std::string connectionType;
    static const std::string contendSizePrefix;


protected:
    HttpResponse request(const RequestTyp typ, const std::string& path, const std::string& message) const;
    HttpResponse::ConnectionState readServer(std::string& received) const;
    HttpResponse::ConnectionState writeServer(const std::string& message) const;
    std::string requestText(const RequestTyp typ, const std::string& message) const;
    std::size_t requestSize(const RequestTyp typ, const std::string_view path, const std::string_view message)const;

private:
    int serverId = 0;
};
