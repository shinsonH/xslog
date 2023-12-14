#include <windows.h>
#include <Shlwapi.h>
#include <iostream>
#include <xslog/include/xslog.hpp>

#pragma comment(lib, "xslog_dll.lib")
#pragma comment(lib, "shlwapi.lib")

int main(int argc, const char* argv[])
{
    wchar_t Path[MAX_PATH] = { 0 };
    ::GetModuleFileNameW(NULL, Path, _countof(Path));
    ::PathAppend(Path, L"..\\log\\xslog");

    XsSetLogLevel(xs::ELogLevel::LEVEL_INFO);
    XsAddSingleFileSink(Path, true);

    XSLOGI << "我是main";
    XSLOGE << L"我是main[" << std::setw(3) << std::setfill(L'0') << 3 << "]";
    std::ofstream ofs;
    ofs << std::setw(3) << std::setfill('0');

    for (int i = 0; i < 10; i++)
    {
        std::thread a([i] {
            do
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                XSLOGI << L"线程：" << std::string(10, '0' + i);
            } while (1);
            });
        a.detach();
    }
    std::this_thread::sleep_for(std::chrono::seconds(4));
    return 0;
}
