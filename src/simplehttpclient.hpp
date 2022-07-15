#ifndef SIMPLEHTTPCLIENT_H
#define SIMPLEHTTPCLIENT_H

#include <string>
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
    SimpleHttpClient();
    ~SimpleHttpClient();
    HttpResponse::ConnectionState connectToIp(const std::string& ip, int port);
    void close();
    HttpResponse get(const std::string& path, const std::string& message = "") const;
    HttpResponse post(const std::string& path, const std::string& message = "") const;
    HttpResponse put(const std::string& path, const std::string& message = "") const;


    static const std::string protocol;
    static const std::string newLine;
    static const std::string acceptTypes;


protected:
    HttpResponse request(RequestTyp typ, const std::string& path, const std::string& message) const;
    HttpResponse::ConnectionState readServer(std::string& received) const;
    HttpResponse::ConnectionState writeServer(const std::string& message) const;
    std::string requestText(RequestTyp typ, const std::string& message) const;
    int serverId = 0;
};

#endif // SIMPLEHTTPCLIENT_H
