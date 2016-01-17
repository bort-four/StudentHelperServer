#include <QApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

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

    /*
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("db.sqlite");

    if (!db.open())
    {
        qDebug() << db.lastError().text();
        return 1;
    }

    db.exec("create table `table1` ("
            "`id` int not null primary key,"
            "`name` varchar(64) not null default ''"
            ");");

    if (db.lastError().type() != QSqlError::NoError)
    {
        qDebug() << "Can't create table: " << db.lastError().text();
        return 1;
    }

    db.exec("insert into `table1` values"
            "(1, 'abc'),"
            "(2, 'def'),"
            "(3, 'qwerty');");

    if (db.lastError().type() != QSqlError::NoError)
    {
        qDebug() << db.lastError().text();
//        return 1;
    }

    QSqlQuery query = db.exec("select * from table1;");

    if (db.lastError().type() != QSqlError::NoError)
    {
        qDebug() << db.lastError().text();
        return 1;
    }

    while (query.next())
    {
        qDebug() << query.value(0).toInt() << query.value(1).toString();
    }*/

    return a.exec();
}
