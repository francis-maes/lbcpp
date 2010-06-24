/*
 *  SshConnection.h
 *  LBCpp
 *
 *  Created by Becker Julien on 12/05/10.
 *  Copyright 2010 University of Liège. All rights reserved.
 *
 */

#include <ne7ssh.h>

using namespace lbcpp;

class SshConnection
{
public:
  SshConnection(String userName, String userPassword, String remoteHost, size_t remotePort)
  : userName(userName), userPassword(userPassword), remoteHost(remoteHost), remotePort(remotePort), retry(maximumRetry), channel(-1), ssh(new ne7ssh())
  {
    ssh->setOptions("aes192-cbc", "hmac-md5");
  }
  
  virtual ~SshConnection()
  {ssh->close(channel); delete ssh;}
  
  void test(String cmd)
  {
    channel = ssh->connectWithPassword(remoteHost, remotePort, userName, userPassword);
    std::cout << "Send command: " << cmd << std::endl;
    ssh->send ("echo OK\n", channel);
    std::cout << "Waiting for response ... " << std::endl;
    ssh->waitFor(channel, " $", 10);
    std::cout << "Response: " << ssh->read(channel) << std::endl;
  }
  
private:
  bool initiateConnection()
  {
    while (channel < 0 && retry)
    {
      channel = ssh->connectWithPassword(remoteHost, remotePort, userName, userPassword);
      jassert(channel);
      
      if (channel >= 0 && !ssh->waitFor(channel, " $", 10))
      {
        clearConnection();
        retry--;
      }      
    }
    
    if (!retry)
      return false;
    retry = maximumRetry;
    return true;
  }
  
  void clearConnection()
  {ssh->close(channel); channel = -1;}
  
  bool checkSSHConnection()
  {
    initiateConnection();
    
    while (!ssh->send ("echo OK\n", channel) && retry)
    {
      clearConnection();
      initiateConnection();
      
      retry--;
    }
    
    if (!retry)
      return false;
    retry = maximumRetry;
    
    // wait responce
    if (!ssh->waitFor(channel, " $", 5))
    {
      clearConnection();
      return false;
    }
    
    // read result
    std::cout << "[SSH] " << ssh->read(channel) << std::endl;
  }
  
  enum {maximumRetry = 3};
  
  String userName;
  String userPassword;
  String remoteHost;
  size_t remotePort;
  size_t retry;
  int channel;
  
  ne7ssh* ssh;
};
