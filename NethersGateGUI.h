#pragma once

#include <QtWidgets/QMainWindow>
#include <QStringList>
#include <QStandardItemModel>
#include "ui_NethersGateGUI.h"

class NethersGateGUI : public QMainWindow
{
    Q_OBJECT

public:
    NethersGateGUI(QWidget* parent = nullptr);
    ~NethersGateGUI();

private slots:
    void onSelectFilesClicked();
    void onEvadeClicked();
    void onSelectBinFilesClicked();
    void onExclusiveShellcodeOptionToggled();


private:
    Ui::NethersGateGUIClass ui;
    QStringList selectedFiles;              // List of selected files
    QStandardItemModel* filesListModel;     // Model for QListView
    QString selectedBinFile;
    QStandardItemModel* binFilesListModel;

    void updateFileListView();              // Updates the QListView with selected files
};
