
#ifndef LBCPP_CLIENT_SERVER_H_
# define LBCPP_CLIENT_SERVER_H_
# include <lbcpp/lbcpp.h>

using namespace lbcpp;

class InetAddress;
typedef ReferenceCountedObjectPtr<InetAddress> InetAddressPtr;

class Socket;
typedef ReferenceCountedObjectPtr<Socket> SocketPtr;

class ServerSocket;
typedef ReferenceCountedObjectPtr<ServerSocket> ServerSocketPtr;

typedef ReferenceCountedObjectPtr<OutputStream> OutputStreamPtr;
typedef ReferenceCountedObjectPtr<InputStream> InputStreamPtr;

extern InetAddressPtr inetAddressByHostName(const String& hostName);

class Socket
{
public:  
  Socket(InetAddressPtr inetAddress, size_t hostPort, MessageCallback& callback = MessageCallback::getInstance());
  
  Socket(const String& hostName, size_t hostPort, MessageCallback& callback = MessageCallback::getInstance())
    : callback(callback)
    {Socket(inetAddressByHostName(hostName), hostPort, callback);}
  
  ~Socket() {}
  
  bool isConnected()
    {return connected;}
  
  void close();

  InputStreamPtr getInputStream();

  OutputStreamPtr getOutputStream();

  static SocketPtr createFromServer(int clientDescriptor, MessageCallback& callback)
    {return new Socket(clientDescriptor, callback);}
  
protected:
  InetAddressPtr address;
  size_t port;
  int socketDescriptor;
  bool connected;
  MessageCallback& callback;
  
  Socket(int clientFileDescriptor, MessageCallback& callback)
    : address(InetAddressPtr()), port(0), socketDescriptor(clientFileDescriptor), connected(true), callback(callback)
    {}
  
  void createAndConnectSocket();
};

class ServerSocket
{
public:
  ServerSocket(size_t port, MessageCallback& callback = MessageCallback::getInstance())
  : port(port), callback(callback), socketDescriptor(-1), bound(false)
    {createAndBindSocket();}
  
  ~ServerSocket() {}
  
  bool isBound()
    {return bound;}
  
  void close();
  
  SocketPtr accept();
  
protected:
  size_t port;
  MessageCallback& callback;
  
  int socketDescriptor;
  bool bound;
  
  void createAndBindSocket();
};

class ServerProgram : public Program
{
  virtual int runProgram(MessageCallback& callback)
  {
    ServerSocketPtr server = new ServerSocket(10021, callback);
    if (!server->isBound())
      return -1;

    std::cout << "Waiting client ... " << std::flush;
    SocketPtr client = server->accept();
    if (!client)
    {
      std::cout << "Error !" << std::endl;
      return -1;
    }
    std::cout << "Connected" << std::endl;

    
    InputStreamPtr is = client->getInputStream();
    OutputStreamPtr os = client->getOutputStream();
    String msg;
    
    while (msg != T("QUIT"))
    {
      msg = is->readString();
      while (!is->isExhausted())
        msg += is->readString();
      std::cout << msg << std::endl;

      std::cout << "Your message: ";
      
      std::string buffer;
      std::getline(std::cin, buffer);
      
      msg = buffer.data();
      os->writeString(msg);
    }
    client->close();
    server->close();

    return 0;
  }
};

class ClientProgram : public Program
{
  virtual int runProgram(MessageCallback& callback)
  {
    InetAddressPtr ip = inetAddressByHostName(T("192.168.1.3"));
    if (!ip)
      return -1;

    SocketPtr socket = new Socket(ip, 10021, callback);
    if (!socket->isConnected())
    {
      std::cout << "Not connected !" << std::endl;
      return -2;
    }

    OutputStreamPtr os = socket->getOutputStream();
    InputStreamPtr is = socket->getInputStream();
    String msg;

    while (msg != T("QUIT"))
    {
      os->writeString(T("Hey, have you something for me ?"));
      
      msg = is->readString();
      while (!is->isExhausted())
        msg += is->readString();
      
      std::cout << "Message: " << msg << std::endl;
    }
    os->writeString(T("QUIT"));
    
    socket->close();

    return 0;
  }
};

#endif //!LBCPP_CLIENT_SERVER_H_
