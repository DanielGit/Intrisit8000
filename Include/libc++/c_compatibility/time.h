// -*- C++ -*- compatibility header.

// Copyright (C) 2002 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.

// As a special exception, you may use this file as part of a free software
// library without restriction.  Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License.  This exception does not however
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.

#ifndef _CPP_TIME_H_
#define _CPP_TIME_H_ 1

#include <ctime>
 
// Get rid of those macros defined in <time.h> in lieu of real functions.
#undef clock
#undef difftime
#undef mktime
#undef time
#undef asctime
#undef ctime
#undef gmtime
#undef localtime
#undef strftime

using std::clock_t;
using std::time_t;
using std::tm;

using std::clock;
using std::difftime;
using std::mktime;
using std::time;
using std::asctime;
using std::ctime;
using std::gmtime;
using std::localtime;

/* DEL K.Watanabe V1.7 <<<<<<< */
#if 0
using std::strftime;
#endif
/* DEL K.Watanabe V1.7 <<<<<<< */

#endif
