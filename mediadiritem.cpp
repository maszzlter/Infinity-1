#include "mediadiritem.h"

MediaDirItem::MediaDirItem(QObject *parent) : QAbstractListModel(parent)
{
    m_roleNames.insert(RoleName, "name");
    m_roleNames.insert(RoleUrl, "url");
    m_roleNames.insert(RoleType, "type");
    createDataBase();
    mediaDirSql = new QSqlQuery(mediaDirDb);
    createDataTable();
}

int MediaDirItem::rowCount(const QModelIndex &parent) const
{
    return elementList.size();
}

QVariant MediaDirItem::data(const QModelIndex &index, int role) const
{
    Element ele = elementList.at(index.row());
    QVariant var;
    switch (role) {
    case RoleName:
        var = ele.name;
        break;
    case RoleUrl:
        var = ele.url;
        break;
    case RoleType:
        var = ele.type;
        break;
    default:
        break;
    }
    return var;
}

QHash<int, QByteArray> MediaDirItem::roleNames() const
{
    return m_roleNames;
}

void MediaDirItem::createDataBase()
{
    //添加驱动
    mediaDirDb = QSqlDatabase::addDatabase("QSQLITE");
    //设置数据库名字
    mediaDirDb.setDatabaseName("InfinityPlayer.db");
    //打开数据库
    if(mediaDirDb.open()) {
        qDebug() << "Success";
    }
    else {
        qDebug() << "Fail";
    }
}

void MediaDirItem::createDataTable()
{
    //判断是否存在数据表--MediaDir
    QString sql = QString("select count(*) from sqlite_master where type = 'table' name='%1'").arg("MediaDir");
    if(mediaDirSql->exec(sql)) {
        qDebug() << "Exist";
    }
    else {
        QString creatTableSql = QString("CREATE TABLE MediaDir("
                                        "dirname TEXT NOT NULL,"
                                        "name TEXT NOT NULL,"
                                        "url TEXT NOT NULL,"
                                        "type INT NOT NULL)");
        if(mediaDirSql->exec(creatTableSql)) {
            qDebug() << "Create Table -- MediaDir Success";
        }
        else {
            qDebug() << creatTableSql;
        }
    }
}


