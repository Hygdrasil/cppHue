#include "simplehttpclient.h"
#include <sstream>
#include <map>

#include <unistd.h> /* read, write, close */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include <string.h> /* memcpy, memset */
#include <cstring>

const std::map<SimpleHttpClient::RequestTyp, std::string> requestToName{
    {SimpleHttpClient::GET, "GET"},
    {SimpleHttpClient::POST, "POST"},
    {SimpleHttpClient::PUT, "PUT"}
};

const std::string SimpleHttpClient::protocol = "HTTP/1.0";
const std::string SimpleHttpClient::newLine = "\r\n";
const std::string SimpleHttpClient::acceptTypes = "Accept: */*";

bool HttpResponse::succeded() const{
    return state == HttpResponse::OK
            && statusCode() == 200;
}

unsigned int HttpResponse::statusCode() const{
    if(message.size() < 15){
        return 0;
    }
    const char* codeHeading = (char*)memchr(message.c_str(), ' ', 15);
    if((size_t)(codeHeading - message.c_str()) +5 > message.size()){
        return 0;
    }
    if(codeHeading[4] != ' '){
        return 0;
    }
    unsigned int value = 0;
    for(int i = 1; i<4; i++){
        char part = codeHeading[i];
        if( part < '0' || part > '9' )
        {
            return 0;
        }
        value*=10;
        value+=part-'0';
    }
    return value;
}

SimpleHttpClient::SimpleHttpClient()
{
}

HttpResponse::ConnectionState SimpleHttpClient::connectToIp(const std::string& ip, int port){

    int sock = 0;
    struct sockaddr_in sockedAddr;
    sockedAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    sockedAddr.sin_family = AF_INET;
    sockedAddr.sin_port = htons(port);
    sock =  socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0){
        return HttpResponse::NO_CONNECTION;
    }
    printf("sockedId: %d\n", sock);
    int err = connect(sock, (struct sockaddr *)&sockedAddr, sizeof(struct sockaddr_in6));
    if (err != 0) {
        printf("Socket unable to connect to %s:%d: error %d errno %d", ip.c_str(), port, err,errno);
        return HttpResponse::NO_CONNECTION;
    }
    serverId = sock;
    return HttpResponse::OK;
}

HttpResponse SimpleHttpClient::get(const std::string& path, const std::string& message) const{
    return request(GET, path, message);
}

HttpResponse SimpleHttpClient::post(const std::string& path, const std::string& message) const{
    return request(POST, path, message);
}

HttpResponse SimpleHttpClient::put(const std::string& path, const std::string& message) const{
    return request(PUT, path, message);
}

HttpResponse SimpleHttpClient::request(RequestTyp typ, const std::string& path, const std::string& message) const{
    printf("in request\n");
    std::stringstream header;
    //first line
    header << requestToName.at(typ);
    header << " ";
    header << path;
    header << " ";
    header << SimpleHttpClient::protocol;
    header << SimpleHttpClient::newLine;
    if(! message.empty()){
        header << SimpleHttpClient::acceptTypes;
        header << SimpleHttpClient::newLine;

        header << "Content-Length: " << message.size() +2;
        header << SimpleHttpClient::newLine << SimpleHttpClient::newLine;
        header << message;
    }
    header << SimpleHttpClient::newLine << SimpleHttpClient::newLine;
    HttpResponse result;
    result.state = writeServer(header.str());

    printf("wrode to server\n");
    if(result.state != HttpResponse::OK){
        printf("error while writing to server\n");
        result.message = "";
        return result;
    }

    result.state =  readServer(result.message);
    return result;
}


HttpResponse::ConnectionState SimpleHttpClient::readServer(std::string& received) const{
    if(serverId == 0){
        return HttpResponse::NO_CONNECTION;
    }
    std::stringstream memoryPool;
    const int shortBufferSize = 32+1;
    char shortBuffer[shortBufferSize];
    int size;
    size_t total = 0;
    do {
        size = read(serverId,shortBuffer, shortBufferSize-1);
        if (size < 0){
            printf("ERROR reading response from socket %d\n", size);
            return HttpResponse::NO_CONNECTION;
        }
        shortBuffer[size] = '\0';
        memoryPool << shortBuffer;
        total += size;
    } while (size != 0);
    received = memoryPool.str();
   return ((total>0)? HttpResponse::OK: HttpResponse::EMPTY);
}

HttpResponse::ConnectionState SimpleHttpClient::writeServer(const std::string& message) const{
    if(serverId == 0){
        return HttpResponse::NO_CONNECTION;
    }
    const char* raw = message.c_str();
    printf("\"%s\"", message.c_str());
    size_t send = 0;
    int bytes;

    do {
        bytes = write(serverId,&(raw[send]),message.size()-send);
        if (bytes < 0){
            printf("ERROR writing message to socket\n");
            return HttpResponse::NO_CONNECTION;
        }
        if (bytes == 0){
            break;
        }
        send+=bytes;
    } while (send < message.size());
    return HttpResponse::OK;
}
