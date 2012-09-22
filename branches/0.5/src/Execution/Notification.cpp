/*-----------------------------------------.---------------------------------.
| Filename: Notification.cpp               | Notification Class              |
| Author  : Francis Maes                   |                                 |
| Started : 30/11/2010 18:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Execution/Notification.h>
using namespace lbcpp;

/*
** Notification
*/
Notification::Notification()
{
  sourceThreadId = Thread::getCurrentThreadId();
  constructionTime = Time::getCurrentTime();
}

/*
** NotificationQueue
*/
void NotificationQueue::push(const NotificationPtr& notification) 
{
  ScopedLock _(lock);
  notifications.push_back(notification);
}

void NotificationQueue::flush(const ObjectPtr& target)
{
  std::vector<NotificationPtr> notifications;
  {
    ScopedLock _(lock);
    this->notifications.swap(notifications);
  }
  for (size_t i = 0; i < notifications.size(); ++i)
    notifications[i]->notify(target);
}

bool NotificationQueue::isEmpty() const
  {ScopedLock _(lock); return notifications.empty();}

