#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <io.h>
#include <direct.h>
#include "logsink.h"
#include "logmsg.h"

namespace xs
{
    std::wstring StringToWString(const std::string& szSrc, int nCodePage)
    {
        try
        {
            if (szSrc.empty())
            {
                return L"";
            }

            /* 获取转码后宽字符的字符数 */
            auto nWideCharCount = ::MultiByteToWideChar(nCodePage, 0, szSrc.c_str(), (int)szSrc.length(), NULL, 0);
            if (nWideCharCount <= 0)
            {
                auto err = ::GetLastError();
                return L"";
            }

            /* 分配宽字符内存 */
            std::shared_ptr<wchar_t> pTmpWideBuff(new wchar_t[nWideCharCount]);

            /* 转码 */
            nWideCharCount = ::MultiByteToWideChar(nCodePage, 0, szSrc.c_str(), (int)szSrc.length(), pTmpWideBuff.get(), nWideCharCount);
            if (nWideCharCount <= 0)
            {
                auto err = ::GetLastError();
                return L"";
            }

            return std::wstring(pTmpWideBuff.get(), nWideCharCount);
        }
        catch (...)
        {
            return L"";
        }
    }

    static std::string WStringToString(const std::wstring& wszSrc, int nCodePage)
    {
        try
        {
            if (wszSrc.empty())
            {
                return "";
            }

            /* 获取转码后多字节字节数 */
            auto nMultiByteCount = ::WideCharToMultiByte(nCodePage, 0, wszSrc.c_str(), (int)wszSrc.length(), NULL, 0, NULL, NULL);
            if (nMultiByteCount <= 0)
            {
                auto err = ::GetLastError();
                return "";
            }

            /* 分配多字节内存 */
            std::shared_ptr<char> pMultiByteBuff(new char[nMultiByteCount]);

            /* 转码 */
            BOOL bUsedDefaultChar = FALSE;
            nMultiByteCount = ::WideCharToMultiByte(nCodePage, 0, wszSrc.c_str(), (int)wszSrc.length(), pMultiByteBuff.get(),
                nMultiByteCount, NULL, (CP_UTF8 == nCodePage || CP_UTF7 == nCodePage) ? NULL : &bUsedDefaultChar);
            if (nMultiByteCount <= 0)
            {
                auto err = ::GetLastError();
                return "";
            }

            return std::string(pMultiByteBuff.get(), nMultiByteCount);
        }
        catch (...)
        {
            return "";
        }
    }

    // 输出基类
    CLogSink::CLogSink(bool bAsyncMode) : m_bAsyncMode(bAsyncMode)
    {
        m_pThreadIds = new std::vector<std::thread::id>();
    }

    CLogSink::~CLogSink()
    {
        if (m_pThreadIds)
        {
            delete m_pThreadIds;
            m_pThreadIds = nullptr;
        }
    }

    void CLogSink::SetThreadFilter(const std::vector<std::thread::id>& vThreadIds)
    {
        m_pThreadIds->clear();
        for (auto& tid : vThreadIds)
        {
            m_pThreadIds->push_back(tid);
        }
    }

    bool CLogSink::MatchThreadFilter(const std::thread::id& ThreadId)
    {
        if (m_pThreadIds->empty())
        {
            // 没有设置线程过滤，则不需要过滤
            return true;
        }

        for (auto& tid : *m_pThreadIds)
        {
            if (tid == ThreadId)
            {
                // 匹配成功
                return true;
            }
        }
        // 匹配失败
        return false;
    }

    // 定义文件输出类
    CFileSink::CFileSink(const std::string& szFilePrefix, bool bAppend, size_t nFileMaxSize, unsigned short nFileMaxCount)
        : CLogSink(true), m_bAppend(bAppend), m_nFileMaxSize(nFileMaxSize), m_nFileMaxCount(nFileMaxCount)
    {
        m_pszLogPath = new std::string();
        m_pszLogName = new std::string();
        m_pFileStream = new std::ofstream();
        m_pszBuffer = new std::string();

        ParseFilePrefix(szFilePrefix);
    }

    CFileSink::CFileSink(const std::wstring& wszFilePrefix, bool bAppend, size_t nFileMaxSize, unsigned short nFileMaxCount)
        : CLogSink(true), m_bAppend(bAppend), m_nFileMaxSize(nFileMaxSize), m_nFileMaxCount(nFileMaxCount)
    {
        m_pszLogPath = new std::string();
        m_pszLogName = new std::string();
        m_pFileStream = new std::ofstream();
        m_pszBuffer = new std::string();
        ParseFilePrefix(CLogMsg::ToString(wszFilePrefix));
    }

    CFileSink::~CFileSink()
    {
        Flush();

        // 记录统计信息
        std::stringstream ss;
        ss << "STOP LOGGING: LOG(" << m_nLogCount << " - " << m_nLogSize << "), WRITTEN(" << m_nWriteCount << " - " << m_nWriteSize << ")";
        *m_pszBuffer = ss.str();
        WriteFile();

        if (m_pszLogPath)
        {
            delete m_pszLogPath;
            m_pszLogPath = nullptr;
        }

        if (m_pszLogName)
        {
            delete m_pszLogName;
            m_pszLogName = nullptr;
        }

        if (m_pFileStream && m_pFileStream->is_open())
        {
            m_pFileStream->close();
            delete m_pFileStream;
            m_pFileStream = nullptr;
        }

        if (m_pszBuffer)
        {
            delete m_pszBuffer;
            m_pszBuffer = nullptr;
        }
    }

    void CFileSink::WriteLog(const std::wstring& wszLog)
    {
        auto szLog = WStringToString(wszLog, CP_UTF8);
        m_nLogCount++;
        m_nLogSize += szLog.size();
        m_pszBuffer->append(szLog);
        if (m_pszBuffer->length() >= 4096)
        {
            WriteFile();
        }
    }

    void CFileSink::Flush()
    {
        if (m_pszBuffer->length() > 0)
        {
            WriteFile();
        }
    }

    void CFileSink::WriteFile()
    {
        if (!m_pFileStream->is_open())
        {
            // 懒加载模式，有日志输出时才打开/创建日志文件
            std::string szFileName = GetLogFullPath();
            m_pFileStream->open(szFileName, m_bAppend ? std::ios::app : std::ios::trunc);
            if (!m_pFileStream->is_open())
            {
                return;
            }
        }

        m_nWriteCount++;
        m_nWriteSize += m_pszBuffer->size();
        *m_pFileStream << *m_pszBuffer;
        m_pszBuffer->clear();

        m_pFileStream->flush();

        if (m_nFileMaxSize > 0)
        {
            // 判断当前日志文件大小是否已达上限
            size_t nFileSize = (size_t)(std::streamoff)m_pFileStream->tellp();
            if (nFileSize >= m_nFileMaxSize)
            {
                // 当前日志文件已满，关闭文件流
                m_pFileStream->close();
            }
        }
    }

    std::string CFileSink::GetLogFullPath()
    {
        // 生成日志文件全路径
        std::string szFullName = *m_pszLogPath + *m_pszLogName;

        // 确保目录存在
        CreatePath(szFullName);

        // 判断是否需要滚动(将之前的所有日志文件序号依次递增，并删除序号超过个数限制的日志文件)
        if (m_bAppend)
        {
            // 追加模式下，如果有限制文件大小且当前日志文件大小已超限制，则进行滚动
            if (m_nFileMaxSize > 0 && GetFileSize(szFullName) >= m_nFileMaxSize)
            {
                RollLogFiles(szFullName);
            }
        }
        else
        {
            // 非追加模式下，均要滚动
            RollLogFiles(szFullName);
        }

        return szFullName;
    }

    void CFileSink::RollLogFiles(const std::string& szBaseLog)
    {
        // 先查找当前进程生成的所有日志文件
        std::vector<std::string> vFileNames;
        std::string szFindPattern = szBaseLog + "*";
        WIN32_FIND_DATAA FindData;
        HANDLE hFindFile = ::FindFirstFileA(szFindPattern.c_str(), &FindData);
        if (INVALID_HANDLE_VALUE != hFindFile)
        {
            do
            {
                if (0 == (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    vFileNames.emplace_back(FindData.cFileName);
                }
            } while (::FindNextFileA(hFindFile, &FindData));

            ::FindClose(hFindFile);
            hFindFile = INVALID_HANDLE_VALUE;
        }

        // 获取每个日志文件的序号
        std::set<size_t> FileIndexSet;
        for (auto& szFileName : vFileNames)
        {
            // 获取日志文件的后缀
            std::string szSuffix = szFileName.substr(m_pszLogName->length());
            if (szSuffix.empty())
            {
                // 没有后缀表示为当前日志文件
                FileIndexSet.emplace(0);
                continue;
            }
            if (szSuffix.length() == 1 || szSuffix[0] != '.')
            {
                // 忽略不正确后缀格式
                continue;
            }
            // 去掉点号
            szSuffix = szSuffix.substr(1);
            // 判断后缀是否为纯数字
            bool bAllDigit = true;
            for (size_t i = 0; i < szSuffix.length(); i++)
            {
                if (szSuffix[i] < '0' || szSuffix[i] > '9')
                {
                    bAllDigit = false;
                    break;
                }
            }
            if (!bAllDigit)
            {
                // 忽略不正确后缀格式
                continue;
            }
            // 获取日志文件序号
            size_t nIndex = (size_t)std::stoull(szSuffix);
            if (nIndex == 0)
            {
                // 序号不能为0(不应该存在xxx.log.0)
                continue;
            }
            if (szSuffix != std::to_string(nIndex))
            {
                // 转换回去却不一样了，说明后缀格式不正确
                continue;
            }
            FileIndexSet.emplace(nIndex);
        }

        // 倒序遍历(先处理序号较大的文件)
        auto it = FileIndexSet.rbegin();
        for (; it != FileIndexSet.rend(); it++)
        {
            size_t nOldIndex = *it;
            std::string szOldFullPath(szBaseLog);
            if (nOldIndex > 0)
            {
                szOldFullPath += "." + std::to_string(nOldIndex);
            }
            // 如果文件序号超出限制，则删除该文件，否则重命名文件(文件序号递增)
            if (m_nFileMaxCount > 0 && nOldIndex >= (size_t)m_nFileMaxCount - 1)
            {
                remove(szOldFullPath.c_str());
            }
            else
            {
                std::string szNewFullPath = szBaseLog + "." + std::to_string(nOldIndex + 1);
                if (0 != rename(szOldFullPath.c_str(), szNewFullPath.c_str()))
                {
                    int err = errno;
                    std::cout << "rename '" << szOldFullPath << "' to '" << szNewFullPath << "' error: " << err << std::endl;
                }
            }
        }
    }

    void CFileSink::ParseFilePrefix(const std::string& szFilePrefix)
    {
        std::string szFullName(szFilePrefix);
        // 如果不是追加模式，则拼接进程ID和时间戳
        if (!m_bAppend)
        {
            std::string szPid = GetCurrentPid();
            std::string szTime = GetFormatTime();
            szFullName.append("_").append(szPid).append("_").append(szTime);
        }
        // 拼接日志文件后缀名
        szFullName.append(".log");
        // 分解日志路径和文件名
        size_t pos = szFullName.find_last_of("/\\");
        if (pos != std::string::npos)
        {
            *m_pszLogPath = szFullName.substr(0, pos + 1);
            *m_pszLogName = szFullName.substr(pos + 1);
        }
        else
        {
            *m_pszLogPath = "";
            *m_pszLogName = szFullName;
        }
    }

    void CFileSink::CreatePath(const std::string& szPath)
    {
        // 先判断全路径是否存在
        if (0 == _access(szPath.c_str(), 0))
        {
            // 全路径存在，无需处理
            return;
        }

        // 逐级检查日志文件目录是否存在(不存在则创建)
        std::string szTempPath;
        size_t pos = szPath.find_first_of("/\\");
        while (pos != std::string::npos)
        {
            pos++;
            szTempPath = szPath.substr(0, pos);
            if (-1 == _access(szTempPath.c_str(), 0))
            {
                if (0 != _mkdir(szTempPath.c_str()))
                {
                    int err = errno;
                    std::cout << "mkdir '" << szTempPath << "' error: " << err << std::endl;
                }
            }

            pos = szPath.find_first_of("/\\", pos);
        }
    }

    std::string CFileSink::GetCurrentPid()
    {
        return std::to_string(::GetCurrentProcessId());
    }

    std::string CFileSink::GetFormatTime()
    {
        time_t t = time(0);
        struct tm m = { 0 };
        localtime_s(&m, &t);
        char buf[64] = { 0 };
        sprintf_s(buf, sizeof(buf), "%02d%02d%02d%02d%02d",
            m.tm_mon + 1, m.tm_mday, m.tm_hour, m.tm_min, m.tm_sec);
        return std::string(buf);
    }

    size_t CFileSink::GetFileSize(const std::string& szFilePath)
    {
        size_t nFileSize = 0;
        std::ifstream ifs(szFilePath);
        if (ifs)
        {
            ifs.seekg(0, std::ios::end);
            nFileSize = (size_t)(std::streamoff)ifs.tellg();
        }
        return nFileSize;
    }

    // 定义网络输出类
    CNetworkSink::CNetworkSink(const std::string& szHost, unsigned short nPort)
        : CLogSink(false), m_pszHost(new std::string(szHost)), m_nPort(nPort)
    {

    }

    CNetworkSink::~CNetworkSink()
    {
        if (m_pszHost)
        {
            delete m_pszHost;
            m_pszHost = nullptr;
        }
    }

    void CNetworkSink::WriteLog(const std::wstring& szLog)
    {
        if (!m_bConnected)
        {
            m_bConnected = Connect(*m_pszHost, m_nPort);
            if (!m_bConnected)
            {
                return;
            }
        }
    }

    bool CNetworkSink::Connect(const std::string& szHost, unsigned short nPort)
    {
        return false;
    }

    // 定义函数输出类
    CFunctionSink::CFunctionSink(TOutputFunc fnCallback) : CLogSink(false)
    {
        m_pfnCallback = new TOutputFunc(fnCallback);
    }

    CFunctionSink::~CFunctionSink()
    {
        if (m_pfnCallback)
        {
            delete m_pfnCallback;
            m_pfnCallback = nullptr;
        }
    }
}
