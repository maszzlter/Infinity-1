#include "mediadir.h"

MediaDir::MediaDir(QMainWindow *parent)
{
    // 初始化
    mediaDir_listView.setParent(parent);
    mediaItem_tableView.setParent(parent);
    mediaDir_listModel = new QStringListModel(parent);
    mediaDir_listView.setModel(mediaDir_listModel);
    dirName_lineEdit = new QLineEdit("新建目录", parent);
    dirName_lineEdit->setVisible(false);
    mediaDir_label = new QLabel("音视频目录", parent);
    mediaDir_label->setGeometry(0, 0, 70, 50);
    addMediaDir_button = new QPushButton("+", parent);
    delMediaDir_button = new QPushButton("-", parent);
    addMediaDir_button->setGeometry(70, 0, 40, 50);
    delMediaDir_button->setGeometry(110, 0, 40, 50);
    mediaDir_listView.setGeometry(0, 50, 150, 430);
    mediaItem_tableView.setGeometry(150, 50, 430, 430);
    mediaItem_tableView.setVisible(false);
    mediaItem_label = new QLabel("粤语歌曲", parent);
    mediaItem_label->setGeometry(150, 0, 70, 50);
    addMediaItem_button = new QPushButton("+", parent);
    delMediaItem_button = new QPushButton("-", parent);
    addMediaItem_button->setGeometry(400, 0, 40, 50);
    delMediaItem_button->setGeometry(440, 0, 40, 50);
    mediaItem_label->setVisible(false);
    addMediaItem_button->setVisible(false);
    delMediaItem_button->setVisible(false);
    //读入目录
//    loadMediaDir();
    //信号与槽函数
//    connect(&mediaDir_listView, &QListView::clicked, this, &MediaDir::showMediaItem);
//    connect(mediaDir_listModel, &QStringListModel::dataChanged, this, &MediaDir::changeMediaDir);
//    connect(addMediaDir_button, &QPushButton::clicked, this, &MediaDir::on_addMediaDir_button_cliicked);
//    connect(dirName_lineEdit, &QLineEdit::editingFinished, this, &MediaDir::addMediaDir);
//    connect(addMediaItem_button, &QPushButton::clicked, this, &MediaDir::on_addMediaItem_button_cliicked);
    //    connect(delMediaItem_button, &QPushButton::clicked, this, &MediaDir::on_delMediaItem_button_cliicked);
}

MediaDir::~MediaDir()
{

}

////加载媒体库目录
//void MediaDir::loadMediaDir()
//{
//    //添加驱动
//    MediaDir_dataBase = QSqlDatabase::addDatabase("QSQLITE");
//    //设置数据库名字
//    MediaDir_dataBase.setDatabaseName("MediaDir.db");
//    //打开数据库
//    if(MediaDir_dataBase.open()) {
//        qDebug() << "Success";
//        MediaDir_sqlQuery = new QSqlQuery(MediaDir_dataBase);
//    }
//    else {
//        qDebug() << "Fail";
//    }
//    //判断是否存在数据表--MediaDir
//    QString sql = QString("select * from sqlite_master where name='%1'").arg("MediaDir");
//    if(MediaDir_sqlQuery->exec(sql)) {
//        if(MediaDir_sqlQuery->next()) qDebug() << "Exist";
//        else {
//            //没有MeidaDir表就创建
//            QString creatTableSql = QString("CREATE TABLE MediaDir("
//                                            "dirname TEXT UNIQUE NOT NULL)");
//            if(MediaDir_sqlQuery->exec(creatTableSql)) {
//                qDebug() << "Create Table -- MediaDir Success";
//            }
//            else {
//                qDebug() << creatTableSql;
//            }
//        }
//    }
//    QStringList str;
//    if(MediaDir_sqlQuery->exec("SELECT * FROM MediaDir")) {
//        while(MediaDir_sqlQuery->next()) {
//            str << MediaDir_sqlQuery->value(0).toString();
//        }
//    }
//    else {
//        qDebug() << "Fail to lada MediaDir";
//    }
//    mediaDir_listModel->setStringList(str);
//    //判断是否存在数据表--MediaItem
//    sql = QString("select * from sqlite_master where name='%1'").arg("MediaItem");
//    if(MediaDir_sqlQuery->exec(sql)) {
//        if(MediaDir_sqlQuery->next()) qDebug() << "Exist";
//        else {
//            //没有MeidaDir表就创建
//            QString creatTableSql = QString("CREATE TABLE MediaItem("
//                                            "dirname TEXT NOT NULL,"
//                                            "name TEXT NOT NULL,"
//                                            "path TEXT NOT NULL,"
//                                            "type TEXT NOT NULL)");
//            if(MediaDir_sqlQuery->exec(creatTableSql)) {
//                qDebug() << "Create Table -- MediaItem Success";
//            }
//            else {
//                qDebug() << creatTableSql;
//            }
//        }
//    }
//}

//void MediaDir::on_addMediaDir_button_cliicked()
//{
//    dirName_lineEdit->setGeometry(0, mediaDir_listModel->rowCount() * 18 + 50, 150, 18);
//    dirName_lineEdit->setVisible(true);
//}

//void MediaDir::addMediaDir()
//{
//    dirName_lineEdit->setVisible(false);
//    if(MediaDir_sqlQuery->exec(QString("INSERT INTO MediaDir VALUES('%1')").arg(dirName_lineEdit->text()))) {
//        mediaDir_listModel->insertRow(mediaDir_listModel->rowCount()); //在尾部插入一空行
//        QModelIndex index=mediaDir_listModel->index(mediaDir_listModel->rowCount()-1,0); //获取最后一行
//        mediaDir_listModel->setData(index,dirName_lineEdit->text()); //设置显示文字
//        mediaDir_listView.setCurrentIndex(index); //设置当前选中的行
//    }
//    else {
//        qDebug() << "新建目录失败!";
//    }
//    dirName_lineEdit->setText("新建目录");
//}

//void MediaDir::delMediaDir()
//{
//    QModelIndex index = mediaDir_listView.currentIndex();
//    QString sql = QString("delete from MediaItem where dirname = '%1'").arg(index.data().toString());
//    if(MediaDir_sqlQuery->exec(sql)) {
//        sql = QString("delete from MediaDir where dirname = '%1'").arg(index.data().toString());
//        if(MediaDir_sqlQuery->exec(sql)) {
//            mediaDir_listModel->removeRow(index.row());
//            mediaItem_tableView.setVisible(false);
//            mediaItem_label->setVisible(false);
//            addMediaItem_button->setVisible(false);
//            delMediaItem_button->setVisible(false);
//        }
//        else {
//            qDebug() << sql;
//        }
//    }
//    else {
//        qDebug() << sql;
//    }
//}

//void MediaDir::changeMediaDir(const QModelIndex &topLeft, const QModelIndex &bottomRight)
//{
//    QString oldName = bottomRight.data().toString();
//    qDebug() << oldName;
//    QString newName = topLeft.data().toString();
////    QString sql = QString("UPDATE ")
////    qDebug() << oldName;
//}

//void MediaDir::showMediaItem(const QModelIndex &index)
//{
//    QString dirname = index.data().toString();
//    mediaItem_tableView.setVisible(true);
//    mediaItem_label->setVisible(true);
//    addMediaItem_button->setVisible(true);
//    delMediaItem_button->setVisible(true);
//    mediaItem_label->setText(dirname);
//    QString str = QString("SELECT name as Name, path as Path, type as Type FROM MediaItem WHERE dirname = '%1'").arg(dirname);
//    mediaItem_sqlQueryModel.setQuery(str);
//    mediaItem_tableView.setModel(&mediaItem_sqlQueryModel);
//}

//void MediaDir::on_addMediaItem_button_cliicked()
//{
//    QString path = QFileDialog::getOpenFileName(this,"open file",".","Audio (*.mp3, *.wav);;Video (*.mp4)");
//    if(path != "") {
//        int a = path.lastIndexOf("/");
//        int b = path.lastIndexOf(".");
//        QModelIndex index = mediaDir_listView.currentIndex();
//        QString dirname = index.data().toString();
//        QString name = path.mid(a + 1, b - a - 1);
//        QString type = path.right(path.length() - b - 1);
//        QString sql = QString("INSERT INTO MediaItem VALUES('%1', '%2', '%3', '%4')").arg(dirname, name, path, type);
//        if(MediaDir_sqlQuery->exec(sql)) {
//            showMediaItem(index);
//        }
//        else {
//            qDebug() << "插入失败!";
//        }
//    }
//}

//void MediaDir::on_delMediaItem_button_cliicked()
//{
//    QModelIndex index = mediaItem_tableView.currentIndex();
//    QString path = mediaItem_sqlQueryModel.index(index.row(), 1).data().toString();
//    QString sql = QString("DELETE FROM MediaItem WHERE path = '%1'").arg(path);
//    if(MediaDir_sqlQuery->exec(sql)) {
//        showMediaItem(mediaDir_listView.currentIndex());
//    }
//    else {
//        qDebug() << "删除失败!";
//    }
