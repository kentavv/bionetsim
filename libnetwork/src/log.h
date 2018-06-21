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

#ifndef _LOG_H_
#define _LOG_H_

#include <ostream>

#include <stdio.h>

class QTextEdit;
class QWidget;

void setLogFP(FILE *def, FILE *debug, FILE *info, FILE *warning);
QTextEdit *createLog(QWidget *);
QTextEdit *pneLog();
void pnePrint(const char *msg, ...);
void pneDebug(const char *msg, ...);
void pneInfo(const char *msg, ...);
void pneWarning(const char *msg, ...);
void pneFatal(const char *msg, ...);

class logbuf: public std::streambuf {
public:
 logbuf(int = 0);
 ~logbuf();

protected:
 int overflow(int);
 int sync();

private:
	std::string buf_;      // accumlating buffer, once a '\n' is seen the line is written to the log widget

 void put_buffer();
 void put_char(int);
};

#endif
