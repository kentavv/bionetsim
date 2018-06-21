// This file is part of GenoDYN.
//
// GenoDYN is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// GenoDYN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with GenoDYN.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright 2008 Kent A. Vander Velden

#ifndef _PNE_ASSERT_H_
#define _PNE_ASSERT_H_

#include <stdlib.h>

void print_bt();

#ifdef NDEBUG

#define pne_assert(expr) (void) 0;

#else

#ifndef WIN32
#define pne_assert(expr) ((void)((expr) ? 0 : (fprintf(stderr, "%s:%d: %s: Assertion `%s` failed.\n", __FILE__, __LINE__, __PRETTY_FUNCTION__, __STRING(expr)), print_bt(), abort(), -1)))
#else
#define __STRING(x) #x
// __FUNCDNAME__ (decrorated) and __FUNCTION__ (undecorated) and __FUNCSIG__
// are available in Visual C++.net
//#define pne_assert(expr) ((void)((expr) ? 0 : (fprintf(stderr, "%s:%d: %s: Assertion `%s` failed.\n", __FILE__, __LINE__, __func__, __STRING(expr)), print_bt(), abort(), -1)))
#define pne_assert(expr) ((void)((expr) ? 0 : (fprintf(stderr, "%s:%d: Assertion `%s` failed.\n", __FILE__, __LINE__, __STRING(expr)), print_bt(), abort(), -1)))
#endif

#endif

#endif
