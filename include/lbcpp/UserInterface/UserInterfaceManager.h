/*-----------------------------------------.---------------------------------.
| Filename: UserInterfaceManager.h         | User Interface Manager          |
| Author  : Francis Maes                   |                                 |
| Started : 26/11/2010 18:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_USER_INTERFACE_MANAGER_H_
# define LBCPP_USER_INTERFACE_MANAGER_H_

# include "../Execution/ExecutionContext.h"
# include "../Execution/Notification.h"
# include "../Data/Consumer.h"

namespace lbcpp
{

class UserInterfaceThread;
class UserInterfaceManager
{
public:
  UserInterfaceManager();
  ~UserInterfaceManager();

  void ensureIsInitialized(ExecutionContext& context);
  bool isRunning() const;
  void shutdown();

  bool hasAtLeastOneVisibleWindowOnDesktop() const;
  void waitUntilAllWindowsAreClosed();

  typedef void* (MessageCallbackFunction) (void* userData);
  void* callFunctionOnMessageThread(MessageCallbackFunction* callback, void* userData);

  const NotificationQueuePtr& getNotificationQueue() const
    {return notifications;}

  juce::Image* getImage(const String& fileName);
  juce::Image* getImage(const String& fileName, int width, int height);

  juce::Component* createComponentIfExists(ExecutionContext& context, const ObjectPtr& object, const String& name = String::empty) const;
  juce::TreeView* createVariableTreeView(ExecutionContext& context, const Variable& variable, const String& name = String::empty,
                                          bool showTypes = true, bool showShortSummaries = true, bool showMissingVariables = false, bool makeRootNodeVisible = true) const;
  juce::TableListBox* createContainerTableListBox(ExecutionContext& context, const ContainerPtr& container) const;

private:
  UserInterfaceThread* userInterfaceThread;
  NotificationQueuePtr notifications;
};

extern UserInterfaceManager& userInterfaceManager();

}; /* namespace smode */

#endif // !LBCPP_USER_INTERFACE_MANAGER_H_
