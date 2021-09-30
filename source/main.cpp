/* Copyright (C) 2005-2018, UNIGINE. All rights reserved.
 *
 * This file is a part of the UNIGINE 2.7.2.1 SDK.
 *
 * Your use and / or redistribution of this software in source and / or
 * binary form, with or without modification, is subject to: (i) your
 * ongoing acceptance of and compliance with the terms and conditions of
 * the UNIGINE License Agreement; and (ii) your inclusion of this notice
 * in any version of this software that you use or redistribute.
 * A copy of the UNIGINE License Agreement is available by contacting
 * UNIGINE. at http://unigine.com/
 */
// Qt
#include "MainWindow.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>

int main(int argc, char *argv[])
{
	// Qt part
	QApplication app(argc, argv);

    /* Arguments parsing */
    const auto args = qApp->arguments();

    bool editableNames = false;
    if(args.contains("--editable"))
        editableNames = true;

    bool importModelsAndDocs = false;
    if(args.contains("--importable"))
        importModelsAndDocs = true;

    QString modelName;
    QString modelPath;
    auto pos = args.indexOf("--model");
    if(pos != -1)
    {
        modelName = args.at(pos + 1);
        modelPath = args.at(pos + 2);
    }

    QString sofficePath;
    pos = args.indexOf("--soffice");
    if(pos != -1)
        sofficePath = args.at(pos + 1);

    MainWindow window(editableNames, importModelsAndDocs, modelName, modelPath, sofficePath);

    window.setWindowTitle("maintenance");    
	window.move((QApplication::desktop()->screen()->width() - window.width()) / 2, (QApplication::desktop()->screen()->height() - window.height()) / 2);
	window.show();

    return app.exec();
}
