### 问题解决方案
1. 在切换为vs编译器时,build阶段会显示乱码
    1. 拟将vs的中文语言包删除并重启:没用
    2. 打开VScode的设置的encoding,在Cmake: Output Log Encoding来自外部命令的输出的编码(例如 cmake -- build).从auto转为utf8:解决
2. python把没有debug的版本，所以python311_d.lib不存在，报错cannot open file 'python311_d.lib',这是pybind11导致的
    1. set(PYBIND11_PYTHON_LIBRARY_DEBUG "${Python_LIBRARIES}")：失败
    2. add_link_options($<$<CONFIG:Debug>:/NODEFAULTLIB:python311_d.lib>),对全局生效,可能会造成影响,但是不这样的话就必须一个个添加,同时,被依赖的目标也要加,该问题实际上就是下一个问题
3. __imp__Py_NegativeRefcount 和 __imp__Py_RefTotal 未定义错误<br>
是因为在 Debug 配置下 MSVC 自动定义了 _DEBUG 宏，导致 Python 头文件启用了 Py_DEBUG 模式，进而使用了调试版本的引用计数函数（如 _Py_NegativeRefcount）。但这些符号只存在于 Python 调试库（python311_d.lib）中，而官方 Python 不提供该库，因此链接失败。
    1. 添加
    ```c++
    if(MSVC)
    add_compile_options($<$<CONFIG:Debug>:/U_DEBUG>)
    endif()
    ```
    没用<br>
    2. 找到include文件夹，在文件夹下找到pyconfig.h,object.h文件
        分别修改
        pyconfig.h
        ```c++
        #ifdef _DEBUG
        //#       define Py_DEBUG // 这已经是注释之后的了
        #endif
        ```
        object.h
        ```c++
        #if defined(Py_DEBUG) && !defined(Py_REF_DEBUG)
        //#       define Py_REF_DEBUG // 这已经是注释之后的了
        #endif
        ```
    上述过程成功实现
4.  ctest的过程中报错Test command: NOT_AVAILABLE
    1. 在add_test中的最后加入$<TARGET_FILE:"filename">, "filename"是目标名字,在单配置生成器（如 MinGW 用的 MinGW Makefiles 或 Ninja）下，CMake 在配置阶段就已经知道可执行文件的最终路径（例如直接放在 build/ 目录下），因此 add_test(NAME TestCore COMMAND test_core) 能直接找到目标。
    而多配置生成器（如 Visual Studio、Xcode）会在构建目录下生成 Debug、Release 等子目录，可执行文件的实际路径依赖于构建配置。此时若直接写 COMMAND test_core，CMake 无法确定应该去哪个子目录查找，CTest 就会报 NOT_AVAILABLE。
    生成器表达式 $<TARGET_FILE:test_core> 在构建时会根据当前配置（如 -C Debug）自动展开为 .../Debug/test_core.exe，因此能适配多配置生成器。为了代码跨平台、跨生成器，建议统一使用生成器表达式，这样无论使用 MinGW 还是 Visual Studio，测试都能正确运行。
5. pybind11的问题,pybind11只支持VS编译器,不支持mingw,上述问题都可以被认为是这个导致的,因为我之前用的就是mingw编译的
    现在已经解决了
6.  ImportError: DLL load failed while importing _ssl:<br>
    这个和import openai有关
    直接把两个dll文件放到bin文件夹里了:这次运行了2分钟
7. Fatal Python error: PyThreadState_Get: the function must be called with the GIL held, but the GIL is released (the current Python thread state is NULL)
    1. 意外调用了该文件Py_Finalize();不过6中的调用是成功的
8. SegFault1
    这是因为把pybind11中的module设置为了extern,导致在析构时出现问题
    1. 把全局的module删除,设置为cpp接口函数的一个参数:解决了
9. 线程问题,和qmutex相关
    原因:task内部维护一个线程安全的buffer,如果生产者不正常退出,就会导致该部分死锁.
    1. 因为ffmpegtask的部分和whispertask的部分抽象基类不同,暂时不更改这部分逻辑,改为在测试中更改<br>
    失败
    2. 结果发现是忘记lockerguard了
10. test_whisper没有输出结果
    原因:buffer的逻辑问题
11. 调用whisper_full显示segfault
    原因:未调用whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
12. encode时报错除以零
    原因:没有写文件头