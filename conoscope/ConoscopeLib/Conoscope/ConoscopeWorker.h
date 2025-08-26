#ifndef CONOSCOPEWORKER_H
#define CONOSCOPEWORKER_H

#include "classcommon.h"
#include <string>

class ConoscopeWorker : public ClassCommon
{
    Q_OBJECT

public:
    enum class Request
    {
        CmdCfgFileReadProcessing,
        CmdCfgFileWriteProcessing
    };
    Q_ENUM(Request)

    static QString EnumToString(const char* enumName, int enumValue)
    {
        const QMetaObject &mo = ConoscopeWorker::staticMetaObject;
        return GetEnumString(mo, enumName, enumValue);
    }

public:
    /*!
     *  \brief  constructor
     *  \param  parent
     *  \return none
     */
    explicit ConoscopeWorker(QObject *parent = nullptr);

    /*!
     *  \brief  destructor
     *  \param  none
     *  \return none
     */
    ~ConoscopeWorker();

private:
    ClassCommon::Error _CmdCfgFileRead();

    ClassCommon::Error _CmdCfgFileWrite();

public slots:
    void OnWorkRequest(int value);

signals:
    void WorkDone(int value, int error);
};

#endif // CONOSCOPEWORKER_H
