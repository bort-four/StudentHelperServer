#ifndef STUDENTHELPERSERVER_H
#define STUDENTHELPERSERVER_H

#include <QObject>
#include <QDebug>
#include <QTcpServer>

#include "filetreeitem.h"
#include "studenthelpercontent.h"
#include "shquery.h"


struct Client
{
//    QTcpSocket* _soketPtr;
    FrameReader _reader;
    size_t _id;

    bool operator==(const Client &other) const
    { return _id == other._id; }
};



class StudentHelperServer : public QObject
{
    Q_OBJECT
public:
    explicit StudentHelperServer(QObject *parent = 0);
    ~StudentHelperServer();

signals:
private slots:
    void onNewConnection();
    void onSocketDisconnected();
//    void onReadyRead();
    void onFrameIsReady();

private:
    StudentHelperContent _content;
    Client *getClientBySocketPtr(const QTcpSocket *socketPtr);

    QTcpServer _qserver;
    QList<Client *> _clientPtrs;
    size_t _newConnectionId = 0;
};

#endif // STUDENTHELPERSERVER_H
