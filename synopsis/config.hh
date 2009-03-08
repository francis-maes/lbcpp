//
// Copyright (C) 2007 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//

#ifndef Synopsis_config_hh_
#define Synopsis_config_hh_

// The following code is an adaptation from suggestions made
// at http://gcc.gnu.org/wiki/Visibility

// Shared library support
#ifdef WIN32
  #define SYN_IMPORT __declspec(dllimport)
  #define SYN_EXPORT __declspec(dllexport)
  #define SYN_DSO_LOCAL
  #define SYN_DSO_PUBLIC
#else
  #define SYN_IMPORT
  #ifdef GCC_HASCLASSVISIBILITY
    #define SYN_IMPORT __attribute__ ((visibility("default")))
    #define SYN_EXPORT __attribute__ ((visibility("default")))
    #define SYN_DSO_LOCAL __attribute__ ((visibility("hidden")))
    #define SYN_DSO_PUBLIC __attribute__ ((visibility("default")))
  #else
    #define SYN_IMPORT
    #define SYN_EXPORT
    #define SYN_DSO_LOCAL
    #define SYN_DSO_PUBLIC
  #endif
#endif

// Define SYNOPSIS_API for DSO builds
#ifdef SYNOPSIS_DSO
  #ifdef SYNOPSIS_DSO_EXPORTS
    #define SYNOPSIS_API SYN_EXPORT
  #else
    #define SYNOPSIS_API SYN_IMPORT
  #endif // SYNOPSIS_DSO_EXPORTS
#else
  #define SYNOPSIS_API
#endif // SYNOPSIS_DSO

#endif
