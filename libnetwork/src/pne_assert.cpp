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

#include <stdio.h>

#ifndef WIN32
#include <execinfo.h>
#endif

#include "pne_assert.h"

// Use addr2line to convert the function address+offset to a function
// name and line number.

void
print_bt() {
#ifndef WIN32
  void * stack[10];
  int items = backtrace (stack, 10);
  
  char ** strings = backtrace_symbols (stack, items);
  
  int i;
  for (i = 0; i < items; ++i)
    printf("%s\n", strings[i]);
#endif
}
