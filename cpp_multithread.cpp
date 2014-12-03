#include <vector>
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <future>
#include <stdexcept>

using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::lock_guard;
using std::runtime_error;
namespace chrono = std::chrono;
namespace this_thread = std::this_thread;

// 用std::thread显式实现线程

// 用回调函数构造线程
//     注意std::thread的构造函数如何把不确定参数传入回调函数
//     这个是C++11的新特性。 下面是std::thread构造函数的原型
//
//        template< class Function, class... Args >
//        explicit thread::thread( Function&& f, Args&&... args );
//
//     注意第一个参数也是模板类型，所以可以传入函数指针，
//     std::function对象，lambda和各种重载了()算符的类

// 下面的函数实现一个线程的计算，将会被作为回调函数用来创建线程
void fun (int a, int b) {
    cout << "th " << this_thread::get_id() << ": " << a + b << endl;
    // 延时
    cout << "wait 1 second." << endl;
    this_thread::sleep_for(chrono::seconds(1));
    cout << "wait 10 milliseconds." << endl;
    this_thread::sleep_for(chrono::milliseconds(10));

    // 另一种延时
    auto now = chrono::system_clock::now();
    cout << "wait another 10 milliseconds." << endl;
    this_thread::sleep_until(now + chrono::milliseconds(10));
}

void fun_sync (int a, int b);

void demo_thread () {
//      注意A：线程对象在析构之前一定要调用join或者detach
//      否则会出错
    {
        std::thread th(fun, 1, 2);
        // 等待线程执行完成
        th.join();
    }
    {
        std::thread th(fun, 1, 2);
        // 脱离与线程的关系，放任它自己完成
        th.detach();
        // detach以后就无法通过th对象对线程进行操作了
    }
    {
        // 下面的程序会出错
        /*
        std::thread th(fun, 1, 2);
        */
    }
// 用lambda启动线程
    {
        std::thread th(  [](int a, int b) {
                                cout << "thread with lambda " << this_thread::get_id() << ": " << a + b << endl;
                          }, 1, 2);
        th.join();
    }
// std::thread和=操作
// 一个thread对象有两种状态：
//  1. 和一个线程关联
//  2. 不和任何线程关联
// 上面说的注意A只对处于状态1的线程对象有关
//
// 如果够造thread对象时不传入参数，那么对象就处于状态2
// 可以直接被析枸
//
// std::thread 用 = 进行赋值的时候实现的是move而不是copy
// 也就是说：
//      A = B;
// 如果B和一个线程关联，那么操作完成后
//  - 关联线程转移到A
//  - B不再和原有的线程关联
//
// 思考：如果=之前A已经和某线程关联了怎么办？
//       我不也不知道答案: )
//       最好的办法就是保证=以前线程对象是空的，或者干脆
//       不要用=操作，除非...
    {
        // 用vector管理线程
        vector<std::thread> threads(10);
        // 起10个线程
        for (unsigned i = 0; i < threads.size(); ++i) {
            threads[i] = std::thread(fun, i, i);  // 注意move操作
        }
        // 等待他们全部完成
        for (auto &th: threads) {
            th.join();
        }
    }

// 上面打印出来的东西因为没有同步，有时会有混乱
// 所以线程内如果要往cout/cerr输出东西，需要考虑进行同步
// 用fun_sync再做一遍
    {
        // 用vector管理线程
        vector<std::thread> threads(10);
        // 起10个线程
        for (unsigned i = 0; i < threads.size(); ++i) {
            threads[i] = std::thread(fun_sync, i, i);  // 注意move操作
        }
        // 等待他们全部完成
        for (auto &th: threads) {
            th.join();
        }
    }
}


// 线程同步
void fun_sync (int a, int b) {
    static std::mutex mutex; // 全局变量
    // 土法lock/unlock，C++里面不要像下面这样写程序!!!!!!!!!!
    mutex.lock();
    cout << "random sleep" << endl;
    mutex.unlock();

    this_thread::sleep_for(chrono::milliseconds(rand() % 100));

    { // 正确的做法：用lock_guard实现RAII式的保护
        lock_guard<std::mutex> guard(mutex);
        cout << "th " << this_thread::get_id() << ": " << a + b << endl;
    }
    // 问题1. C++里面所有的open/close, lock/unlock, malloc/free, new/delete
    // 这种配对操作最好都要通过RAII方式用一个对象实现
    // 为什么？？？ (答案在最后)
    // 问题2. 非要用动态创建对象的话怎么办?
}
// 普通的mutex可以应付绝大多数需求, 更加fancy的mutex在这里找
// http://en.cppreference.com/w/cpp/header/mutex


void demo_async () {
// 3. 用async进行异步计算
// 传统的函数调用是在一行之内启动函数计算和获取返回值，比如
    auto plus = [](int a, int b) { return a + b;};
    {
        int c = plus(1, 2);
        cout << "1 + 2 = " << c << endl;
    }
// 异步计算的目的是把启动计算和获取返回值分开，并且保证
// 获取返回值的时候计算已经完成。这样分开以后计算在何时何地
// 进行就有了很大的自由，可以用各种方式进行优化.
// 
// C++里的异步计算需要用到下面几种特性
//  - std::async,  用于启动异步计算
//  - std::future,  用于获取计算结果 
    {   // 用异步计算重新做上面的是事情

        std::future<int> c = std::async(plus, 1, 2);
        // 在这里干别的各种事情
        // 然后获取结果
        cout << "async: 1 + 2 = " << c.get() << endl;
        // C++保证future::get()返回的时候异步计算已经完成
        //
    }
// 编译器可以自动推断future的类型，所以可以这么搞 
    {
        auto future = std::async(plus, 1, 2);
        future.get();
    }

    // 如果线程启动后就不需要对它进行管理(创建后直接detach)，那么可以用std::async启动线程
    // 而无需显式创建std::thread对象
    {
        cout << "DEMO ASYNC." << endl;
        std::async(fun, 1, 2);
        cout << "1st async done." << endl;
    }
    // !!!!!!!!TODO 注意察看输出
    // (很可能)你会发现fun并没有被运行！
    // 为什么????????
    //
    
// async并不一定真是并行进行的。C++里面的async实现了两种方式：并行处理和lazy evaluation。
// (注意"异步"和"并行"的区别。)
// 在调用async时可以传入std::launch::async或std::launch::deferred指定。如果啥都不传，默认行为
// 由编译器决定。在g++里就是deferred，也就是laze evaluation。所谓lazy，就是指返回值在真正需要用
// 的时候才进行计算。像下面这样：
   {
        cout << "DEMO ASYNC, lazy evaluation." << endl;
        cout << "global thread id: " << this_thread::get_id() << endl;
        std::future<void> future = std::async(std::launch::deferred, fun, 1, 2);
        future.get(); // 即使函数返回void，也需要get一下才能保证函数运行
        cout << "You should see output from fun now." << endl;
        // 注意, fun里面打印的线程ID和全局ID是相同的，也就是说async并没有真正创建线程，
        // 而只是在future.get()的时候运行了fun函数
   }
// 下面我们传入std::launch::async迫使async真正创建一个进程
// 通过launch::async方式创建的异步任务即使没运行future::get也会被完成
// (C++里的命名这里有点混乱, 我觉得"std::launch::parallel"更能代表并行运行的意思) 
    {
        cout << "global thread id: " << this_thread::get_id() << endl;
        cout << "run with launch::async" << endl;
        std::async(std::launch::async, fun, 1, 2);
        cout << "You should see output from fun now." << endl;
    }
// 异步异常处理
    {
        auto thrower = [](){cout << "we are going to throw." << endl; throw std::runtime_error("hello, world!");};
        auto future = std::async(std::launch::async, thrower);
        this_thread::sleep_for(chrono::seconds(1));
        cout << "exception was already thrown in another thread." << endl;
        try { // 异步catch异常
            future.get();
        }
        catch (std::runtime_error const &e) {
            cerr << e.what() << endl;
        }
    }
}


int main () {
    cout << "Hardware concurrency: " << std::thread::hardware_concurrency() << endl;
    demo_thread();
    demo_async();
    return 0;
}


// 答案
// 问题1:
// 如果open和close之间抛出了异常，那么close就会被跳过，引起错误
//
// 问题2:
// C++里面尽量不要用new/delete. 有时候非要用, 最好的办法是用new之后
// 立刻用unique_ptr或者shared_ptr包装起来。事实上C++的智能指针可以直接构造对象，
// 不需要用到new关键字，比如
//      unique_ptr<std::thread> p_th(fun, 1, 2, 3);
// 
// 或者干脆用长度为1的std::vector
//      
// 除非保证new/delete之间有catch (...)抓住所有的异常，否则delete都有可能会被跳过。
