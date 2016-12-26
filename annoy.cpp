/**
 * Author: Joshua Inscoe
 * Last modified: November 17, 2016
 *
 * File: annoy.cpp
 */



#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>


#define DEBUG               0
#define DEBUG_WITH_SIGNALS  0


#define RANDN_R(min,max) \
    ((min) + (((double)rand() / RAND_MAX) * ((max) + 1 - (min))))

#define SECONDS_IN_HOUR     3600

#define MIN_SECOND  0
#define MAX_SECOND  (SECONDS_IN_HOUR - 1)


// Don't look at me, I'm ugly!
#define EXPAND_(e) #e
#define CONSTRUCT_LOCKV_CMD(v) "osascript -e 'set Volume " EXPAND_(v) "'"
#define LOCK_VOLUME_COMMAND CONSTRUCT_LOCKV_CMD(VOLUME)
//


#define CHILD_USLEEP_TIME   100
#define PARENT_USLEEP_TIME  100


// Configurable
#define ANNOY_COMMAND   "say 'Annoy, annoy, annoy!'"

#define DURATION    300     // in seconds
#define VOLUME      7       // 0 - 7
//



namespace // anonymous
{

#if DEBUG
// Program log file (for debugging)
std::ofstream _log_file (".annoy.log");
#endif

const std::string _lock_volume_command (LOCK_VOLUME_COMMAND);
const std::string _annoy_command (ANNOY_COMMAND);

time_t _stime = 0;
time_t _etime = 0;

} // anonymous namespace


// Prototypes
size_t ignore_all_signals();

void run_double_process();
void run_single_process();
void generate_exec_interval(time_t t, time_t* start, time_t* end);
time_t next_hour_start(time_t t);

void execute_payload();
//


int main(int argc, const char *argv[])
{
#if DEBUG
    if (argc != 2 && argc != 3)
    {
        std::cerr << "usage: " << argv[0];
        std::cerr << " start-time [duration]" << std::endl;
        exit(1);
    }

    int start = atoi(argv[1]);
    if (start < MIN_SECOND || start > MAX_SECOND)
    {
        std::cerr << "error: Invalid `start-time' ";
        std::cerr << "(" << argv[1] << ")" << std::endl;
        exit(2);
    }

    int duration;
    if (argc == 3)
    {
        /* Use duration time specified by user */
        duration = atoi(argv[2]);
        if (duration > MAX_SECOND)
        {
            std::cerr << "error: Invalid `duration' ";
            std::cerr << "(" << argv[2] << ")" << std::endl;
            exit(2);
        }
    }
    else
    {
        /* Use default duration time */
        duration = DURATION;
    }

    time_t t = time(NULL);

    /*
     * We want `this_hour' to calculate the start time of the
     * current hour, rather than the next, so we subtract an hour
     * from the current time
     */
    t -= SECONDS_IN_HOUR;
    time_t this_hour = next_hour_start(t);

    _stime = this_hour + start;
    _etime = _stime + duration;
#endif

#if DEBUG && DEBUG_WITH_SIGNALS
    // nothing
#else
    /* Ignore all signals possible */
    ignore_all_signals();
#endif

#if DEBUG
    // nothing
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);

    srand(tv.tv_usec);
#endif

    /*
     * Run two processes, both of which will monitor the execution of the
     * other, and one of which will execute the annoy script
     */
    run_double_process();

    // unreachable
    return 0;
}


size_t ignore_all_signals()
{
    static const int _signals_list[] =
    {
        SIGHUP,
        SIGINT,
        SIGQUIT,
        SIGILL,
        SIGTRAP,
        SIGABRT,
        SIGEMT,
        SIGFPE,
        //SIGKILL,  // <-- cannot be ignored
        SIGBUS,
        SIGSEGV,
        SIGSYS,
        SIGPIPE,
        SIGALRM,
        SIGTERM,
        SIGURG,
        //SIGSTOP,  // <-- cannot be ignored
        SIGCONT,
        //SIGCHLD,  // Signal parent process when child dies
        SIGTTIN,
        SIGTTOU,
        SIGIO,
        SIGXCPU,
        SIGXFSZ,
        SIGVTALRM,
        SIGPROF,
        SIGWINCH,
        SIGINFO,
        SIGUSR1,
        SIGUSR2
    };

    size_t count = 0;
    size_t n;
    
    n = sizeof(_signals_list) / sizeof(int);

    for (size_t i = 0; i < n; ++i)
    {
        // Ignore each signal in the list above
        sig_t sig = signal(_signals_list[i], SIG_IGN);
        if (sig == SIG_ERR)
        {
#if DEBUG
            _log_file << "warning: Failed to ignore signal ";
            _log_file << "(sig=" << _signals_list[i] << ")" << std::endl;
#endif
        }
        else
        {
            ++count;
        }
    }

    // Number of signals successfully ignored
    return count;
}


void run_double_process()
{
    while (true)
    {
        pid_t ppid = getpid();

        /* Spawn or re-spawn second process */
        pid_t pid = fork();

        if (pid == -1)
        {
#if DEBUG
            _log_file << "warning: Failed to spawn second process: ";
            _log_file << strerror(errno) << std::endl;
            _log_file << "info: Continuing program with single process";
            _log_file << std::endl;
#endif

            run_single_process();

            // unreachable
        }

        if (pid == 0)
        {
            /*
             * Detach the child process from the current session, so that
             * sending a KILL signal to the parent's process group will not
             * stop the program
             */
            if (setsid() == -1)
            {
#if DEBUG
                _log_file << "warning: Failed to create new session: ";
                _log_file << strerror(errno) << std::endl;
                _log_file << "info: Continuing program in original session";
                _log_file << std::endl;
#endif
            }

            while (getppid() == ppid)
            {
                /* Put monitoring process to sleep */
                usleep(CHILD_USLEEP_TIME);
            }
        }
        else
        {
            int stat_loc;
            while (waitpid(pid, &stat_loc, WNOHANG | WUNTRACED) != pid)
            {
                time_t t = time(NULL);

                if (t < _stime)
                {
                    /* Allow parent to sleep a bit */
                    usleep(PARENT_USLEEP_TIME);
                    continue;
                }

                if (t < _etime)
                {
                    execute_payload();
                }
                else
                {
#if DEBUG
                    _stime += SECONDS_IN_HOUR;
                    _etime += SECONDS_IN_HOUR;
#else
                    /*
                     * After each full execution of the payload, change the
                     * start time to another random second of the hour
                     */
                    generate_exec_interval(t, &_stime, &_etime);
#endif
                }
            }

            /*
             * Kill child process if it has been stopped (i.e. via a
             * signal of type SIGSTOP, which cannot be ignored)
             */
            if (WIFSTOPPED(stat_loc))
            {
                kill(pid, SIGKILL);
            }
        }
    }

    // unreachable
}

void run_single_process()
{
    while (true)
    {
        time_t t = time(NULL);

        if (t < _stime)
        {
            continue;
        }

        if (t < _etime)
        {
            execute_payload();
        }
        else
        {
#if DEBUG
            _stime += SECONDS_IN_HOUR;
            _etime += SECONDS_IN_HOUR;
#else
            /*
             * After each full execution of the payload, change the start
             * time to another random second of the hour
             */
            generate_exec_interval(t, &_stime, &_etime);
#endif

            /* Sleep until one second before the next payload execution */
            sleep(_stime - 1 - t);
        }
    }

    // unreachable
}

void generate_exec_interval(time_t t, time_t* start, time_t* end)
{
    time_t next_hour = next_hour_start(t);

    /* Generate random execution time interval for the next hour */
    *start = next_hour + RANDN_R(MIN_SECOND, MAX_SECOND);
    *end = *start + DURATION;

    return;
}

time_t next_hour_start(time_t t)
{
    t += SECONDS_IN_HOUR;

    struct tm* ltime = localtime(&t);
    ltime->tm_min = 0;
    ltime->tm_sec = 0;

    /* Returns time, HH:00:00, where 'HH' is the next hour */
    return mktime(ltime);
}


void execute_payload()
{
    /*
     * Override the computer's speaker volume, and execute the
     * `annoy' command once
     */
    system(_lock_volume_command.c_str());
    system(_annoy_command.c_str());

    return;
}
