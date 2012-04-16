/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkMacro.h,v $
  Language:  C++
  Date:      $Date: 2006/05/10 20:27:16 $
  Version:   $Revision: 1.65 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

  Portions of this code are covered under the VTK copyright.
  See VTKCopyright.txt or http://www.kitware.com/VTKCopyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
/**
 * itkMacro.h defines standard system-wide macros, constants, and other
 * parameters. One of its most important functions is to define macros used
 * to interface to instance variables in a standard fashion. For example,
 * these macros manage modified time, debugging information, and provide a
 * standard interface to set and get instance variables.  Macros are
 * available for built-in types; for string classe; vector arrays;
 * object pointers; and debug, warning, and error printout information.
 */

#ifndef __UserMacro_h
#define __UserMacro_h

#include "itkWin32Header.h"
#include "itkConfigure.h"

#include <string>

/** Set built-in type.  Creates member Set"name"() (e.g., SetVisibility()); */
#define UserSetMacro(name, type) \
  virtual void Set##name(int i, const type _arg) \
    { \
    itkDebugMacro("setting " #name " to " << _arg); \
    if( this->m_##name[i] != _arg ) \
      { \
      this->m_##name[i] = _arg; \
      this->Modified(); \
      } \
    }

/** Set pointer to object; uses Object reference counting methodology.
 * Creates method Set"name"() (e.g., SetPoints()). Note that using
 * smart pointers requires using real pointers when setting input,
 * but returning smart pointers on output. */
#define UserSetObjectMacro(name, type) \
  virtual void Set##name(int i, type * _arg) \
    { \
    if( this->m_##name[i] != _arg ) \
      { \
      this->m_##name[i] = _arg; \
      this->Modified(); \
      } \
    }

/** Get a smart pointer to an object.  Creates the member
 * Get"name"() (e.g., GetPoints()). */
#define UserGetObjectMacro(name, type) \
  virtual type * Get##name(int i) \
    { \
    itkDebugMacro("returning " #name " address " << this->m_##name[i] ); \
    return this->m_##name[i].GetPointer(); \
    }

/** Set const pointer to object; uses Object reference counting methodology.
 * Creates method Set"name"() (e.g., SetPoints()). Note that using
 * smart pointers requires using real pointers when setting input,
 * but returning smart pointers on output. */
#define UserSetConstObjectMacro(name, type) \
  virtual void Set##name(int i, const type * _arg) \
    { \
    itkDebugMacro("setting " << #name " to " << _arg ); \
    if( this->m_##name[i] != _arg ) \
      { \
      this->m_##name[i] = _arg; \
      this->Modified(); \
      } \
    }

/** Get a smart const pointer to an object.  Creates the member
 * Get"name"() (e.g., GetPoints()). */
#define UserGetConstObjectMacro(name, type) \
  virtual const type * Get##name(int i) const \
    { \
    itkDebugMacro("returning " #name " address " << this->m_##name[i] ); \
    return this->m_##name[i].GetPointer(); \
    }

/** Get a const reference to a smart pointer to an object.
 * Creates the member Get"name"() (e.g., GetPoints()). */
#define UserGetConstReferenceObjectMacro(name, type) \
  virtual const typename type::Pointer & Get##name(int i) const \
    { \
    itkDebugMacro("returning " #name " address " << this->m_##name[i] ); \
    return this->m_##name[i]; \
    }

#endif
