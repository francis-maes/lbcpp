/*-----------------------------------------.---------------------------------.
| Filename: NetworkRequest.h               | Network Request                 |
| Author  : Julien Becker                  |                                 |
| Started : 01/02/2011 19:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NETWORK_REQUEST_H_
# define LBCPP_NETWORK_REQUEST_H_

# include <lbcpp/Core/Object.h>
# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Core/XmlSerialisation.h>

namespace lbcpp
{

class NetworkRequest;
typedef ReferenceCountedObjectPtr<NetworkRequest> NetworkRequestPtr;

class NetworkRequest : public Object
{
public:
  enum {communicationError, unknown, waitingOnManager, waitingOnServer, running, finished, iDontHaveThisWorkUnit};
  
  NetworkRequest(const String& identifier, const String& projectName, const String& source, const String& destination, size_t requiredCpus = 1, size_t requiredMemory = 2, size_t requiredTime = 10);
  NetworkRequest(NetworkRequestPtr request);
  NetworkRequest() {}
  
  String getIdentifier() const
    {return identifier;}
  
  String getProjectName() const
    {return projectName;}
  
  String getSource() const
    {return source;}
  
  String getDestination() const
    {return destination;}
  
  int getStatus() const
    {return status;}
  
  void setStatus(int status)
    {this->status = status;}
  
  void selfGenerateIdentifier()
    {identifier = generateIdentifier();}
  
protected:
  friend class NetworkRequestClass;
  
  String identifier;
  String projectName;
  String source;
  String destination;
  size_t requiredCpus;
  size_t requiredMemory; // Gb
  size_t requiredTime; // Hour
  int status;

  static juce::int64 lastIdentifier;

  static String generateIdentifier();
};

class WorkUnitNetworkRequest : public NetworkRequest
{
public:
  WorkUnitNetworkRequest(ExecutionContext& context, WorkUnitPtr workUnit, const String& projectName, const String& source, const String& destination, size_t requiredCpus = 1, size_t requiredMemory = 2, size_t requiredTime = 10);
  WorkUnitNetworkRequest(ExecutionContext& context, NetworkRequestPtr request, WorkUnitPtr workUnit);
  WorkUnitNetworkRequest() {}
  
  WorkUnitPtr getWorkUnit() const
    {return workUnitXmlElement->createObjectAndCast<WorkUnit>();}
  
  NetworkRequestPtr getNetworkRequest() const;
  
protected:
  friend class WorkUnitNetworkRequestClass;
  
  XmlElementPtr workUnitXmlElement;
};

typedef ReferenceCountedObjectPtr<WorkUnitNetworkRequest> WorkUnitNetworkRequestPtr;

}; /* namespace */

#endif // !LBCPP_NETWORK_REQUEST_H_
