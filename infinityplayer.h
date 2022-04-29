#ifndef INFINITYPLAYER_H
#define INFINITYPLAYER_H

#include <QWidget>
#include <QListView>
#include <QStringListModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QTableView>
#include <QFileDialog>
#include "playercontrols.h"
#include "player.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QMediaPlayer;
class QModelIndex;
class QPushButton;
class QComboBox;
class QSlider;
class QStatusBar;
class QVideoWidget;
class QLineEdit;
QT_END_NAMESPACE

class InfinityPlayer : public QWidget
{
    Q_OBJECT

public:
    InfinityPlayer(QWidget *parent = nullptr);
    ~InfinityPlayer();
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
    QMediaPlayer *player_mediaPlayer;   //播放器
    QVideoWidget *video_videoWidget;    //播放器视频通道
    QAudioOutput *audio_audioOutput;    //播放器音频通道
    Player *player;   //后端播放模块
};
#endif // INFINITYPLAYER_H
