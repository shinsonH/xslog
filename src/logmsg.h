#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

#ifdef XSLOG_LIB
#define XSLOG_API
#else
#ifdef XSLOG_EXPORTS
#define XSLOG_API __declspec(dllexport)
#else
#define XSLOG_API __declspec(dllimport)
#endif /* XSLOG_EXPORTS */
#endif /* XSLOG_LIB */

namespace xs
{
    class CLogger;
    enum class ELogLevel;

    struct SLogEndl
    {
    };

    class XSLOG_API CLogMsg
    {
    public:
        CLogMsg(CLogger& Logger, ELogLevel eLevel, const wchar_t* pszFile, unsigned int nLine);
        CLogMsg(const CLogMsg& Other) = delete;
        CLogMsg(CLogMsg&& Other) noexcept;
        ~CLogMsg();

        CLogMsg& operator=(const CLogMsg& Other) = delete;
        CLogMsg& operator=(CLogMsg&& Other) = delete;

        // 支持输出基础数据结构
        CLogMsg& operator<<(bool val);
        CLogMsg& operator<<(char val);
        CLogMsg& operator<<(unsigned char val);
        CLogMsg& operator<<(short val);
        CLogMsg& operator<<(unsigned short val);
        CLogMsg& operator<<(int val);
        CLogMsg& operator<<(unsigned int val);
        CLogMsg& operator<<(long val);
        CLogMsg& operator<<(unsigned long val);
        CLogMsg& operator<<(long long val);
        CLogMsg& operator<<(unsigned long long val);
        CLogMsg& operator<<(float val);
        CLogMsg& operator<<(double val);
        CLogMsg& operator<<(long double val);
        CLogMsg& operator<<(void* val);
        CLogMsg& operator<<(const void* val);

        // 支持输出多字节字符串
        CLogMsg& operator<<(char* val);
        CLogMsg& operator<<(const char* val);
        CLogMsg& operator<<(std::string& val);
        CLogMsg& operator<<(const std::string& val);
        CLogMsg& operator<<(std::string&& val);

        // 支持输出宽字符串
        CLogMsg& operator<<(wchar_t* val);
        CLogMsg& operator<<(const wchar_t* val);
        CLogMsg& operator<<(std::wstring& val);
        CLogMsg& operator<<(const std::wstring& val);
        CLogMsg& operator<<(std::wstring&& val);

        // 支持输出刷新缓存的操作符
        CLogMsg& operator<<(SLogEndl&);
        CLogMsg& operator<<(const SLogEndl&);
        CLogMsg& operator<<(SLogEndl&&);

        // 支持流操作函数
        // call basic_ostream manipulator: std::endl/std::flush/...
        CLogMsg& operator<<(std::ostream& (__cdecl* Func)(std::ostream&));
        // call basic_ios manipulator: 
        CLogMsg& operator<<(std::ios& (__cdecl* Func)(std::ios&));
        // 支持格式化操作: std::setw/std::setfill
        CLogMsg& operator<<(const std::_Smanip<std::streamsize>& _Manip);
        CLogMsg& operator<<(const std::_Fillobj<char>& _Manip);
        CLogMsg& operator<<(const std::_Fillobj<wchar_t>& _Manip);

    public:
        // 宽字符(UNICODE)与多字节(ANSI)的编码转换
        static std::string ToString(const std::wstring& szInput);
        static std::wstring ToWString(const std::string& szInput);

        static SLogEndl m_sLogEndl;

    private:
        CLogger& m_Logger;                  // 日志对象的引用
        ELogLevel m_eLevel;                 // 日志等级(当前这条日志记录的等级)
        std::wostringstream* m_pOSStream;   // 日志信息流(用于转码并缓存当前这条日志的每个片段)
        bool m_bFlush = false;
    };
}
