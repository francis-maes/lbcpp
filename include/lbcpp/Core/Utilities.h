/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2009 by Francis Maes, francis.maes@lip6.fr.
**
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
| Filename: Utilities.h                    | Misc Utilities                  |
| Author  : Francis Maes                   |                                 |
| Started : 25/08/2010 21:21               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_UTILITIES_H_
# define LBCPP_CORE_UTILITIES_H_

# include "../common.h"
# include <cmath>
# include <cfloat>

namespace lbcpp
{

inline std::ostream& operator <<(std::ostream& ostr, const String& value)
  {return ostr << (const char* )value;}

template<bool> struct StaticAssert;
template<> struct StaticAssert<true> {};

inline bool isNumberValid(double number)
{
#ifdef JUCE_WIN32
    return (number == number) && (number != DBL_MAX) && (number != -DBL_MAX) && (std::fabs(number) != HUGE_VAL);
#else
    return !std::isnan(number) && !std::isinf(number);
#endif
}

inline bool isNumberNearlyNull(double value, double epsilon = 0.00001)
  {return fabs(value) < epsilon;}

inline double normalizeAngle(double angle)
{
  double res = fmod(angle, M_2_TIMES_PI);
  if (res < -M_PI)
    res += M_2_TIMES_PI;
  else if (res > M_PI)
    res -= M_2_TIMES_PI;
  jassert(res >= -M_PI && res < M_PI);
  return res;
}

#ifdef JUCE_WIN32
inline double log2(double x)
{
  static const double oneOverLog2 = 1.0/log(2.0);
  return log(x) * oneOverLog2;
}
#endif // JUCE_WIN32

extern String getTypeName(const std::type_info& info);

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_UTILITIES_H_
