#include "simplehttpclient.hpp"

#include <array>
#include <list>
#include <map>
#include <numeric>

#include <unistd.h> /* read, write, close */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include <cstdarg>
#include <cstring> /* memcpy, memset */
#include <cstring>

void logHue(const char * fmt, ...){
#ifdef HUE_VERBOSE
    va_list argp;
    va_start(argp, fmt);
    printf("[cppHUE:] ");
    printf(fmt, argp);
    va_end(argp);   
#endif
}

const std::map<SimpleHttpClient::RequestTyp, std::string> requestToName{
    {SimpleHttpClient::GET, "GET"},
    {SimpleHttpClient::POST, "POST"},
    {SimpleHttpClient::PUT, "PUT"}
};

const std::string SimpleHttpClient::protocol = "HTTP/1.0";
const std::string SimpleHttpClient::newLine = "\r\n";
const std::string SimpleHttpClient::acceptTypes = "Accept: */*";
const std::string SimpleHttpClient::connectionType = "Connection: close";
const std::string SimpleHttpClient::contendSizePrefix ="Content-Length: ";

bool HttpResponse::succeeded() const{
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

SimpleHttpClient::~SimpleHttpClient(){
    if(serverId != 0){
        ::close(serverId);
    }
}

void SimpleHttpClient::SimpleHttpClient::close(){
    if(serverId != 0){
        ::close(serverId);
        serverId = 0;
    }
}

HttpResponse::ConnectionState SimpleHttpClient::connectToIp(const std::string& ip, int port){

    int sock = 0;
    struct sockaddr_in sockedAddr;
    sockedAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    sockedAddr.sin_family = AF_INET;
    sockedAddr.sin_port = htons(port);
    sock =  socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0){
        logHue("failed connection \n");
        return HttpResponse::NO_CONNECTION;
    }
    int err = connect(sock, (struct sockaddr *)&sockedAddr, sizeof(struct sockaddr_in6));
    if (err != 0) {
        logHue("Socket unable to connect to %s:%d: error %d errno %d", ip.c_str(), port, err,errno);
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

std::size_t SimpleHttpClient::requestSize(const RequestTyp typ, const std::string_view path, const std::string_view message) const{
    std::size_t size = requestToName.at(typ).size();
    size += path.size();
    size += SimpleHttpClient::protocol.size();
    constexpr std::size_t spaces = 2;
    size += SimpleHttpClient::connectionType.size();
    size += spaces;
    const std::size_t ending = SimpleHttpClient::newLine.size()*2;

    size += ending;

    if(! message.empty()){
        size += SimpleHttpClient::acceptTypes.size();
        size += SimpleHttpClient::newLine.size();
        size += SimpleHttpClient::contendSizePrefix.size();
        std::size_t messageLengthSize = 2;
        std::size_t messageLength = message.size();
        while(messageLength > 0){
            messageLengthSize++;
            messageLength /=10;
        }
        size += messageLengthSize;
        size += ending;
        size += message.size();
    }
    size += ending;
    return size;

}

HttpResponse SimpleHttpClient::request(const RequestTyp typ, const std::string& path, const std::string& message) const{

    std::string header;
    header.reserve(requestSize(typ, path, message));
    //first line
    header += requestToName.at(typ);
    header += " ";
    header += path;
    header += " ";
    header +=  SimpleHttpClient::protocol;
    header += SimpleHttpClient::newLine;
    header += SimpleHttpClient::connectionType + SimpleHttpClient::newLine;
    if(! message.empty()){
        header += SimpleHttpClient::acceptTypes;
        header += SimpleHttpClient::newLine;
        header += SimpleHttpClient::contendSizePrefix + std::to_string(message.size());
        header += SimpleHttpClient::newLine + SimpleHttpClient::newLine;
        header += message;
    }
    header += SimpleHttpClient::newLine + SimpleHttpClient::newLine;
    
    HttpResponse result;
    result.state = writeServer(header);

    if(result.state != HttpResponse::OK){
        logHue("error while writing to server\n");
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
    constexpr int maxChunckSize = 32;
    std::list<std::array<char,maxChunckSize>> memoryPool;
    long chunkSize = 0;
    size_t total = 0;
    long size = 0;
    do {
        if(chunkSize == 0){
            memoryPool.emplace_back(std::array<char, maxChunckSize>());
            chunkSize = maxChunckSize;
        }
        size = read(serverId, memoryPool.rbegin()->data(), chunkSize);
        if (size < 0){
            logHue("ERROR reading response from socket %d\n", size);
            return HttpResponse::NO_CONNECTION;
        }
        chunkSize -= size;
        total += size;
    } while (size != 0);
    if(total == 0){
        return HttpResponse::EMPTY;
    }
    std::string buffer;
    buffer.reserve(total+1); //null terminator;
    long copied = total;
    for(const auto& chunk : memoryPool){
        const long maxSize = (copied > maxChunckSize)?maxChunckSize : copied;
        for(int i=0; i<maxSize; i++){
            buffer +=chunk.at(i);
        }
        copied -= maxSize;
    }
    received = std::move(buffer);
    return HttpResponse::OK;
}

HttpResponse::ConnectionState SimpleHttpClient::writeServer(const std::string& message) const{
    if(serverId == 0){
        logHue("server is closed\n");
        return HttpResponse::NO_CONNECTION;
    }
    const char* raw = message.c_str();
    size_t send = 0;

    do {
        long bytes = write(serverId,&(raw[send]),message.size()-send);
        if (bytes < 0){
            logHue("ERROR writing message to socket\n");
            return HttpResponse::NO_CONNECTION;
        }
        if (bytes == 0){
            break;
        }
        send+=bytes;
    } while (send < message.size());
    return HttpResponse::OK;
}
