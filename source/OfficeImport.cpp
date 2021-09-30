#include "OfficeImport.h"

#include <QFileInfo>
#include <QFile>

#include <UnigineLog.h>

using namespace Unigine;

OfficeImport::OfficeImport(const QString& sofficePath)
    : _validStatus(false)
{
    QFileInfo info(sofficePath);
    if(!info.isFile() || !info.isExecutable())
        return;

    _programPath = sofficePath;

    _validStatus = true;
}

bool OfficeImport::import(const QString &pathToFile, const QString &outputDir)
{
    _importResult.clear();

    QFileInfo info(pathToFile);
    if(!info.isFile())
    {
        Log::warning("File %s does not exist\n", pathToFile.toStdString().c_str());
        return false;
    }

    QStringList args;
    args << "--headless" << "--convert-to" << "html:XHTML Writer File:UTF8"
         << "--outdir" << outputDir << pathToFile;

    _sofficeProc.start(_programPath, args);
    if(!_sofficeProc.waitForStarted(5000))
    {
        Log::warning("Import process start timeout.\n");
        return false;
    }

    if (!_sofficeProc.waitForFinished())
    {
        Log::warning("Import process finish timeout.\n");
        return false;
    }

    /* Load result */
    QFile html(outputDir + "/" + info.baseName() + ".html");
    if(!html.open(QFile::ReadOnly | QFile::Text))
    {
        Log::warning("Read from imported file failed.\n");
        return false;
    }

    _importResult = html.readAll();

    html.close();
    return true;
}
