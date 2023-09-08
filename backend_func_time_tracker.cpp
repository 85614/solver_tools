#include <iostream>
#include <stdio.h>
#include <vector>
#include <map>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <unistd.h>
#include <thread>
#include <sstream>

using func_time_t = long;

extern "C"
{
    void backend_push__(const char *func);
    void backend_pop__();

    void backend_push_with_nesc__(const char *func, func_time_t t);
    void backend_pop_with_nesc__(func_time_t t);
}

static long backend_real_time_nsec()
{
    timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    constexpr long nano = (long)1e9;
    return t.tv_sec * nano + t.tv_nsec;
}
struct backend_func_time_tracker_t
{
    struct func_record_t
    {
        int n;

        func_time_t t_local_sum;
        func_time_t t_local_max;
        func_time_t t_dur_sum;
        func_time_t t_dur_max;

        void count(func_time_t t_duration, func_time_t t_subroutine)
        {
            ++n;

            func_time_t t_local = t_duration - t_subroutine;

            t_local_max = std::max(t_local_max, t_local);
            t_local_sum += t_local;

            t_dur_max = std::max(t_dur_max, t_duration);
            t_dur_sum += t_duration;
        }

        void print(std::ostream &os)
        {
#define OS_TIME(t) ", \"" #t "\": " << ((t)*1e-9)
            os << "\"n\": " << n;
            os << OS_TIME(t_dur_sum);
            // if (t_dur_sum != t_local_sum)
            os << OS_TIME(t_local_sum);
            os << OS_TIME(t_dur_max);
            // if (t_dur_max != t_local_max)
            os << OS_TIME(t_local_max);
            os << "";
#undef OS_TIME
        }
    };

    struct func_state_t
    {
        std::string path;
        func_time_t t_start;
        func_time_t t_subroutine;
    };

    std::map<std::string, func_record_t> func_t_map; // 计时map
    std::vector<func_state_t> func_stack{{""}};      // 调用路径栈
    func_time_t t_timer{0};                          // 累计计时开销

    static backend_func_time_tracker_t &instance()
    {
        // 因为使用了MPI，全局静态变量会有问题
        // 延后初始化的时间，在第一次使用时才初始化
        // 也应此，第一次使用必须在MPI初始化之后
        // thread_local 表示线程独有
        // 多线程使用没问题，但是只能追踪自己线程的计时开销
        thread_local static backend_func_time_tracker_t _instance;
        return _instance;
    }

    ~backend_func_time_tracker_t()
    {
        backend_print_func_time();
    }

    // push 并处理计时开销
    void backend_push_cpp(const char *func, func_time_t t)
    {
        _backend_push_cpp(func, t - t_timer);
        t_timer += backend_real_time_nsec() - t;
    }

    // pop 并处理计时开销
    void backend_pop_cpp(func_time_t t)
    {
        _backend_pop_cpp(t - t_timer);
        t_timer += backend_real_time_nsec() - t;
    }

    // _push
    void _backend_push_cpp(const char *func, func_time_t t)
    {
        func_stack.emplace_back(func_state_t{func_stack.back().path + "/" + func, t, 0});
        // t 为开始时间
    }

    // _pop
    void _backend_pop_cpp(func_time_t t)
    {
        auto &node = func_stack.back();
        auto &parent = func_stack.rbegin()[1];
        // t - 开始时间 - 调用其他 SUBROUTINE 的时间
        func_time_t t_dur = t - node.t_start;
        parent.t_subroutine += t_dur; // 父节点

        func_t_map[node.path].count(t_dur, node.t_subroutine);

        func_stack.pop_back();
    }

    // print
    void backend_print_func_time()
    {
        std::string t_id = static_cast<std::ostringstream &>(std::ostringstream{} << std::this_thread::get_id()).str();
        std::ofstream os("func_time_" + std::to_string(getpid()) + "_" + t_id + ".log");
        os << "# t_timer = " << t_timer * 1e-9 << "s\n";
        os << "func_time_raw_data = [\n";
        auto delimiter = "";
        for (auto it = func_t_map.begin(); it != func_t_map.end(); ++it)
        {
            os << delimiter << "{ ";
            os << "\"path\": \"" << it->first << "\", ";
            it->second.print(os);
            os << " }";
            delimiter = ",\n";
        }
        os << "\n]" << std::endl;
    }
};

void backend_push_with_nesc__(const char *func, func_time_t t)
{
    backend_func_time_tracker_t::instance().backend_push_cpp(func, t);
}
void backend_pop_with_nesc__(func_time_t t)
{
    backend_func_time_tracker_t::instance().backend_pop_cpp(t);
}

void backend_push__(const char *func)
{
    backend_push_with_nesc__(func, backend_real_time_nsec());
}
void backend_pop__()
{
    backend_pop_with_nesc__(backend_real_time_nsec());
}
