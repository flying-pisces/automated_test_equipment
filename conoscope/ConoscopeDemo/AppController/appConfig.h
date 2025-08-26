#ifndef APPCONFIG_H
#define APPCONFIG_H

#include "classcommon.h"
#include "appConfigTypes.h"
#include "conoscopeTypes.h"

class AppConfig : public ClassCommon
{
    Q_OBJECT

public:
    AppConfig(QObject *parent = nullptr);

    ~AppConfig();

    void SetConfig(AppConfig_t& config);
    void GetConfig(AppConfig_t& config);

    void GetDisplayStreamConfig(ProcessingConfig_t &processingConfig);

private:
    QString mFileName;

    AppConfig_t mAppConfig;

    void _Default();
    bool _Load();
    void _Save();

    void _SaveDisplayStreamConfig(QString fileName, ProcessingConfig_t& processingConfig);
};

#endif // APPCONFIG_H
