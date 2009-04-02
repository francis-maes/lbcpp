/*-----------------------------------------.---------------------------------.
| Filename: Macros.hpp                     | Macros to try simplifying       |
| Author  : Francis Maes                   |  the code of template classes   |
| Started : 12/03/2009 19:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LCPP_CORE_IMPL_MACROS_H_
# define LCPP_CORE_IMPL_MACROS_H_

/*
** Template Inheritance
*/
# define TEMPLATE_INHERIT_BEGIN_0(_Class, _BaseClass) \
  struct _Class : public _BaseClass  { \
    typedef _Class Class; \
    typedef _BaseClass BaseClass;

# define TEMPLATE_INHERIT_BEGIN_1(_Class, _BaseClass, _BaseClassArg0) \
  struct _Class : public _BaseClass<_BaseClassArg0>  { \
    typedef _Class Class; \
    typedef _BaseClass<_BaseClassArg0> BaseClass;

# define TEMPLATE_INHERIT_BEGIN_2(_Class, _BaseClass, _BaseClassArg0, _BaseClassArg1) \
  struct _Class : public _BaseClass<_BaseClassArg0, _BaseClassArg1>  { \
    typedef _Class Class; \
    typedef _BaseClass<_BaseClassArg0, _BaseClassArg1> BaseClass;

# define TEMPLATE_INHERIT_BEGIN_3(_Class, _BaseClass, _BaseClassArg0, _BaseClassArg1, _BaseClassArg2) \
  struct _Class : public _BaseClass<_BaseClassArg0, _BaseClassArg1, _BaseClassArg2>  { \
    typedef _Class Class; \
    typedef _BaseClass<_BaseClassArg0, _BaseClassArg1, _BaseClassArg2> BaseClass;

#endif // !LCPP_CORE_IMPL_MACROS_H_
