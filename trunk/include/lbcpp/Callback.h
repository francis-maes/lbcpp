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
**@brief  #FIXME: all
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
** @brief #FIXME
**
*/
class Callback
{
public:

  /*!
  **
  **
  **
  ** @return
  */
  virtual ~Callback() {}

  /*!
  **
  **
  ** @param choose
  */
  virtual void choose(ChoosePtr choose) {}

  /*!
  **
  **
  ** @param reward
  */
  virtual void reward(double reward) {}
};

}; /* namespace lbcpp */

#endif // !LBCPP_CALLBACK_H_
