#ifndef MEDIADIRITEM_H
#define MEDIADIRITEM_H

#include <QAbstractListModel>
#include <QList>
#include <QString>
#include <QHash>
#include <QByteArray>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QDebug>

enum ElementType {
    audio_type,
    video_type
};

class Element{
public:
    Element(QString name_, QString url_, ElementType type_){
        name = name_;
        url = url_;
        type = type_;
    }
    QString name;
    QString url;
    ElementType type;
};


class MediaDirItem : public QAbstractListModel
{
    Q_OBJECT

public:
    enum LineInfo_Roles{
        RoleName = Qt::UserRole + 1,
        RoleUrl,
        RoleType,
    };

public:
    MediaDirItem(QObject* parent = 0);
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

//    Q_INVOKABLE void changeOne(QString name);
//    void setOne(QString name);
//    Q_INVOKABLE void delDir(QString name);
    void createDataBase();  //创建数据库
    void createDataTable(); //创建数据表

private:
    QList<Element> elementList;
    QHash<int, QByteArray> m_roleNames;
    QSqlDatabase mediaDirDb;    //数据库连接
    QSqlQuery *mediaDirSql;  //数据库查询
};

#endif // MEDIADIRITEM_H
