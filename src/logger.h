#pragma once
#include <mutex>
#include "logmsg.h"
#include "logsink.h"

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
    // 日志等级枚举
    enum class ELogLevel
    {
        LEVEL_DEBUG = 0,        // 调试日志，仅在Debug模式下才有效，用于打印调试信息或排查错误时需要更详细的日志
        LEVEL_TRACE = 1,        // 追踪日志，用于打印详细的日志信息
        LEVEL_INFO = 2,         // 重要日志，用于记录一些不会频繁产生的重要信息，比如关键参数、重要逻辑处理流程、运行状态等
        LEVEL_WARNING = 3,      // 警告日志，用于记录可能发生但不影响业务流程的错误信息，方便日后完善逻辑处理
        LEVEL_ERROR = 4,        // 错误日志，用于记录程序运行错误的信息，方便排查程序问题
        LEVEL_FATAL = 5,        // 致命日志，谨慎使用，输出该日志后，程序将自动终止或触发自定义信号
        LEVEL_MAX = LEVEL_FATAL // 最大日志等级
    };

    // 日志管理类
    class XSLOG_API CLogger
    {
    public:
        // 单例
        static CLogger& Inst();

        // 设置日志输出等级，只有等于或更严重的日志才会输出
        // 如果不设置，则默认INFO级别
        void SetOutputLevel(ELogLevel eOutputLevel);

        // 添加日志输出对象
        // 如果不添加任何输出对象，则默认输出到控制台
        void InsertLogSink(CLogSink::Ptr LogSink);
        void RemoveLogSink(CLogSink::Ptr LogSink);

        // 重载操作符，用于创建一个相应等级的日志消息的临时对象
        CLogMsg operator()(ELogLevel eLevel, const wchar_t* pFile, int nLine);

    protected:
        friend class CLogMsg;

        // 获取日志等级对应的名称或简称
        const std::wstring& LevelName(ELogLevel eLevel, bool bShortName = false);

        // 用于日志流对象推送一条完整日志记录
        void PushLog(ELogLevel eLevel, std::wstring&& szLog, bool bFlush);

    private:
        CLogger();
        ~CLogger();

        // 日志异步触发线程入口函数
        void AsyncTriggerThread();

    private:
        struct SSinkData
        {
            CLogSink::Ptr pSink;
            bool bHasWritten = false;
        };

        struct SClassData
        {
            std::mutex m_globalLocker;              // 全局互斥锁
            ELogLevel m_eOutputLevel = ELogLevel::LEVEL_INFO;   // 日志输出等级，小于该等级的日志将会被忽略掉，默认为INFO
            std::vector<SSinkData> m_vSinks;        // 日志输出对象列表，同一条日志会同步写入每一个输出对象
            std::thread m_asyncTriggerThread;       // 日志异步输出触发线程，实现日志异步打印
            std::atomic_bool m_bThreadRun;          // 线程的运行标记，true指示继续运行，false指示线程退出
            std::condition_variable m_cvThreadStop; // 线程退出事件，指示线程立即退出
        };

        SClassData* m_pClsData = nullptr;
    };
}
