// Copyright 2012 Matthew Nourse and n plus 1 computing pty limited unless otherwise noted.
// Please see LICENSE file for details.
#ifndef NP1_PROCESS_HPP
#define NP1_PROCESS_HPP

#ifndef _WIN32
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#endif

#include <list>
#include "np1/io/log.hpp"
#include "np1/skip_list.hpp"

namespace np1 {
namespace process {

// Low-level process creation helpers.


// Kill all children and ourselves.
void kill_all() {
  sigset_t new_mask;
  sigset_t old_mask;
  sigfillset(&new_mask);
  sigprocmask(SIG_BLOCK, &new_mask, &old_mask);
  // This also kills the current process.
  kill(0, SIGTERM);
  sigprocmask(SIG_SETMASK, &old_mask, 0);
}


void mandatory_pipe_create(int *p) { NP1_ASSERT(pipe(p) == 0, "pipe() failed"); }

// Kills all on failure.
pid_t mandatory_fork() {

  pid_t pid = fork();
  if (-1 == pid) {
    np1::assert::write_pre_crash_message("fork() != -1", "fork() failed");
    kill_all();  
  }

  // Trash the rand64 state to ensure that 2 processes don't generate the same random numbers.
  math::rand64_state::trash();

  return pid;
}




// Kills all on failure.  If hang is true then will wait for the child to exit,
// otherwise will return false immediately if the child is still running.
bool mandatory_wait_for_child(pid_t child_pid, bool hang = true) {
  int status;
  pid_t pid = ::waitpid(child_pid, &status, hang ? 0 : WNOHANG);
  if (-1 == pid) {
    np1::assert::write_pre_crash_message("pid != -1", "Unable to wait for child process");
    process::kill_all();
  }

  if (0 == pid) {
    NP1_ASSERT(!hang, "waitpid returned 0 but we asked it to hang.");
    return false;
  }

  if (!WIFEXITED(status) || (WEXITSTATUS(status) != 0)) {
    std::string detail = WIFEXITED(status) ? str::to_dec_str(WEXITSTATUS(status)) : "Abnormal termination";
    np1::assert::write_pre_crash_message(
          "WIFEXITED(status) && (WEXITSTATUS(status) == 0)",
          ("Terminating- error in child process " + str::to_dec_str(child_pid) + ": " + detail).c_str());
    process::kill_all();
  }

  return true;
}

bool mandatory_try_wait_for_child(pid_t child_pid) {
  return mandatory_wait_for_child(child_pid, false);
}

template <typename Iterator>
void mandatory_wait_for_children(Iterator b, Iterator e) {
  for (; b != e; ++b) {
    mandatory_wait_for_child(*b);
  }
}

template <typename Container>
void mandatory_wait_for_children(Container c) {
  mandatory_wait_for_children(c.begin(), c.end());
}


// Exec- start a program, overwriting this process's address space.
// args[0] is the name of the program.
void mandatory_execvp(const std::vector<std::string> &args, int stdin_fd, int stdout_fd, int stderr_fd) {
  NP1_ASSERT(args.size() > 0, "mandatory_execvp on empty args");
  
  // Get the arguments into the right format.
  char *argv[args.size() + 1];
  std::vector<std::string>::const_iterator i = args.begin();
  std::vector<std::string>::const_iterator iz = args.end();
  char **argv_p = argv;
  for (; i < iz; ++i, ++argv_p) {
    *argv_p = (char *)i->c_str();
  }

  *argv_p = NULL;

  // Sort out file descriptors.  TODO: should we close all other file descriptors?
  dup2(stdin_fd, 0);
  dup2(stdout_fd, 1);
  dup2(stderr_fd, 2);

  int result = execvp(argv[0], argv);
  int error = errno;
  NP1_ASSERT(
    result != -1,
    "execvp failed for " + args[0] + "  Error: (" + str::to_dec_str(error) + ")");
}



// Class to help get stuff done concurrently.  A process is forked for each job, it's not literally a pool of long-lived
// processes.  This class uses a very basic control system algorithm to set the maximum number of processes allowed
// in a pool.  It varies the pool size dynamically.
template <typename On_Exit>
class pool {
public:
  enum { YIELD_USEC = 10000 };

private:
  struct process_info {
    process_info(pid_t pid, const On_Exit &on_exit)
      : m_pid(pid), m_on_exit(on_exit), m_start_time(time::now_epoch_usec()) {}
    pid_t m_pid;
    On_Exit m_on_exit;
    uint64_t m_start_time;
  };

public:
  explicit pool(size_t initial_max_size)
    : m_max_size(initial_max_size),
      m_prev_max_size(initial_max_size),
      m_prev_throughput(0.0),
      m_name("[unnamed]"){
    NP1_ASSERT(initial_max_size > 0, "Invalid process pool max size");
  }

  ~pool() { wait_all(); }

  // Add a job to the "pool", waiting for space if necessary.
  template <typename F>
  void add(F f, On_Exit on_exit) {
    wait_for_processes(m_max_size - 1);
    pid_t child_pid = mandatory_fork();
    if (0 == child_pid) {
      f();
      exit(0);
    }

    m_process_infos.push_back(process_info(child_pid, on_exit));
  }


  // Returns true if a process has exited.
  bool has_process_exited() {
    if (m_process_infos.size() <= 0) {
      return false;
    }
  
    typename std::list<process_info>::iterator i = m_process_infos.begin();
    typename std::list<process_info>::iterator iz = m_process_infos.end();
    for (; i != iz; ++i) {
      if (mandatory_try_wait_for_child(i->m_pid)) {
        update_process_times(*i);
        calculate_new_max_size();
        i->m_on_exit();
        m_process_infos.erase(i);
        return true;
      }
    }

    return false;
  }

  // Wait for the remainder of the jobs to finish.
  void wait_all() { wait_for_processes(0); }

  size_t size() const { return m_process_infos.size(); }
  size_t max_size() const { return m_max_size; }
  const std::string &name() const { return m_name; }
  void name(const std::string &n) { m_name = n; }
  

private:
  /// Disable copy.
  pool(const pool &);
  pool &operator = (const pool &);
  
private:
  void wait_for_processes(size_t max_remaining_child_processes) {
    while (m_process_infos.size() > max_remaining_child_processes) {
      if (!has_process_exited()) {
        ::usleep(YIELD_USEC);      
      }
    }
  }

  void calculate_new_max_size() {
    if (!have_enough_process_start_times_for_throughput_calculation()) {
      return;
    }

    // Paranoia- m_max_size should never be 0.
    if (m_max_size <= 0) {
      ++m_max_size;
    }

    size_t old_max_size = m_max_size;

    double throughput = calculate_throughput();
    if (m_max_size >= m_prev_max_size) {
      if (throughput >= m_prev_throughput) {
        io::log::info(log_id(), "name=", m_name.c_str(), " throughput=", throughput, " prev_throughput=",
                      m_prev_throughput, " max_size=", m_max_size, " prev_max_size=", m_prev_max_size,
                      " new m_max_size=", m_max_size+1,
                      " m_max_size >= m_prev_max_size, throughput is up, increasing max size");
        ++m_max_size;
      } else if (throughput < m_prev_throughput) {
        io::log::info(log_id(), "name=", m_name.c_str(), " throughput=", throughput, " prev_throughput=",
              m_prev_throughput, " max_size=", m_max_size, " prev_max_size=", m_prev_max_size,
              " new m_max_size=", m_max_size-1,
              " m_max_size >= m_prev_max_size, throughput is down, decreasing max size");
        --m_max_size;
      }
    } else {
      if (throughput >= m_prev_throughput) {
        io::log::info(log_id(), "name=", m_name.c_str(), " throughput=", throughput, " prev_throughput=",
              m_prev_throughput, " max_size=", m_max_size, " prev_max_size=", m_prev_max_size,
              " new m_max_size=", m_max_size-1,
              " m_max_size < m_prev_max_size, throughput is up, decreasing max size");
        --m_max_size;
      } else if (throughput < m_prev_throughput) {
        io::log::info(log_id(), "name=", m_name.c_str(), " throughput=", throughput, " prev_throughput=",
              m_prev_throughput, " max_size=", m_max_size, " prev_max_size=", m_prev_max_size,
              " new m_max_size=", m_max_size+1,
              " m_max_size < m_prev_max_size, throughput is down, increasing max size");
        ++m_max_size;
      }
    }

    // Ensure that we're always allowed at least 1 process.
    if (m_max_size <= 0) {
      m_max_size = 1;
    }

    m_prev_max_size = old_max_size;
    m_prev_throughput = throughput;
    m_recent_process_start_times.clear();
  }


  void update_process_times(const process_info &last_finished_process) {
    m_recent_process_start_times.push_back(last_finished_process.m_start_time);
  }

  double calculate_throughput() {
    size_t count = m_recent_process_start_times.size();
    if (count <= 0) {
      return 0;
    }

    uint64_t elapsed_time = time::now_epoch_usec() - m_recent_process_start_times.front();

    if (elapsed_time <= 0) {
      return 1.0;
    }

    // Elapsed time is in microseconds and count is likely to be quite small
    // so make sure the ratio is well clear of the low-end of double's resolution.
    return (double)(count * 100000)/(double)elapsed_time;
  }

  bool have_enough_process_start_times_for_throughput_calculation() const {
    return (m_recent_process_start_times.size()
              >= get_required_number_process_start_times_for_throughput_calculation());
  }

  size_t get_required_number_process_start_times_for_throughput_calculation() const {
    return (m_max_size * 2) + 1;  
  }

  static const char *log_id() { return "process_pool"; }

private:
  size_t m_max_size;
  size_t m_prev_max_size;
  double m_prev_throughput;
  std::list<process_info> m_process_infos;
  std::vector<uint64_t> m_recent_process_start_times;
  std::string m_name;
};




// A pool that queues up jobs when full.
template <typename F, typename On_Exit>
class queued_pool {
public:
  explicit queued_pool(size_t max_pool_size) : m_pool(max_pool_size) {}
  ~queued_pool() { wait_all(); }

  void add(const F &f, const On_Exit &on_exit) {
    if (m_pool.size() >= m_pool.max_size()) {
      m_queue.push_back(std::make_pair(f, on_exit));
    } else {
      m_pool.add(f, on_exit);
    }
  }

  // Returns true if a process has exited, false if the pool is empty or no process has exited.
  bool make_progress() {
    if (size() <= 0) {
      return false;
    }

    while (!m_queue.empty() && (m_pool.size() < m_pool.max_size())) {
      deque_one_to_pool();
    }
    
    return m_pool.has_process_exited();
  }

  size_t size() const { return m_pool.size() + m_queue.size(); }
  void name(const std::string &n) { m_pool.name(n); }

private:
  void deque_one_to_pool() {
    m_pool.add(m_queue.front().first, m_queue.front().second);
    m_queue.pop_front();
  }

  void wait_all() {
    while(m_queue.size() > 0) {
      deque_one_to_pool();
    }

    m_pool.wait_all();
  }

private:
  /// Disable copy.
  queued_pool(const queued_pool &);
  queued_pool &operator = (const queued_pool &);

private:
  pool<On_Exit> m_pool;
  std::list<std::pair<F, On_Exit> > m_queue;
};


// A map of queued pools.
template <typename F, typename On_Exit>
class queued_pool_map {
public:
  enum { DEFAULT_INITIAL_MAX_POOL_SIZE = 2 };

public:
  queued_pool_map() {}
  ~queued_pool_map() {
    wait_all();
    typename skip_list<entry>::iterator i = m_entries.begin();
    typename skip_list<entry>::iterator iz = m_entries.end();
    for (; i != iz; ++i) {
      std::detail::mem::destruct_and_free(i->m_pool_p);
    }
  }

  void add(const std::string &key, const F &f, const On_Exit &on_exit) {
    typename skip_list<entry>::iterator iter = m_entries.find(entry(key));
    if (iter == m_entries.end()) {
      NP1_ASSERT(add_queued_pool(key), "Queued pool " + key + " not found and can't add it either");
      iter = m_entries.find(entry(key));
      NP1_ASSERT(iter != m_entries.end(), "Queued pool " + key + " not found immediately after adding it!");
    }

    iter->m_pool_p->add(f, on_exit);
  }
  
  void wait_all() {
    bool found_one_non_empty_pool = true;

    while (found_one_non_empty_pool) {
      found_one_non_empty_pool = false;
      bool one_process_has_exited = false;

      typename skip_list<entry>::iterator i = m_entries.begin();
      typename skip_list<entry>::iterator iz = m_entries.end();
      for (; i != iz; ++i) {
        if (i->m_pool_p->size() > 0) {
          found_one_non_empty_pool = true;
          if (i->m_pool_p->make_progress()) {
            one_process_has_exited = true;
          }
        }                
      }

      if (one_process_has_exited) {
        ::usleep(pool<On_Exit>::YIELD_USEC);
      }
    }
  }

private:
  /// Disable copy.
  queued_pool_map(const queued_pool_map &);
  queued_pool_map &operator = (const queued_pool_map &);

private:
  bool add_queued_pool(const std::string &key) {
    queued_pool<F, On_Exit> *pool_p =
      std::detail::mem::alloc_construct<queued_pool<F, On_Exit> >((size_t)DEFAULT_INITIAL_MAX_POOL_SIZE);
    pool_p->name(key);
    if (!m_entries.insert(entry(key, pool_p))) {
      std::detail::mem::destruct_and_free(pool_p);
      return false;
    }

    return true;
  }


private:
  struct entry {
    entry() : m_pool_p(0) {}
    explicit entry(const std::string &k) : m_key(k), m_pool_p(0) {}
    entry(const std::string &k, queued_pool<F, On_Exit> *pool_p) : m_key(k), m_pool_p(pool_p) {}

    bool operator < (const entry &other) const { return m_key < other.m_key; }

    std::string m_key;
    queued_pool<F, On_Exit> *m_pool_p;
  };

  skip_list<entry> m_entries;  
};


} // namespaces
}


#endif
