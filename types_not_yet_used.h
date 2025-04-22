
/**
 * Prints a condition object to a stream.
 *
 * @param s A stream to which the condition object should be printed.
 * @param value A condition object.
 * @return The stream to which was the condition object printed.
 */
inline
std::ostream& operator<<(std::ostream& s, const COND& value)
{
  return s << "COND(index=" << value.q() << ")";
}


/**
 * Concatenates a string with a condition object.
 *
 * @param s A string.
 * @param cond A condition object.
 * @return A new string with a value of @em s followed by a string
 *   representation of @em cond.
 */
inline
std::string operator+(const std::string& s, const COND& cond)
{
  return s + "COND(index=" + decstr(cond.q()) + ")";
}

/**
 * Concatenates a condition object with a string.
 *
 * @param cond A condition object.
 * @param s A string.
 * @return A new string with a value of a string representation of @em cond
 *   followed by @em s.
 */
inline
std::string operator+(const COND& cond, const std::string& s)
{
  return "COND(index=" + decstr(cond.q()) + ")" + s;
}



// Definitions of classes representing synchronisation primitives
typedef class INDEX< 200 > LOCK; //!< A class representing a lock.
typedef class INDEX< 201 > COND; //!< A class representing a condition.
typedef class INDEX< 202 > THREAD; //!< A class representing a thread.
