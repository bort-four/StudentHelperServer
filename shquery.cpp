#include <QDebug>
#include <QDataStream>
#include <QPixmap>

#include "studenthelpercommon.h"
#include "shquery.h"

// //// SHQueryBase

SHQueryBase::SHQueryBase(SHQueryType type)
    : _type(type)
{
}


SHQueryBase *SHQueryBase::fromQByteArray(const QByteArray &array)
{
    SHQueryType type = (SHQueryType)array[0];

    switch (type)
    {
        case SHQueryBase::SHQ_TEXT:
        {
            SHQText *queryPtr = new SHQText("");
            queryPtr->readQByteArray(array);

            return queryPtr;
        }
        case SHQueryBase::SHQ_CONTENT:
        {
            SHQContent *queryPtr = new SHQContent();
            queryPtr->readQByteArray(array);
            return queryPtr;
        }
        case SHQueryBase::SHQ_IMAGE_REQUEST:
        {
            SHQImageRequest *queryPtr = new SHQImageRequest();
            queryPtr->readQByteArray(array);
            return queryPtr;
        }
        case SHQueryBase::SHQ_IMAGE:
        {
            SHQImage *queryPtr = new SHQImage();
            queryPtr->readQByteArray(array);
            return queryPtr;
        }
        case SHQueryBase::SHQ_NULL:
        default:
            throw SHException(QString("SHQueryBase::fromQByteArray(): "
                                      "unknown query type %1").arg(type));

    }
}


SHQueryBase::SHQueryType SHQueryBase::getType() const
{
    return (SHQueryType)_type;
}


qint8 SHQueryBase::getTypeByte() const
{
    return _type;
}



// //// SHQImage

SHQImage::SHQImage()
    : SHQueryBase(SHQ_IMAGE)
{
}

QByteArray SHQImage::toQByteArray() const
{
    if (getImagePtr() == nullptr)
        throw SHException("SHQImage::toQByteArray(): "
                          "image pointer is null");

    QByteArray data;
    QDataStream stream(&data, QIODevice::ReadWrite);

    stream << getTypeByte();
    stream << getFileId();
    stream << *getImagePtr();

    return data;
}


void SHQImage::readQByteArray(const QByteArray &array)
{
    QDataStream stream(array);

    qint8 type;
    stream >> type;

    if ((SHQueryType)type != SHQ_IMAGE)
        throw SHException("SHQImage::readQByteArray(): "
                          "invalid query type");

    stream >> _fileId;
    _imagePtr = new QPixmap;
    stream >> (*_imagePtr);
}


QPixmap *SHQImage::getImagePtr() const
{
    return _imagePtr;
}

void SHQImage::setImagePtr(QPixmap *imagePtr)
{
    _imagePtr = imagePtr;
}

quint64 SHQImage::getFileId() const
{
    return _fileId;
}

void SHQImage::setFileId(const quint64 &fileId)
{
    _fileId = fileId;
}



// //// SHQImageRequest

SHQImageRequest::SHQImageRequest()
    : SHQueryBase(SHQ_IMAGE_REQUEST)
{
}


QByteArray SHQImageRequest::toQByteArray() const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::ReadWrite);

    stream << getTypeByte();
    stream << getFileId();

    return data;
}


void SHQImageRequest::readQByteArray(const QByteArray &array)
{
    QDataStream stream(array);

    qint8 type;
    stream >> type;

    if ((SHQueryType)type != SHQ_IMAGE_REQUEST)
        throw SHException("SHQImageRequest::readQByteArray(): "
                          "invalid query type");

    quint64 fileId;
    stream >> fileId;
    setFileId(fileId);
}


void SHQImageRequest::debugOutput() const
{
    qDebug() << "    Request for file bu id " << getFileId();
}


quint64 SHQImageRequest::getFileId() const
{
    return _fileId;
}

void SHQImageRequest::setFileId(const quint64 &fileId)
{
    _fileId = fileId;
}



// //// SHQContent

SHQContent::SHQContent()
    : SHQueryBase(SHQ_CONTENT)
{
}


QByteArray SHQContent::toQByteArray() const
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::ReadWrite);
    stream << getTypeByte();

    if (_rootFolderPtr == nullptr)
        throw SHException("SHQContent::toQByteArray(): "
                          "root folder pointer is null");

//    if (_fileListPtr == nullptr)
//        throw SHException("SHQContent::toQByteArray(): "
//                          "file list pointer is null");

    // write file list
    stream << (quint64)getFileList().size();

    for (auto filePtr : getFileList())
    {
        stream << filePtr->getName();
        stream << filePtr->getTagString();
    }

    // write file tree
    writeFolder(stream, getRootFolderPtr());

    return data;
}


void SHQContent::readQByteArray(const QByteArray &array)
{
    QDataStream stream(array);

    qint8 type;
    stream >> type;

    if ((SHQueryType)type != SHQ_CONTENT)
        throw SHException("SHQContent::readQByteArray(): "
                          "invalid query type");

    // read file list
    quint64 fileCt;
    stream >> fileCt;
    _fileList.clear();

    for (quint64 fileNum = 0; fileNum < fileCt; ++fileNum)
    {
        QString fileName, tagString;
        stream >> fileName >> tagString;

        File* filePtr = new File(fileName);
        filePtr->inputTagsFromString(tagString);
        _fileList.append(filePtr);
    }

    // read file tree
    _rootFolderPtr = new FolderItem("");
    readFolder(stream, _rootFolderPtr);
}


const QList<File *> &SHQContent::getFileList() const
{
    return _fileList;
}


void SHQContent::setFileList(const QList<File *> &fileList)
{
    _fileList = fileList;
}


FolderItem *SHQContent::getRootFolderPtr() const
{
    return _rootFolderPtr;
}


void SHQContent::setRootFolderPtr(FolderItem *rootFolderPtr)
{
    _rootFolderPtr = rootFolderPtr;
}


void SHQContent::writeFolder(QDataStream &stream, FolderItem* folderPtr) const
{
    if (folderPtr->getParent() == nullptr)
    {
        stream << (quint64)folderPtr->getChildCount();
        stream << (quint64)folderPtr->getChildFolderCount();
        stream << folderPtr->getName();
    }

    for (auto chFolderPtr : folderPtr->getFolders())
    {
        stream << (quint64)chFolderPtr->getChildCount();
        stream << (quint64)chFolderPtr->getChildFolderCount();
        stream << chFolderPtr->getName();

        writeFolder(stream, chFolderPtr);
    }

    for (auto chFilePtr : folderPtr->getFiles())
    {
        int fileId = getFileList().indexOf(chFilePtr->getFilePtr());

        if (fileId < 0)
            throw SHException("SHQFileTree::writeFolder(): "
                              "can't find file in global list");

        stream << (quint64)fileId;
    }
}


void SHQContent::readFolder(QDataStream &stream, FolderItem* folderPtr)
{
    quint64 childCt, folderChildCt;
    QString name;

    stream >> childCt >> folderChildCt >> name;
    folderPtr->setName(name);

    for (size_t chNum = 0; chNum < folderChildCt; ++chNum)
        readFolder(stream, new FolderItem("", folderPtr));

    for (size_t chNum = folderChildCt; chNum < childCt; ++chNum)
    {
        quint64 fileId;
        stream >> fileId;

        new FileItem(_fileList[fileId], folderPtr);
    }
}



// //// SHTextQuery

SHQText::SHQText(QString text)
    : SHQueryBase(SHQ_TEXT), _text(text)
{
}

QByteArray SHQText::toQByteArray() const
{
    QByteArray array;
    array.append(getTypeByte());
    array.append(_text.toUtf8());

    return array;
}


void SHQText::readQByteArray(const QByteArray &array)
{
    if ((SHQueryType)array[0] != SHQ_TEXT)
        throw SHException("readQByteArray(): invalid query type");

    _text = QString::fromUtf8(array.data() +1, array.size() - 1);
}


void SHQText::debugOutput() const
{
    qDebug() << "    " << _text;
}



// //// FrameReader

FrameReader::FrameReader(QTcpSocket *socketPtr)
{
    setSocketPtr(socketPtr);
    _frames.append(QByteArray());
}


FrameReader::FrameReader(const FrameReader &other)
    : QObject(), _frames(other._frames)
    , _currFrameSize(other._currFrameSize)
{
    setSocketPtr(other._socketPtr);
}


FrameReader::~FrameReader()
{
}

FrameReader &FrameReader::operator=(const FrameReader &other)
{
    setSocketPtr(other._socketPtr);
    _currFrameSize = other._currFrameSize;
    _frames = other._frames;

    return *this;
}


QByteArray FrameReader::getFrameData()
{
    if (_frames.count() > 1)
    {
        QByteArray frame = _frames[0];
        _frames.erase(_frames.begin());
        return frame.mid(sizeof(_currFrameSize));
    }
    else
        throw SHException("FrameReader::getFrameData(): "
                          "try to get not ready frame");
}


void FrameReader::writeData(const QByteArray &data)
{
    if (data.size() == 0)
        throw SHException("FrameReader::writeFrame(): "
                          "frame size must be > 0");

    QByteArray head;
    QDataStream stream(&head, QIODevice::ReadWrite);

    stream << (quint64)data.size();

    _socketPtr->write(head);
    _socketPtr->write(data);
}


void FrameReader::onReadyRead()
{
    acceptData(_socketPtr->readAll());
}


void FrameReader::acceptData(const QByteArray &data)
{
    getCurrFrame().append(data);

    // read frame size if possible
    if (_currFrameSize == 0 && (size_t)getCurrFrame().size() > HEAD_SIZE)
    {
        QDataStream stream(getCurrFrame());
        stream >> _currFrameSize;
    }

    // extract data of another frames
    if (_currFrameSize > 0
            && (size_t)getCurrFrame().size() >= _currFrameSize + HEAD_SIZE)
    {
        QByteArray extraData = getCurrFrame().mid(_currFrameSize + HEAD_SIZE);
        getCurrFrame() = getCurrFrame().mid(0, _currFrameSize + HEAD_SIZE);

        _frames.append(QByteArray());
        _currFrameSize = 0;

        emit hasReadyFrame();

        acceptData(extraData);
    }
}


QByteArray &FrameReader::getCurrFrame()
{
    return _frames[_frames.count() - 1];
}


const QTcpSocket *FrameReader::getSocketPtr() const
{
    return _socketPtr;
}

void FrameReader::setSocketPtr(QTcpSocket *socketPtr)
{
    _socketPtr = socketPtr;

    if (_socketPtr != nullptr)
        connect(_socketPtr, SIGNAL(readyRead()),
                this,       SLOT(onReadyRead()));
}
