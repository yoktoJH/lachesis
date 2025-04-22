/*
 * Copyright (C) 2013-2020 Jan Fiedor <fiedorjan@centrum.cz>
 *
 * This file is part of ANaConDA.
 *
 * ANaConDA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * ANaConDA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ANaConDA. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @brief Contains definitions of types used in various parts of the framework.
 *
 * A file containing definitions of types used in various parts of the framework
 *   together with some utility functions for their formatting or serialisation.
 *
 * @file      types.h
 * @author    Jan Fiedor (fiedorjan@centrum.cz)
 * @date      Created 2013-02-13
 * @date      Last Update 2016-05-09
 * @version   0.4.2
 */

#ifndef __PINTOOL_ANACONDA__TYPES_H__
  #define __PINTOOL_ANACONDA__TYPES_H__

#include <cinttypes>
#include <iomanip>
#include <ios>
#include <ostream>
#include <iostream>
#include <sstream>
#include <thread>
//#include "pin.H"

/// this is from pin

#if !defined(VOID)
typedef void VOID; 
#endif

typedef char            CHAR;
typedef unsigned int    UINT;
typedef int             INT;
typedef double          FLT64;
typedef float           FLT32;
typedef unsigned int    USIZE;
typedef signed int      SIZE;

/*
 * Generic type for three-state logic.
 */
enum TRI
{
    TRI_YES,
    TRI_NO,
    TRI_MAYBE
};

#if defined(_MSC_VER)
typedef unsigned __int8 UINT8 ;
typedef unsigned __int16 UINT16;
typedef unsigned __int32 UINT32;
typedef unsigned __int64 UINT64;
typedef __int8 INT8;
typedef __int16 INT16;
typedef __int32 INT32;
typedef __int64 INT64;

#else

typedef uint8_t  UINT8;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef uint64_t UINT64;
typedef int8_t  INT8;
typedef int16_t INT16;
typedef int32_t INT32;
typedef int64_t INT64;
# endif


 /// ADDRINT is UINT32 or UINT64 based on the architecture.
 /// I should find a way to make this architecture dependant, for now I can just go with UINT64 as thats this computer

typedef UINT64 ADDRINT;
typedef INT64 ADDRDELTA;

// Definitions of basic types
typedef ADDRINT index_t;

// Special values of basic types
#define INVALID_INDEX (index_t)-1;



/**
 * @brief A structure representing a variable.
 */
typedef struct Variable_s
{
  std::string name; //!< A name of the variable.
  std::string type; //!< A type of the variable.
  UINT32 offset; //!< An offset within the variable which was accessed.

  /**
   * Constructs a Variable_s object.
   */
  Variable_s() : name(), type(), offset(0) {}

  /**
   * Constructs a Variable_s object.
   *
   * @param n A name of a variable.
   * @param t A type of a variable.
   * @param o An offset within a variable which was accessed.
   */
  Variable_s(const std::string& n, const std::string& t, const UINT32 o)
    : name(n), type(t), offset(o) {}
} VARIABLE;

/**
 * @brief A structure representing a source code location.
 */
typedef struct Location_s
{
  std::string file; //!< A name of a file.
  INT32 line; //!< A line number.

  /**
   * Constructs an object representing a source code location.
   */
  Location_s() : file(), line(-1) {}

  /**
   * Constructs an object representing a source code location.
   *
   * @param f A name of the file in which is the source code location situated.
   * @param l A line number in the file in which is the source code location
   *   situated.
   */
  Location_s(const std::string& f, INT32 l) : file(f), line(l) {}
} LOCATION;

/**
 * @brief A structure representing an image (executable, shared library, etc.).
 */
typedef struct Image_s
{
  const std::string& path; //!< A path to the image.

  /**
   * Constructs an object representing an image.
   *
   * @param p A path to the image.
   */
  Image_s(const std::string& p) : path(p) {}
} IMAGE;

/**
 * @brief A structure representing a function (or method).
 */
typedef struct Function_s
{
  const std::string name; //!< A name of the function.
  const std::string& signature; //!< A (mangled) signature of the function.
  const index_t image; //!< An index of the image containing the function.

  /**
   * Constructs an object representing a function.
   *
   * @param n A name of the function.
   * @param s A (mangled) signature of the function.
   * @param i An index of the image containing the function.
   */
  Function_s(const std::string& n, const std::string& s, const index_t i)
    : name(n), signature(s), image(i) {}
} FUNCTION;

/**
 * @brief A structure representing an instruction.
 */
typedef struct Instruction_s
{
  const ADDRINT offset; //!< An offset of the instruction in the image.
  /**
   * @brief An index of the function containing the instruction.
   */
  const index_t function;
  /**
   * @brief An index of the source code location containing the code which the
   *   instruction is performing.
   */
  const index_t location;

  /**
   * Constructs an object representing an instruction.
   *
   * @param o An offset of the instruction in the image.
   * @param f An index of the function containing the instruction.
   * @param l An index of the source code location containing the code which the
   *   instruction is performing.
   */
  Instruction_s(const ADDRINT o, const index_t f, const index_t l) : offset(o),
    function(f), location(l)  {}
} INSTRUCTION, CALL;



/** added by meee **/
// Plugin functions called from the framework should be like C API functions
#define PLUGIN_FUNCTION(name) \
  extern "C" void name

// Define macros for the plugin entry and exit functions (init and finish)
#define PLUGIN_INIT_FUNCTION PLUGIN_FUNCTION(__atomrace_init)
#define PLUGIN_FINISH_FUNCTION PLUGIN_FUNCTION(__anaconda_finish)

typedef UINT32 THREADID;
typedef bool BOOL;
// stolen from PIN
template<int dummy>
class INDEX
{
public:
  /*
    INT32 index; must be public - so that both vs8 and icc will treat
    functions that return  INDEX<num> classes (such as BBL) the same  - i.e. will return
    the value in a register without passing an implicit param into the function that is
    a pointer to the location to store the return value to.
    If it is private then vs8 will do the implicit passing and icc will not.
    This incompatability is supposed to be fixed in icc version 11.0
  */
  UINT64 index;
  BOOL operator==(const INDEX<dummy> right) const { return right.index == index;}\
  BOOL operator!=(const INDEX<dummy> right) const { return right.index != index;}\
  BOOL operator<(const INDEX<dummy> right)  const { return index < right.index;}\

  // please do not try to introduce a constructor here
  // otherwise programs will not compile
  // and there are possibly performance penalties as well
  //INDEX(UINT64 x=0) : index(x) {}
  UINT64 q() const {return index;}
  BOOL is_valid() const { return (index>0);}
  VOID  q_set(UINT64 y) {index=y;}
  VOID  invalidate() {index=0;}

};

typedef INDEX< 200 > LOCK;

struct stack_frame {
  char *function_name;
  char *file_name;
  int32_t line = -1;
};


std::string decstr(bool) = delete;

template <typename N>
typename std::enable_if<std::is_integral<N>::value, std::string>::type
decstr(N number, int width = 0)
{
  std::ostringstream oss; 
  oss.width(width);
  oss << std::setfill('0') << number; 
  return oss.str();
  
}
std::string hexstr(bool) = delete;

template <typename N>
typename std::enable_if<std::is_integral<N>::value, std::string>::type
hexstr(N number, int width = 0)
{
  std::string buffer;
  buffer.resize(width == 0? 20:width);
  sprintf(buffer.data(),"0x%0*lx",width,number);
  return buffer;
}

/**
 * Prints a lock object to a stream.
 *
 * @param s A stream to which the lock object should be printed.
 * @param value A lock object.
 * @return The stream to which was the lock object printed.
 */
inline
std::ostream& operator<<(std::ostream& s, const LOCK& value)
{
  return s << "LOCK(index=" << value.q() << ")";
}
/**
 * Concatenates a string with a lock object.
 *
 * @param s A string.
 * @param lock A lock object.
 * @return A new string with a value of @em s followed by a string
 *   representation of @em lock.
 */
inline
std::string operator+(const std::string& s, const LOCK& lock)
{
  return s + "LOCK(index=" + decstr(lock.q()) + ")";
}

/**
 * Concatenates a lock object with a string.
 *
 * @param lock A lock object.
 * @param s A string
 * @return A new string with a value of a string representation of @em lock
 *   followed by @em s.
 */
inline
std::string operator+(const LOCK& lock, const std::string& s)
{
  return "LOCK(index=" + decstr(lock.q()) + ")" + s;
}

#endif /* __PINTOOL_ANACONDA__TYPES_H__ */

/** End of file types.h **/

