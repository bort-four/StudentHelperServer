#ifndef STUDENTHELPER_H
#define STUDENTHELPER_H

#include <QObject>
#include <QSettings>

#include "filetreeitem.h"

static const QString ROOT_FOLDER_NAME = "Все разделы";


class StudentHelperContent : public QObject
{
    Q_OBJECT

public:
    StudentHelperContent(QObject* parPtr = 0);
    ~StudentHelperContent();

    // getters
    FolderItem* getRootFolder();
    const FolderItem* getRootFolder() const;

//    QList<File*>* getFileListPtr();

    void addFile(File* filePtr, FolderItem* folderPtr = nullptr);
//    File* findFileByName(const QString& name);
//    const File* findFileByName(const QString& name) const;

    const File *findFileByUuid(const QString &uuid) const;

    int getFileId(const File *file_ptr);

    void saveSettings();
    void loadSettings();

    const QList<File *> &getFileList() const;
    QList<File *> &getFileList();

    void debugOutput() const;

    void setFileList(const QList<File *> &fileList);
    void setRootFolderPtr(FolderItem *rootFolderPtr);

private slots:
    void onFileAdded(File *filePtr);
    void onFileRemoved(File *filePtr);
    void onTreeContentEdited(bool isFolder);

signals:
    void sendToPrint(File*);
    void contentEdited();

private:
    void writeFolderSettings(QSettings &settings, FolderItem *folderPtr);
    void readFolderSettings(QSettings &settings, FolderItem *folderPtr);

    QList<File*> _fileList;
    FolderItem*  _rootFolderPtr = nullptr;
};




#endif // STUDENTHELPER_H
