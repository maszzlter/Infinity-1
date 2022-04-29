#include "infinityplayer.h"
#include "./ui_infinityplayer.h"
#include <QAudioDevice>
#include <QAudioOutput>
#include <QVideoWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QListWidget>

InfinityPlayer::InfinityPlayer(QWidget *parent)
    : QWidget(parent)
{
    // 初始化
    this->resize(680, 480);
    mediaDir_listWidget = new QListWidget(this);
    mediaDir_listWidget->setGridSize(QSize(mediaDir_listWidget->width(), mediaDir_listWidget->height() * 0.8));
    mediaDir_listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    dirName_lineEdit = new QLineEdit(mediaDir_listWidget);
    dirName_lineEdit->resize(mediaDir_listWidget->gridSize());
    dirName_lineEdit->setVisible(false);
    dirName_lineEdit_1 = new QLineEdit(mediaDir_listWidget);
    dirName_lineEdit_1->resize(mediaDir_listWidget->gridSize());
    dirName_lineEdit_1->setVisible(false);
    mediaItem_tableView = new QTableView(this);
    mediaDir_label = new QLabel("音视频目录", this);
    addMediaDir_button = new QToolButton(this);
    addMediaDir_button->setIcon(style()->standardIcon(QStyle::SP_TitleBarShadeButton));
    delMediaDir_button = new QToolButton(this);
    delMediaDir_button->setIcon(style()->standardIcon(QStyle::SP_TitleBarUnshadeButton));
    mediaItem_tableView->setVisible(false);
    closeMediaItem_button = new QToolButton(this);
    closeMediaItem_button->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
    mediaItem_label = new QLabel("粤语歌曲", this);
    addMediaItem_button = new QToolButton(this);
    addMediaItem_button->setIcon(style()->standardIcon(QStyle::SP_TitleBarShadeButton));
    delMediaItem_button = new QToolButton(this);
    delMediaItem_button->setIcon(style()->standardIcon(QStyle::SP_TitleBarUnshadeButton));
    closeMediaItem_button->setVisible(false);
    mediaItem_label->setVisible(false);
    addMediaItem_button->setVisible(false);
    delMediaItem_button->setVisible(false);
    QHBoxLayout *layout_topleft = new QHBoxLayout;
    layout_topleft->setContentsMargins(0, 0, 0, 0);
    layout_topleft->addWidget(mediaDir_label,2);
    layout_topleft->addWidget(addMediaDir_button, 1);
    layout_topleft->addWidget(delMediaDir_button, 1);
    QVBoxLayout *layout_left = new QVBoxLayout;
    layout_left->setContentsMargins(0, 0, 0, 0);
    layout_left->addLayout(layout_topleft, 1);
    layout_left->addWidget(mediaDir_listWidget, 6);
    QHBoxLayout *layout_tabletop = new QHBoxLayout;
    layout_tabletop->setContentsMargins(0, 0, 0, 0);
    layout_tabletop->addWidget(closeMediaItem_button, 1);
    layout_tabletop->addWidget(mediaItem_label, 1);
    layout_tabletop->addWidget(addMediaItem_button, 1);
    layout_tabletop->addWidget(delMediaItem_button, 1);
    QVBoxLayout *layout_table = new QVBoxLayout;
    layout_topleft->setSpacing(0);
    layout_table->setContentsMargins(0, 0, 0, 0);
    layout_table->addLayout(layout_tabletop, 1);
    layout_table->addWidget(mediaItem_tableView, 6);
    //读入目录
    loadMediaDir();
    //信号与槽函数
    connect(mediaDir_listWidget, &QListWidget::itemClicked, this, &InfinityPlayer::showMediaItem);
    connect(addMediaDir_button, &QToolButton::clicked, this, &InfinityPlayer::on_addMediaDir_button_clicked);
    connect(delMediaDir_button, &QToolButton::clicked, this, &InfinityPlayer::on_delMediaDir_button_clicked);
    connect(dirName_lineEdit, &QLineEdit::editingFinished, this, &InfinityPlayer::addMediaDir);
    connect(dirName_lineEdit_1, &QLineEdit::editingFinished, this, &InfinityPlayer::changeMediaDir);
    connect(addMediaItem_button, &QToolButton::clicked, this, &InfinityPlayer::on_addMediaItem_button_clicked);
    connect(delMediaItem_button, &QToolButton::clicked, this, &InfinityPlayer::on_delMediaItem_button_clicked);
    connect(mediaDir_listWidget, &QListWidget::customContextMenuRequested, this, &InfinityPlayer::on_mediaItem_menu);
    connect(closeMediaItem_button, &QToolButton::clicked, this, &InfinityPlayer::closeMediaItem);
    PlayerControls *playerControls = new PlayerControls(this);
    player_mediaPlayer = new QMediaPlayer(this);
    audio_audioOutput = new QAudioOutput(this);
    player_mediaPlayer->setAudioOutput(audio_audioOutput);
    video_videoWidget = new QVideoWidget(this);
    player_mediaPlayer->setVideoOutput(video_videoWidget);
    QHBoxLayout *layout_center = new QHBoxLayout;
    layout_center->setContentsMargins(0, 0, 0, 0);
    layout_center->addLayout(layout_left, 1);
    layout_center->addLayout(layout_table, 4);
    layout_center->addWidget(video_videoWidget, 4);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addLayout(layout_center, 9);
    layout->addWidget(playerControls, 1);
    setLayout(layout);
    player = new Player();
}

InfinityPlayer::~InfinityPlayer()
{
}

//加载媒体库目录
void InfinityPlayer::loadMediaDir()
{
    //添加驱动
    infinityPlayer_dataBase = QSqlDatabase::addDatabase("QSQLITE");
    //设置数据库名字a
    infinityPlayer_dataBase.setDatabaseName("InfinityPlayer.db");
    //打开数据库
    if(infinityPlayer_dataBase.open()) {
        qDebug() << "Success";
        infinityPlayer_sqlQuery = new QSqlQuery(infinityPlayer_dataBase);
    }
    else {
        qDebug() << "Fail";
    }
    //判断是否存在数据表--MediaDir
    QString sql = QString("select * from sqlite_master where name='%1'").arg("MediaDir");
    if(infinityPlayer_sqlQuery->exec(sql)) {
        if(infinityPlayer_sqlQuery->next()) qDebug() << "Exist";
        else {
            //没有MeidaDir表就创建
            QString creatTableSql = QString("CREATE TABLE MediaDir("
                                            "dirname TEXT UNIQUE NOT NULL)");
            if(infinityPlayer_sqlQuery->exec(creatTableSql)) {
                qDebug() << "Create Table -- MediaDir Success";
            }
            else {
                qDebug() << creatTableSql;
            }
        }
    }
    if(infinityPlayer_sqlQuery->exec("SELECT * FROM MediaDir")) {
        while(infinityPlayer_sqlQuery->next()) {
            mediaDir_listWidget->addItem(infinityPlayer_sqlQuery->value(0).toString());
        }
    }
    else {
        qDebug() << "Fail to lada MediaDir";
    }
    //判断是否存在数据表--MediaItem
    sql = QString("select * from sqlite_master where name='%1'").arg("MediaItem");
    if(infinityPlayer_sqlQuery->exec(sql)) {
        if(infinityPlayer_sqlQuery->next()) qDebug() << "Exist";
        else {
            //没有MeidaDir表就创建
            QString creatTableSql = QString("CREATE TABLE MediaItem("
                                            "dirname TEXT NOT NULL,"
                                            "name TEXT NOT NULL,"
                                            "path TEXT NOT NULL,"
                                            "type TEXT NOT NULL)");
            if(infinityPlayer_sqlQuery->exec(creatTableSql)) {
                qDebug() << "Create Table -- MediaItem Success";
            }
            else {
                qDebug() << creatTableSql;
            }
        }
    }
}

void InfinityPlayer::on_addMediaDir_button_clicked()
{

    dirName_lineEdit->move(0, mediaDir_listWidget->count() * dirName_lineEdit->size().height());
    dirName_lineEdit->setText("新建目录");
    dirName_lineEdit->setVisible(true);
}

void InfinityPlayer::addMediaDir()
{
    QString dirname = dirName_lineEdit->text();
    dirName_lineEdit->setVisible(false);
    if(infinityPlayer_sqlQuery->exec(QString("INSERT INTO MediaDir VALUES('%1')").arg(dirname))) {
        mediaDir_listWidget->addItem(dirname);
    }
    else {
        dirname += "_0";
        while(!infinityPlayer_sqlQuery->exec(QString("INSERT INTO MediaDir VALUES('%1')").arg(dirname))) {
            dirname += "_0";
        }
        mediaDir_listWidget->addItem(dirname);
    }
}

void InfinityPlayer::on_delMediaDir_button_clicked()
{
    QListWidgetItem *item = mediaDir_listWidget->currentItem();
    QString dirname = item->data(0).toString();
    QString sql = QString("delete from MediaItem where dirname = '%1'").arg(dirname);
    if(infinityPlayer_sqlQuery->exec(sql)) {
        sql = QString("delete from MediaDir where dirname = '%1'").arg(dirname);
        if(infinityPlayer_sqlQuery->exec(sql)) {
            mediaDir_listWidget->takeItem(mediaDir_listWidget->currentRow());
            mediaItem_tableView->setVisible(false);
            mediaItem_label->setVisible(false);
            addMediaItem_button->setVisible(false);
            delMediaItem_button->setVisible(false);
            closeMediaItem_button->setVisible(false);
            video_videoWidget->setVisible(true);
        }
        else {
            qDebug() << sql;
        }
    }
    else {
        qDebug() << sql;
    }
}

void InfinityPlayer::changeMediaDir()
{
    QListWidgetItem *item = mediaDir_listWidget->currentItem();
    QString oldname = item->data(0).toString();
    QString newname = dirName_lineEdit_1->text();
    if(infinityPlayer_sqlQuery->exec(QString("UPDATE MediaDir SET dirname = '%1' WHERE dirname = '%2'").arg(newname, oldname))) {
        if(infinityPlayer_sqlQuery->exec(QString("UPDATE MediaItem SET dirname = '%1' WHERE dirname = '%2'").arg(newname, oldname))) {
            item->setData(0, newname);
            dirName_lineEdit_1->setVisible(false);
            mediaItem_label->setText(newname);
        }
        else {
            qDebug() << "修改失败!";
        }
    }
    else {
        qDebug() << "修改失败!";
    }
}

void InfinityPlayer::showMediaItem(QListWidgetItem *item)
{
    QString dirname = item->data(0).toString();
    mediaItem_tableView->setVisible(true);
    mediaItem_label->setVisible(true);
    addMediaItem_button->setVisible(true);
    delMediaItem_button->setVisible(true);
    closeMediaItem_button->setVisible(true);
    mediaItem_label->setText(dirname);
    player_mediaPlayer->pause();
    video_videoWidget->setVisible(false);
    QString str = QString("SELECT name as Name, path as Path, type as Type FROM MediaItem WHERE dirname = '%1'").arg(dirname);
    mediaItem_sqlQueryModel.setQuery(str);
    mediaItem_tableView->setModel(&mediaItem_sqlQueryModel);
}

void InfinityPlayer::on_addMediaItem_button_clicked()
{
    QString path = QFileDialog::getOpenFileName(this,"open file",".","Audio (*.mp3, *.wav);;Video (*.mp4)");
    if(path != "") {
        int a = path.lastIndexOf("/");
        int b = path.lastIndexOf(".");
        QListWidgetItem *item = mediaDir_listWidget->currentItem();
        QString dirname = item->data(0).toString();
        QString name = path.mid(a + 1, b - a - 1);
        QString type = path.right(path.length() - b - 1);
        QString sql0 = QString("SELECT * FROM MediaItem WHERE dirname = '%1' and path = '%2'").arg(dirname, path);
        infinityPlayer_sqlQuery->exec(sql0);
        if(!infinityPlayer_sqlQuery->next()) {
            QString sql = QString("INSERT INTO MediaItem VALUES('%1', '%2', '%3', '%4')").arg(dirname, name, path, type);
            if(infinityPlayer_sqlQuery->exec(sql)) {
                showMediaItem(item);
            }
            else {
                qDebug() << "插入失败!";
            }
        }
        else {
            qDebug() << "插入失败!";
        }
    }
}

void InfinityPlayer::on_delMediaItem_button_clicked()
{
    QModelIndex index = mediaItem_tableView->currentIndex();
    QString path = mediaItem_sqlQueryModel.index(index.row(), 1).data().toString();
    QString dirname = mediaItem_label->text();
    QString sql = QString("DELETE FROM MediaItem WHERE dirname = '%1' and path = '%2'").arg(dirname, path);
    if(infinityPlayer_sqlQuery->exec(sql)) {
        showMediaItem(mediaDir_listWidget->currentItem());
    }
    else {
        qDebug() << "删除失败!";
    }
}

void InfinityPlayer::on_mediaItem_menu(const QPoint &pos)
{
    QMenu *menu = new QMenu(this);
    QAction* change = new QAction(tr("重命名"), menu);
    connect(change, &QAction::triggered, [=] {
        QListWidgetItem *item = mediaDir_listWidget->itemAt(pos);
        QModelIndex index = mediaDir_listWidget->indexAt(pos);
        QString oldname = item->data(0).toString();
        dirName_lineEdit_1->setText(oldname);
        dirName_lineEdit_1->move(0, index.row() * dirName_lineEdit_1->size().height());
        dirName_lineEdit_1->setVisible(true);
    });
    if (mediaDir_listWidget->itemAt(pos)!=nullptr)
    {
        menu->addAction(change);
        menu->popup(mediaDir_listWidget->mapToGlobal(pos));
    }
}

void InfinityPlayer::closeMediaItem()
{
    mediaItem_tableView->setVisible(false);
    mediaItem_label->setVisible(false);
    addMediaItem_button->setVisible(false);
    delMediaItem_button->setVisible(false);
    closeMediaItem_button->setVisible(false);
    player_mediaPlayer->play();
    video_videoWidget->setVisible(true);
}
