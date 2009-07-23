/*
** $PROJECT_PRESENTATION_AND_CONTACT_INFOS$
**
** Copyright (C) 2009 Francis MAES
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
