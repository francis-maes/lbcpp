/*-----------------------------------------.---------------------------------.
| Filename: NetworkClient.cpp              | Network Client                  |
| Author  : Julien Becker                  |                                 |
| Started : 01/12/2010 18:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Network/NetworkClient.h>
#include <lbcpp/Core/XmlSerialisation.h>
#include <lbcpp/Network/NetworkMessage.h>

using namespace lbcpp;

/** NetworkClient **/
NetworkClient::NetworkClient(ExecutionContext& context)
  : InterprocessConnection(false, magicNumber), context(context) {}

bool NetworkClient::startClient(const String& host, int port)
{
  const size_t maxTimeToSleep = 300000; // 5"
  size_t timeToSleep = 1000;
  while (!connectToSocket(host, port, 1000))
  {
    context.warningCallback(T("NetworkClient::startClient"), T("Connection to ")
                            + host + T(" at port ") + String((int)port) 
                            + (" failed ! Retry in ") + String((int)timeToSleep / 1000) + T("s"));
    juce::Thread::sleep(timeToSleep);

    timeToSleep *= 2;
    if (timeToSleep > maxTimeToSleep)
      timeToSleep = maxTimeToSleep;
  }
  return isConnected();
}

void NetworkClient::stopClient()
{
  disconnect();
}

bool NetworkClient::sendVariable(const Variable& variable)
{
  XmlExporter exporter(context);
  exporter.saveVariable(String::empty, variable, TypePtr());
  String text = exporter.toString();
  if (text == String::empty)
    return false;
#ifdef JUCE_DEBUG
  context.informationCallback(T("sendVariable"), text);
#endif // !JUCE_DEBUG
  juce::MemoryBlock block(text.toUTF8(), text.length());
  return sendMessage(block);
}

void NetworkClient::messageReceived(const juce::MemoryBlock& message)
{
#ifdef JUCE_DEBUG
  context.informationCallback(T("messageReceived"), message.toString());
#endif // !JUCE_DEBUG
  juce::XmlDocument document(message.toString());
  XmlImporter importer(context, document);
  variableReceived(importer.isOpened() ? importer.load() : Variable());
}

bool lbcpp::isValidNetworkMessage(ExecutionContext& context, const Variable& variable)
{
  if (!variable.inheritsFrom(networkMessageClass) || !variable.exists())
  {
    context.warningCallback(T("isValidNetworkMessage")
                            , T("The message is not a valid NetworkMessage ! The message is ") + variable.toString().quoted());
    return false;
  }

  return true;
}

void ManagerNetworkClient::variableReceived(const Variable& variable)
{
  if (!isValidNetworkMessage(context, variable))
    return;

  const ObjectPtr obj = variable.getObject();
  const ClassPtr objClass = obj->getClass();
  if (objClass == workUnitAcknowledgementNetworkMessageClass)
  {
    WorkUnitAcknowledgementNetworkMessagePtr message = obj.staticCast<WorkUnitAcknowledgementNetworkMessage>();
    if (callback)
      callback->workUnitAcknowledgementReceived(message->getSourceIdentifier(), message->getUniqueIdentifier());
  }
  else if (objClass == workUnitResultNetworkMessageClass)
  {
    WorkUnitResultNetworkMessagePtr message = obj.staticCast<WorkUnitResultNetworkMessage>();
    if (callback)
      callback->workUnitResultReceived(message->getUniqueIdentifier(), message->getResult(context));
  }
  else
    context.warningCallback(T("ManagerNetworkClient::variableReceived")
                            , T("Unknwon object of type: ") + objClass->toString());
}


bool ManagerNetworkClient::sendWorkUnit(size_t sourceIdentifier, const WorkUnitPtr& workUnit,
                          const String& projectName, const String& source, const String& destination,
                          size_t requiredCpus, size_t requiredMemory, size_t requiredTime)
{
  return sendVariable(new WorkUnitRequestNetworkMessage(context, sourceIdentifier, workUnit, projectName, source, destination, requiredCpus, requiredMemory, requiredTime));
}

void GridNetworkClient::variableReceived(const Variable& variable)
{
  if (!isValidNetworkMessage(context, variable))
    return;

  const ObjectPtr obj = variable.getObject();
  const ClassPtr objClass = obj->getClass();
  if (objClass == workUnitRequestsNetworkMessageClass)
  {
    WorkUnitRequestsNetworkMessagePtr message = obj.staticCast<WorkUnitRequestsNetworkMessage>();
    callback->workUnitRequestsReceived(message->getWorkUnitRequests());
  }
  else
    context.warningCallback(T("GridNetworkClient::variableReceived")
                            , T("Unknwon object of type: ") + objClass->toString());
}
