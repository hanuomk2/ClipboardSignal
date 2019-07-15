#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QSettings>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    unsigned int value_ = 0;
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void show_systemtrayicon(bool flagCaptureONOFF);                // システムトレイにアイコンを表示
    void runExe(bool flagExplorer);

private:
    Ui::MainWindow *ui;
    ////////////////////////////////////////////////////////////////// iniファイルの設定用
    QSettings *settings;
    QString iniFilePath;

    QString strOutputFolder;
    bool flagPrifix;
    QString strPrifix;
    QString strViewerPath;
    QString strViewerFolder;

    enum typeFilename{
        YYYYMMDD,
        COUNTNO,
        COUNTNO0
    }enumFilename;
    unsigned int count;
    int digit;

    void initSettingsFile(QString dirPath);
    void readSettingsFile(QString dirPath);
    void writeSettingsFile();

    ////////////////////////////////////////////////////////////////// システムトレイアイコンの設定用
    QSystemTrayIcon tray_icon;
    QString icon_path;
    QMenu *menu;
    QAction *action_captureONOFF;
    QAction *action_settings;
    QAction *action_openfolder;
    QAction *action_openviewer;
    QAction *action_quit;
    // QAction *action_qt;

public slots:
    void get_clipboardimage();                                      // クリックボードからイメージを取得
private slots:
    void on_checkBox_stateChanged(int arg1);
    void on_radioButton_1_toggled(bool checked);
    void on_radioButton_2_toggled(bool checked);
    void on_radioButton_3_toggled(bool checked);
    void on_pushButton_OK_clicked();
    void on_pushButton_Cansel_clicked();
    void on_pushButton_Apply_clicked();
    void on_pushButton_clicked();
    void on_pushButton_3_clicked();
};

#endif // MAINWINDOW_H
