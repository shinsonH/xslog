#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <vector>
#include <memory>
#include <thread>
#include <functional>

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
    ////////////////////////////////////////////////////////////////////////
    // 日志输出基类
    ////////////////////////////////////////////////////////////////////////
    class XSLOG_API CLogSink
    {
    public:
        typedef std::shared_ptr<CLogSink> Ptr;

        CLogSink(bool bAsyncMode);
        virtual ~CLogSink();

        bool IsAsyncMode() const { return m_bAsyncMode; }

        // 设置线程过滤列表，非线程安全，必须在使用该Sink前设置
        void SetThreadFilter(const std::vector<std::thread::id>& vThreadIds);

        // 判断某线程是在存在于过滤列表
        bool MatchThreadFilter(const std::thread::id& ThreadId);

        // 写日志，异步模式时可能是仅暂存起来
        virtual void WriteLog(const std::wstring& szLog) = 0;

        // 同步Dump日志
        virtual void Flush() {}

    private:
        bool m_bAsyncMode = false;  // 是否为异步模式
        std::vector<std::thread::id>* m_pThreadIds = nullptr; // 只输出该列表中的线程产生的日志消息
    };

    ////////////////////////////////////////////////////////////////////////
    // 控制台输出
    ////////////////////////////////////////////////////////////////////////
    class XSLOG_API CConsoleSink : public CLogSink
    {
    public:
        CConsoleSink() : CLogSink(false) {}
        virtual ~CConsoleSink() = default;

        void WriteLog(const std::wstring& szLog) override
        {
            std::wcout << szLog;
            std::wcout.flush();
        }
    };

    ////////////////////////////////////////////////////////////////////////
    // 文件输出
    // - 有日志输出才会打开/创建日志文件
    // - 日志文件名格式：prefix[_pid_timestamp].log[.index]
    //      默认情况为不限大小单文件追加模式的格式 prefix.log
    //      非追加模式则会自动添加 _pid_timestamp
    //      多文件模式则会自动添加 .index
    ////////////////////////////////////////////////////////////////////////
    class XSLOG_API CFileSink : public CLogSink
    {
    public:
        CFileSink(const std::string& szFilePrefix, bool bAppend = true, size_t nFileMaxSize = 0, unsigned short nFileMaxCount = 0);
        CFileSink(const std::wstring& wszFilePrefix, bool bAppend = true, size_t nFileMaxSize = 0, unsigned short nFileMaxCount = 0);
        virtual ~CFileSink();

        void WriteLog(const std::wstring& szLog) override;
        void Flush() override;

    protected:
        // 写日志文件
        void WriteFile();
        // 获取日志文件全路径
        std::string GetLogFullPath();
        // 滚动日志文件(递增日志文件序号，删除超出个数的日志文件)
        void RollLogFiles(const std::string& szBaseLog);

    private:
        // 根据日志文件前缀
        void ParseFilePrefix(const std::string& szFilePrefix);
        // 创建路径(支持多级目录)
        void CreatePath(const std::string& szPath);
        // 获取当前进程标识
        std::string GetCurrentPid();
        // 获取当前时间戳(格式化为MMDDhhmmss)
        std::string GetFormatTime();
        // 获取文件大小
        size_t GetFileSize(const std::string& szFilePath);

    protected:
        std::string* m_pszLogPath = nullptr;        // 日志存储目录
        std::string* m_pszLogName = nullptr;        // 日志文件名称
        bool m_bAppend = false;                     // 是否为追加模式
        size_t m_nFileMaxSize = 0;                  // 日志文件大小限制，单位字节，默认为0，表示不限制
        unsigned short m_nFileMaxCount = 0;         // 日志文件个数限制，默认为0，表示不限制
        std::ofstream* m_pFileStream = nullptr;     // 日志文件流对象
        std::string* m_pszBuffer = nullptr;         // 日志缓存
        int64_t m_nLogCount = 0;
        int64_t m_nLogSize = 0;
        int64_t m_nWriteCount = 0;
        int64_t m_nWriteSize = 0;
    };

    ////////////////////////////////////////////////////////////////////////
    // 网络输出
    ////////////////////////////////////////////////////////////////////////
    class XSLOG_API CNetworkSink : public CLogSink
    {
    public:
        CNetworkSink(const std::string& szHost, unsigned short nPort);
        virtual ~CNetworkSink();

        void WriteLog(const std::wstring& szLog) override;

    protected:
        bool Connect(const std::string& szHost, unsigned short nPort);

    protected:
        bool m_bConnected = false;
        std::string* m_pszHost = nullptr;
        unsigned short m_nPort = 0;
    };

    ////////////////////////////////////////////////////////////////////////
    // 函数输出
    ////////////////////////////////////////////////////////////////////////
    class XSLOG_API CFunctionSink : public CLogSink
    {
    public:
        typedef std::function<void(const std::wstring& szLog)> TOutputFunc;

        CFunctionSink(TOutputFunc fnCallback);
        virtual ~CFunctionSink();

        void WriteLog(const std::wstring& szLog) override
        {
            if (*m_pfnCallback)
            {
                (*m_pfnCallback)(szLog);
            }
        }

    protected:
        TOutputFunc* m_pfnCallback = nullptr;
    };
}
