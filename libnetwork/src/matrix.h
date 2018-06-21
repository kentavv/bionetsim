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

#ifndef _MATRIX_H_
#define _MATRIX_H_

#include <ostream>

#include <stdio.h>

#include <qdom.h>
#include <qfile.h>
#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>

#include "math_utils.h"
#include "pne_assert.h"

template<class T>
class TMatrix {
public:
  TMatrix();
  TMatrix(int nr, int nc, T fill=0);
  TMatrix(const TMatrix&);
  ~TMatrix();
  TMatrix &operator=(const TMatrix&);

  void resize(int nr, int nc);
  void resize(int nr, int nc, const T &fill, bool preserve=false);
  void expand(int extra_rows, int extra_cols, const T &fill);
  void clear();
  void fill(const T&);
  T &operator()(int r, int c);
  T operator()(int r, int c) const;
  int nrows() const;
  int ncolumns() const;
  void print(FILE *fp=stdout) const;
  void printVector(FILE *fp=stdout) const;
	void print(std::ostream &os) const;
	void printVector(std::ostream &os) const;
  void load(FILE *fp);
  bool empty() const;

  void save(QDomDocument&, QDomElement&) const;
  
  const T *raw() const;
  void setRaw(const T*);
  
protected:
  T *v_;   //! Serialized version of the matrix
  int nr_; //! Number of rows
  int nc_; //! Number of columns
};

typedef TMatrix<bool> BMatrix;
typedef TMatrix<int> IMatrix;
typedef TMatrix<double> DMatrix;
typedef TMatrix<double> Matrix;

/*!
  \class TMatrix
  \brief Template class for 2-dimensional matrices
  \ingroup libnetwork

  TMatrix is restricted to objects that can be assigned a value of 0
  because of the default argument of one of the constructors.
  
  Preinstantiated versions of TMatrix include:
  \code
  typedef TMatrix<int> IMatrix;
  typedef TMatrix<double> DMatrix;
  typedef TMatrix<double> Matrix;
  \endcode
*/

/*!
  \brief Default constructor initilizing a null matrix

  Default constructor initializing a null matrix with rows and columns set
  to zero.
*/
template<class T>
inline TMatrix<T>::TMatrix() 
  : v_(NULL),
    nr_(0),
    nc_(0)
{}

/*!
  \brief Default constructor initilizing a matrix of known size and fill value

  Construtor that allows the number of rows and columns to be set at the
  time of creation.  Also, each element is initilized to a fill value.
  If no fill value is specified, the default of 0 is used.
*/
template<class T>
inline TMatrix<T>::TMatrix(int nr, int nc, T v)
  : v_(NULL),
    nr_(0),
    nc_(0)
{
  resize(nr, nc, v);
}

/*!
  \brief Copy constructor
*/
template<class T>
inline TMatrix<T>::TMatrix(const TMatrix &m) 
  : v_(NULL),
    nr_(m.nr_),
    nc_(m.nc_) 
{
  if(nr_ > 0 && nc_ > 0) {
    v_ = new T[nr_ * nc_];    
  }
  for(int i=0; i<nr_*nc_; i++) {
    v_[i] = m.v_[i];
  }
}

/*!
  \brief Destructor

  Deletes internal memory if matrix has been allocated.  All pointers to
  internal data will be invalid afterwards.
*/
template<class T>
inline TMatrix<T>::~TMatrix() {
  if(v_) delete[] v_;
}

/*!
  \brief Assignment operator
*/
template<class T>
inline TMatrix<T> &
TMatrix<T>::operator=(const TMatrix &m) {
  resize(m.nr_, m.nc_);
  for(int i=0; i<nr_*nc_; i++) {
    v_[i] = m.v_[i];
  }
  return *this;
}

/*!
  \brief Resizes matrix

  Resizes matrix to nr rows and nc columns.  Internal data structure is
  replaced only if nc*nr != nc'*nr'.  If either nr == 0 or nc == 0
  then the internal pointer is set to null, but nrows() and ncolumns() will
  return the values that were passed.  Values of matrix after resize are
  undefined.
*/
template<class T>
inline void 
TMatrix<T>::resize(int nr, int nc) {
  pne_assert(0 <= nr && 0 <= nc);
  if(nr == 0 || nc == 0) {
    if(v_) delete[] v_;
    v_ = NULL;
  } else if(nr_ * nc_  != nr * nc) {
    if(v_) delete[] v_;
    v_ = new T[nr * nc];
  }
  nr_ = nr;
  nc_ = nc;
}

/*!
  \brief Resizes matrix optionally preserving contents

  Resizes matrix to nr rows and nc columns.  If either nr == 0 or nc == 0
  then the internal pointer is set to null, but nrows() and ncolumns() will
  return the values that were passed.  Exisiting values for elements that exist
  in both matrices will be preserved.  Values of new elements will be initialized
  with fill values.
*/
template<class T>
inline void 
TMatrix<T>::resize(int nr, int nc, const T &fill, bool preserve) {
  pne_assert(0 <= nr && 0 <= nc);
  if(nr == 0 || nc == 0) {
    if(v_) delete[] v_;
    v_ = NULL;
    nr_ = nr;
    nc_ = nc;
  } else {
    if(preserve) {
      expand(nr - nr_, nc - nc_, fill);
    } else {
      resize(nr, nc);
      this->fill(fill);
    }
  }
}

/*!
  \brief Expands a matrix preserving contents

  Expands matrix to have extra_rows additional rows and extra_cols additional
  columns.  Negative arguments results in the matrix shrinking with the elements
  from removed columns and rows being lost.  Arguments must be at least [-nr, -nc].
  Values of new cells will be initilized to value in fill.
*/
template<class T>
inline void
TMatrix<T>::expand(int extra_rows, int extra_cols, const T &fill) {
  int nc = nc_ + extra_cols;
  int nr = nr_ + extra_rows; 
  pne_assert(0 <= nr && 0 <= nc);
  if(nr == 0 || nc == 0) {
    resize(nr, nc);
  } else {
    //pne_assert(0 <= extra_rows && 0 <= extra_cols);
    if(extra_rows != 0 || extra_cols != 0) {
      TMatrix<T> tmp(nr, nc, fill);
      for(int r=0; r<GenoDYN::min(nr_, nr); r++) {
	for(int c=0; c<GenoDYN::min(nc_, nc); c++) {
	  //tmp(r, c) = operator()(r, c);
          tmp.v_[r*nc + c] = v_[r*nc_ + c];
	}
      }
      *this = tmp;
    }
  }
}

/*!
  \brief Clears matrix

  Resizes matrix to 0 rows and 0 columns.
*/
template<class T>
inline void 
TMatrix<T>::clear() {
  resize(0, 0);
}

/*!
  \brief Assigns a value to all elements of the matrix.
*/
template<class T>
inline void
TMatrix<T>::fill(const T &v) {
  for(int i=0; i<nr_*nc_; i++) {
    v_[i] = v;
  }
}  

/*!
  \brief Return a reference to the element at position (r, c)

  Modifying the returned reference will modify the value in the matrix.
*/
template<class T>
inline T &
TMatrix<T>::operator()(int r, int c) {
  pne_assert(0 <= r && r < nr_);
  pne_assert(0 <= c && c < nc_);
  return v_[r*nc_ + c];
  //return v_[c*nr_ + r];
}

/*!
  \brief Return the value of the element at position (r, c)

  Modifying the returned value will \em not modify the value in the matrix.
*/
template<class T>
inline T
TMatrix<T>::operator()(int r, int c) const {
  pne_assert(0 <= r && r < nr_);
  pne_assert(0 <= c && c < nc_);
  return v_[r*nc_ + c];
  //return v_[c*nr_ + r];
}

/*!
  \brief Return the number of rows in this matrix.
*/
template<class T>
inline int 
TMatrix<T>::nrows() const {
  return nr_;
}

/*!
  \brief Return the number of columns in this matrix.
*/
template<class T>
inline int 
TMatrix<T>::ncolumns() const {
  return nc_;
}

/*!
  \brief An abstract print interface

  The fp argument is used to redirect the output to any FILE *.  The default
  argument is stdout.
  
  Before a TMatrix of a particular type can be printed, this function has
  to be defined for that type.  This has already been done for int and double.
  An example of this is shown below for the int case:
  \code
    template<>
    inline void
    TMatrix<int>::print(FILE *fp) const {
      for(int r=0; r<nr_; r++) {
        for(int c=0; c<nc_; c++) {
          if(c != 0) fputc('\t', fp);
          fprintf(fp, "%d", v_[r*nc_ + c]);
        }
        fputc('\n', fp);
      }
    }
  \endcode
*/
template<class T>
inline void
TMatrix<T>::print(FILE *fp) const {
  fprintf(fp, "Please define specilized TMatrix<T>::print()\n");
}

template<class T>
inline void
TMatrix<T>::print(std::ostream &os) const {
	for(int r=0; r<nr_; r++) {
		for(int c=0; c<nc_; c++) {
			if(c != 0) os << '\t';
			os << v_[r*nc_ + c];
		}
		os << '\n';
	}
}

/*!
  \brief Loads a tab delimited matrix from a FILE *

  Loads a tab delimited matrix from a FILE * starting at the 
  current position and reads until reaching the end of the file.
  The first line read is used to determine the number of columns.
  In subsequent rows the columns are filled from left to right 
  up to the minimum  number of columns found in the the first
  row and the current row.
*/
template<class T>
inline void
TMatrix<T>::load(FILE *fp) {
  QFile f;
  f.open(IO_ReadOnly, fp);
  QString str;
  QStringList strs;
  while(f.readLine(str, 1024000) > 0) {
    strs.append(str);
  }

  int nr = strs.count();
  int nc = -1;
  int r, c;
  QStringList::Iterator i, j;

  for(r = 0, i = strs.begin(); i != strs.end(); ++r, ++i) {
    QStringList tokens = QStringList::split('\t', *i);
    if(nc == -1) {
      fprintf(stderr, "Need to implement specialized function for TMatrix<T>::load(FILE *)\n");
      nc =  tokens.count();
      resize(nr, nc);
      //fill(0);
    }
    for(c = 0, j = tokens.begin(); c < nc && j != tokens.end(); ++c, ++j) {
      //v_[r*nc + c] = j->toInt();
    }
  }
}

/*!
  \brief Preinstantiated load(FILE *) method for int data type
*/
template<>
inline void
TMatrix<int>::load(FILE *fp) {
  QFile f;
  f.open(IO_ReadOnly, fp);
  QString str;
  QStringList strs;
  while(f.readLine(str, 1024000) > 0) {
    strs.append(str);
  }

  int nr = (int)strs.count();
  int nc = -1;
  int r, c;
  QStringList::Iterator i, j;

  for(r = 0, i = strs.begin(); i != strs.end(); ++r, ++i) {
    QStringList tokens = QStringList::split('\t', *i);
    if(nc == -1) {
      nc =  (int)tokens.count();
      resize(nr, nc);
      fill(0);
    }
    for(c = 0, j = tokens.begin(); c < nc && j != tokens.end(); ++c, ++j) {
      v_[r*nc + c] = (*j).toInt();
    }
  }
}

/*!
  \brief Preinstantiated load(FILE *) method for double data type
*/
template<>
inline void
TMatrix<double>::load(FILE *fp) {
  QFile f;
  f.open(IO_ReadOnly, fp);
  QString str;
  QStringList strs;
  while(f.readLine(str, 1024000) > 0) {
    strs.append(str);
  }

  int nr = (int)strs.count();
  int nc = -1;
  int r, c;
  QStringList::Iterator i, j;

  for(r = 0, i = strs.begin(); i != strs.end(); ++r, ++i) {
    QStringList tokens = QStringList::split('\t', *i);
    if(nc == -1) {
      nc =  (int)tokens.count();
      resize(nr, nc);
      fill(0.0);
    }
    for(c = 0, j = tokens.begin(); c < nc && j != tokens.end(); ++c, ++j) {
      v_[r*nc + c] = (*j).toDouble();
    }
  }
}

/*!
  \brief An abstract print interface similar to print()

  Similar to print() but print using in the format {{row1}, {row2}, ... {rown}}.
*/
template<class T>
inline void
TMatrix<T>::printVector(FILE *fp) const {
  fprintf(fp, "Please define specilized TMatrix<T>::printVector()\n");
}

template<class T>
inline void
TMatrix<T>::printVector(std::ostream &os) const {
  os << '{';  
  for(int r=0; r<nr_; r++) {
    if(r != 0) os << ", ";
    os << '{';
    for(int c=0; c<nc_; c++) {
      if(c != 0) os << ", ";
      os << v_[r*nc_ + c];
    }
    os << '}';
  }
  os << '}';  
}

/*!
  \brief Return true if matrix is empty

  The matrix is empty whenever at least either nrows()==0 or ncolumns()==0.
*/
template<class T>
inline bool
TMatrix<T>::empty() const {
  return v_ == NULL;
}

/*!
  \brief Return raw internal data.
*/
template<class T>
const T *
TMatrix<T>::raw() const {
  return v_;
}

/*!
  \brief Set the internal data using a C array.

  Directly set the raw internal data using a C array.  The length
  of the passesd array is assumed to be equal in size to the matrix's
  internal array.
*/
template<class T>
void
TMatrix<T>::setRaw(const T *v) {
  for(int i=0; i<nr_*nc_; i++) {
    v_[i] = v[i];
  }  
}

/*!
  \brief Preinstantiated print() method for int data type

  Values are printed using the "%d" format specification.  All values are
  tab deliminated.
*/
template<>
inline void
TMatrix<int>::print(FILE *fp) const {
  for(int r=0; r<nr_; r++) {
    for(int c=0; c<nc_; c++) {
      if(c != 0) fputc('\t', fp);
      fprintf(fp, "%d", v_[r*nc_ + c]);
    }
    fputc('\n', fp);
  }
}

/*!
  \brief Preinstantiated printVector() method for int data type

  Values are printed using the "%d" format specification.
*/
template<>
inline void
TMatrix<int>::printVector(FILE *fp) const {
  fputc('{', fp);  
  for(int r=0; r<nr_; r++) {
    if(r != 0) fprintf(fp, ", ");
    fputc('{', fp);
    for(int c=0; c<nc_; c++) {
      if(c != 0) fprintf(fp, ", ");
      fprintf(fp, "%d", v_[r*nc_ + c]);
    }
    fputc('}', fp);
  }
  fputc('}', fp);  
}

/*!
  \brief Preinstantiated print() method for double data type

  Values are printed using the "%.4g" format specification, which uses
  scientific notation when appropiate, with a lower case "e", and with
  four digits of precision.  All values are tab deliminated.
*/
template<>
inline void
TMatrix<double>::print(FILE *fp) const {
  for(int r=0; r<nr_; r++) {
    for(int c=0; c<nc_; c++) {
      if(c != 0) fputc('\t', fp);
      fprintf(fp, "%.4g", v_[r*nc_ + c]);
    }
    fputc('\n', fp);
  }
}

/*!
  \brief Preinstantiated printVector() method for double data type

  Values are printed using the "%.4g" format specification.
*/
template<>
inline void
TMatrix<double>::printVector(FILE *fp) const {
  fputc('{', fp);  
  for(int r=0; r<nr_; r++) {
    if(r != 0) fprintf(fp, ", ");
    fputc('{', fp);
    for(int c=0; c<nc_; c++) {
      if(c != 0) fprintf(fp, ", ");
      fprintf(fp, "%.4g", v_[r*nc_ + c]);
    }
    fputc('}', fp);
  }
  fputc('}', fp);  
}

/*!
  \brief Save matrix as DOM object

  Abstract method to save contents of the matrix into a DOM object.  Concrete
  versions must be formed for each datatype.
*/
template<class T>
inline void
TMatrix<T>::save(QDomDocument &doc, QDomElement &n) const {
  QDomElement e, e2;
  QDomText t;
  
  QDomElement mat = doc.createElement("Matrix");
  n.appendChild(mat);
  
  mat.setAttribute("nrows", nr_);
  mat.setAttribute("ncolumns", nc_);
  for(int r=0; r<nr_; r++) {
    e = doc.createElement("r");
    for(int c=0; c<nc_; c++) {
      e2 = doc.createElement("c");
      //t = doc.createTextNode(QObject::tr("%1").arg(v_[r*nc_ + c]));
      e.appendChild(e2); e2.appendChild(t);
    }
    mat.appendChild(e);
  }
}

/*!
  \brief Preinstantiated save() method for double data type

  Values are printed using Qt's default format specification.
*/
template<>
inline void
TMatrix<int>::save(QDomDocument &doc, QDomElement &n) const {
  QDomElement e, e2;
  QDomText t;
  
  QDomElement mat = doc.createElement("Matrix");
  n.appendChild(mat);
  
  mat.setAttribute("nrows", nr_);
  mat.setAttribute("ncolumns", nc_);
  for(int r=0; r<nr_; r++) {
    e = doc.createElement("r");
    for(int c=0; c<nc_; c++) {
      e2 = doc.createElement("c");
      t = doc.createTextNode(QObject::tr("%1").arg(v_[r*nc_ + c]));
      e2.appendChild(e2); e2.appendChild(t);
    }
    mat.appendChild(e);
  }
}

/*!
  \brief Preinstantiated save() method for double data type

  Values are printed using Qt's default format specification.
*/
template<>
inline void
TMatrix<double>::save(QDomDocument &doc, QDomElement &n) const {
  QDomElement e, e2;
  QDomText t;
  
  QDomElement mat = doc.createElement("Matrix");
  n.appendChild(mat);
  
  mat.setAttribute("nrows", nr_);
  mat.setAttribute("ncolumns", nc_);
  for(int r=0; r<nr_; r++) {
    e = doc.createElement("r");
    for(int c=0; c<nc_; c++) {
      e2 = doc.createElement("c");
      t = doc.createTextNode(QObject::tr("%1").arg(v_[r*nc_ + c]));
      e.appendChild(e2); e2.appendChild(t);
    }
    mat.appendChild(e);
  }
}

#endif
