
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

#if 0
class Socket
{
public:  
  Socket(ExecutionContext& context, InetAddressPtr inetAddress, size_t hostPort);
  
  Socket(ExecutionContext& context, const String& hostName, size_t hostPort)
    : context(context)
    {Socket(inetAddressByHostName(hostName), hostPort, callback);}
  
  ~Socket() {}
  
  bool isConnected()
    {return connected;}
  
  void close();

  InputStreamPtr getInputStream();

  OutputStreamPtr getOutputStream();

  static SocketPtr createFromServer(ExecutionContext& context, int clientDescriptor)
    {return new Socket(context, clientDescriptor);}
  
protected:
  InetAddressPtr address;
  size_t port;
  int socketDescriptor;
  bool connected;
  ExecutionContext& context;
  
  Socket(ExecutionContext& context, int clientFileDescriptor)
    : address(InetAddressPtr()), port(0), socketDescriptor(clientFileDescriptor), connected(true), context(context)
    {}
  
  void createAndConnectSocket();
};

class ServerSocket
{
public:
  ServerSocket(ExecutionContext& context, size_t port)
  : port(port), context(context), socketDescriptor(-1), bound(false)
    {createAndBindSocket();}
  
  ~ServerSocket() {}
  
  bool isBound()
    {return bound;}
  
  void close();
  
  SocketPtr accept();
  
protected:
  size_t port;
  ExecutionContext& context;
  
  int socketDescriptor;
  bool bound;
  
  void createAndBindSocket();
};
#endif

class ServerProgram : public WorkUnit
{
  virtual bool run(ExecutionContext& context)
  {
#if 0
    ServerSocketPtr server = new ServerSocket(context, 10021);
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
#endif 
    return true;
  }
};

class ClientProgram : public WorkUnit
{
  virtual bool run(ExecutionContext& context)
  {
#if 0
    InetAddressPtr ip = inetAddressByHostName(T("192.168.1.3"));
    if (!ip)
      return false;

    SocketPtr socket = new Socket(context, ip, 10021);
    if (!socket->isConnected())
    {
      std::cout << "Not connected !" << std::endl;
      return false;
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
#endif
    return true;
  }
};

#endif //!LBCPP_CLIENT_SERVER_H_
