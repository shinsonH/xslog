#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "logger.h"

namespace xs
{
    CLogger& CLogger::Inst()
    {
        static CLogger inst;
        return inst;
    }

    CLogger::CLogger()
    {
        m_pClsData = new SClassData();
        // 创建异步触发线程
        m_pClsData->m_bThreadRun = true;
        m_pClsData->m_asyncTriggerThread = std::thread(&CLogger::AsyncTriggerThread, this);
    }

    CLogger::~CLogger()
    {
        // 退出异步触发线程
        m_pClsData->m_bThreadRun = false;
        m_pClsData->m_cvThreadStop.notify_all();
        if (m_pClsData->m_asyncTriggerThread.joinable())
        {
            m_pClsData->m_asyncTriggerThread.join();
        }

        m_pClsData->m_vSinks.clear();

        if (m_pClsData)
        {
            delete m_pClsData;
            m_pClsData = nullptr;
        }
    }

    void CLogger::SetOutputLevel(ELogLevel eOutputLevel)
    {
        std::lock_guard<std::mutex> LockGuard(m_pClsData->m_globalLocker);
        m_pClsData->m_eOutputLevel = eOutputLevel;
    }

    void CLogger::InsertLogSink(CLogSink::Ptr LogSink)
    {
        std::lock_guard<std::mutex> LockGuard(m_pClsData->m_globalLocker);
        for (auto& sink : m_pClsData->m_vSinks)
        {
            if (sink.pSink == LogSink)
            {
                // 已存在
                return;
            }
        }
        SSinkData sink;
        sink.pSink = LogSink;
        sink.bHasWritten = false;
        m_pClsData->m_vSinks.push_back(sink);
    }

    void CLogger::RemoveLogSink(CLogSink::Ptr LogSink)
    {
        std::lock_guard<std::mutex> LockGuard(m_pClsData->m_globalLocker);
        for (auto iter = m_pClsData->m_vSinks.begin(); iter != m_pClsData->m_vSinks.end(); iter++)
        {
            if (iter->pSink == LogSink)
            {
                m_pClsData->m_vSinks.erase(iter);
                break;
            }
        }
    }

    CLogMsg CLogger::operator()(ELogLevel eLevel, const wchar_t* pFile, int nLine)
    {
        return CLogMsg(*this, eLevel, pFile, nLine);
    }

    const std::wstring& CLogger::LevelName(ELogLevel eLevel, bool bShortName)
    {
        static const unsigned int nNameCount = static_cast<unsigned int>(ELogLevel::LEVEL_MAX) + 2;
        static std::wstring LevelNames[nNameCount][2] = {
            { L"DEBUG",  L"D" },
            { L"TRACE",  L"T" },
            { L"INFO",   L"I" },
            { L"WARNING",L"W" },
            { L"ERROR",  L"E" },
            { L"FATAL",  L"F" },
            { L"NONE",   L"N" }
        };

        unsigned int nIndex = static_cast<unsigned int>(eLevel);
        if (nIndex >= nNameCount)
        {
            nIndex = nNameCount - 1;
        }
        return LevelNames[nIndex][bShortName ? 1 : 0];
    }

    void CLogger::PushLog(ELogLevel eLevel, std::wstring&& szLog, bool bFlush)
    {
        std::lock_guard<std::mutex> LockGuard(m_pClsData->m_globalLocker);

        static std::wstring szLogHeader;
        if (szLogHeader.empty())
        {
            std::wstring szPid = std::to_wstring(::GetCurrentProcessId());
            szLogHeader.append(L"START LOGGING PROCESS(").append(szPid).append(L") ...\n");
            szLogHeader.append(L"[LEVEL YYYY-MM-DD HH:MM:SS.SSS THREAD FILE:LINE] MESSAGE\n");
        }

        // 根据日志输出等级过滤
        if (eLevel < m_pClsData->m_eOutputLevel)
        {
            return;
        }

        if (szLog.length() > 0)
        {
            // 确保日志内容以换行符结尾
            if (szLog.back() != L'\n')
            {
                szLog += L"\n";
            }
        }

        // 如果没有添加任何输出对象，则默认输出到标准输出
        if (m_pClsData->m_vSinks.empty())
        {
            std::wcout << szLog;
            std::wcout.flush();
            return;
        }

        auto ThreadId = std::this_thread::get_id();
        // 将日志写入所有输出对象
        for (auto& sink : m_pClsData->m_vSinks)
        {
            if (sink.pSink->MatchThreadFilter(ThreadId))
            {
                if (szLog.length() > 0)
                {
                    // 如果是第一次输出日志，则添加日志引导信息
                    if (!sink.bHasWritten)
                    {
                        sink.bHasWritten = true;
                        szLog.insert(0, szLogHeader);
                    }
                    sink.pSink->WriteLog(szLog);
                }

                if (bFlush && sink.pSink->IsAsyncMode())
                {
                    sink.pSink->Flush();
                }
            }
        }
    }

    void CLogger::AsyncTriggerThread()
    {
        std::unique_lock<std::mutex> Lock(m_pClsData->m_globalLocker);

        while (m_pClsData->m_bThreadRun)
        {
            m_pClsData->m_cvThreadStop.wait_for(Lock, std::chrono::seconds(3));
            if (!m_pClsData->m_bThreadRun)
            {
                break;
            }
            for (auto& sink : m_pClsData->m_vSinks)
            {
                if (sink.pSink->IsAsyncMode())
                {
                    sink.pSink->Flush();
                }
            }
        }
    }
}
