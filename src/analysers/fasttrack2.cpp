/**
 * @brief	Fasttrack 2
 *
 * @file      fasttrack2.cpp
 * @author    Ondrej Vasicek
 * @date      Created 2017-11-25
 * @version   1.1
 * 
 * @note	  Version extended to report a pair of conflicting accesses
 */

#include "tool.h"

#include <vector>
#include <set>
#include <assert.h>
#include <algorithm>	///< find
#include <pthread.h>	///< mutex

#include <iostream>	 			///< debug
#define DBUG_OUT_BASIC		0	///< Enables basic debugging information (function start / end)
#define DBUG_OUT_DETAILS	0	///< Enables detailed debug info (prints vector clocks etc.)

/// Helper functions - not really related to fasttrack
namespace helper {
    // TODO revisit
    pthread_mutex_t Threads_mutex; ///< Mutex protecting the Threads class object
    pthread_mutex_t Variables_mutex; ///< Mutex protecting the Variables class object
    pthread_mutex_t Locks_mutex; ///< Mutex protecting the Locks class object


    /**	@brief	A wrapper around mutex locking. To make the code easier to read.
     * 	@mutex	in	Pointer to the mutex to lock.
     */
    void mutex_lock(pthread_mutex_t *mutex) {
        if (pthread_mutex_lock(mutex) != 0) {
            perror("pthread_mutex_lock: Failed to lock a mutex!\n");
            exit(EXIT_FAILURE);
        }
    }

    /**	@brief	A wrapper around mutex unlocking. To make the code easier to read.
     * 	@mutex	in	Pointer to the mutex to unlock.
     */
    void mutex_unlock(pthread_mutex_t *mutex) {
        if (pthread_mutex_unlock(mutex) != 0) {
            perror("pthread_mutex_unlock: Failed to unlock a mutex!\n");
            exit(EXIT_FAILURE);
        }
    }

    /**	@brief	Gets a declaration of a variable.
     * 	@param	variable A structure containing the information about the variable.
     * 	@return	A string containing the declaration of the variable.
     */
    inline std::string getVariableDeclaration(const VARIABLE &variable) {
        // Format the name, type and offset to a 'type name[+offset]' string
        return ((variable.type.size() == 0) ? "" : variable.type + " ")
               + ((variable.name.empty()) ? "<unknown>" : variable.name)
               + ((variable.offset == 0) ? "" : "+" + decstr(variable.offset));
    }

    ///	Used as a parameter of report_race() below. Represents what kind of race to report.
    typedef enum {
        write_read_race,
        write_write_race,
        read_write_race,
        shared_write_race
    } race_type;

    /// Used to conver race_type enum value to a string -> for txt output
    std::string racetype[]{
        "Write-Read",
        "Write-Write",
        "Read-Write",
        "Shared-Write"
    };

    /** @brief	Outputs a formated report of a data race
     * 	@note	Mostly copied from Atomrace.
     * 	@param	acc_type	in	What type of access caused the data race.
     * 	@param	tid			in	Id of the thread that is accessing.
     * 	@param	addr		in	Address of the variable.
     * 	@param	size		in	Variable size in TODO (have to check).
     * 	@param	variable	in	Object containing extra information about the variable (name, type, offset).
     * 	@param	location	in	File name and line number in the source code.
     */
    void report_race(race_type rt, ADDRINT addr, UINT32 size, const VARIABLE &variable, THREADID first_tid,
                     const LOCATION &first_loc, THREADID second_tid, const LOCATION &second_loc) {
        static bool FLAG_race_reported = false; ///< Used to tell if the first race has already been reported

        CONSOLE_NOPREFIX(
            racetype[rt] + ((FLAG_race_reported) ?
                " possible race detected on memory address " : " race detected on memory address ")
            + hexstr(addr) + "\n"

            + "  Thread " + decstr(first_tid)
            + ((rt != write_read_race) ? " written to " : " read from ")
            + getVariableDeclaration(variable) + "\n"
            + "    accessed at line " + decstr(first_loc.line)
            + " in file " + ((first_loc.file.empty()) ?
                "<unknown>" : first_loc.file) + "\n\n"

            + "  Thread " + decstr(second_tid)
            + ((rt == write_read_race || rt == write_write_race) ? " written to " : " read from ")
            + getVariableDeclaration(variable) + "\n"
            + "    accessed at line " + decstr(second_loc.line)
            + " in file " + ((second_loc.file.empty()) ?
                "<unknown>" : second_loc.file) + "\n"
        );
        FLAG_race_reported = true; // to alter future race outputs

        // Helper variables
        Backtrace bt;
        Symbols symbols;
        std::string tcloc;

        // Get the backtrace of the current thread
        THREAD_GetBacktrace(first_tid, bt);

        // Translate the return addresses to locations
        THREAD_GetBacktraceSymbols(bt, symbols);

        CONSOLE_NOPREFIX("\n  Thread " + decstr(first_tid) + " backtrace:\n");

        for (Symbols::size_type i = 0; i < symbols.size(); i++) {
            // Print information about each return address in the backtrace
            CONSOLE_NOPREFIX("    #" + decstr(i) + (i > 10 ? " " : "  ")
                + symbols[i] + "\n");
        }

        THREAD_GetThreadCreationLocation(first_tid, tcloc);

        CONSOLE_NOPREFIX("\n    Thread created at " + tcloc + "\n\n");
    }
};

/// Analyser main functions and structures
namespace {
    using namespace helper;

    /// Represents a pair of thread id and its clock (denoted: t@c)
    class epoch {
    protected:
        THREADID tid; ///< id of the coresponding thread
        unsigned int clock; ///< current logic time stored in the epoch

    public:
        bool shared; ///< special state of the epoch -> indicates read-shared variable state

        explicit epoch() : tid{0}, clock{0}, shared{false} {
        };

        explicit epoch(THREADID t) : tid{t}, clock{0}, shared{false} {
        };

        explicit epoch(THREADID t, unsigned int c) : tid{t}, clock{c}, shared{false} {
        };

        /// @return Tid of the epoch.
        unsigned int get_tid() const {
            return tid;
        }

        /// @return Clock of the epoch.
        unsigned int get_clk() const {
            return clock;
        }

        /// Increment the clock of the epoch.
        void tick() {
            clock++;
        }

        /** Returns an epoch with higher clock value out of the two.
         * @param	e in Epoch to compare with.
         * @return	Epoch with higher clock.
         */
        epoch max(epoch e) {
            assert(tid == e.tid && "ASSERT FAIL: max() with unmatching epochs");
            return (clock > e.clock) ? *this : e;
        }

        /// epoch happens before relation (defined for same tid only)
        bool operator<=(const epoch &e) const {
            assert(tid == e.tid && "ASSERT FAIL: comparison \"<=\" of unmatching epochs");
            return clock <= e.clock;
        }

        /// epoch identity operation
        bool operator==(const epoch &e) const { return clock == e.clock && tid == e.tid; }
    };

    /// Extended epoch - in addition holds location information of the associated access
    ///	@note Used to report a pair of accesses when a race is detected
    class xepoch : public epoch {
        LOCATION loc; ///<< Location iformation of the associated access

    public:
        explicit xepoch() : epoch{}, loc{} {
        };

        explicit xepoch(THREADID t) : epoch{t}, loc{} {
        };

        explicit xepoch(THREADID t, unsigned int c) : epoch{t, c}, loc{} {
        };

        /// epoch to xepoch conversion
        xepoch(epoch e) : epoch{e.get_tid(), e.get_clk()}, loc{} {
        };

        /// set location
        void set_loc(const LOCATION &l) { loc = l; }

        /// get location
        LOCATION get_loc() { return loc; }

        /// xepoch-epoch happens before relation (defined for same tid only)
        bool operator<=(const epoch &e) const {
            assert(tid == e.get_tid() && "ASSERT FAIL: comparison \"<=\" of unmatching epochs");
            return clock <= e.get_clk();
        }

        /// xepoch happens before relation (defined for same tid only)
        bool operator<=(const xepoch &e) const {
            assert(tid == e.get_tid() && "ASSERT FAIL: comparison \"<=\" of unmatching epochs");
            return clock <= e.get_clk();
        }

        /// xepoch and epoch identity operation
        bool operator==(const epoch &e) const { return clock == e.get_clk() && tid == e.get_tid(); }
        /// xepoch identity operation
        bool operator==(const xepoch &e) const { return clock == e.get_clk() && tid == e.get_tid(); }
    };

    /// A vector of epochs indexed by tid. Holds one epoch for each thread.
    /// @note	Used in a std::set so most methods have to be declared as const even though they alter mutable variables.
    template<typename T_epoch>
    class vector_clock {
    private:
        mutable std::vector<T_epoch> vc;

    public:
        /// Default ctor called byt he var_state and lock_state child classes.
        vector_clock() : vc{} {
        };

        /// Ctor called by the thread_state child class.
        /// Fills vc with minimal epochs to reach the desired index and then inserts an epoch with clock equal to one
        explicit vector_clock(THREADID tid) : vc{} {
            for (unsigned int i = 0; i < tid; i++) {
                T_epoch e{i};
                vc.push_back(e);
            }
            T_epoch e{tid, 1};
            vc.push_back(e);
        };

        /// @return Number of items inside the vector
        unsigned int size() const {
            return vc.size();
        }

        /** Extends one of the VCs to match the size of the other. (if needed)
         * @param other in/out Pointer to a VC to match with.
         * @note	Declared as const but altering a mutable variable.
         */
        void match_size(const vector_clock &other) const {
            if (size() == other.size())
                return;

            if (size() > other.size()) {
                for (unsigned int i = other.size(); i < size(); i++) {
                    T_epoch tmp{i};
                    other.set(tmp);
                }
            } else // (size() < other.size())
            {
                for (unsigned int i = size(); i < other.size(); i++) {
                    T_epoch tmp{i};
                    set(tmp);
                }
            }
        }

        /** Set an epoch inside the vector clock.
         * @param e in Src epoch.
         * @note	Declared as const but altering a mutable variable.
         */
        void set(T_epoch e) const {
            // VC does not yet contain a record for the right thread -> resize
            if (e.get_tid() >= size()) {
                for (unsigned int i = size(); i < e.get_tid(); i++) {
                    T_epoch tmp{i};
                    vc.push_back(tmp);
                }
                vc.push_back(e);
            }
            // no need to resize
            else
                vc[e.get_tid()] = e;
        }

        /** Increment an epoch inside the vector clock.
         * @param idx in Index of the epoch to increment.
         * @note	Declared as const but altering a mutable variable.
         */
        void inc(unsigned int idx) const {
            // VC does not yet contain a record for the right thread -> resize
            if (idx >= size()) {
                for (unsigned int i = size(); i < idx; i++) {
                    T_epoch tmp{i};
                    set(tmp);
                }
                T_epoch tmp{idx, 1};
                set(tmp);
            }
            // no need to resize
            else
                vc[idx].tick();
        }

        /** Access a token at index.
         * @param	idx in Index into the vector (most likely tid)
         * @return	Value of the coresponding epoch.
         * 			Minimal epoch if the VC is too small. (minimal: t@0)
         */
        T_epoch get(unsigned int idx) const {
            if (idx >= size()) {
                T_epoch tmp{idx};
                return tmp;
            } else
                return vc[idx];
        }

        /** Join two VCs -> for each index select the higher value out of both VCs
         *  Can resize both VCs if their sizes differ. (thats why the pointer)
         * @param other in Pointer to a VC to join with.
         * @note	Declared as const but altering a mutable variable.
         */
        void join(const vector_clock &other) const {
            match_size(other);

            for (auto i: other) {
                set(i.max(get(i.get_tid())));
            }
        }

        /** Copy from a VC -> for each index replace the current epoch with the new one
         *  Can resize both VCs if their sizes differ. (thats why the pointer)
         * @param other in Pointer to a VC to copy from.
         * @note	Declared as const but altering a mutable variable.
         */
        void copy(const vector_clock &other) const {
            match_size(other);

            for (auto i: other) {
                set(i);
            }
        }

        /// vector clock happens before relation using epochs
        bool operator<=(const vector_clock<epoch> &other) const {
            for (auto &i: vc) {
                if (!(i <= other.get(i.get_tid())))
                    return false;
            }
            return true;
        }

        /// vector clock happens before relation using xepochs
        bool operator<=(const vector_clock<xepoch> &other) const {
            for (auto i: vc) {
                if (!(i <= other.get(i.get_tid())))
                    return false;
            }
            return true;
        }

        typename std::vector<T_epoch>::iterator begin() const { return vc.begin(); }
        typename std::vector<T_epoch>::iterator end() const { return vc.end(); }
    };

    /// Vector clock extension for tracking threads.
    class thread_state : public vector_clock<epoch> {
    public:
        const THREADID tid; ///< thread id
        mutable epoch E; ///< cached epoch of this thread (inv: E == get(tid))

        explicit thread_state(THREADID t) : vector_clock(t), tid{t}, E{t, 1} {
        };

        bool operator==(const thread_state &t) const { return tid == t.tid; }
        bool operator!=(const thread_state &t) const { return tid != t.tid; }
        bool operator<(const thread_state &t) const { return tid < t.tid; }
    };

    /// Vector clock extension for tracking variables.
    class var_state : public vector_clock<xepoch> {
    public:
        mutable xepoch R; ///< epoch of the last read access to the var
        mutable xepoch W; ///< epoch of the last write access to the var

        const ADDRINT adr; ///< info about the variable

        explicit var_state(ADDRINT a) : R{0, 0}, W{0, 0}, adr{a} {
        };

        bool operator==(const var_state &v) const { return adr == v.adr; }
        bool operator!=(const var_state &v) const { return adr != v.adr; }
        bool operator<(const var_state &v) const { return adr < v.adr; }
    };

    /// Vector clock extension for tracking locks.
    class lock_state : public vector_clock<epoch> {
    public:
        const LOCK lid; ///< lock identifier

        explicit lock_state(LOCK id) : lid{id.index} {
        };
        bool operator==(const lock_state &l) const { return lid == l.lid; }
        bool operator!=(const lock_state &l) const { return lid != l.lid; }
        bool operator<(const lock_state &l) const { return lid < l.lid; }
    };

    /// List of state objects implemented as a set
    template<typename T>
    class state_list {
    private:
        std::set<T> list;

    public:
        /** Insert an item into the sorted list.
         * 	@param 	Item	in	Item to insert.
         *  @return Pointer to the inserted item.
         */
        typename std::set<T>::iterator insert(T item) {
            auto i = list.insert(item);
            return std::get<0>(i);
        }

        /**	@brief Searches for an item in the sorted list.
         * 	@param Item	in	Item to look for.
         * 	@return	Iterator pointing to the item found.
         * 			list.end() if the object was not found.
         */
        typename std::set<T>::iterator find(T item) {
            auto i = list.find(item);
            return i;
        }

        typename std::set<T>::iterator begin() { return list.begin(); }
        typename std::set<T>::iterator end() { return list.end(); }
    };

    state_list<thread_state> Threads; ///< Global vector of all known threads
    state_list<var_state> Variables; ///< Global vector of all known variables
    state_list<lock_state> Locks; ///< Global vector of all known locks

#if DBUG_OUT_DETAILS
		/// for debugging, prints information about all threads
		void print_threads()
		{
			std::cout << "Threads list:" << std::endl;
			for (auto itr : Threads)
			{
				std::cout << "  " << itr.tid << ": E = " << itr.E.get_clk() << ", vc = <";
				for (unsigned int i = 0; i < itr.size(); i++)
					std::cout << itr.get(i).get_clk() << ((i == itr.size() - 1) ? "" : ",");

				std::cout << ">" << endl;
			}
		}
		/// for debugging, prints information about all variables
		void print_variables()
		{
			std::cout << "Variables list:" << std::endl;
			for (auto itr : Variables)
			{
				std::cout << "  " << itr.adr << ": W = " << itr.W.get_clk() << ", R = " << itr.R.get_clk() << ", shared = " << itr.R.shared <<", vc = <";
				for (unsigned int i = 0; i < itr.size(); i++)
					std::cout << itr.get(i).get_clk() << ((i == itr.size() - 1) ? "" : ",");

				std::cout << ">" << endl;
			}
		}
		/// for debugging, prints information about all locks
		void print_locks()
		{
			std::cout << "Locks list:" << std::endl;
			for (auto itr : Locks)
			{
				std::cout << "  " << itr.lid.index << ": <";
				for (unsigned int i = 0; i < itr.size(); i++)
					std::cout << itr.get(i).get_clk() << ((i == itr.size() - 1) ? "" : ",");

				std::cout << ">" << endl;
			}
		}
#endif

    /**	@brief	Read access handeling function. Inserts new vars into the Variables list.
     * 			And performs race detection actions and analysis state updates.
     * 	@param	tid			in	Id of the thread that is accessing.
     * 	@param	addr		in	Address of the variable.
     * 	@param	size		in	Variable size in TODO (have to check).
     * 	@param	variable	in	Object containing extra information about the variable (name, type, offset).
     * 	@param	location	in	File name and line number in the source code.
     */
    void access_read(THREADID tid, ADDRINT addr, UINT32 size, const VARIABLE &variable, const LOCATION &location) {
        mutex_lock(&Threads_mutex);
        mutex_lock(&Variables_mutex);

#if DBUG_OUT_BASIC
			std::cout << "read of " << addr << " by " << tid << ", line " << location.line << std::endl;
#endif

        // find thread
        thread_state tmpt{tid};
        auto thr = Threads.find(tmpt);
        assert(thr != Threads.end() && "ASSERT FAIL: access_read by unknown thread");

        // find or insert the var
        var_state tmpv{addr};
        auto var = Variables.insert(tmpv);
        assert(var != Variables.end() && "ASSERT FAIL: access_read var insert fail");


        // do analysis actions

        xepoch e = thr->E;
        e.set_loc(location);

        xepoch r = var->R;
        if (r == e) ///< [READ SAME EPOCH]
        {
#if DBUG_OUT_BASIC
				std::cout << "read same epoch"<< std::endl;
#endif
            mutex_unlock(&Variables_mutex);
            mutex_unlock(&Threads_mutex);
            return;
        }
        if (r.shared && var->get(thr->tid) == e) ///< [READ SHARED SAME EPOCH]
        {
#if DBUG_OUT_BASIC
				std::cout << "read shared same epoch"<< std::endl;
#endif
            mutex_unlock(&Variables_mutex);
            mutex_unlock(&Threads_mutex);
            return;
        }

        xepoch w = var->W;
        if (!(w <= thr->get(w.get_tid()))) ///< [WRITE-READ RACE]
        {
            report_race(write_read_race, addr, size, variable, tid, location, w.get_tid(), w.get_loc());
        }

        if (!r.shared) {
            if (r <= thr->get(r.get_tid())) {
                var->R = e; ///< [READ EXCLUSIVE]

#if DBUG_OUT_BASIC
					std::cout << "read exclusive"<< std::endl;
#endif
            } else {
                var->set(r);
                var->set(e);
                var->R.shared = true; ///< [READ SHARE]
                var->R = e; // TODO

#if DBUG_OUT_BASIC
					std::cout << "read share (epoch -> VC)"<< std::endl;
#endif
            }
        } else {
            var->set(e); ///< [READ SHARED]
            var->R = e; // TODO

#if DBUG_OUT_BASIC
				std::cout << "read shared"<< std::endl;
#endif
        }

#if DBUG_OUT_DETAILS
			print_threads();
			print_variables();
#endif

        mutex_unlock(&Variables_mutex);
        mutex_unlock(&Threads_mutex);
    }

    /**	@brief	Write access handeling function. Inserts new vars into the Variables list.
     * 			And performs race detection actions and analysis state updates.
     * 	@param	tid			in	Id of the thread that is accessing.
     * 	@param	addr		in	Address of the variable.
     * 	@param	size		in	Variable size in TODO (have to check).
     * 	@param	variable	in	Object containing extra information about the variable (name, type, offset).
     * 	@param	location	in	File name and line number in the source code.
     */
    void access_write(THREADID tid, ADDRINT addr, UINT32 size, const VARIABLE &variable, const LOCATION &location) {
        mutex_lock(&Threads_mutex);
        mutex_lock(&Variables_mutex);

#if DBUG_OUT_BASIC
			std::cout << "write to " << addr << " by " << tid << ", line " << location.line << std::endl;
#endif

        // find thread
        thread_state tmpt{tid};
        auto thr = Threads.find(tmpt);
        assert(thr != Threads.end() && "ASSERT FAIL: access_write by unknown thread");

        // find or insert the var
        var_state tmpv{addr};
        auto var = Variables.insert(tmpv);
        assert(var != Variables.end() && "ASSERT FAIL: access_write var insert fail");


        // do analysis actions

        xepoch e = thr->E;
        e.set_loc(location);


        xepoch w = var->W;
        if (w == e) ///< [WRITE SAME EPOCH]
        {
#if DBUG_OUT_BASIC
				std::cout << "write same epoch"<< std::endl;
#endif
            mutex_unlock(&Variables_mutex);
            mutex_unlock(&Threads_mutex);
            return;
        }

        if (!(w <= (thr->get(w.get_tid())))) ///< [WRITE-WRITE RACE]
        {
            report_race(write_write_race, addr, size, variable, tid, location, w.get_tid(), w.get_loc());
        }

        xepoch r = var->R;
        if (!r.shared) {
            if (!(r <= thr->get(r.get_tid()))) ///< [READ-WRITE RACE]
            {
                report_race(read_write_race, addr, size, variable, tid, location, r.get_tid(), r.get_loc());
            }
            var->W = e;
        } else {
            if (!(*var <= *thr)) ///< [SHARED-WRITE RACE]
            {
                report_race(shared_write_race, addr, size, variable, tid, location, r.get_tid(), r.get_loc()); // TODO
            }

            var->W = e; ///< [WRITE SHARED]

#if DBUG_OUT_BASIC
				std::cout << "write shared (VC -> epoch)"<< std::endl;
#endif
        }

#if DBUG_OUT_DETAILS
			print_threads();
			print_variables();
#endif

        mutex_unlock(&Variables_mutex);
        mutex_unlock(&Threads_mutex);
    }

    /**	@brief	Thread fork handeling function. Inserts new threads into the Threads list and updates analysis state.
     * 	@param	tid 	in	Id of the forking thread (parent)
     *	@param	ntid	in	Id of the forked thread (child)
     */
    void thread_fork(THREADID tid, THREADID ntid) {
        mutex_lock(&Threads_mutex);

#if DBUG_OUT_BASIC
			std::cout << "thread_fork start" << std::endl;
#endif

        // find the forking thread
        thread_state tmp{tid};
        auto forker = Threads.find(tmp);
        assert(forker != Threads.end() && "ASSERT FAIL: thread_fork by unknown thread");

        // insert the forked thread
        thread_state thr{ntid};
        auto forkee = Threads.insert(thr);
        assert(forkee != Threads.end() && "ASSERT FAIL: thread_fork thr insert fail");

        // update vector clocks
        forkee->join(*forker);
        forker->inc(forker->tid);
        forker->E = forker->get(forker->tid);

#if DBUG_OUT_BASIC
			std::cout << "thread " << ntid << " forked from " << tid << std::endl;
#endif
#if DBUG_OUT_DETAILS
			print_threads();
#endif

        mutex_unlock(&Threads_mutex);
    }

    /**	@brief	Thread join handeling function. Updates analysis state.
     * 	@param	tid 	in	Id of the waiting thread (parent)
     *	@param	jtid	in	Id of the joined thread (child)
     */
    void thread_join(THREADID tid, THREADID jtid) {
        mutex_lock(&Threads_mutex);

#if DBUG_OUT_BASIC
			std::cout << "thread_join start" << std::endl;
#endif

        // find both threads
        thread_state tmp{tid};
        thread_state tmp2{jtid};
        auto joiner = Threads.find(tmp);
        auto joinee = Threads.find(tmp2);

        assert(joiner != Threads.end() && "ASSERT FAIL: thread_join by unknown thread");
        assert(joinee != Threads.end() && "ASSERT FAIL: thread_join of unknown thread");

        // update VC
        joiner->join(*joinee);

#if DBUG_OUT_BASIC
			std::cout << "thread " << jtid << " joined into" << tid << std::endl;
#endif
#if DBUG_OUT_DETAILS
			print_threads();
#endif

        mutex_unlock(&Threads_mutex);
    }

    /**	@brief	Lock acquire handeling function. Inserts new locks into the Locks list and updates analysis state.
     *	@param	tid		in	Unique id of the thread that acquired the lock.
     *	@param	lock	in	An object representing the lock that has been acquired.
     */
    void lock_acquire(THREADID tid, LOCK lock) {
        mutex_lock(&Threads_mutex);
        mutex_lock(&Locks_mutex);

#if DBUG_OUT_BASIC
			std::cout << "lock_acquire start" << std::endl;
#endif

        // find the thread
        thread_state tmpt{tid};
        auto thr = Threads.find(tmpt);
        assert(thr != Threads.end() && "ASSERT FAIL: lock_acquire by unknown thread");

        // find or insert the lock
        lock_state tmpl{lock};
        auto lck = Locks.insert(tmpl);
        assert(lck != Locks.end() && "ASSERT FAIL: lock_acquire lock insert fail");

        // update VC
        thr->join(*lck);

#if DBUG_OUT_BASIC
			std::cout << "lock " << lock.index << " acquired by " << tid << std::endl;
#endif
#if DBUG_OUT_DETAILS
			print_threads();
			print_locks();
#endif

        mutex_unlock(&Locks_mutex);
        mutex_unlock(&Threads_mutex);
    }

    /**	@brief	Lock release handeling function. Updates analysis state.
     *	@param	tid		in	Unique id of the thread that released the lock.
     *	@param	lock	in	An object representing the lock that has been released.
     */
    void lock_release(THREADID tid, LOCK lock) {
        mutex_lock(&Threads_mutex);
        mutex_lock(&Locks_mutex);

#if DBUG_OUT_BASIC
			std::cout << "lock_release start" << std::endl;
#endif

        // find the thread
        thread_state tmpt{tid};
        auto thr = Threads.find(tmpt);
        assert(thr != Threads.end() && "ASSERT FAIL: lock_release by unknown thread");

        // find the lock
        lock_state tmpl{lock};
        auto lck = Locks.find(tmpl);
        assert(thr != Threads.end() && "ASSERT FAIL: lock_release of unknown lock");

        // update VC
        lck->copy(*thr);
        thr->inc(thr->tid);
        thr->E = thr->get(thr->tid);

#if DBUG_OUT_BASIC
			std::cout << "lock " << lock.index << " released by " << tid << std::endl;
#endif
#if DBUG_OUT_DETAILS
			print_threads();
			print_locks();
#endif

        mutex_unlock(&Locks_mutex);
        mutex_unlock(&Threads_mutex);
    }
};

/**	@brief Initializes the analyzer plug-in. Called before the tested program starts.
 */
void fasttrack_init() {
    // insert thread zero
    thread_state t{0};
    Threads.insert(t);

    //	Initialize mutexes
    int err;
    if ((err = pthread_mutex_init(&Threads_mutex, NULL)) != 0) {
        fprintf(stderr, "pthread_mutex_init() error %d\n", err);
        exit(EXIT_FAILURE);
    }
    if ((err = pthread_mutex_init(&Variables_mutex, NULL)) != 0) {
        fprintf(stderr, "pthread_mutex_init() error %d\n", err);
        exit(EXIT_FAILURE);
    }
    if ((err = pthread_mutex_init(&Locks_mutex, NULL)) != 0) {
        fprintf(stderr, "pthread_mutex_init() error %d\n", err);
        exit(EXIT_FAILURE);
    }

    // register callback functions
    ACCESS_BeforeMemoryRead(::access_read);
    ACCESS_BeforeMemoryWrite(::access_write);

    THREAD_ThreadForked(::thread_fork);
    SYNC_AfterJoin(::thread_join);

    SYNC_AfterLockAcquire(::lock_acquire);
    SYNC_BeforeLockRelease(::lock_release);
}

/**	@brief Terminates the analyzer plug-in. Called after the tested program ends.
 */
void fasttrack_terminate() {
#if DBUG_OUT_DETAILS
		print_threads();
		print_locks();
		print_variables();
#endif

    //	Destroy mutexes
    int err;
    if ((err = pthread_mutex_destroy(&Threads_mutex)) != 0) {
        fprintf(stderr, "pthread_mutex_destroy() error %d\n", err);
        exit(EXIT_FAILURE);
    }
    if ((err = pthread_mutex_destroy(&Variables_mutex)) != 0) {
        fprintf(stderr, "pthread_mutex_destroy() error %d\n", err);
        exit(EXIT_FAILURE);
    }
    if ((err = pthread_mutex_destroy(&Locks_mutex)) != 0) {
        fprintf(stderr, "pthread_mutex_destroy() error %d\n", err);
        exit(EXIT_FAILURE);
    }
}

/** End of file fasttrack2.cpp **/
