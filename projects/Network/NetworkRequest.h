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

namespace lbcpp
{

class NetworkRequest;
typedef ReferenceCountedObjectPtr<NetworkRequest> NetworkRequestPtr;

class NetworkRequest : public Object
{
public:
  enum {communicationError, unknown, waitingOnManager, waitingOnServer, running, finished, iDontHaveThisWorkUnit};
  
  NetworkRequest(const String& identifier, const String& projectName, const String& source, const String& destination);
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
  int status;
  
  static juce::int64 lastIdentifier;
  
  static String generateIdentifier();
};

class WorkUnitNetworkRequest : public NetworkRequest
{
public:
  WorkUnitNetworkRequest(WorkUnitPtr workUnit, const String& projectName, const String& source, const String& destination);
  WorkUnitNetworkRequest(NetworkRequestPtr request, WorkUnitPtr workUnit);
  WorkUnitNetworkRequest() {}
  
  WorkUnitPtr getWorkUnit() const
    {return workUnit;}
  
  NetworkRequestPtr getNetworkRequest() const;
  
protected:
  friend class WorkUnitNetworkRequestClass;
  
  WorkUnitPtr workUnit;
};

typedef ReferenceCountedObjectPtr<WorkUnitNetworkRequest> WorkUnitNetworkRequestPtr;

}; /* namespace */

#endif // !LBCPP_NETWORK_REQUEST_H_
