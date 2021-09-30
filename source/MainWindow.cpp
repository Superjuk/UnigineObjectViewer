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


#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QLayout>
#include <QSplitter>
#include <QDebug>
#include <QFileDialog>

MainWindow::MainWindow(bool isEditable, bool isImport, QString modelName, QString modelPath, QString sofficePath, QWidget *parent)
	: QMainWindow(parent)
    , _ui(new Ui::MainWindow)
    , _isEditable(isEditable)
    , _isImport(isImport)
    , _modelName(modelName)
    , _modelPath(modelPath)
    , _sofficePath(sofficePath)
{
    _ui->setupUi(this);

    //! \todo define indexes in define.h
    _ui->splitter->setStretchFactor(0,1);
    _ui->splitter->setStretchFactor(1,6);
    _ui->splitter->setStretchFactor(2,1);

    _maintenance = _ui->maintenance3D;
    _maintenance->initEngine(_ui->centralWidget, "../data");

    _maintenance->setVisible(false);

    _resizeWidget = _ui->widget;
    _resizeWidget->setControlledWidget(_maintenance);

    _treeView = _ui->treeModelsParts;
    _treeView->setEditTriggers(
        _isEditable ? QAbstractItemView::DoubleClicked : QAbstractItemView::NoEditTriggers);

    const auto canLoadModel = _isImport & (_modelName.isEmpty() | _modelPath.isEmpty());
    _ui->btnLoadModel->setVisible(canLoadModel);
    _ui->btnSaveChanges->setVisible(_isEditable);

    _ui->editSectionDescription->setReadOnly(!_isEditable);

    _officeImporter = new OfficeImport(_sofficePath);

    const auto canLoadSection = _isImport & _officeImporter->isValid() & !canLoadModel;
    const auto canEditSection = _isEditable & !canLoadModel;
    _ui->btnLoadSection->setVisible(canLoadSection);
    _ui->btnSaveSection->setVisible(canEditSection);
    _ui->btnDeleteSection->setVisible(canEditSection);

    _ui->treeInitParts->hide();

    connect(_maintenance, &Maintenance::initComplete, this, &MainWindow::_onInitComplete);
    connect(_maintenance, &Maintenance::partSelected, this, &MainWindow::_onPartSelected);
    connect(_maintenance, &Maintenance::importComplete, this, &MainWindow::_onImportComplete);
    connect(_ui->btnBack, &QPushButton::clicked, this, &MainWindow::_onBackButtonPush);
    connect(_ui->btnLoadModel, &QPushButton::clicked, this, &MainWindow::_onFbxStpSelect);
    connect(_ui->btnSaveChanges, &QPushButton::clicked, this, &MainWindow::_onPartNamesSave);
    connect(_ui->btnLoadSection, &QPushButton::clicked, this, &MainWindow::_onDocumentSelect);
    connect(_ui->btnInitialNames, &QPushButton::clicked, this, &MainWindow::_onChangeInitialTreeVisibility);
}

MainWindow::~MainWindow()
{
    if(_officeImporter)
        delete _officeImporter;

    _maintenance->shutdown();    
    delete _ui;
}

void MainWindow::_updateTreeView(const QStringList &header, const QVariantList &items)
{
    auto oldModel = _treeView->model();
    auto model = new TreeModel(header, items);

    _treeView->setModel(model);
    for(int column = 0; column < model->columnCount(); ++column)
        _treeView->resizeColumnToContents(column);

    if(oldModel)
        delete oldModel;
}

void MainWindow::_updateUpTreeViewCheckState(const QModelIndex &index)
{
    auto parent = index.parent();
    if(parent == QModelIndex())
        return;

    const auto model = dynamic_cast<TreeModel*>(_treeView->model());
    const auto rowCount = model->rowCount(parent);
    int stateSum = 0;
    for(int row = 0; row < rowCount; ++row)
    {
        auto child =_treeView->model()->index(row, 0, parent);
        stateSum += child.data(Qt::CheckStateRole).toInt();
    }

    int parentState;
    if(stateSum == rowCount * Qt::Checked)
        parentState = Qt::Checked;
    else if(stateSum == Qt::Unchecked)
        parentState = Qt::Unchecked;
    else
        parentState = Qt::PartiallyChecked;

    model->setData(parent, QVariant(parentState), Qt::CheckStateRole);

    auto id = model->index(parent.row(), Ui::COLUMN_ID_INDEX, parent.parent()).data().toInt();
    _visibleUpdateMap[id] = parentState;

    _updateUpTreeViewCheckState(parent);

}

void MainWindow::_updateDownTreeViewCheckState(const QModelIndex &index, int state)
{
    const auto model = dynamic_cast<TreeModel*>(_treeView->model());
    if(!model->hasChildren(index))
        return;

    const auto rowCount = model->rowCount(index);
    for(int row = 0; row < rowCount; ++row)
    {
        auto child =model->index(row, Ui::COLUMN_DATA_INDEX, index);
        model->setData(child, QVariant(state), Qt::CheckStateRole);

        auto id = model->index(row, Ui::COLUMN_ID_INDEX, index).data().toInt();
        _visibleUpdateMap[id] = state;

        if(model->hasChildren(index))
            _updateDownTreeViewCheckState(child, state);
    }
}

void MainWindow::_updateCheckbox(const QModelIndex &index)
{
    const auto row = index.row();
    int state = index.data(Qt::CheckStateRole).toInt();
    auto id = index.model()->index(row, Ui::COLUMN_ID_INDEX, index.parent()).data().toInt();
    _visibleUpdateMap[id] = state;

    disconnect(_treeView->model(), &QAbstractItemModel::dataChanged, this, &MainWindow::_onTreeViewItemChange);
    _updateDownTreeViewCheckState(index, state);
    _updateUpTreeViewCheckState(index);
    connect(_treeView->model(), &QAbstractItemModel::dataChanged, this, &MainWindow::_onTreeViewItemChange);

    for(auto& pair : _visibleUpdateMap)
        _maintenance->setPartVisible(pair.first, pair.second);

    _visibleUpdateMap.clear();
}

void MainWindow::_updateName(const QModelIndex &index)
{
    const auto row = index.row();
    auto name = index.data(Qt::EditRole).toString();
    auto id = index.model()->index(row, Ui::COLUMN_ID_INDEX, index.parent()).data().toInt();    

    _ui->btnSaveChanges->setEnabled(_maintenance->setPartName(id, name));
}

void MainWindow::_onBackButtonPush()
{
    qApp->exit();
}

void MainWindow::_onPartSelect()
{
    auto list = _treeView->selectionModel()->selectedRows(Ui::COLUMN_ID_INDEX);
    const auto rowCount = list.count();
    if(rowCount == 1)
        _maintenance->viewPart(list.at(rowCount - 1).data().toInt());
}

void MainWindow::_onFbxStpSelect()
{
    const auto modelPath = QFileDialog::getOpenFileName(this, tr("Import 3D models:"), "",
                                                  tr("CAD models (*.iges *.stp *.step *.stl);;FBX models (*.fbx *.obj *.dae *.3ds)"));
    _maintenance->importModel(modelPath);
}

void MainWindow::_onDocumentSelect()
{
    const auto documentPath = QFileDialog::getOpenFileName(this, tr("Import office documents:"), "",
                                                        tr("Microsoft (*.doc *.docx);;Open documents (*.odt);;All (*.*)"));
    QFileInfo info(documentPath);
    auto dir = info.absoluteDir();
    dir.mkdir("temp");

    _officeImporter->import(documentPath, dir.absolutePath() + "/temp");

    _ui->editSectionDescription->setHtml(_officeImporter->getData());
}

void MainWindow::_onPartSelected(int partId)
{
    const auto id = QString::number(partId);
    auto model = _treeView->model();

    QModelIndex index = model->index(0, 1);
    QModelIndexList indexes = model->match(index, Qt::DisplayRole, QVariant(id),
                                                           1, Qt::MatchFlags(Qt::MatchRecursive |
                                                                          Qt::MatchFixedString |
                                                                          Qt::MatchStartsWith));
    if(!indexes.isEmpty())
    {
        index = indexes.at(0);
        _treeView->setCurrentIndex(index);
        _treeView->scrollTo(index);
    }
}

void MainWindow::_onTreeViewItemChange(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    //! in this case topLeft equal bottomRight
    for(auto& role : roles)
    {
        switch(role)
        {
        case Qt::CheckStateRole:
            _updateCheckbox(topLeft);
            break;
        case Qt::EditRole:
            _updateName(topLeft);
            break;
        default:
            break;
        }
    }
}

void MainWindow::_onPartNamesSave()
{
    _maintenance->savePartNames();

    _ui->btnSaveChanges->setEnabled(false);
}

void MainWindow::_onImportComplete(const QString& modelName, const QVariantList& parts)
{
    if(modelName.isEmpty() || parts.isEmpty())
        return;

    _ui->lblModelName->setText(modelName);

    _updateTreeView({tr("Parts"), tr("id")}, parts);
    _treeView->setColumnHidden(Ui::COLUMN_ID_INDEX, true);

    connect(_treeView, &QTreeView::clicked, this, &MainWindow::_onPartSelect);
    connect(_treeView->model(), &QAbstractItemModel::dataChanged, this, &MainWindow::_onTreeViewItemChange);

    _ui->btnLoadModel->hide();
    _ui->btnLoadSection->show();
    _ui->btnSaveSection->show();
    _ui->btnDeleteSection->show();
}

void MainWindow::_onChangeInitialTreeVisibility()
{
    if(_ui->treeInitParts->isVisible())
        _ui->treeInitParts->hide();
    else
        _ui->treeInitParts->show();
}

void MainWindow::_onInitComplete()
{
    _ui->lblModelName->setText(_modelName);

    _updateTreeView({tr("Parts"), tr("id")}, _maintenance->getParts(_modelPath));
    _treeView->setColumnHidden(Ui::COLUMN_ID_INDEX, true);

    // Initial names tree
    auto oldModel = _ui->treeInitParts->model();
    auto model = new TreeModel({tr("Initial names")}, _maintenance->getInitParts());

    _ui->treeInitParts->setModel(model);
    for(int column = 0; column < model->columnCount(); ++column)
        _ui->treeInitParts->resizeColumnToContents(column);

    if(oldModel)
        delete oldModel;

    connect(_treeView, &QTreeView::clicked, this, &MainWindow::_onPartSelect);
    connect(_treeView->model(), &QAbstractItemModel::dataChanged, this, &MainWindow::_onTreeViewItemChange);
}
