#include "gwmtaskthread.h"

#include <string>

using namespace gwm;

void GwmTaskThreadTelegram::print(std::string message, ITelegram::LogLevel level, std::string fun_name, std::string file_name)
{
    (void)fun_name;
    (void)file_name;
    switch (level)
    {
    case ITelegram::LogLevel::LOG_EMERG:
    case ITelegram::LogLevel::LOG_ALERT:
    case ITelegram::LogLevel::LOG_CRIT:
        mTask->print_error(QString::fromStdString(message));
        break;
    
    case ITelegram::LogLevel::LOG_ERR:
    case ITelegram::LogLevel::LOG_WARNING:
    case ITelegram::LogLevel::LOG_NOTICE:
    case ITelegram::LogLevel::LOG_INFO:
    case ITelegram::LogLevel::LOG_DEBUG:
    default:
        mTask->print_message(QString::fromStdString(message));
        break;
    }
}

QString GwmTaskThread::name() const
{
    return tr("Task");
}

bool GwmTaskThread::checkCanceled() const
{
    if (isCanceled())
    {
        emit canceled();
        return true;
    }
    else return false;
}
