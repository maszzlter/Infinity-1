#include "infinityplayer.h"
#include "./ui_infinityplayer.h"
#include <QAudioDevice>
#include <QAudioOutput>
#include <QVideoWidget>

InfinityPlayer::InfinityPlayer(QWidget *parent)
    : QWidget(parent)
{
    // 初始化
    this->resize(680, 480);
    //    mediaDir_listView.setParent(this);
    //    mediaItem_tableView.setParent(this);
    //    mediaDir_listModel = new QStringListModel(this);
    //    mediaDir_listView.setModel(mediaDir_listModel);
    //    dirName_lineEdit = new QLineEdit("新建目录", this);
    //    dirName_lineEdit->setVisible(false);
    //    mediaDir_label = new QLabel("音视频目录", this);
    //    mediaDir_label->setGeometry(0, 0, 70, 50);
    //    addMediaDir_button = new QPushButton("+", this);
    //    delMediaDir_button = new QPushButton("-", this);
    //    addMediaDir_button->setGeometry(70, 0, 40, 50);
    //    delMediaDir_button->setGeometry(110, 0, 40, 50);
    //    mediaDir_listView.setGeometry(0, 50, 150, 430);
    //    mediaItem_tableView.setGeometry(150, 50, 430, 430);
    //    mediaItem_tableView.setVisible(false);
    //    mediaItem_label = new QLabel("粤语歌曲", this);
    //    mediaItem_label->setGeometry(150, 0, 70, 50);
    //    addMediaItem_button = new QPushButton("+", this);
    //    delMediaItem_button = new QPushButton("-", this);
    //    addMediaItem_button->setGeometry(400, 0, 40, 50);
    //    delMediaItem_button->setGeometry(440, 0, 40, 50);
    //    mediaItem_label->setVisible(false);
    //    addMediaItem_button->setVisible(false);
    //    delMediaItem_button->setVisible(false);
    //    //读入目录
    //    loadMediaDir();
    //    //信号与槽函数
    //    connect(&mediaDir_listView, &QListView::clicked, this, &InfinityPlayer::showMediaItem);
    //    connect(mediaDir_listModel, &QStringListModel::dataChanged, this, &InfinityPlayer::changeMediaDir);
    //    connect(addMediaDir_button, &QPushButton::clicked, this, &InfinityPlayer::on_addMediaDir_button_cliicked);
    //    connect(dirName_lineEdit, &QLineEdit::editingFinished, this, &InfinityPlayer::addMediaDir);
    //    connect(addMediaItem_button, &QPushButton::clicked, this, &InfinityPlayer::on_addMediaItem_button_cliicked);
    //    connect(delMediaItem_button, &QPushButton::clicked, this, &InfinityPlayer::on_delMediaItem_button_cliicked);
    PlayerControls *playerControls = new PlayerControls(this);
    player_mediaPlayer = new QMediaPlayer(this);
    audio_audioOutput = new QAudioOutput(this);
    player_mediaPlayer->setAudioOutput(audio_audioOutput);
    video_videoWidget = new QVideoWidget(this);
    player_mediaPlayer->setVideoOutput(video_videoWidget);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(video_videoWidget, 5);
    layout->addWidget(playerControls, 1);
    setLayout(layout);
    player = new Player();
    player->play("E:/CourseProject/Infinity/video/test.mp4");
}

InfinityPlayer::~InfinityPlayer()
{
}

////加载媒体库目录
//void InfinityPlayer::loadMediaDir()
//{
//    //添加驱动
//    infinityPlayer_dataBase = QSqlDatabase::addDatabase("QSQLITE");
//    //设置数据库名字
//    infinityPlayer_dataBase.setDatabaseName("InfinityPlayer.db");
//    //打开数据库
//    if(infinityPlayer_dataBase.open()) {
//        qDebug() << "Success";
//        infinityPlayer_sqlQuery = new QSqlQuery(infinityPlayer_dataBase);
//    }
//    else {
//        qDebug() << "Fail";
//    }
//    //判断是否存在数据表--MediaDir
//    QString sql = QString("select * from sqlite_master where name='%1'").arg("MediaDir");
//    if(infinityPlayer_sqlQuery->exec(sql)) {
//        if(infinityPlayer_sqlQuery->next()) qDebug() << "Exist";
//        else {
//            //没有MeidaDir表就创建
//            QString creatTableSql = QString("CREATE TABLE MediaDir("
//                                            "dirname TEXT UNIQUE NOT NULL)");
//            if(infinityPlayer_sqlQuery->exec(creatTableSql)) {
//                qDebug() << "Create Table -- MediaDir Success";
//            }
//            else {
//                qDebug() << creatTableSql;
//            }
//        }
//    }
//    QStringList str;
//    if(infinityPlayer_sqlQuery->exec("SELECT * FROM MediaDir")) {
//        while(infinityPlayer_sqlQuery->next()) {
//            str << infinityPlayer_sqlQuery->value(0).toString();
//        }
//    }
//    else {
//        qDebug() << "Fail to lada MediaDir";
//    }
//    mediaDir_listModel->setStringList(str);
//    //判断是否存在数据表--MediaItem
//    sql = QString("select * from sqlite_master where name='%1'").arg("MediaItem");
//    if(infinityPlayer_sqlQuery->exec(sql)) {
//        if(infinityPlayer_sqlQuery->next()) qDebug() << "Exist";
//        else {
//            //没有MeidaDir表就创建
//            QString creatTableSql = QString("CREATE TABLE MediaItem("
//                                            "dirname TEXT NOT NULL,"
//                                            "name TEXT NOT NULL,"
//                                            "path TEXT NOT NULL,"
//                                            "type TEXT NOT NULL)");
//            if(infinityPlayer_sqlQuery->exec(creatTableSql)) {
//                qDebug() << "Create Table -- MediaItem Success";
//            }
//            else {
//                qDebug() << creatTableSql;
//            }
//        }
//    }
//}

//void InfinityPlayer::on_addMediaDir_button_cliicked()
//{
//    dirName_lineEdit->setGeometry(0, mediaDir_listModel->rowCount() * 18 + 50, 150, 18);
//    dirName_lineEdit->setVisible(true);
//}

//void InfinityPlayer::addMediaDir()
//{
//    dirName_lineEdit->setVisible(false);
//    if(infinityPlayer_sqlQuery->exec(QString("INSERT INTO MediaDir VALUES('%1')").arg(dirName_lineEdit->text()))) {
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

//void InfinityPlayer::delMediaDir()
//{
//    QModelIndex index = mediaDir_listView.currentIndex();
//    QString sql = QString("delete from MediaItem where dirname = '%1'").arg(index.data().toString());
//    if(infinityPlayer_sqlQuery->exec(sql)) {
//        sql = QString("delete from MediaDir where dirname = '%1'").arg(index.data().toString());
//        if(infinityPlayer_sqlQuery->exec(sql)) {
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

//void InfinityPlayer::changeMediaDir(const QModelIndex &topLeft, const QModelIndex &bottomRight)
//{
//    QString oldName = bottomRight.data().toString();
//    qDebug() << oldName;
//    QString newName = topLeft.data().toString();
////    QString sql = QString("UPDATE ")
////    qDebug() << oldName;
//}

//void InfinityPlayer::showMediaItem(const QModelIndex &index)
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

//void InfinityPlayer::on_addMediaItem_button_cliicked()
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
//        if(infinityPlayer_sqlQuery->exec(sql)) {
//            showMediaItem(index);
//        }
//        else {
//            qDebug() << "插入失败!";
//        }
//    }
//}

//void InfinityPlayer::on_delMediaItem_button_cliicked()
//{
//    QModelIndex index = mediaItem_tableView.currentIndex();
//    QString path = mediaItem_sqlQueryModel.index(index.row(), 1).data().toString();
//    QString sql = QString("DELETE FROM MediaItem WHERE path = '%1'").arg(path);
//    if(infinityPlayer_sqlQuery->exec(sql)) {
//        showMediaItem(mediaDir_listView.currentIndex());
//    }
//    else {
//        qDebug() << "删除失败!";
//    }
//}

