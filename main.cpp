#include <QApplication>
#include "studenthelperserver.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    MainWindow w;
//    w.show();

    new StudentHelperServer /*server*/(&a);

//    QTcpServer server;
//    server.listen(QHostAddress::Any, 1234);

//    char msg[50];
//    server >> msg;

//    qDebug() <<

    return a.exec();
}
