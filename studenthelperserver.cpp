#include <QTcpSocket>
#include <QDir>
#include <QImage>
#include <QPixmap>

#include "studenthelpercommon.h"
#include "studenthelperserver.h"
#include "shquery.h"


StudentHelperServer::StudentHelperServer(QObject *parent)
try
    : QObject(parent)
{
    _folderPath = QDir::homePath()  + QDir::separator()
            + "StudentHelperServer" + QDir::separator();

    StudentHelperContent *contentPtr = new StudentHelperContent();
    contentPtr->loadSettings();
    contentPtr->debugOutput();
    setContentPtr(contentPtr);

    qDebug() << "Server started";
    qDebug() << "Wait for connections...";

    _qserver.listen(QHostAddress::Any, 4321);

    connect(&_qserver,  SIGNAL(newConnection()),
            this,       SLOT(onNewConnection()));

    deleteGarbage();
}
catch (SHException exc)
{
    qDebug() << exc.getMsg();
}
catch (QString str)
{
    qDebug() << str;
}


StudentHelperServer::~StudentHelperServer()
{
}


void StudentHelperServer::onNewConnection()
{
    if (_qserver.hasPendingConnections())
    {
        Client *clientPtr = new Client;
        *clientPtr = {FrameReader(_qserver.nextPendingConnection()),
                      _newConnectionId++};
        _clientPtrs.append(clientPtr);

        qDebug() << "New connection" << clientPtr->_id;
        qDebug() << "    current client count:" << _clientPtrs.count();

        connect(clientPtr->_reader.getSocketPtr(),  SIGNAL(disconnected()),
                this,                               SLOT(onSocketDisconnected()));

        connect(&clientPtr->_reader,    SIGNAL(hasReadyFrame()),
                this,                   SLOT(onFrameIsReady()));

        // send content
        SHQContent query;
        query.setFileList(_contentPtr->getFileList());
        query.setRootFolderPtr(_contentPtr->getRootFolder());

        clientPtr->_reader.writeData(query.toQByteArray());
    }
    else
        qDebug() << "Can't find new connection";
}


void StudentHelperServer::onSocketDisconnected()
{
    QTcpSocket *socketPtr = dynamic_cast<QTcpSocket *>(sender());

    if (socketPtr == NULL)
        return;

    Client *clientPtr = getClientBySocketPtr(socketPtr);
    _clientPtrs.removeAll(clientPtr);
    qDebug() << "client" << clientPtr->_id << "disconnected";
    qDebug() << "    current client count:" << _clientPtrs.count();
}



void StudentHelperServer::onFrameIsReady()
{
    FrameReader *readerPtr = dynamic_cast<FrameReader *>(sender());

    if (readerPtr == nullptr)
    {
        qDebug() << "Sender is not FrameReader";
        return;
    }

    Client *clientPtr = getClientBySocketPtr(readerPtr->getSocketPtr());
    SHQueryBase *queryPtr = nullptr;

    try
    {
        queryPtr = SHQueryBase::fromQByteArray(readerPtr->getFrameData());
        qDebug() << "Query from client" << clientPtr->_id;
        queryPtr->debugOutput();

        // read updated content
        if (dynamic_cast<SHQContent *>(queryPtr) != nullptr)
        {
            SHQContent *contentQueryPtr = dynamic_cast<SHQContent *>(queryPtr);
            StudentHelperContent *contentPtr = new StudentHelperContent(this);

            contentPtr->setFileList(contentQueryPtr->getFileList());
            contentPtr->setRootFolderPtr(contentQueryPtr->getRootFolderPtr());

            setContentPtr(contentPtr);
            _contentPtr->saveSettings();

            // request for new files
            for (auto filePtr : _contentPtr->getFileList())
            {
                if (filePtr->getImage() == nullptr)
                {
                    SHQImageRequest request;
                    request.setFileId(_contentPtr->getFileList().indexOf(filePtr));
                    readerPtr->writeData(request.toQByteArray());
                }

            }
        }

        // if there is image from client
        if (dynamic_cast<SHQImage *>(queryPtr) != nullptr)
        {
            QDir dir(_folderPath);

            if (!dir.exists() && !dir.mkpath("."))
                throw SHException("Can't create pictures folder");

            SHQImage *imageQueryPtr = dynamic_cast<SHQImage *>(queryPtr);

            File *filePtr = _contentPtr->getFileList()[imageQueryPtr->getFileId()];

            filePtr->setPixmap(imageQueryPtr->getImagePtr());

            QImage image = imageQueryPtr->getImagePtr()->toImage();

            if (!image.save(getFilePath(filePtr)))
                throw SHException("Can't create picture file");
        }

        // if there is image request
        if (dynamic_cast<SHQImageRequest *>(queryPtr) != nullptr)
        {
            SHQImageRequest *requestPtr = dynamic_cast<SHQImageRequest *>(queryPtr);
            SHQImage answer;

            answer.setFileId(requestPtr->getFileId());
            answer.setImagePtr(_contentPtr->getFileList()[requestPtr->getFileId()]->getImage());

            qDebug() << requestPtr->getFileId();

            // send image to client
            readerPtr->writeData(answer.toQByteArray());
        }
    }
    catch (SHException exc)
    {
        qDebug() << exc.getMsg();
    }

    if (queryPtr != nullptr)
        delete queryPtr;
}

QString StudentHelperServer::getFilePath(const File *filePtr) const
{
    return _folderPath + filePtr->getUuid() + ".jpg";
}


void StudentHelperServer::deleteGarbage() const
{
    QDir folder(_folderPath);
    QStringList fileList = folder.entryList(QStringList() << "*.jpg");

    for (QString fileName : fileList)
    {
        QString uuid = fileName.split(QDir::separator()).last().split('.').first();

        if (_contentPtr->findFileByUuid(uuid) == nullptr)
            QFile(_folderPath + fileName).remove();
    }
}


StudentHelperContent *StudentHelperServer::getContentPtr() const
{
    return _contentPtr;
}

void StudentHelperServer::setContentPtr(StudentHelperContent *contentPtr)
{
    if (_contentPtr != nullptr)
        delete _contentPtr;

    _contentPtr = contentPtr;

    for (auto filePtr : _contentPtr->getFileList())
    {
        QString path = getFilePath(filePtr);

        if (QFile().exists(path))
            filePtr->setPixmap(new QPixmap(path));

//        if (filePtr->getImage() == nullptr)
//            qDebug() << "Can't fing image by uuid" << filePtr->getUuid();
    }
}


/*
void StudentHelperServer::onReadyRead()
{
    QTcpSocket *socketPtr = dynamic_cast<QTcpSocket *>(sender());

    if (socketPtr == NULL)
        return;

    Client &client = getClientBySocketPtr(socketPtr);
    SHQueryBase *queryPtr = nullptr;

    try
    {
        queryPtr = SHQueryBase::fromQByteArray(socketPtr->readAll());
        qDebug() << "Query from client" << client._id;
        queryPtr->debugOutput();

        // if there is image request
        if (dynamic_cast<SHQImageRequest *>(queryPtr) != nullptr)
        {
            SHQImageRequest *requestPtr = dynamic_cast<SHQImageRequest *>(queryPtr);
            SHQImage answer;

            answer.setFileId(requestPtr->getFileId());
            answer.setImagePtr(_content.getFileList()[requestPtr->getFileId()]->getImage());

            // send image to client
            socketPtr->write(answer.toQByteArray());
        }
    }
    catch (SHException exc)
    {
        qDebug() << exc.getMsg();
    }

    if (queryPtr != nullptr)
        delete queryPtr;
}
*/

Client *StudentHelperServer::getClientBySocketPtr(const QTcpSocket *socketPtr)
{
    for (auto &clientPtr : _clientPtrs)
        if (clientPtr->_reader.getSocketPtr() == socketPtr)
            return clientPtr;

    throw SHException("getClientBySocketPtr(): can't find client");
}



