/**
 * @brief	Implementation of the Eraser algorithm.
 *
 * @file    eraser.cpp
 * @author  Ondrej Vasicek
 * @date    Created 2017-08-28
 * @date	Last update 2017-11-25
 * @version 1.0
 */

#include "tool.h"
#include <vector>
#include <set>
#include <assert.h>
#include <algorithm>	///< find
#include <pthread.h>	///< mutex

// TODO replace with a conf file
#define REP_ONCE	1	///< report only one race for each variable (causes other races to be ignored)

namespace {

	//TODO check if the below format of documenting works
	//@{
	/**	@note Mutexes are always acquired in this order (thread, var, lockset) to prevent deadlocks
	 */
	pthread_mutex_t threads_mutex;		///< Mutex protecting the thread_list class object
	pthread_mutex_t variables_mutex;	///< Mutex protecting the variable_list class object
	pthread_mutex_t locksets_mutex;		///< Mutex protecting the locksets_mutex class object
	//@}

	/**	@brief	A wrapper around mutex locking. To make the code easier to read.
	 * 	@mutex	in	Pointer to the mutex to lock.
	 */
	void mutex_lock(pthread_mutex_t* mutex)
	{
		if (pthread_mutex_lock(mutex) != 0)
		{
			perror("pthread_mutex_lock: Failed to lock a mutex!\n");
			exit(EXIT_FAILURE);
		}
	}

	/**	@brief	A wrapper around mutex unlocking. To make the code easier to read.
	 * 	@mutex	in	Pointer to the mutex to unlock.
	 */
	void mutex_unlock(pthread_mutex_t* mutex)
	{
		if (pthread_mutex_unlock(mutex) != 0)
		{
			perror("pthread_mutex_unlock: Failed to unlock a mutex!\n");
			exit(EXIT_FAILURE);
		}
	}

	/**	@brief	Gets a declaration of a variable.
	 * 	@param	variable A structure containing the information about the variable.
	 * 	@return	A string containing the declaration of the variable.
	 */
	inline
	std::string getVariableDeclaration(const VARIABLE& variable)
	{
	  // Format the name, type and offset to a 'type name[+offset]' string
	  return ((variable.type.size() == 0) ? "" : variable.type + " ")
	    + ((variable.name.empty()) ? "<unknown>" : variable.name)
	    + ((variable.offset == 0) ? "" : "+" + decstr(variable.offset));
	}

	/**	@brief	Used as a parameter of report_race() below. To know what type of access called the function.
	 */
	typedef enum {
		read,
		write
	} access_type;

	/** @brief	Outputs a formated report of a data race
	 * 	@note	Mostly copied from Atomrace.
	 * 	@param	acc_type	in	What type of access caused the data race.
	 * 	@param	tid			in	Id of the thread that is accessing.
	 * 	@param	addr		in	Address of the variable.
	 * 	@param	size		in	Variable size in TODO (have to check).
	 * 	@param	variable	in	Object containing extra information about the variable (name, type, offset).
	 * 	@param	location	in	File name and line number in the source code.
	 */
	void report_race(access_type acc_type, THREADID tid, ADDRINT addr, UINT32 size, const VARIABLE& variable, const LOCATION& location)
	{
	      CONSOLE_NOPREFIX("Data race detected on memory address " + hexstr(addr) + "\n"
	        + "  Thread " + decstr(tid)
	        + ((acc_type == write) ? " written to " : " read from ")
	        + ::getVariableDeclaration(variable) + "\n"
	        + "    accessed at line " + decstr(location.line)
	        + " in file " + ((location.file.empty()) ?
	          "<unknown>" : location.file) + "\n");

	      // Helper variables
	      Backtrace bt;
	      Symbols symbols;
	      std::string tcloc;

	      // Get the backtrace of the current thread
	      THREAD_GetBacktrace(tid, bt);

	      // Translate the return addresses to locations
	      THREAD_GetBacktraceSymbols(bt, symbols);

	      CONSOLE_NOPREFIX("\n  Thread " + decstr(tid) + " backtrace:\n");

	      for (Symbols::size_type i = 0; i < symbols.size(); i++)
	      { // Print information about each return address in the backtrace
	        CONSOLE_NOPREFIX("    #" + decstr(i) + (i > 10 ? " " : "  ")
	          + symbols[i] + "\n");
	      }

	      THREAD_GetThreadCreationLocation(tid, tcloc);

	      CONSOLE_NOPREFIX("\n    Thread created at " + tcloc + "\n");
	}

	/**	@brief		Represents a single thread.
	 * 	@details	A thread is identified by a unique id (tid). Lockset vector tracks all locks held by the thread.
	 * 				Locks can be added or removed using lock_acquire and lock_release respectively.
	 */
	class thread {

		const THREADID tid;				///< Unique identifier (passed by a callback).
		mutable std::vector<bool> lockset;	///< A bit vector containing ones on indexes corresponding to locks that the thread currently holds

	public:

		explicit thread() : tid{} {}
		explicit thread(THREADID id) : tid{id} {}

		bool operator==(const thread &t) const { return t.tid == tid; }
		bool operator!=(const thread &t) const { return t.tid != tid; }
		bool operator<(const thread &t) const { return t.tid > tid; }

		/**	@brief	Allows to retrieve the thread id.
		 *	@return	Value of tid.
		 */
		THREADID getid() const { return tid; }

		/**	@brief	Returns the lockset vector
		 *	@return	A bit vector represantation of locks held.
		 */
		std::vector<bool> locks_held() const
		{
			return lockset;
		}

		/**	@brief	Adds a newly acquired lock into the bit vector lockset.
		 * 	@param	lock	in	Lock to be added (passed by a callback).
		 */
		void lock_acquire(LOCK lock) const
		{
			std::size_t size = lockset.size();	///< to fix comparison between unsigned and signed
			if (size < lock.index+1)
			{
				lockset.resize(lock.index + 10);	//TODO best resize step (+1, +10, +1000, ...)
			}

			lockset[lock.index] = true;
		}

		/**	@brief	Removes a lock from the bit vector lockset.
		 * 	@param	lock	in	Lock to be removed (passed by a callback).
		 */
		void lock_release(LOCK lock) const
		{
			assert(lock.index < lockset.size() && "thread::lockrelease() : releasing a lock that was never added");

			lockset[lock.index] = false;
		}
	};

	/**	@brief		A set of ::thread objects.
	 *	@details	A sorted set of all threads registered so far. Provides a find function and an add function.
	 */
	class thread_list {

		std::set<::thread> list;	///<	Core set of known threads.

	public:

		//TODO check if the below format of documenting works
		//@{
		/**	@brief	Defines begin() and end() to allow usage with for-each loop.	*/
		typename std::set<::thread>::iterator begin() {return list.begin();}
		typename std::set<::thread>::iterator end() {return list.end();}
		//@}

		/**	@brief Adds a new ::thread object to the list.
		 * 	@param tid	in	The unique id of the thread (passed by a callback).
		 */
		void add_new(THREADID tid)
		{
			::thread t(tid);
			list.insert(t);
		}

		/**	@brief	Searches for a thread in the set based on its id (tid).
		 * 	@param	tid	in	Unique id of the thread to be found.
		 * 	@return	Iterator pointing to the thread object found.
		 * 			list.end() if the object was not found.
		 */
		std::set<::thread>::iterator find(THREADID tid)
		{
			::thread t(tid);

			auto i = list.find(t);

			return i;
		}
	} threads;

	/**	@brief		A vector of locksets for all variables.
	 * 	@details	Locksets are implemented as a vector<bool> to save space. Each bit represents a lock of the corresponding index.
	 * 	@note		Currently each variable has its own lockset. Variables could share locksets to reduce memory consumption. TODO
	 */
	class lockset_list{

		/**	@brief		Core vector of locksets.
		 * 	@details	In the lockset each lock is represented by a bit at the same index as the lock identifying index
		 */
		std::vector<std::vector<bool>> list;

	public:

		/**	@brief 	Adds new empty lockset into the list.
		 * 	@return	The assigned index of the new lockset.
		 */
		unsigned int make_index()
		{
			std::vector<bool> lockset;
			unsigned int index = list.size();

			list.push_back(lockset);

			return index;
		}

		/**	@brief	Used to fill an empty vector<bool> with locks from a vector<LOCK>.
		 * 	@param	index	in	Index of the lockset to be modified.
		 * 	@param	src		in	The lockset to take values from.
		 */
		void set_lockset(unsigned int index, std::vector<bool> src)
		{
			if (index < list.size())
			{
				list[index] = src;
			}

			else
				assert(0 && "Setting a lockset at a nonexisting index");

		}

		/**	@brief	Intersects a lockset from the list with another lockset.
		 * 	@param	index	in	Index of the lockset to be modified.
		 * 	@param	src	in	A lockset to intersect with.
		 */
		void intersect_lockset(unsigned int index, std::vector<bool> src)
		{
			if (index < list.size())
			{	// check if any bits from list[index] should be set to zero
				for (unsigned int i = 0; i < list[index].size(); i++)
				{
					if (list[index][i] == 1 && i < src.size())
					{
						list[index][i] = src[i];	///< intersection
					}
				}
			}
			else
				assert(0 && "Intersecting a lockset at a nonexisting index");
		}

		/**	@brief	Used to check if all bits in a lockset are zero.
		 * 	@param	index	in	Index of the lockset to check.
		 * 	@return	True if all bits are zero
		 * 			False otherwise
		 */
		bool empty(unsigned int index)
		{
			if (index < list.size())
			{
				auto i = std::find(list[index].begin(), list[index].end(), 1);

				if (i == list[index].end())
					return true;

				else
					return false;
			}
			else
			{
				assert(0 && "Checking a lockset at a nonexisting index");
				return false;
			}
		}

	} locksets;

	/**	@brief		Represents all possible states of a variable.
	 *	@details	Each variable should be declared as a 'virgin'. Initialization moves it to the 'exclusive' state and
	 *				the initializing thread is remembered as the 'first_writer'.
	 *				A read access by a new thread would change the state of the variable from 'exclusive' to 'shared'.
	 *				While the write access skips the 'shared' state and moves the variable straight to 'shared_modified'.
	 *				While in 'shared' state the lockset has to be updated but no races are reported.
	 *				Any write performed by any thread then moves it to the 'shared_modified' state.
	 *				In the 'shared_modified' state locks get updated and races reported if a lockset gets empty.
	 *				If a race is reported the final state 'race_reported' comes in (optional).
	 *				Nothing happens in this state anymore (no need to do anything)
	 *	@note		State graph on page number eight here: http://cseweb.ucsd.edu/~savage/papers/Tocs97.pdf
	 */
	typedef enum {
		virgin,	///< not used (the framework does not allow to detect variable declaration)
		exclusive,	///< only one thread has accessed the variable -> first_writer (first write = initialization)
		shared,	///< multiple threads have read from the variable (the first_writer could have written beforehand)
		shared_modified, ///< multiple threads have written to the variable - races get reported here
		race_reported ///< an extra state to prevent multiple reports of the same race
	} var_state;

	/**	@brief		Represents a single variable.
	 * 	@details	A variable is identified by its address.
	 * 				The corresponding lockset is in the lockset_list at the lockset_index.
	 * 				The class takes care of state transitions,
	 * 				using only write/read access report with thread id passed as argument.
	 */
	class variable{

		const ADDRINT adr;			///< Actual address of the variable (passed by a callback).
		const THREADID first_writer;		///< First write access came from this thread.
		mutable unsigned int lockset_index; ///< Index into the list of locksets. To access the variables candidate locks.
		mutable ::var_state state;			///< Holds the state in which the variable currently is.

	public:

		explicit variable(ADDRINT a) : adr{a}, first_writer{}, lockset_index{}, state{virgin} {};	///< for temporary variables (eg. variable_list::find(ADDRINT))
		explicit variable(ADDRINT a, THREADID t) : adr{a}, first_writer{t},lockset_index{}, state{exclusive} {};

		bool operator==(const variable &v) const { return v.adr == adr;}
		bool operator!=(const variable &v) const { return v.adr != adr;}
		bool operator<(const variable &v) const { return v.adr > adr;}

		/**	@brief	Takes care of variable state transferring and lockset updating.
		 * 	@param	th	in	Id of the thread that is accessing the variable.
		 * 	@return	Bool value representing if a race was detected.
		 * 	@note	State transitions are explained at the enum of var_state (search for enum..)
		 */
		bool write_access(const ::thread th) const
		{
			switch (state)
			{
			case virgin:
				assert(0 && "::variable::write_access : there should be no virgin variables");

				//state = exclusive;
				//first_writer = th;
				break;

			case exclusive:

				if (th.getid() != first_writer)
				{
					state = shared_modified;

					::mutex_lock(&locksets_mutex);	///< Lock the lockset_list object.

						lockset_index = ::locksets.make_index();
						::locksets.set_lockset(lockset_index, th.locks_held());
						if(::locksets.empty(lockset_index))
						{
							if (REP_ONCE)
								state = race_reported;

							::mutex_unlock(&locksets_mutex);	///< Unlock the lockset_list object.
							return true;	// report race
						}

					::mutex_unlock(&locksets_mutex);	///< Unlock the lockset_list object.
				}
				break;

			case shared:

				state = shared_modified;

				::mutex_lock(&locksets_mutex);	///< Lock the lockset_list object.

					::locksets.intersect_lockset(lockset_index, th.locks_held());
					if (::locksets.empty(lockset_index))
					{
						if (REP_ONCE)
							state = race_reported;

						::mutex_unlock(&locksets_mutex);	///< Unlock the lockset_list object.
						return true;	// report race
					}

				::mutex_unlock(&locksets_mutex);	///< Unlock the lockset_list object.
				break;

			case shared_modified:

				::mutex_lock(&locksets_mutex);	///< Lock the lockset_list object.

					::locksets.intersect_lockset(lockset_index, th.locks_held());

					if (::locksets.empty(lockset_index))
					{
						if (REP_ONCE)
							state = race_reported;

						::mutex_unlock(&locksets_mutex);	///< Unlock the lockset_list object.
						return true;	// report race
					}

				::mutex_unlock(&locksets_mutex);	///< Unlock the lockset_list object.
				break;

			case race_reported:
				break;
			}

			return false;	// no race detected
		}

		/**	@brief 	Takes care of variable state transferring and lockset updating.
		 * 	@param	th	in	Id of the thread that is accessing the variable.
		 * 	@return	Bool value representing if a race was detected.
		 * 	@note 	State transitions are explained at the enum of var_state (search for enum..)
		 */
		bool read_access(const ::thread th) const
		{
			switch (state)
			{
			case virgin:
				assert(0 && "::variable::write_access : there should be no virgin variables");
				break;

			case exclusive:

				if (th.getid() != first_writer)
				{
					state = shared;

					::mutex_lock(&locksets_mutex);	///< Lock the lockset_list object.

						lockset_index = ::locksets.make_index();
						::locksets.set_lockset(lockset_index, th.locks_held());

					::mutex_unlock(&locksets_mutex);	///< Unlock the lockset_list object.
				}
				break;

			case shared:

				::mutex_lock(&locksets_mutex);	///< Lock the lockset_list object.

					::locksets.intersect_lockset(lockset_index, th.locks_held());

				::mutex_unlock(&locksets_mutex);	///< Unlock the lockset_list object.
				break;

			case shared_modified:

				::mutex_lock(&locksets_mutex);	///< Lock the lockset_list object.

					::locksets.intersect_lockset(lockset_index, th.locks_held());
					if (::locksets.empty(lockset_index))
					{
						if (REP_ONCE)
							state = race_reported;

						::mutex_unlock(&locksets_mutex);	///< Unlock the lockset_list object.
						return true;	// report race
					}

				::mutex_unlock(&locksets_mutex);	///< Unlock the lockset_list object.
				break;

			case race_reported:
				break;
			}

			return false;	// no race detected
		}
	};

	/**	@brief		A set of ::variable objects.
	 *	@details	A sorted list of all variables registered so far. Provides a find function and an add function.
	 */
	class variable_list{

		std::set<variable> list;	///<	Core set of known variables.

	public:

		//TODO check if the below format of documenting works
		//@{
		/**	@brief	Defines begin() and end() to allow usage with for-each loop.
		 */
		typename std::set<variable>::iterator begin() {return list.begin();}
		typename std::set<variable>::iterator end() {return list.end();}
		//@}

		/**	@brief Adds a new ::variable object to the list.
		 * 	@param adr	in	The unique address of the variable (passed by a callback).
		 * 	@param t	in	Thread id of the thread that created the var.
		 */
		void add_new(ADDRINT adr, THREADID t)
		{
			::variable v(adr, t);
			list.insert(v);
		}

		/**	@brief Searches for a variable in the sorted list based on its address.
		 * 	@param adr	in	Unique address of the variable to be found.
		 * 	@return	Iterator pointing to the variable object found.
		 * 			list.end() if the object was not found.
		 */
		std::set<variable>::iterator find(ADDRINT adr)
		{
			::variable v(adr);

			auto i = list.find(v);

			return i;
		}
	} variables;

	/**	@brief	Creates the list of threads by adding new ones as they start.
	 * 	@param	tid	in	Unique id of the thread
	 */
	VOID threadStarted(THREADID tid)
	{
		// Lock the thread_list object
		::mutex_lock(&threads_mutex);

			::threads.add_new(tid);

		::mutex_unlock(&threads_mutex);
	}

	/**	@brief	Creates the list of variables by adding new found ones and reports read access to already known variables.  Reports data races.
	 * 	@param	tid			in	Id of the thread that is accessing.
	 * 	@param	addr		in	Address of the variable.
	 * 	@param	size		in	Variable size in TODO (have to check).
	 * 	@param	variable	in	Object containing extra information about the variable (name, type, offset).
	 * 	@param	location	in	File name and line number in the source code.
	 */
	VOID beforeMemoryRead(THREADID tid, ADDRINT addr, UINT32 size, const VARIABLE& variable, const LOCATION& location)
	{
		// Lock the thread_list and variable_list objects
		::mutex_lock(&threads_mutex);
		::mutex_lock(&variables_mutex);

			auto v = ::variables.find(addr);
			auto t = ::threads.find(tid);
			assert (t != threads.end() && "an untracked thread has accessed a variable");	//TODO

			if (v == variables.end())
			{
				::variables.add_new(addr, t->getid());
			}
			else
			{
				if (v->read_access(*t))
				{
					::report_race(read, tid, addr, size, variable, location);
				}
			}

		::mutex_unlock(&variables_mutex);
		::mutex_unlock(&threads_mutex);
	}

	/**	@brief	Creates the list of variables by adding new found ones and reports write access to already known variables.  Reports data races.
	 * 	@param	tid			in	Id of the thread that is accessing.
	 * 	@param	addr		in	Address of the variable.
	 * 	@param	size		in	Variable size in TODO (have to check).
	 * 	@param	variable	in	Object containing extra information about the variable (name, type, offset).
	 * 	@param	location	in	File name and line number in the source code.
	 */
	VOID beforeMemoryWrite(THREADID tid, ADDRINT addr, UINT32 size, const VARIABLE& variable, const LOCATION& location)
	{
		//	Lock the thread_list and variable_list objects
		::mutex_lock(&threads_mutex);
		::mutex_lock(&variables_mutex);

			auto v = ::variables.find(addr);
			auto t = ::threads.find(tid);
			assert (t != threads.end() && "an untracked thread has accessed a variable");	//TODO

			if (v == variables.end())
			{
				::variables.add_new(addr, t->getid());
			}
			else
			{
				if (v->write_access(*t))
				{
					::report_race(read, tid, addr, size, variable, location);
				}
			}

		::mutex_unlock(&variables_mutex);
		::mutex_unlock(&threads_mutex);
	}

	/**	@brief	Adds newly acquired locks to the lockset of held locks for each thread.
	 *	@param	tid		in	Unique id of the thread that acquired the lock.
	 *	@param	lock	in	An object representing the lock that has been acquired.
	 */
	VOID beforeLockAcquire(THREADID tid, LOCK lock)
	{
		// Lock the thread_list object
		::mutex_lock(&threads_mutex);

			auto t = ::threads.find(tid);
			assert(t != ::threads.end() && "unknown thread acquired a lock"); //TODO


			t->lock_acquire(lock);

		::mutex_unlock(&threads_mutex);
	}

	/**	@brief	Removes released locks from the lockset of each thread.
	 *	@param	tid		in	Unique id of the thread that released the lock.
	 *	@param	lock	in	An object representing the lock that has been released.
	 */
	VOID beforeLockRelease(THREADID tid, LOCK lock)
	{
		// Lock the thread_list object
		::mutex_lock(&threads_mutex);

			auto t = ::threads.find(tid);
			assert(t != ::threads.end() && "unknown thread released a lock"); //TODO

			t->lock_release(lock);

		::mutex_unlock(&threads_mutex);
	}
};

/**	@brief Initializes the analyser plug-in. Called before the tested program starts.
 */
void eraser_init()
{
	//	Initialize mutexes
	int err;
	if ((err = pthread_mutex_init(&threads_mutex, NULL)) != 0)
	{
		fprintf(stderr, "pthread_mutex_init() error %d\n", err);
		exit(EXIT_FAILURE);
	}
	if ((err = pthread_mutex_init(&variables_mutex, NULL)) != 0)
	{
		fprintf(stderr, "pthread_mutex_init() error %d\n", err);
		exit(EXIT_FAILURE);
	}
	if ((err = pthread_mutex_init(&locksets_mutex, NULL)) != 0)
	{
		fprintf(stderr, "pthread_mutex_init() error %d\n", err);
		exit(EXIT_FAILURE);
	}

	//	register callback functions
	THREAD_ThreadStarted(::threadStarted);
	ACCESS_BeforeMemoryRead(::beforeMemoryRead);
	ACCESS_BeforeMemoryWrite(::beforeMemoryWrite);
	SYNC_BeforeLockAcquire(::beforeLockAcquire);
	SYNC_BeforeLockRelease(::beforeLockRelease);
}

/**	@brief Terminates the analyser plug-in. Called after the tested program ends.
 */
void eraser_terminate()
{
	//	Destroy mutexes
	int err;
	if ((err = pthread_mutex_destroy(&threads_mutex)) != 0)
	{
		fprintf(stderr, "pthread_mutex_destroy() error %d\n", err);
		exit(EXIT_FAILURE);
	}
	if ((err = pthread_mutex_destroy(&variables_mutex)) != 0)
	{
		fprintf(stderr, "pthread_mutex_destroy() error %d\n", err);
		exit(EXIT_FAILURE);
	}
	if ((err = pthread_mutex_destroy(&locksets_mutex)) != 0)
	{
		fprintf(stderr, "pthread_mutex_destroy() error %d\n", err);
		exit(EXIT_FAILURE);
	}
}

/** End of file eraser.cpp **/
