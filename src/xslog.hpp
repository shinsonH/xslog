#pragma once
#include "logger.h"

#define XsSetLogLevel(eLogLevel) xs::CLogger::Inst().SetOutputLevel(eLogLevel)
#define XsAddLogSink(ptrSink) xs::CLogger::Inst().InsertLogSink(ptrSink)
#define XsAddConsoleSink() XsAddLogSink(std::shared_ptr<xs::CLogSink>(new xs::CConsoleSink()))
#define XsAddSingleFileSink(szFilePrefix, bAppend) XsAddLogSink(std::shared_ptr<xs::CLogSink>(new xs::CFileSink(szFilePrefix, bAppend)))
#define XsAddRollingFileSink(szFilePrefix, bAppend, nFileMaxSize, nFileMaxCount) XsAddLogSink(std::shared_ptr<xs::CLogSink>(new xs::CFileSink(szFilePrefix, bAppend, nFileMaxSize, nFileMaxCount)))
#define XsAddNetworkSink(szHost, nPort) XsAddLogSink(std::shared_ptr<xs::CLogSink>(new xs::CNetworkSink(szHost, nPort)))
#define XsAddFunctionSink(fnCallback) XsAddLogSink(std::shared_ptr<xs::CLogSink>(new xs::CFunctionSink(fnCallback)))

#define XsLogEndl xs::CLogMsg::m_sLogEndl

#define XSLOGD xs::CLogger::Inst()(xs::ELogLevel::LEVEL_DEBUG, __FILEW__, __LINE__)
#define XSLOGT xs::CLogger::Inst()(xs::ELogLevel::LEVEL_TRACE, __FILEW__, __LINE__)
#define XSLOGI xs::CLogger::Inst()(xs::ELogLevel::LEVEL_INFO, __FILEW__, __LINE__)
#define XSLOGW xs::CLogger::Inst()(xs::ELogLevel::LEVEL_WARNING, __FILEW__, __LINE__)
#define XSLOGE xs::CLogger::Inst()(xs::ELogLevel::LEVEL_ERROR, __FILEW__, __LINE__)
#define XSLOGF xs::CLogger::Inst()(xs::ELogLevel::LEVEL_FATAL, __FILEW__, __LINE__)
