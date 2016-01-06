//#include <QSettings>
#include <QDebug>

#include "studenthelpercommon.h"
#include "studenthelpercontent.h"
#include "filetreeitem.h"


// //// Inplementation of StudentHelperContent

StudentHelperContent::StudentHelperContent(QObject *parPtr)
    : QObject(parPtr), _rootFolderPtr(new FolderItem("root folder"))
{
    connect(_rootFolderPtr, SIGNAL(fileAdded(File*)),
            this,           SLOT(onFileAdded(File*)));

    connect(_rootFolderPtr, SIGNAL(fileRemoved(File*)),
            this,           SLOT(onFileRemoved(File*)), Qt::QueuedConnection);

    try
    {
//        loadSettings();
    }
    catch (SHException exc)
    {
        qDebug() << exc.getMsg();
    }
    catch (QString str)
    {
        qDebug() << str;
    }

}



StudentHelperContent::~StudentHelperContent()
{
//    saveSettings();

    for (auto filePtr : _fileList)
        delete filePtr;
}


FolderItem *StudentHelperContent::getRootFolder()
{
    return _rootFolderPtr;
}


const FolderItem *StudentHelperContent::getRootFolder() const
{
    return _rootFolderPtr;
}


void StudentHelperContent::addFile(File *filePtr, FolderItem *folderPtr)
{
    if (folderPtr != nullptr)
        folderPtr->addChild(new FileItem(filePtr));
    else
        _rootFolderPtr->addChild(new FileItem(filePtr));
}


File* StudentHelperContent::findFileByName(const QString& name)
{
    for (auto filePtr : getFileList())
        if (filePtr->getFullName() == name)
            return filePtr;

    return nullptr;
}


const File* StudentHelperContent::findFileByName(const QString& name) const
{
    for (auto filePtr : getFileList())
        if (filePtr->getFullName() == name)
            return filePtr;

    return nullptr;
}


void StudentHelperContent::saveSettings()
{
    QSettings settings("MMCS","StudentHelper");
    settings.clear();

    settings.setValue("globalFileListCount", _fileList.size());
    settings.beginWriteArray("globalFileList");

    for (int i = 0; i < _fileList.size(); ++i)
    {
        settings.setArrayIndex(i);
        settings.setValue("fullName", _fileList[i]->getFullName());
        settings.setValue("tagString", _fileList[i]->getTagString());
    }

    settings.endArray();

    writeFolderSettings(settings, _rootFolderPtr);
}


void StudentHelperContent::loadSettings()
{
    QSettings settings("MMCS","StudentHelper");

    int fileCount = settings.value("globalFileListCount", 0).toInt();
    _fileList.clear();

    settings.beginReadArray("globalFileList");

    for (int i = 0; i < fileCount; ++i)
    {
        settings.setArrayIndex(i);
        QString fileFullName = settings.value("fullName", "___").toString();
        File *filePtr = new File(fileFullName);
        filePtr->inputTagsFromString(settings.value("tagString", "").toString());

        _fileList.append(filePtr);
    }

    settings.endArray();

    readFolderSettings(settings, _rootFolderPtr);

    foreach (File *filePtr, _fileList)
        if (filePtr->getLinkCount() == 0)
            _fileList.removeAll(filePtr);
}


void StudentHelperContent::writeFolderSettings(QSettings &settings, FolderItem *folderPtr)
{
    // write child folders
    settings.setValue("folderCount", folderPtr->getChildFolderCount());
    settings.beginWriteArray("folders");

    for (int i = 0; i < folderPtr->getChildFolderCount(); ++i)
    {
        settings.setArrayIndex(i);
        FolderItem *childFolderPtr = folderPtr->getChild(i)->toFolder();

        if (childFolderPtr == nullptr)
            throw QString("StudentHelperContent::writeFolderSettings(): invalid child folder");

        settings.setValue("folderName", childFolderPtr->getName());

        writeFolderSettings(settings, childFolderPtr);
    }
    settings.endArray();

    // write child files
    settings.setValue("fileCount", folderPtr->getChildCount() - folderPtr->getChildFolderCount());
    settings.beginWriteArray("files");

    for (int i = folderPtr->getChildFolderCount(); i < folderPtr->getChildCount(); ++i)
    {
        settings.setArrayIndex(i - folderPtr->getChildFolderCount());
        FileItem *childFilePtr = folderPtr->getChild(i)->toFile();

        if (childFilePtr == nullptr)
            throw QString("StudentHelperContent::writeFolderSettings(): invalid child file");

        File *filePtr = childFilePtr->getFilePtr();
        int numInGlobalList = _fileList.indexOf(filePtr);

        if (numInGlobalList == -1)
            throw QString("StudentHelperContent::writeFolderSettings(): can't find file in global list");

        settings.setValue("numInGlobalList", numInGlobalList);
    }
    settings.endArray();
}


void StudentHelperContent::readFolderSettings(QSettings &settings, FolderItem *folderPtr)
{
    // read child folders
    int folderCount = settings.value("folderCount", 0).toInt();
    settings.beginReadArray("folders");

    for (int i = 0; i < folderCount; ++i)
    {
        settings.setArrayIndex(i);
        QString folderName = settings.value("folderName", "[]").toString();
        FolderItem *childFolderPtr = new FolderItem(folderName, folderPtr);

        readFolderSettings(settings, childFolderPtr);
    }
    settings.endArray();

    // read child files
    int fileCount = settings.value("fileCount", 0).toInt();
    settings.beginReadArray("files");

    for (int i = 0; i < fileCount; ++i)
    {
        settings.setArrayIndex(i);
        int numInGlobalList = settings.value("numInGlobalList", -1).toInt();

        if (numInGlobalList == -1)
            throw QString("StudentHelperContent::readFolderSettings(): invalid file num");

        File *filePtr = _fileList.at(numInGlobalList);
        new FileItem(filePtr, folderPtr);
    }
    settings.endArray();
}

void StudentHelperContent::setRootFolderPtr(FolderItem *rootFolderPtr)
{
    _rootFolderPtr = rootFolderPtr;
}

void StudentHelperContent::setFileList(const QList<File *> &fileList)
{
    _fileList = fileList;
}


const QList<File *> &StudentHelperContent::getFileList() const
{
    return _fileList;
}


QList<File *> &StudentHelperContent::getFileList()
{
    return _fileList;
}


void StudentHelperContent::debugOutput() const
{
    qDebug() << "Files:";

    for (auto filePtr : getFileList())
        qDebug() << "    " << filePtr->getName() << "-" << filePtr->getTagString();

    _rootFolderPtr->debugOutput();
}


void StudentHelperContent::onFileAdded(File *filePtr)
{
    if (_fileList.indexOf(filePtr) == -1)
        _fileList.append(filePtr);
}


void StudentHelperContent::onFileRemoved(File *filePtr)
{
    int linksCount = filePtr->getLinkCount();

    if (linksCount == 0)
        _fileList.removeAll(filePtr);
}
