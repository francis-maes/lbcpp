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
  Notification();

  virtual void notify(const ObjectPtr& target) = 0;

  const Time& getConstructionTime() const
    {return constructionTime;}

  Thread::ThreadID getSourceThreadId() const
    {return sourceThreadId;}

private:
  Thread::ThreadID sourceThreadId;
  Time constructionTime;
};

typedef ReferenceCountedObjectPtr<Notification> NotificationPtr;

class NotificationQueue : public Consumer
{
public:
  virtual void consume(ExecutionContext& context, const Variable& variable)
    {push(variable.getObjectAndCast<Notification>());}
  
  void push(const NotificationPtr& notification);
  void flush(const ObjectPtr& target);
  bool isEmpty() const;

protected:
  CriticalSection lock;
  std::vector<NotificationPtr> notifications;
};

typedef ReferenceCountedObjectPtr<NotificationQueue> NotificationQueuePtr;

}; /* namespace smode */

#endif // !LBCPP_EXECUTION_NOTIFICATION_H_
