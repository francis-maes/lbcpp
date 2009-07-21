/*-----------------------------------------.---------------------------------.
| Filename: Callback.h                     | CR-algorithm callback           |
| Author  : Francis Maes                   |                                 |
| Started : 04/02/2009 18:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*!
**@file   Callback.h
**@author Francis MAES
**@date   Fri Jun 12 17:00:21 2009
**
**@brief  CR-algorithm callback.
**
**
*/

#ifndef LBCPP_CALLBACK_H_
# define LBCPP_CALLBACK_H_

# include <string>

namespace lbcpp
{

/*!
** @class Callback
** @brief CR-algorithm callback.
**
** deprecated
**
*/
class Callback
{
public:

  /*!
  ** Destructor.
  */
  virtual ~Callback() {}

  /*!
  ** #FIXME
  **
  ** @param choose
  */
  virtual void choose(ChoosePtr choose) {}

  /*!
  ** @brief Reward classifier.
  **
  ** #FIXME
  **
  ** @param reward : reward value.
  */
  virtual void reward(double reward) {}
};

}; /* namespace lbcpp */

#endif // !LBCPP_CALLBACK_H_
