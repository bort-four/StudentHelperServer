#ifndef FILETREEITEM_H
#define FILETREEITEM_H

#include <QObject>
#include <QString>


class FolderItem;
class FileItem;


class File : public QObject
{
    Q_OBJECT

public:
    File(QString name, QString uuid = "");
    ~File();

    QString getName() const;
//    QString getFullName() const;
    QString getTagString() const;
    const QStringList* getTagListPtr() const;
    const QStringList &getTags() const;

    const QPixmap* getImage() const;
    QPixmap* getImage();
    void setPixmap(QPixmap* pixmapPtr);

    void inputTagsFromString(const QString &tagString);
    void addTag(QString tag);

    int getLinkCount() const;
    void setLinkCount(int newCount);

    QString getUuid() const;
    void setUuid(const QString &uuid);

signals:
    void tagsChenged();
    void imageChanged();

private:
    QString /*_fullName,*/ _name, _uuid;
    QStringList _tags;
    QPixmap* _pixMapPtr = nullptr;
    int _linkCount = 0;
};



class FileTreeItem : public QObject
{
    Q_OBJECT

public:

    virtual ~FileTreeItem() {}

    virtual bool isFile() const { return false; }
    virtual bool isFolder() const { return false; }

    virtual FolderItem* toFolder();
    virtual FileItem* toFile();

    virtual QString getName() const;

    virtual FileTreeItem* getChild(int num);
    virtual const FileTreeItem* getChild(int num) const;
    virtual FolderItem* getParent();
    virtual int getChildCount() const;

    virtual bool addChild(FileTreeItem* childPtr);
    virtual bool removeChild(FileTreeItem* childPtr);

    virtual void debugOutput(int space = 0) = 0;

protected:
    FileTreeItem(QObject* parPtr = 0);
};



class FolderItem : public FileTreeItem
{
    Q_OBJECT

public:
    FolderItem(QString name, QObject* parPtr = 0);

    virtual ~FolderItem();

    virtual bool isFolder() const;
    virtual FolderItem* toFolder();

    virtual FileTreeItem* getChild(int num);
    virtual const FileTreeItem* getChild(int num) const;
    virtual int getChildCount() const;

    virtual bool addChild(FileTreeItem* childPtr);
    virtual bool removeChild(FileTreeItem* childPtr);

    virtual QString getName() const;
    void setName(QString newName);

    int getChildFolderCount() const;

    virtual void debugOutput(int space = 0);

    const QList<FolderItem *> &getFolders() const;
    const QList<FileItem *> &getFiles() const;

signals:
    void nameChanged(QString);
    void structureChanged();
    void contentEdited(bool isFolder);
    void fileAdded(File *);
    void fileRemoved(File *);

private slots:
    void onNameChanged(QString);

private:
    bool removeChildRecursive(FileTreeItem* childPtr);

    QList<FolderItem *> _folders;
    QList<FileItem *> _files;
};



class FileItem : public FileTreeItem
{
    Q_OBJECT

public:
    FileItem(File* filePtr, QObject* parPtr = 0);

    virtual ~FileItem();

    virtual bool isFile() const;

    virtual FileItem* toFile();

    virtual QString getName();
    File* getFilePtr();
    const File* getFilePtr() const;

    virtual void debugOutput(int space = 0);

signals:
    void fileTagsChanged();

private:
    File* _filePtr;
};



#endif // FILETREEITEM_H
