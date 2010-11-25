#include <lbcpp/lbcpp.h>
#if 0
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "ClientServer.h"

using namespace lbcpp;

class IPv4InetAddress;
class IPv6InetAddress;

class InetAddress
{
public:
  virtual ~InetAddress() {}
  
  virtual void loadFromSockAddr(const struct sockaddr*const addrress) = 0;

  virtual int getFamily() const = 0;
  
  virtual size_t getLength() const = 0;
  
  virtual void setPort(size_t port) = 0;
  
  virtual size_t getPort() const = 0;
  
  virtual void setAddress(const String& addr) = 0;
  
  virtual String getAddress() const = 0;
  
  virtual struct sockaddr* getAddress() = 0;
  
  virtual String toString() const = 0;
};

class IPv6InetAddress : public InetAddress
{
public:
  IPv6InetAddress()
  {
    bzero(&address, sizeof(address));
    address.sin6_family = AF_INET6;
  }
  
  virtual void loadFromSockAddr(const struct sockaddr* const addr)
  {
    struct sockaddr_in6* ipv6Addr = (struct sockaddr_in6*)addr;
    memcpy(&address.sin6_addr, &ipv6Addr->sin6_addr, sizeof(struct in6_addr));
    address.sin6_port = ipv6Addr->sin6_port;
  }
  
  virtual int getFamily() const
    {return AF_INET6;}
  
  virtual size_t getLength() const
    {return sizeof(address);}
  
  virtual void setPort(size_t port)
    {address.sin6_port = htons(port);}
  
  virtual size_t getPort() const
    {return ntohs(address.sin6_port);}
  
  virtual void setAddress(const String& addr)
    {inet_pton(AF_INET6, addr.toUTF8(), &address.sin6_addr);}
  
  virtual String getAddress() const
  {
    char res[INET6_ADDRSTRLEN];
    bzero(res, INET6_ADDRSTRLEN);
    inet_ntop(AF_INET6, &address.sin6_addr, res, INET6_ADDRSTRLEN);
    return res;
  }

  virtual struct sockaddr* getAddress()
    {return (struct sockaddr*)&address;}

  virtual String toString() const
    {return T("IPv6: ") + getAddress();}
  
protected:
  struct sockaddr_in6 address;
};

class IPv4InetAddress : public InetAddress
{
public:
  IPv4InetAddress()
  {
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
  }
  
  virtual void loadFromSockAddr(const struct sockaddr* const addr)
  {
    struct sockaddr_in* ipv4Addr = (struct sockaddr_in*)addr;
    memcpy(&address.sin_addr, &ipv4Addr->sin_addr, sizeof(struct in_addr));
    address.sin_port = ipv4Addr->sin_port;
  }
  
  virtual int getFamily() const
    {return AF_INET;}
  
  virtual size_t getLength() const
    {return sizeof(address);}
  
  virtual void setPort(size_t port)
    {address.sin_port = htons(port);}
  
  virtual size_t getPort() const
    {return ntohs(address.sin_port);}
  
  virtual void setAddress(const String& addr)
    {inet_pton(AF_INET, addr.toUTF8(), &address.sin_addr);}
  
  virtual String getAddress() const
  {
    char res[INET_ADDRSTRLEN];
    bzero(res, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &address.sin_addr, res, INET_ADDRSTRLEN);
    return res;
  }
  
  virtual struct sockaddr* getAddress()
    {return (struct sockaddr*)&address;}
  
  virtual String toString() const
    {return T("IPv4: ") + getAddress();}
  
protected:
  struct sockaddr_in address;
};

InetAddressPtr inetAddressByHostName(const String& hostName)
{
  struct addrinfo* addresses;
  if (getaddrinfo(hostName.toUTF8(), NULL, NULL, &addresses) != 0)
    return InetAddressPtr();
  /* select IPv6 address if exists */
  struct addrinfo* bestAddress;
  for (struct addrinfo* it = addresses; it != NULL; it = it->ai_next)
  {
    bestAddress = it;
    if (it->ai_family == AF_INET6)
      break;
  }
  /* copy data */
  InetAddressPtr res;
  switch (bestAddress->ai_family)
  {
    case AF_INET:
      res = new IPv4InetAddress();
      break;
    case AF_INET6:
      res = new IPv6InetAddress();
      break;
    default:
      return InetAddressPtr();
  }
  
  res->loadFromSockAddr(bestAddress->ai_addr);
  freeaddrinfo(addresses);
  
  return res;
}

static String getErrorMessage()
{
  char error[256];
  bzero(error, 256);
  strerror_r(errno, error, 255);
  return error;
}
  /*************************/
 /*** SocketInputStream ***/
/*************************/
class SocketInputStream : public InputStream
{
public:
  SocketInputStream(int socketFileDescriptor, MessageCallback& callback)
    : callback(callback), socketDescriptor(socketFileDescriptor)
    {}
  
  virtual juce::int64 getTotalLength()
  {
    loadBuffer(false);
    return bufferQueue.size();
  }
  
  virtual bool isExhausted()
  {
    loadBuffer(false);
    return bufferQueue.size() == 0;
  }
  
  virtual int read(void* buffer, int bytesToRead)
  {
    jassert(bytesToRead >= 0);
    loadBuffer(false);
    
    if (!bufferQueue.size())
      loadBuffer(true);
    
    size_t n = (int)bufferQueue.size();
    if ((size_t)bytesToRead < n)
      n = bytesToRead;
    
    for (size_t i = 0; i < n; ++i)
    {
      ((char*)buffer)[i] = bufferQueue.front();
      bufferQueue.pop_front();
    }
    
    return n;
  }
  
  virtual juce::int64 getPosition()
    {return 0;}
  
  virtual bool setPosition(juce::int64)
    {return false;}
  
protected:
  MessageCallback& callback;
  
  int socketDescriptor;
  std::deque<char> bufferQueue;
  
private:
  enum {bufferSize = 512};
  
  void loadBuffer(bool blockAndWait)
  {
    char buffer[bufferSize + 1];
    bzero(buffer, bufferSize + 1);
    int numByteRead = recv(socketDescriptor, buffer, bufferSize, blockAndWait ? 0 : MSG_DONTWAIT);

    if (!blockAndWait && numByteRead == -1)
      return;
    
    if (numByteRead == -1)
      callback.errorMessage(T("SocketInputStream"), getErrorMessage());
    else
    {
      for (size_t i = 0; i < (size_t)numByteRead; ++i)
        bufferQueue.push_back(buffer[i]);
      
      if (numByteRead == bufferSize)
        loadBuffer(false); // Do not block, even in case of blockAndWait set to TRUE
      // because we already have waited for the bufferSize first bytes
    }
  }
};
  /**************************/
 /*** SocketOutputStream ***/
/**************************/
class SocketOutputStream : public OutputStream
{
public:
  SocketOutputStream(int socketFileDescriptor, MessageCallback& callback)
    : callback(callback), socketDescriptor(socketFileDescriptor) {}

  virtual void flush() {}
  
  virtual bool setPosition(juce::int64)
    {return false;}
  
  virtual juce::int64 getPosition()
    {return 0;}
  
  virtual bool write(const void* const buffer, int numBytes)
  {
    if (::send(socketDescriptor, buffer, numBytes, 0) < 0)
    {
      callback.errorMessage(T("SocketOutputStream::write"), getErrorMessage());
      return false;
    }
    return true;
  }
  
protected:
  
  MessageCallback& callback;
  int socketDescriptor;
};

  /**************/
 /*** Socket ***/
/**************/
Socket::Socket(InetAddressPtr inetAddress, size_t hostPort, MessageCallback& callback)
: address(inetAddress), port(hostPort), socketDescriptor(-1), connected(false), callback(callback)
{
  if (address)
    createAndConnectSocket();
}

void Socket::close()
{
  if (socketDescriptor < 0)
    return;
  shutdown(socketDescriptor, 2);
  connected = false;
}

InputStreamPtr Socket::getInputStream()
{
  if (!isConnected())
    return InputStreamPtr();
  static InputStreamPtr res = new SocketInputStream(socketDescriptor, callback);
  return res;
}

OutputStreamPtr Socket::getOutputStream()
{
  if (!isConnected())
    return OutputStreamPtr();
  static OutputStreamPtr res = new SocketOutputStream(socketDescriptor, callback);
  return res;
}

void Socket::createAndConnectSocket()
{
  connected = false;
  socketDescriptor = ::socket(address->getFamily(), SOCK_STREAM, 0);
  if (socketDescriptor < 0)
  {
    callback.errorMessage(T("Socket::createAndConnectSocket"), getErrorMessage());
    return;
  }
  
  address->setPort(port);
  if (::connect(socketDescriptor, address->getAddress(), address->getLength()) == 0)
  {
    connected = true;
    return;
  }
  
  callback.errorMessage(T("Socket::createAndConnectSocket"), getErrorMessage());
}
  /********************/
 /*** ServerSocket ***/
/********************/
void ServerSocket::close()
{
  if (socketDescriptor < 0)
    return;
  shutdown(socketDescriptor, 2);
  bound = false;
}

SocketPtr ServerSocket::accept()
{
  int clientDescriptor = ::accept(socketDescriptor, NULL, NULL);
  if (clientDescriptor < 0)
  {
    callback.errorMessage(T("Socket::accept"), getErrorMessage());
    return SocketPtr();
  }

  return Socket::createFromServer(clientDescriptor, callback);
}

void ServerSocket::createAndBindSocket()
{
  bound = false;
  socketDescriptor = ::socket(AF_INET6, SOCK_STREAM, 0);
  if (socketDescriptor < 0)
  {
    callback.errorMessage(T("Socket::createSocket"), getErrorMessage());
    return;
  }
  
  struct sockaddr_in6 addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin6_family = AF_INET6;
  addr.sin6_port = htons(port);
  addr.sin6_addr = in6addr_any;
  if (::bind(socketDescriptor, (struct sockaddr*)&addr, sizeof addr) == 0)
    if (::listen(socketDescriptor, 5) == 0)
    {
      bound = true;
      return;
    }
  
  callback.errorMessage(T("ServerSocket::createAndBindSocket"), getErrorMessage());
}

#endif // JUCE_LINUX