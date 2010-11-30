/*-----------------------------------------.---------------------------------.
| Filename: Notification.h                 | Notification Class              |
| Author  : Francis Maes                   |                                 |
| Started : 26/11/2010 19:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_NOTIFICATION_H_
# define LBCPP_EXECUTION_NOTIFICATION_H_

# include "ExecutionContext.h"
# include "../Core/Variable.h"
# include "../Data/Consumer.h"

namespace lbcpp
{

class Notification : public Object
{
public:
  virtual void notify() = 0;
};

typedef ReferenceCountedObjectPtr<Notification> NotificationPtr;

class NotificationQueue : public Consumer
{
public:
  virtual void consume(ExecutionContext& context, const Variable& variable)
    {push(variable.getObjectAndCast<Notification>());}
  
  void push(const NotificationPtr& notification) 
  {
    ScopedLock _(lock);
    notifications.push_back(notification);
  }

  void flush()
  {
    std::vector<NotificationPtr> notifications;
    {
      ScopedLock _(lock);
      this->notifications.swap(notifications);
    }
    for (size_t i = 0; i < notifications.size(); ++i)
      notifications[i]->notify();
  }

  bool isEmpty() const
    {ScopedLock _(lock); return notifications.empty();}

protected:
  CriticalSection lock;
  std::vector<NotificationPtr> notifications;
};

typedef ReferenceCountedObjectPtr<NotificationQueue> NotificationQueuePtr;

}; /* namespace smode */

#endif // !LBCPP_EXECUTION_NOTIFICATION_H_
