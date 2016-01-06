#ifndef SHQUERY_H
#define SHQUERY_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QTcpSocket>

#include "filetreeitem.h"


class FrameReader : public QObject
{
    Q_OBJECT

public:
    FrameReader(QTcpSocket *getSocketPtr = nullptr);
    FrameReader(const FrameReader &other);
    ~FrameReader();

    FrameReader &operator=(const FrameReader &other);

    QByteArray getFrameData();

    void writeData(const QByteArray &data);

    const QTcpSocket *getSocketPtr() const;
    void setSocketPtr(QTcpSocket *getSocketPtr);

    void acceptData(const QByteArray &data);

signals:
    void hasReadyFrame();

private slots:
    void onReadyRead();

private:
    QByteArray &getCurrFrame();

    QList<QByteArray> _frames;
    QTcpSocket *_socketPtr = nullptr;
    quint64 _currFrameSize = 0;

    const size_t HEAD_SIZE = sizeof(quint64);
};



class SHQueryBase
{
public:
    virtual QByteArray toQByteArray() const = 0;
    static SHQueryBase* fromQByteArray(const QByteArray &array);
    virtual void readQByteArray(const QByteArray &array) = 0;
    virtual void debugOutput() const {}

    virtual ~SHQueryBase() {}

    enum SHQueryType {SHQ_NULL, SHQ_TEXT, SHQ_CONTENT, SHQ_IMAGE_REQUEST, SHQ_IMAGE};

protected:
    SHQueryBase(SHQueryType type);

    SHQueryType getType() const;
    qint8 getTypeByte() const;

private:
    const qint8 _type;
};



class SHQImage : public SHQueryBase
{
public:
    SHQImage();
    QByteArray toQByteArray() const;
    virtual void readQByteArray(const QByteArray &array);

    QPixmap *getImagePtr() const;
    void setImagePtr(QPixmap *getImagePtr);

    quint64 getFileId() const;
    void setFileId(const quint64 &fileId);

private:
    quint64 _fileId;
    QPixmap *_imagePtr = nullptr;
};



class SHQImageRequest : public SHQueryBase
{
public:
    SHQImageRequest();
    QByteArray toQByteArray() const;
    virtual void readQByteArray(const QByteArray &array);

    quint64 getFileId() const;
    void setFileId(const quint64 &getFileId);

    virtual void debugOutput() const;

private:
    quint64 _fileId;
};



class SHQContent : public SHQueryBase
{
public:
    SHQContent();
    QByteArray toQByteArray() const;
    virtual void readQByteArray(const QByteArray &array);

    const QList<File *> &getFileList() const;
    void setFileList(const QList<File *> &getFileList);

    FolderItem *getRootFolderPtr() const;
    void setRootFolderPtr(FolderItem *getRootFolderPtr);

private:
    void writeFolder(QDataStream &stream, FolderItem* folderPtr) const;
    void readFolder(QDataStream &stream, FolderItem* folderPtr);

    QList<File *> _fileList;
    FolderItem *_rootFolderPtr;
};



class SHQText : public SHQueryBase
{
public:
    SHQText(QString text);

    QByteArray toQByteArray() const;
    virtual void readQByteArray(const QByteArray &array);

    virtual void debugOutput() const;

private:
    QString _text;
};


#endif // SHQUERY_H



