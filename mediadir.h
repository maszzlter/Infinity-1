#ifndef MEDIADIR_H
#define MEDIADIR_H

#include <QMainWindow>
#include <QListView>
#include <QStringListModel>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QStyle>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QTableView>
#include <QFileDialog>
#include <QWidget>

class MediaDir : public QWidget
{
    Q_OBJECT

public:
    MediaDir(QMainWindow *parent = nullptr);
    ~MediaDir();
//    void loadMediaDir();    //加载媒体库目录
//    void on_addMediaDir_button_cliicked();
//    void addMediaDir();
//    void delMediaDir();
//    void changeMediaDir(const QModelIndex &topLeft, const QModelIndex &bottomRight);
//    void showMediaItem(const QModelIndex &index);
//    void on_addMediaItem_button_cliicked();
//    void on_delMediaItem_button_cliicked();

private:
    QSqlDatabase infinityPlayer_dataBase;  //数据库
    QSqlQuery *infinityPlayer_sqlQuery;     //数据库Sql
    QListView mediaDir_listView;    //媒体库目录
    QStringListModel *mediaDir_listModel; //媒体库目录内容
    QLabel *mediaDir_label;
    QPushButton *addMediaDir_button;    //添加目录按钮
    QPushButton *delMediaDir_button;    //删除目录按钮
    QLineEdit *dirName_lineEdit;    //添加目录
    QTableView mediaItem_tableView; //目录项
    QLabel *mediaItem_label;
    QPushButton *addMediaItem_button;    //添加目录项按钮
    QPushButton *delMediaItem_button;    //删除目录项按钮
    QSqlQueryModel mediaItem_sqlQueryModel;  //存放目录查询结果

};

#endif // MEDIADIR_H
