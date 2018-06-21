/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_MUTEX_H
#define QWT_MUTEX_H 1

#include "qwt_global.h"

class QWT_EXPORT QwtMutex
{
public:
    /*!
      Lock types to serialize reader and writer access for multi-threading
      \sa QwtMutex::lock, QwtMutex::isLocked and QwtMutex::unlock.
     */
    enum LockType 
    {
        ReadLock,
        WriteLock
    };

    virtual ~QwtMutex() {};

    /*! 
      Attempt to lock the mutex.

      - timeout < 0\n
        If the mutex is locked then this call will block until it is 
        the mutex is unlocked, or an error occurs. 
      - timeout == 0\
        If the mutex is locked then this call will fail, but doesn´t block.
      - timeout > 0
        Try to lock the mutex and give up after timout msecs.

      \param lock Type of lock
      \param timeout Timout for this operation

      \return TRUE if the mutex could be locked, FALSE in case of error 
              or timeout
    */
    virtual bool lock(LockType lock, int timeout = -1) const = 0;

    /*!
        TRUE if the mutex is locked; otherwise returns FALSE. 
     */
    virtual bool isLocked(LockType) const = 0;

    /*!
      Unlock the mutex

      \param lock Type of lock
     */
    virtual void unlock(LockType) const = 0;
};

#endif

// Local Variables:
// mode: C++
// c-file-style: "stroustrup"
// indent-tabs-mode: nil
// End:

