//
// Copyright (C) 2005 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//

#include "Type.hh"

namespace Synopsis
{
namespace TypeAnalysis
{

BuiltinType BOOL("bool");
BuiltinType CHAR("char");
BuiltinType WCHAR("wchar_t");
BuiltinType SHORT("short");
BuiltinType INT("int");
BuiltinType LONG("long");
BuiltinType FLOAT("float");
BuiltinType DOUBLE("double");
BuiltinType UCHAR("unsigned char");
BuiltinType USHORT("unsigned short");
BuiltinType UINT("unsigned int");
BuiltinType ULONG("unsigned long");
BuiltinType SCHAR("signed char");
BuiltinType SSHORT("signed short");
BuiltinType SINT("signed int");
BuiltinType SLONG("signed long");

std::string const CVType::names[4] = {"",
				      "const",
				      "volatile",
				      "const volatile"};

}
}
