/*-----------------------------------------.---------------------------------.
| Filename: WorkUnitNetworkRequest.h       |  Work Unit Network Request      |
| Author  : Julien Becker                  |                                 |
| Started : 17/08/2011 16:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_WORK_UNIT_NETWORK_REQUEST_H_
# define LBCPP_WORK_UNIT_NETWORK_REQUEST_H_

# include <lbcpp/Core/Object.h>
# include <lbcpp/Core/XmlSerialisation.h>

namespace lbcpp
{

class WorkUnitNetworkRequest : public Object
{
public:
  WorkUnitNetworkRequest(ExecutionContext& context,
                         const WorkUnitPtr& workUnit,
                         const String& projectName, const String& source, const String& destination,
                         size_t requiredCpus, size_t requiredMemory, size_t requiredTime)
  : workUnit(new XmlElement()),
    projectName(projectName), source(source), destination(destination),
    requiredCpus(requiredCpus), requiredMemory(requiredMemory), requiredTime(requiredTime)
    {this->workUnit->saveObject(context, workUnit, T("workUnit"));}
  
 void setUniqueIdentifier(const String& uniqueIdentifier)
    {this->uniqueIdentifier = uniqueIdentifier;}
  
  const String& getUniqueIdentifier() const
    {return uniqueIdentifier;}

  const String& getProjectName() const
    {return projectName;}

  const String& getSource() const
    {return source;}

  const String& getDestination() const
    {return destination;}

  const size_t& getRequiredMemory() const
    {return requiredMemory;}

  const size_t& getRequiredTime() const
    {return requiredTime;}

  WorkUnitPtr getWorkUnit(ExecutionContext& context) const
    {return workUnit ? workUnit->createObjectAndCast<WorkUnit>(context) : WorkUnitPtr();}

  XmlElementPtr getXmlElementWorkUnit() const
    {return workUnit;}

protected:
  friend class WorkUnitNetworkRequestClass;

  WorkUnitNetworkRequest() {}

  String uniqueIdentifier;
  XmlElementPtr workUnit;
  String projectName;
  String source;
  String destination;
  size_t requiredCpus;
  size_t requiredMemory; // Gb
  size_t requiredTime; // Hour
};

typedef ReferenceCountedObjectPtr<WorkUnitNetworkRequest> WorkUnitNetworkRequestPtr;

}; /* namespace */
  
#endif // !LBCPP_WORK_UNIT_NETWORK_REQUEST_H_
