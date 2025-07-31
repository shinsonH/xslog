#include "logmsg.h"
#include "logger.h"

namespace xs
{
    SLogEndl CLogMsg::m_sLogEndl;

    CLogMsg::CLogMsg(CLogger& Logger, ELogLevel eLevel, const wchar_t* pszFile, unsigned int nLine)
        : m_Logger(Logger), m_eLevel(eLevel), m_pOSStream(new std::wostringstream())
    {
        // 获取当前时间
        std::chrono::time_point<std::chrono::system_clock> tpNowTime = std::chrono::system_clock::now();
        std::chrono::milliseconds msDuration = std::chrono::duration_cast<std::chrono::milliseconds>(tpNowTime.time_since_epoch());
        std::time_t ctNowTime = std::chrono::system_clock::to_time_t(tpNowTime);
        struct tm tmNowTime = { 0 };
        localtime_s(&tmNowTime, &ctNowTime);

        // 截取文件名
        const wchar_t* pName = nullptr;
        do
        {
            pName = wcsrchr(pszFile, L'\\');
            if (pName)
            {
                pName++;
                break;
            }
            pName = wcsrchr(pszFile, L'/');
            if (pName)
            {
                pName++;
                break;
            }
            pName = pszFile;
        } while (0);

        // 输出日志前缀: [LEVEL YYYY-MM-DD HH:MM:SS.ZZZ THREAD FILE:LINE] MESSAGE
        *m_pOSStream << L"[" << m_Logger.LevelName(m_eLevel, true)
            << std::put_time(&tmNowTime, L" %F %T")
            << L"." << std::setw(3) << std::setfill(L'0') << msDuration.count() % 1000 << L" "
            << std::this_thread::get_id() << " "
            << pName << L":" << nLine << L"] ";
    }

    CLogMsg::CLogMsg(CLogMsg&& Other) noexcept
        : m_Logger(Other.m_Logger), m_eLevel(Other.m_eLevel), m_pOSStream(Other.m_pOSStream)
    {
        Other.m_pOSStream = nullptr;
    }

    CLogMsg::~CLogMsg()
    {
        if (m_pOSStream)
        {
            m_Logger.PushLog(m_eLevel, std::move(m_pOSStream->str()), m_bFlush);

            delete m_pOSStream;
            m_pOSStream = nullptr;
        }
    }

    CLogMsg& CLogMsg::operator<<(bool val)
    {
        *m_pOSStream << val;
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(char val)
    {
        *m_pOSStream << val;
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(unsigned char val)
    {
        *m_pOSStream << val;
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(short val)
    {
        *m_pOSStream << val;
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(unsigned short val)
    {
        *m_pOSStream << val;
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(int val)
    {
        *m_pOSStream << val;
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(unsigned int val)
    {
        *m_pOSStream << val;
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(long val)
    {
        *m_pOSStream << val;
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(unsigned long val)
    {
        *m_pOSStream << val;
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(long long val)
    {
        *m_pOSStream << val;
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(unsigned long long val)
    {
        *m_pOSStream << val;
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(float val)
    {
        *m_pOSStream << val;
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(double val)
    {
        *m_pOSStream << val;
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(long double val)
    {
        *m_pOSStream << val;
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(void* val)
    {
        *m_pOSStream << val;
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(const void* val)
    {
        *m_pOSStream << val;
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(char* val)
    {
        *m_pOSStream << ToWString(val);
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(const char* val)
    {
        *m_pOSStream << ToWString(val);
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(std::string& val)
    {
        *m_pOSStream << ToWString(val);
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(const std::string& val)
    {
        *m_pOSStream << ToWString(val);
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(std::string&& val)
    {
        *m_pOSStream << ToWString(val);
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(wchar_t* val)
    {
        *m_pOSStream << val;
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(const wchar_t* val)
    {
        *m_pOSStream << val;
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(std::wstring& val)
    {
        *m_pOSStream << val;
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(const std::wstring& val)
    {
        *m_pOSStream << val;
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(std::wstring&& val)
    {
        *m_pOSStream << val;
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(SLogEndl&)
    {
        m_bFlush = true;
        return *this;
    }
    CLogMsg& CLogMsg::operator<<(const SLogEndl&)
    {
        m_bFlush = true;
        return *this;
    }
    CLogMsg& CLogMsg::operator<<(SLogEndl&&)
    {
        m_bFlush = true;
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(std::ostream& (__cdecl* Func)(std::ostream&))
    {
        *m_pOSStream << Func;
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(std::ios& (__cdecl* Func)(std::ios&))
    {
        *m_pOSStream << Func;
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(std::ios_base& (__cdecl* Func)(std::ios_base&))
    {
        *m_pOSStream << Func;
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(const std::_Smanip<std::streamsize>& _Manip)
    {
        *m_pOSStream << _Manip;
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(const std::_Fillobj<char>& _Manip)
    {
        *m_pOSStream << std::_Fillobj<wchar_t>((int)_Manip._Fill);
        return *this;
    }

    CLogMsg& CLogMsg::operator<<(const std::_Fillobj<wchar_t>& _Manip)
    {
        *m_pOSStream << _Manip;
        return *this;
    }

    std::string CLogMsg::ToString(const std::wstring& szInput)
    {
        std::string szOutput;

        if (szInput.length() > 0)
        {
            static auto local = _create_locale(LC_ALL, "");
            const wchar_t* pSrc = szInput.c_str();
            size_t nDstCnt = 0; // included null terminator
            errno_t err = _wcstombs_s_l(&nDstCnt, NULL, 0, pSrc, 0, local);
            if (err == 0)
            {
                std::shared_ptr<char> dstPtr(new char[nDstCnt]);
                if (dstPtr)
                {
                    char* pDst = dstPtr.get();
                    err = _wcstombs_s_l(&nDstCnt, pDst, nDstCnt, pSrc, nDstCnt - 1, local);
                    if (err == 0)
                    {
                        szOutput.assign(pDst, nDstCnt - 1);
                    }
                }
            }
        }
        return szOutput;
    }

    std::wstring CLogMsg::ToWString(const std::string& szInput)
    {
        std::wstring szOutput;

        if (szInput.length() > 0)
        {
            static auto local = _create_locale(LC_ALL, "");
            const char* pSrc = szInput.c_str();
            size_t nDstCnt = 0; // included null terminator
            errno_t err = _mbstowcs_s_l(&nDstCnt, NULL, 0, pSrc, 0, local);
            if (err == 0)
            {
                std::shared_ptr<wchar_t> dstPtr(new wchar_t[nDstCnt]);
                if (dstPtr)
                {
                    wchar_t* pDst = dstPtr.get();
                    err = _mbstowcs_s_l(&nDstCnt, pDst, nDstCnt, pSrc, nDstCnt - 1, local);
                    if (err == 0)
                    {
                        szOutput.assign(pDst, nDstCnt - 1);
                    }
                }
            }
        }

        return szOutput;
    }

}
