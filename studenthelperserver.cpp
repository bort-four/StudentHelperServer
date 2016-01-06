#include <QTcpSocket>

#include "studenthelpercommon.h"
#include "studenthelperserver.h"
#include "shquery.h"


StudentHelperServer::StudentHelperServer(QObject *parent)
    : QObject(parent)
{
    try
    {
        _content.loadSettings();
        _content.debugOutput();
    }
    catch (SHException exc)
    {
        qDebug() << exc.getMsg();
    }
    catch (QString str)
    {
        qDebug() << str;
    }

    qDebug() << "Server started";
    qDebug() << "Wait for connections...";

    _qserver.listen(QHostAddress::Any, 1234);

    connect(&_qserver,  SIGNAL(newConnection()),
            this,       SLOT(onNewConnection()));
}


StudentHelperServer::~StudentHelperServer()
{
//    _content.saveSettings();
}


void StudentHelperServer::onNewConnection()
{
    if (_qserver.hasPendingConnections())
    {
//        QTcpSocket *socketPtr = _qserver.nextPendingConnection();
        Client *clientPtr = new Client;
        *clientPtr = {FrameReader(_qserver.nextPendingConnection()),
                      _newConnectionId++};
//        client._reader.setSocketPtr(socketPtr);
//        client._id = _newConnectionId++;
        _clientPtrs.append(clientPtr);

        qDebug() << "New connection" << clientPtr->_id;
        qDebug() << "    current client count:" << _clientPtrs.count();

        connect(clientPtr->_reader.getSocketPtr(),  SIGNAL(disconnected()),
                this,                               SLOT(onSocketDisconnected()));

//        connect(socketPtr,  SIGNAL(readyRead()),
//                this,       SLOT(onReadyRead()));

        connect(&clientPtr->_reader,    SIGNAL(hasReadyFrame()),
                this,                   SLOT(onFrameIsReady()));

        // send content
        SHQContent query;
        query.setFileList(_content.getFileList());
        query.setRootFolderPtr(_content.getRootFolder());

        clientPtr->_reader.writeData(query.toQByteArray());
//        socketPtr->write(query.toQByteArray());
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

        // if there is image request
        if (dynamic_cast<SHQImageRequest *>(queryPtr) != nullptr)
        {
            SHQImageRequest *requestPtr = dynamic_cast<SHQImageRequest *>(queryPtr);
            SHQImage answer;

            answer.setFileId(requestPtr->getFileId());
            answer.setImagePtr(_content.getFileList()[requestPtr->getFileId()]->getImage());

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



