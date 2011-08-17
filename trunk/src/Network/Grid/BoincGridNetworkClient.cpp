
/*
 # include <lbcpp/Network/NetworkInterface.h>
 
 namespace lbcpp
 {
 
 extern ClassPtr gridNetworkInterfaceClass;
 
 class ForwarderGridNetworkInterface : public ForwarderNetworkInterface<GridNetworkInterface>
 {
 public:
 ForwarderGridNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& nodeName)
 : ForwarderNetworkInterface<GridNetworkInterface>(context, client, nodeName) {}
 ForwarderGridNetworkInterface() {}
 
 virtual ContainerPtr pushWorkUnits(ContainerPtr networkRequests);
 virtual ContainerPtr getFinishedExecutionTraces();
 virtual void removeExecutionTraces(ContainerPtr networkResponses);
 };
 
 class SgeGridNetworkInterface : public GridNetworkInterface
 {
 public:
 SgeGridNetworkInterface(ExecutionContext& context, const String& nodeName);
 SgeGridNetworkInterface() {}
 
 virtual ContainerPtr pushWorkUnits(ContainerPtr networkRequests);
 virtual ContainerPtr getFinishedExecutionTraces();
 virtual void removeExecutionTraces(ContainerPtr networkResponses);
 
 protected:
 ExecutionTraceNetworkResponsePtr getExecutionTraceNetworkResponse(const String& identifier)
 {
 File f = context.getFile(T("Traces/") + identifier + T(".trace"));
 if (!f.exists())
 return new ExecutionTraceNetworkResponse(identifier);
 ExecutionTracePtr trace = ExecutionTrace::createFromFile(context, f);
 return new ExecutionTraceNetworkResponse(context, identifier, trace);
 }
 
 File getRequestFile(WorkUnitNetworkRequestPtr request)
 {return context.getFile(T("Requests/") + request->getIdentifier() + T(".request"));}
 
 File getWaitingFile(WorkUnitNetworkRequestPtr request)
 {return context.getFile(T("PreProcessing/") + request->getIdentifier() + T(".workUnit"));}
 
 File getFinishDirectory()
 {return context.getProjectDirectory().getChildFile(T("Finished"));}
 
 void createDirectoryIfNotExists(const String& directory)
 {
 File f = context.getFile(directory);
 if (!f.exists())
 f.createDirectory();
 }
 };
 
 class BoincGridNetworkInterface : public GridNetworkInterface
 {
 public:
 BoincGridNetworkInterface(ExecutionContext& context, const String& nodeName);
 BoincGridNetworkInterface() {}
 
 virtual ContainerPtr pushWorkUnits(ContainerPtr networkRequests);
 virtual ContainerPtr getFinishedExecutionTraces();
 virtual void removeExecutionTraces(ContainerPtr networkResponses);
 
 protected:
 ExecutionTraceNetworkResponsePtr getExecutionTraceNetworkResponse(const String& identifier)
 {
 File f = context.getFile(T("Traces/") + identifier + T(".trace"));
 if (!f.exists())
 return new ExecutionTraceNetworkResponse(identifier);
 ExecutionTracePtr trace = ExecutionTrace::createFromFile(context, f);
 return new ExecutionTraceNetworkResponse(context, identifier, trace);
 }
 
 File getRequestFile(WorkUnitNetworkRequestPtr request)
 {return context.getFile(T("Requests/") + request->getIdentifier() + T(".request"));}
 File getWaitingFile(WorkUnitNetworkRequestPtr request)
 {return context.getFile(T("Waiting/") + request->getIdentifier() + T(".workUnit"));}
 File getFinishDirectory()
 {return context.getProjectDirectory().getChildFile(T("Finished"));}
 void createDirectoryIfNotExists(const String& directory)
 {
 File f = context.getFile(directory);
 if (!f.exists())
 f.createDirectory();
 }
 
 };
 
 };

 BoincGridNetworkInterface::BoincGridNetworkInterface(ExecutionContext& context, const String& Name)
 : GridNetworkInterface(context, Name)
 {
 createDirectoryIfNotExists(T("Requests"));
 createDirectoryIfNotExists(T("Waiting"));
 createDirectoryIfNotExists(T("InProgress"));
 createDirectoryIfNotExists(T("Finished"));
 createDirectoryIfNotExists(T("Traces"));
 }
 
 ContainerPtr BoincGridNetworkInterface::pushWorkUnits(ContainerPtr networkRequests)
 {
 size_t n = networkRequests->getNumElements();
 VectorPtr res = vector(stringType, n);
 for (size_t i = 0; i < n; ++i)
 {
 WorkUnitNetworkRequestPtr request = networkRequests->getElement(i).getObjectAndCast<WorkUnitNetworkRequest>();
 WorkUnitPtr workUnit = request->getWorkUnit(context);
 if (!workUnit)
 {
 res->setElement(i, T("Error"));
 continue;
 }
 
 request->saveToFile(context, getRequestFile(request));
 workUnit->saveToFile(context, getWaitingFile(request));
 res->setElement(i, request->getIdentifier());
 }
 return res;
 }
 
 ContainerPtr BoincGridNetworkInterface::getFinishedExecutionTraces()
 {
 VectorPtr res = vector(workUnitNetworkRequestClass);
 StreamPtr files = directoryFileStream(context, getFinishDirectory(), T("*.workUnit"));
 while (!files->isExhausted())
 {
 String identifier = files->next().getFile().getFileNameWithoutExtension();
 res->append(getExecutionTraceNetworkResponse(identifier));
 }
 return res;
 }
 
 void BoincGridNetworkInterface::removeExecutionTraces(ContainerPtr networkResponses)
 {
 size_t n = networkResponses->getNumElements();
 for (size_t i = 0; i < n; ++i)
 {
 String identifier = networkResponses->getElement(i).getObjectAndCast<ExecutionTraceNetworkResponse>()->getIdentifier();
 File f = context.getFile(T("Traces/") + identifier + T(".trace"));
 f.deleteFile();
 f = context.getFile(T("Requests/") + identifier + T(".request"));
 f.deleteFile();
 f = context.getFile(T("Finished/") + identifier + T(".workUnit"));
 f.deleteFile();
 }
 }
 */
