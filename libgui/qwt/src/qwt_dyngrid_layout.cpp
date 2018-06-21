/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_dyngrid_layout.h"

#if QT_VERSION >= 300
#include <qptrlist.h>
#else
#include <qlist.h>
#ifndef QPtrList
#define QPtrList QList
#define QPtrListIterator QListIterator
#endif
#endif


class QwtDynGridLayoutPrivate
{
public:
   QPtrList<QLayoutItem> itemList;
};

class QwtDynGridLayoutIterator: public QGLayoutIterator
{
public:
    QwtDynGridLayoutIterator(QPtrList<QLayoutItem> *);

    virtual QLayoutItem *current();
    virtual QLayoutItem *next();
    virtual QLayoutItem *takeCurrent();

private:
    uint d_index;
    QPtrList<QLayoutItem> *d_itemList;
};

QwtDynGridLayoutIterator::QwtDynGridLayoutIterator(
        QPtrList<QLayoutItem> *itemList):
    d_index(0), 
    d_itemList(itemList)  
{
}

QLayoutItem *QwtDynGridLayoutIterator::current()
{ 
    QLayoutItem *item = (d_index < d_itemList->count()) 
        ? d_itemList->at(d_index) : 0;  

    return item;
}

QLayoutItem *QwtDynGridLayoutIterator::next()
{ 
    d_index++; 
    return current(); 
}

QLayoutItem *QwtDynGridLayoutIterator::takeCurrent()
{ 
    QLayoutItem *item = (d_index < d_itemList->count()) 
        ? d_itemList->take(d_index) : 0;  

    return item;
}

/*!
  \param parent Parent widget
  \param margin Margin
  \param space Spacing
  \param name Widget name
*/

QwtDynGridLayout::QwtDynGridLayout(QWidget *parent, 
        int margin, int space, const char *name):
    QLayout(parent, margin, space, name)
{
    init();
}

/*!
  \param parent Parent widget
  \param space Spacing
  \param name Widget name
*/

QwtDynGridLayout::QwtDynGridLayout(QLayout *parent, 
        int space, const char *name):
    QLayout(parent, space, name)
{
    init();
}

/*!
  \param space Spacing
  \param name Widget name
*/

QwtDynGridLayout::QwtDynGridLayout(int space, const char *name):
    QLayout(space, name)
{
    init();
}

/*!
  Initialize the layout with default values.
*/
void QwtDynGridLayout::init()
{
    d_layoutData = new QwtDynGridLayoutPrivate;
    d_maxCols = d_numRows = d_numCols = 0;
    d_expanding = QSizePolicy::NoDirection;

    setSupportsMargin(TRUE);
}

//! Destructor

QwtDynGridLayout::~QwtDynGridLayout()
{
    deleteAllItems();
    delete d_layoutData;
}

/*!
  Limit the number of columns.
  \param maxCols upper limit, 0 means unlimited
  \sa QwtDynGridLayout::maxCols()
*/
  
void QwtDynGridLayout::setMaxCols(uint maxCols)
{
    d_maxCols = maxCols;
}

/*!
  Return the upper limit for the number of columns.
  0 means unlimited, what is the default.
  \sa QwtDynGridLayout::setMaxCols()
*/

uint QwtDynGridLayout::maxCols() const 
{ 
    return d_maxCols; 
}

//! Adds item to the next free position.

void QwtDynGridLayout::addItem(QLayoutItem *item)
{
    d_layoutData->itemList.append(item);
}

/*! 
  \return TRUE if this layout is empty. 
*/

bool QwtDynGridLayout::isEmpty() const
{
    return d_layoutData->itemList.isEmpty();
}

/*! 
  \return number of layout items
*/

uint QwtDynGridLayout::itemCount() const
{
    return d_layoutData->itemList.count();
}

/*! 
  \return An iterator over the children of this layout.
*/

QLayoutIterator QwtDynGridLayout::iterator()
{       
    return QLayoutIterator( 
		new QwtDynGridLayoutIterator(&d_layoutData->itemList) );
}

/*!
  Reorganizes columns and rows and resizes managed widgets within 
  the rectangle rect. 
*/

void QwtDynGridLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);

    if ( isEmpty() )
        return;

    d_numCols = columnsForWidth(rect.width());
    d_numRows = itemCount() / d_numCols;
    if ( itemCount() % d_numCols )
        d_numRows++;

    QValueList<QRect> itemGeometries = layoutItems(rect, d_numCols);

    int index;

    QLayoutItem *item;
    QPtrListIterator<QLayoutItem> it(d_layoutData->itemList);
    for ( index = 0, item = it.toFirst(); item != 0; item = ++it )
    {
        QWidget *w = item->widget();
        if ( w )
        {
            w->setGeometry(itemGeometries[index]);
            index++;
        }
    }
}

/*! 
  Calculate the number of columns for a given width. It tries to
  use as many columns as possible (limited by maxCols())

  \param width Available width for all columns
  \sa QwtDynGridLayout::maxCols(), QwtDynGridLayout::setMaxCols()
*/

uint QwtDynGridLayout::columnsForWidth(int width) const
{
    if ( isEmpty() )
        return 0;

    for (uint numCols = (d_maxCols > 0) ? d_maxCols : itemCount();
        numCols > 1; numCols--)
    {
        int col;

        QwtArray<int> colWidth(numCols);
        for ( col = 0; col < (int)numCols; col++ )
            colWidth[col] = 0;

        int index = 0;

        QPtrListIterator<QLayoutItem> it(d_layoutData->itemList);
        for (const QLayoutItem *item = it.toFirst(); item != 0; 
            item = ++it, index++ )
        {
            const int w = item->sizeHint().width();

            col = index % numCols;
            colWidth[col] = QMAX(colWidth[col], w);
        }

        int rowWidth = 2 * margin() + (numCols - 1) * spacing();
        for ( col = 0; col < (int)numCols; col++ )
            rowWidth += colWidth[col];

        if ( rowWidth <= width )
            return numCols;
    }

    return 1; // At least 1 column
}

/*!
  \return the maximum width of all layout items
*/

int QwtDynGridLayout::maxItemWidth() const
{
    if ( isEmpty() )
        return 0;

    int w = 0;
    QPtrListIterator<QLayoutItem> it(d_layoutData->itemList);
    for (const QLayoutItem *item = it.toFirst(); item != 0; item = ++it )
    {
        const int itemW = item->sizeHint().width();
        if ( itemW > w )
            w = itemW;
    }

    return w;
}

/*!
  Calculate the geometries of the layout items for a layout
  with numCols columns and a given rect.
  \param rect Rect where to place the items
  \param numCols Number of columns
  \return item geometries
*/

QValueList<QRect> QwtDynGridLayout::layoutItems(const QRect &rect,
    uint numCols) const
{
    QValueList<QRect> itemGeometries;
    if ( numCols == 0 || isEmpty() )
        return itemGeometries;

    uint numRows = itemCount() / numCols;
    if ( numRows % itemCount() )
        numRows++;
 
    QwtArray<int> rowHeight(numRows);
    QwtArray<int> colWidth(numCols);
 
    layoutGrid(numCols, rowHeight, colWidth);

    if ( expanding() != QSizePolicy::NoDirection )
        stretchGrid(rect, numCols, rowHeight, colWidth);

    QwtDynGridLayout *that = (QwtDynGridLayout *)this;
    const int maxCols = d_maxCols;
    that->d_maxCols = numCols;
    const QRect alignedRect = alignmentRect(rect);
    that->d_maxCols = maxCols;

#if QT_VERSION < 300
    const int xOffset = ( expanding() & QSizePolicy::Horizontal ) 
        ? 0 : alignedRect.x();
    const int yOffset = ( expanding() & QSizePolicy::Vertical ) 
        ? 0 : alignedRect.y();
#else
    const int xOffset = ( expanding() & QSizePolicy::Horizontally ) 
        ? 0 : alignedRect.x();
    const int yOffset = ( expanding() & QSizePolicy::Vertically ) 
        ? 0 : alignedRect.y();
#endif

    QwtArray<int> colX(numCols);
    QwtArray<int> rowY(numRows);

    const int xySpace = spacing();

    rowY[0] = yOffset + margin();
    for ( int r = 1; r < (int)numRows; r++ )
        rowY[r] = rowY[r-1] + rowHeight[r-1] + xySpace;

    colX[0] = xOffset + margin();
    for ( int c = 1; c < (int)numCols; c++ )
        colX[c] = colX[c-1] + colWidth[c-1] + xySpace;
    
    int index;
    QLayoutItem *item;

    QPtrListIterator<QLayoutItem> it(d_layoutData->itemList);
    for ( item = it.toFirst(), index = 0; item != 0; item = ++it, index++ )
    {
        const int row = index / numCols;
        const int col = index % numCols;

        QRect itemGeometry(colX[col], rowY[row], 
            colWidth[col], rowHeight[row]);
        itemGeometries.append(itemGeometry);
    }

    return itemGeometries;
}


/*!
  Calculate the dimensions for the columns and rows for a grid
  of numCols columns.
  \param numCols Number of columns.
  \param rowHeight Array where to fill in the calculated row heights.
  \param colWidth Array where to fill in the calculated column widths.
*/

void QwtDynGridLayout::layoutGrid(uint numCols, 
    QwtArray<int>& rowHeight, QwtArray<int>& colWidth) const
{
    if ( numCols <= 0 )
        return;

    int index = 0;

    QPtrListIterator<QLayoutItem> it(d_layoutData->itemList);
    for (const QLayoutItem *item = it.toFirst(); item != 0; 
        item = ++it, index++ )
    {
        const int row = index / numCols;
        const int col = index % numCols;

        const QSize size = item->sizeHint();

        rowHeight[row] = (col == 0) 
            ? size.height() : QMAX(rowHeight[row], size.height());
        colWidth[col] = (row == 0) 
            ? size.width() : QMAX(colWidth[col], size.width());
    }
}

/*!
  Set whether this layout can make use of more space than sizeHint(). 
  A value of Vertical or Horizontal means that it wants to grow in only 
  one dimension, while BothDirections means that it wants to grow in 
  both dimensions. The default value is NoDirection. 
  \sa QwtDynGridLayout::expanding()
*/

void QwtDynGridLayout::setExpanding(QSizePolicy::ExpandData expanding)
{
    d_expanding = expanding;
}

/*!
  Returns whether this layout can make use of more space than sizeHint(). 
  A value of Vertical or Horizontal means that it wants to grow in only 
  one dimension, while BothDirections means that it wants to grow in 
  both dimensions. 
  \sa QwtDynGridLayout::setExpanding()
*/

QSizePolicy::ExpandData QwtDynGridLayout::expanding() const
{
    return d_expanding;
}

/*!
  \return TRUE: QwtDynGridLayout implements heightForWidth.
  \sa QwtDynGridLayout::heightForWidth()
*/

bool QwtDynGridLayout::hasHeightForWidth() const
{
    return TRUE;
}

/*!
  \return The preferred height for this layout, given the width w. 
  \sa QwtDynGridLayout::hasHeightForWidth()
*/

int QwtDynGridLayout::heightForWidth(int width) const
{
    if ( isEmpty() )
        return 0;

    const uint numCols = columnsForWidth(width);
    uint numRows = itemCount() / numCols;
    if ( itemCount() % numCols )
        numRows++;

    QwtArray<int> rowHeight(numRows);
    QwtArray<int> colWidth(numCols);

    layoutGrid(numCols, rowHeight, colWidth);

    int h = 2 * margin() + (numRows - 1) * spacing();
    for ( int row = 0; row < (int)numRows; row++ )
        h += rowHeight[row];

    return h;
}

/*!
  Stretch columns in case of expanding() & QSizePolicy::Horizontal and
  rows in case of expanding() & QSizePolicy::Vertical to fill the entire
  rect. Rows and columns are stretched with the same factor.
  \sa QwtDynGridLayout::setExpanding(), QwtDynGridLayout::expanding()
*/

void QwtDynGridLayout::stretchGrid(const QRect &rect, 
    uint numCols, QwtArray<int>& rowHeight, QwtArray<int>& colWidth) const
{
    if ( numCols == 0 || isEmpty() )
        return;

#if QT_VERSION < 300
    if ( expanding() & QSizePolicy::Horizontal )
#else
    if ( expanding() & QSizePolicy::Horizontally )
#endif
    {
        int xDelta = rect.width() - 2 * margin() - (numCols - 1) * spacing();
        for ( int col = 0; col < (int)numCols; col++ )
            xDelta -= colWidth[col];

        if ( xDelta > 0 )
        {
            for ( int col = 0; col < (int)numCols; col++ )
            {
                const int space = xDelta / (numCols - col);
                colWidth[col] += space;
                xDelta -= space;
            }
        }
    }

#if QT_VERSION < 300
    if ( expanding() & QSizePolicy::Vertical )
#else
    if ( expanding() & QSizePolicy::Vertically )
#endif
    {
        uint numRows = itemCount() / numCols;
        if ( itemCount() % numCols )
            numRows++;

        int yDelta = rect.height() - 2 * margin() - (numRows - 1) * spacing();
        for ( int row = 0; row < (int)numRows; row++ )
            yDelta -= rowHeight[row];

        if ( yDelta > 0 )
        {
            for ( int row = 0; row < (int)numRows; row++ )
            {
                const int space = yDelta / (numRows - row);
                rowHeight[row] += space;
                yDelta -= space;
            }
        }
    }
}

/*!
   Return the size hint. If maxCols() > 0 it is the size for
   a grid with maxCols() columns, otherwise it is the size for
   a grid with only one row.
   \sa QwtDynGridLayout::maxCols(), QwtDynGridLayout::setMaxCols()
*/

QSize QwtDynGridLayout::sizeHint() const
{
    if ( isEmpty() )
        return QSize();

    const uint numCols = (d_maxCols > 0 ) ? d_maxCols : itemCount();
    uint numRows = itemCount() / numCols;
    if ( itemCount() % numCols )
        numRows++;

    QwtArray<int> rowHeight(numRows);
    QwtArray<int> colWidth(numCols);

    layoutGrid(numCols, rowHeight, colWidth);

    int h = 2 * margin() + (numRows - 1) * spacing();
    for ( int row = 0; row < (int)numRows; row++ )
        h += rowHeight[row];

    int w = 2 * margin() + (numCols - 1) * spacing(); 
    for ( int col = 0; col < (int)numCols; col++ )
        w += colWidth[col];

    return QSize(w, h);
}

/*!
  \return Number of rows of the current layout.
  \sa QwtDynGridLayout::numCols
  \warning The number of rows might change whenever the geometry changes
*/
uint QwtDynGridLayout::numRows() const 
{ 
    return d_numRows; 
}

/*!
  \return Number of columns of the current layout.
  \sa QwtDynGridLayout::numRows
  \warning The number of columns might change whenever the geometry changes
*/
uint QwtDynGridLayout::numCols() const 
{ 
    return d_numCols; 
}

// Local Variables:
// mode: C++
// c-file-style: "stroustrup"
// indent-tabs-mode: nil
// End:

