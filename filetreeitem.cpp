#include <QPixmap>
#include <QFileInfo>
#include <QDebug>

#include "filetreeitem.h"


// //// Implementation of File

File::File(QString fullFileName)
    : _fullName(fullFileName), _pixMapPtr(nullptr), _linkCount(0)
{
    QFileInfo info(fullFileName);

    if (info.exists() && info.isFile())
        _pixMapPtr = new QPixmap(fullFileName);

    int i = _fullName.size() - 1;
    for (; i >= 0 && _fullName[i] != '\\' && _fullName[i] != '/'; --i);
    _name = _fullName.mid(i + 1);

    emit imageChanged();
}

File::~File()
{
    if (_pixMapPtr != nullptr)
        delete _pixMapPtr;
}

QString File::getName() const
{
    return _name;
}

QString File::getFullName() const
{
    return _fullName;
}

const QStringList *File::getTagListPtr() const
{
    return &_tags;
}

const QStringList &File::getTags() const
{
    return _tags;
}


const QPixmap *File::getImage() const
{
    return _pixMapPtr;
}

QPixmap *File::getImage()
{
    return _pixMapPtr;
}

void File::setPixmap(QPixmap *pixmapPtr)
{
    _pixMapPtr = pixmapPtr;

    emit imageChanged();
}


QString File::getTagString() const
{
    QString str;

    for (int i = 0; i < _tags.count(); ++i)
    {
        str += _tags.at(i);

        if (i < _tags.count() - 1)
            str += ", ";
    }

    return str;
}

void File::inputTagsFromString(const QString &tagString)
{
    QStringList tags = tagString.split(',');

    _tags.clear();

    foreach (QString tag, tags)
    {
        QString newTag = tag.trimmed();

        if (newTag != "")
            _tags.append(newTag);
    }

    emit tagsChenged();
}


void File::addTag(QString tag)
{
    _tags.append(tag);

    emit tagsChenged();
}


int File::getLinkCount() const
{
    return _linkCount;
}

void File::setLinkCount(int newCount)
{
    _linkCount = newCount;
}





// //// Implementation of FileTreeItem

FileTreeItem::FileTreeItem(QObject *parPtr)
    : QObject(parPtr)
{
}


FolderItem *FileTreeItem::toFolder()
{
    throw QString("Invalid cast to FolderItem*");
}

FileItem *FileTreeItem::toFile()
{
    throw QString("Invalid cast to FileItem*");
}


/*
QVariant FileTreeItem::getValue() const { return QVariant(); }

bool FileTreeItem::setValue(QVariant) { return false; }
*/

QString FileTreeItem::getName() const { return objectName(); }

FileTreeItem *FileTreeItem::getChild(int) { throw QString("Try to get child from not a folder"); }

const FileTreeItem *FileTreeItem::getChild(int /*num*/) const
    { throw QString("Try to get child from not a folder"); }

FolderItem *FileTreeItem::getParent() { return dynamic_cast<FolderItem*>(parent()); }

int FileTreeItem::getChildCount() const { return 0; }

bool FileTreeItem::addChild(FileTreeItem *) { throw QString("Try to add child into not a folder"); }

bool FileTreeItem::removeChild(FileTreeItem *) { throw QString("Try to remove child from not a folder"); }


/*
void FileTreeItem::debbugOutput(int space)
{
    QString str;

    for (int i = 0; i < space; i++)
        str += "  ";

    if (isFolder())
        str += "[folder] ";

    str += getName();

    qDebug() << str;

    for (int i = 0; i < getChildCount(); ++i)
        getChild(i)->debbugOutput(space + 1);
}
*/

/*
FileTreeItem::SelectionState FileTreeItem::getSelectionState() const
{
    bool hasSelected = false;
    bool hasNotSelected = false;

    for (int chNum = 0; chNum < getChildCount(); ++chNum)
    {
        SelectionState state = getChild(chNum)->getSelectionState();

        hasSelected     |= state != NOT_SELECTED;
        hasNotSelected  |= state != SELECTED;
    }

    return (hasSelected && hasNotSelected) ? PARTIALLY_SELECTED
                                           : (hasSelected) ? SELECTED
                                                           : NOT_SELECTED;
}
*/



// //// Implementation of FolderItem

FolderItem::FolderItem(QString name, QObject *parPtr) : FileTreeItem(parPtr)
{
    setObjectName(name);
    emit nameChanged(name);

    if (dynamic_cast<FileTreeItem*>(parPtr) != NULL)
        dynamic_cast<FileTreeItem*>(parPtr)->addChild(this);
}

FolderItem::~FolderItem()
{
    /*
    foreach (FileItem *filePtr, _files)
        delete filePtr;

    foreach (FolderItem *folderPtr, _folders)
        delete folderPtr;
    */
}

bool FolderItem::isFolder() const { return true; }

FolderItem *FolderItem::toFolder()
{
    return this;
}

FileTreeItem *FolderItem::getChild(int num)
{
//    return dynamic_cast<FileTreeItem*>(children().at(num));

    if (num < 0 || num >= getChildCount())
        throw QString("FolderItem::getChild(): index out of range");

    if (num < _folders.size())
        return _folders[num];

    return _files[num - _folders.size()];
}

const FileTreeItem *FolderItem::getChild(int num) const
{
//    return dynamic_cast<const FileTreeItem*>(children().at(num));

    if (num < 0 || num >= getChildCount())
        throw QString("FolderItem::getChild(): index out of range");

    if (num < _folders.size())
        return _folders[num];

    return _files[num - _folders.size()];
}


int FolderItem::getChildCount() const
{
    return _folders.size() + _files.size();
}


bool FolderItem::addChild(FileTreeItem *childPtr)
{
    if (childPtr->isFolder())
    {
        FolderItem *folderPtr = childPtr->toFolder();
        _folders.append(folderPtr);

        connect(folderPtr,  SIGNAL(fileAdded(File*)),
                this,       SIGNAL(fileAdded(File*)));

        connect(folderPtr,  SIGNAL(fileRemoved(File*)),
                this,       SIGNAL(fileRemoved(File*)), Qt::QueuedConnection);
    }
    else if (childPtr->isFile())
    {
        _files.append(childPtr->toFile());

        emit fileAdded(childPtr->toFile()->getFilePtr());
    }
    else
        return false;

    childPtr->setParent(this);

    /*
    connect(childPtr,   SIGNAL(selectionStateCnahged(FileTreeItem::SelectionState)),
            this,       SIGNAL(selectionStateCnahged(FileTreeItem::SelectionState)));
    */

    emit structureChanged();
    return true;
}

bool FolderItem::removeChild(FileTreeItem *childPtr)
{
    return removeChildRecursive(childPtr);
}

QString FolderItem::getName() const
{
    return objectName();
}

void FolderItem::setName(QString newName)
{
    setObjectName(newName);
    emit nameChanged(newName);
}

int FolderItem::getChildFolderCount() const
{
    return _folders.count();
}


void FolderItem::debugOutput(int space)
{
    QString str;

    for (int i = 0; i < space; i++)
        str += "  ";

    if (isFolder())
        str += "[folder] ";

    str += getName();

    qDebug() << str;

    for (int i = 0; i < getChildCount(); ++i)
        getChild(i)->debugOutput(space + 1);
}


bool FolderItem::removeChildRecursive(FileTreeItem *childPtr)
{
    if (childPtr->isFolder() && _folders.indexOf(childPtr->toFolder()) == -1)
        return false;

    if (childPtr->isFile() && _files.indexOf(childPtr->toFile()) == 1)
        return false;

//    childPtr->setParent(NULL);

    if (childPtr->isFolder())
    {
        _folders.removeAll(childPtr->toFolder());
        FolderItem *folderPtr = childPtr->toFolder();

        for (int i = 0; i < folderPtr->getChildCount(); ++i)
//            if (folderPtr->getChild(i)->isFolder())
            folderPtr->removeChildRecursive(folderPtr->getChild(i));

        delete childPtr;
    }
    else if (childPtr->toFile())
    {
        _files.removeAll(childPtr->toFile());
        File *filePtr = childPtr->toFile()->getFilePtr();

        delete childPtr;

        emit fileRemoved(filePtr);
    }
    else
        return false;

    emit structureChanged();
    return true;
}

const QList<FileItem *> &FolderItem::getFiles() const
{
    return _files;
}

const QList<FolderItem *> &FolderItem::getFolders() const
{
    return _folders;
}

/*
void FolderItem::setSelectionRecursive(bool selection)
{
    for (int chNum = 0; chNum < getChildCount(); ++chNum)
        if (getChild(chNum)->isFile())
            getChild(chNum)->toFile()->getFilePtr()->setSelectedToPrint(selection);
        else if (getChild(chNum)->isFolder())
            getChild(chNum)->toFolder()->setSelectionRecursive(selection);
}

void FolderItem::onChildCelectionStateChanged(FileTreeItem::SelectionState state)
{
    emit selectionStateCnahged(getSelectionState());
}
*/


// //// Implementation of FileItem

FileItem::FileItem(File *filePtr, QObject *parPtr)
    : FileTreeItem(parPtr), _filePtr(filePtr)
{
    _filePtr->setLinkCount(_filePtr->getLinkCount() + 1);

    /*
    connect(_filePtr,   SIGNAL(selectionChenged(bool)),
            this,       SLOT(onFileSelectionChanged(bool)));
    */

    connect(_filePtr,   SIGNAL(tagsChenged()),
            this,       SIGNAL(fileTagsChanged()));

    if (dynamic_cast<FileTreeItem*>(parPtr) != NULL)
        dynamic_cast<FileTreeItem*>(parPtr)->addChild(this);
}

FileItem::~FileItem()
{
    _filePtr->setLinkCount(_filePtr->getLinkCount() - 1);
}

bool FileItem::isFile() const { return true; }

FileItem *FileItem::toFile()
{
    return this;
}

QString FileItem::getName()
{
    return getFilePtr()->getName();
}

File *FileItem::getFilePtr()
{
    return _filePtr;
}

const File *FileItem::getFilePtr() const
{
    return _filePtr;
}

/*
FileTreeItem::SelectionState FileItem::getSelectionState() const
{
    return getFilePtr()->isSelectedToPrint() ? SELECTED : NOT_SELECTED;
}

void FileItem::onFileSelectionChanged(bool isSelected)
{
//    qDebug() << "FileItem::onFileSelectionChanged";
    emit selectionStateCnahged(getSelectionState());
}
*/

/*
bool FileItem::isSelected() const { return _isSelected; }

bool FileItem::setSelection(bool selection) { return _isSelected = selection; }

const QStringList &FileItem::getTags() const { return _tagList; }

QStringList &FileItem::getTags() { return _tagList; }
*/


void FileItem::debugOutput(int space)
{
    QString str;

    for (int i = 0; i < space; i++)
        str += "  ";

    if (isFolder())
        str += "[file] ";

    str += getName();

    qDebug() << str;

    for (int i = 0; i < getChildCount(); ++i)
        getChild(i)->debugOutput(space + 1);
}
