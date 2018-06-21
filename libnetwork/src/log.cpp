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
#include <stdlib.h>
#include <stdarg.h>

#include <qpixmap.h>
#include <qtextedit.h>

#include "log.h"

#ifdef GUI
#include "document.xpm"
#endif

static QTextEdit *log_ = NULL;
static FILE *def_fp = stdout;
static FILE *debug_fp = NULL;
static FILE *info_fp = NULL;
static FILE *warning_fp = stderr;
static FILE *fatal_fp = stderr;

#define BUF_LEN 8192

#ifdef WIN32
#define vsnprintf _vsnprintf
#endif

using namespace std;

void
setLogFP(FILE *def, FILE *debug, FILE *info, FILE *warning) {
  def_fp = def;
  debug_fp = debug;
  info_fp = info;
  warning_fp = warning;
}

QTextEdit *
createLog(QWidget *w) {
#ifdef GUI
  if(!log_) {
    log_ = new QTextEdit(w, 0);
    //log_->setTextFormat(Qt::LogText);
	log_->resize(500, 200);
    log_->setReadOnly(true);
    log_->setCaption("GenoDYN Log messages");
    log_->setIcon(QPixmap(document_xpm));
  }
#endif
  return log_;
}

QTextEdit *
pneLog() {
	return log_;
}

// Based off Qt's qDebug
void
pnePrint(const char *msg, ...) {
  char buf[BUF_LEN];
  va_list ap;
  va_start(ap, msg);                        // use variable arg list
  if(log_) {    
    vsnprintf(buf, BUF_LEN, msg, ap);
    va_end(ap);
    //log_->insert(QObject::tr("(%1): %2\n").arg(network_name).arg(buf));
    //log_->insert(QObject::tr("%1\n").arg(buf));
		log_->insert(QObject::tr("%1").arg(buf));
    //log_->show();
    //log_->raise();
  } else {
    if(def_fp) {
      vfprintf(def_fp, msg, ap);
      va_end(ap);
      //fprintf(def_fp, "\n");                // add newline
    }
  }
}

// Based off Qt's qDebug
void
pneDebug(const char *msg, ...) {
  char buf[BUF_LEN];
  va_list ap;
  va_start(ap, msg);                        // use variable arg list
  if(log_) {    
    vsnprintf(buf, BUF_LEN, msg, ap);
    va_end(ap);
    //log_->insert(QObject::tr("DEBUG (%1): %2\n").arg(network_name).arg(buf));
    log_->insert(QObject::tr("DEBUG: %1\n").arg(buf));
    //log_->show();
    //log_->raise();
  } else {
    if(debug_fp) {
      vfprintf(debug_fp, msg, ap);
      va_end(ap);
      fprintf(debug_fp, "\n");                // add newline
    }
  }
}

// Based off Qt's qDebug
void
pneInfo(const char *msg, ...) {
  char buf[BUF_LEN];
  va_list ap;
  va_start(ap, msg);                        // use variable arg list
  if(log_) {    
    vsnprintf(buf, BUF_LEN, msg, ap);
    va_end(ap);
    //log_->insert(QObject::tr("INFO (%1): %2\n").arg(network_name).arg(buf));
    log_->insert(QObject::tr("INFO: %1\n").arg(buf));
    //log_->show();
    //log_->raise();
  } else {
    if(info_fp) {
      vfprintf(info_fp, msg, ap);
      va_end(ap);
      fprintf(info_fp, "\n");                // add newline
    }
  }
}

// Based off Qt's qWarning
void
pneWarning(const char *msg, ...) {
  char buf[BUF_LEN];
  va_list ap;
  va_start(ap, msg);                        // use variable arg list
  if(log_) {
    vsnprintf(buf, BUF_LEN, msg, ap);
    va_end(ap);
    //log_->insert(QObject::tr("WARNING (%1): %2\n").arg(network_name).arg(buf));
    log_->insert(QObject::tr("WARNING: %1\n").arg(buf));
    //log_->show();
    //log_->raise();
  } else {
    if(warning_fp) {
      vfprintf(warning_fp, msg, ap);
      va_end(ap);
      fprintf(warning_fp, "\n");                // add newline
    }
  }
}

// Based off Qt's qFatal
void
pneFatal(const char *msg, ...) {
  char buf[BUF_LEN];
  va_list ap;
  va_start(ap, msg);                        // use variable arg list
  if(log_) {
    vsnprintf(buf, BUF_LEN, msg, ap);
    va_end(ap);
    //log_->insert(QObject::tr("FATAL (%1): %2\n").arg(network_name).arg(buf));
    log_->insert(QObject::tr("FATAL: %1\n").arg(buf));
    //log_->show();
    //log_->raise();
    abort();
  } else {
    if(fatal_fp) {
      vfprintf(fatal_fp, msg, ap);
      va_end(ap);
      fprintf(fatal_fp, "\n");                // add newline
    }
    abort();                                  // trap; generates core dump
  }
}

logbuf::logbuf(int bsize):
 streambuf()
{
 if(bsize) {
   char *p = new char[bsize];
   setp(p, p + bsize);
 } else {
   setp(NULL, 0);
 }

 setg(0, 0, 0);
}

logbuf::~logbuf() {
 sync();
 delete[] pbase();
}

int logbuf::overflow(int c) {
 put_buffer();

 if(c != EOF) {
   if(pbase() == epptr()) {
     put_char(c);
   } else {
     sputc(c);
   }
 }

 return 0;
}

int
logbuf::sync() {
 put_buffer();
 return 0;
}

void
logbuf::put_char(int c) {
 if(c != '\n') {
   buf_ += c;
 } else {
   pneDebug("%s", buf_.c_str());
   buf_.clear();
 }
}

void
logbuf::put_buffer() {
 if(pbase() != pptr()) {
   int n = (pptr() - pbase());
   char *p = pbase();
   for(int i=0; i<n; i++) {
     if(p[i] != '\n') {
       buf_ += p[i];
     } else {
       pneDebug("%s", buf_.c_str());
       buf_ = "";
     }
   }

   setp(pbase(), epptr());
 }
}
