#pragma once

#include <QObject>
#include <QProcess>
#include <QString>

class OfficeImport
{
public:
    OfficeImport(const QString& sofficePath);

    //! Return of importer validity
    bool isValid() {return _validStatus;}

    //! Import office file to HTML in specified directory
    //! and return import status
    bool import(const QString& pathToFile, const QString& outputDir);

    //! Get import data.
    //! If \return QByteArray{} after import,
    //! import failed or document is empty
    QByteArray getData() { return _importResult; }

private:
    QProcess _sofficeProc;

    QString _programPath;

    bool _validStatus;

    QByteArray _importResult;
};
