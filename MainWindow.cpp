#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QTextCodec>
#include <QClipboard>
#include <QMimeData>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QDateTime>
#include <QProcess>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->lineEdit_CountNo->setValidator(new QIntValidator(0, 999999999, this));
    ui->lineEdit_CountNo_0filled->setValidator(new QIntValidator(0, 999999999, this));

    ////////////////////////////////////////////////////////////////// iniファイルの設定 start
    const QString dirPath = QApplication::applicationDirPath();     // 実行ファイルのFolderPath
    const QString IniFileName("ClipboardSignal.ini");               // iniファイルのBaseName
    iniFilePath = dirPath + tr("/") + IniFileName;                  // iniファイルのPath

    settings = new QSettings(iniFilePath, QSettings::IniFormat);
    settings->setIniCodec(QTextCodec::codecForName("UTF-8"));

    initSettingsFile(dirPath);              // iniファイルがない場合作成する
    readSettingsFile(dirPath);

    ////////////////////////////////////////////////////////////////// システムトレイアイコン化 start
    // 最大化ボタン・最小化ボタン・閉じるボタン・メニューを非表示
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);

    // メニュー・アクションの生成
    menu = new QMenu(this);
    action_captureONOFF = new QAction(tr("Caputre ON/OFF"),this);
    action_captureONOFF->setCheckable(true);
    action_captureONOFF->setChecked(true);
    action_settings = new QAction(tr("Settings"), this);
    action_openfolder = new QAction(tr("Open Folder"), this);
    action_openviewer = new QAction(tr("Open Viewer"), this);
    action_quit = new QAction(tr("Quit"), this);

    // メニューにアクションを追加
    menu->addAction(action_captureONOFF);
    menu->addSeparator();
    menu->addAction(action_settings);
    menu->addSeparator();
    menu->addAction(action_openfolder);
    menu->addSeparator();
    menu->addAction(action_openviewer);
    menu->addSeparator();
    menu->addAction(action_quit);

    // トレイアイコンにメニューを追加
    tray_icon.setContextMenu(menu);

    // シグナル・スロットの設定
    connect(action_captureONOFF, &QAction::triggered, [=] {
        if(action_captureONOFF->isChecked()) show_systemtrayicon(true);
        else show_systemtrayicon(false);
    });

    connect(action_settings, &QAction::triggered, this, &MainWindow::show);
    connect(action_openfolder, &QAction::triggered, [=] { runExe(true);} );
    connect(action_openviewer, &QAction::triggered, [=] { runExe(false);} );
    connect(action_quit, &QAction::triggered, this, &QApplication::quit);
    ////////////////////////////////////////////////////////////////// システムトレイアイコン化 end

    // クリップボードのデータが変化した場合、get_glipboardimage()を呼び出す
    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(get_clipboardimage()));
}

MainWindow::~MainWindow()
{
    // 終了前にcountをiniファイルに追記
    settings->setValue("CountNo", ui->lineEdit_CountNo->text());
    settings->setValue("CountNo_0filled", ui->lineEdit_CountNo_0filled->text());
    settings->sync();

    delete menu;
    delete action_captureONOFF;
    delete action_settings;
    delete action_openfolder;
    delete action_openviewer;
    delete action_quit;
    delete settings;
    delete ui;
}

////////////////////////////////////////////////////////////////////// システムトレイにアイコンを表示
void MainWindow::show_systemtrayicon(bool flagCaptureONOFF)
{
    tray_icon.hide();
    if(flagCaptureONOFF) tray_icon.setIcon(QPixmap(":/images/Capture.bmp"));
    else tray_icon.setIcon(QPixmap(":/images/CaptureOFF.bmp"));
    tray_icon.show();
}

////////////////////////////////////////////////////////////////////// 右クリックメニューからExplorer/Viewerを起動
void MainWindow::runExe(bool flagExplorer)
{
#if defined(Q_OS_WIN)
        qDebug("strOutputFolder: %s", strOutputFolder.toUtf8().data());
        qDebug("strViewerPath %s", strViewerPath.toUtf8().data());
        QString option = strOutputFolder;
        option.replace(QString("/"), QString("\\"));
        QStringList options;
        options << option;
        if (flagExplorer) QProcess::startDetached("C:/WINDOWS/explorer.exe ", options);
        else {
            action_captureONOFF->setChecked(false);
            show_systemtrayicon(false);
            QProcess::startDetached(strViewerPath.toUtf8().data(), options);
        }
#endif
}

////////////////////////////////////////////////////////////////////// クリックボードからイメージを取得
void MainWindow::get_clipboardimage()
{
    if(!action_captureONOFF->isChecked()) return;

    // アプリケーションのグローバルクリップボードのポインタを返す。mimeDataをクリップボードから取得
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    if(mimeData->hasImage())                                        // クリップボードがimageを取得した場合
    {
        // クリップボードからimageを取り出し、メッセージ表示
        QPixmap pixmap = qvariant_cast<QPixmap>(mimeData->imageData());
        tray_icon.showMessage(tr("Screen Capture!"),
                              tr("Pressed [PrintScreen]key!"),
                              QIcon(pixmap),
                              1000/*msec*/);

        // imageを指定フォルダにjpgファイルで保存
        QString strFilePath, strFilename;
        if (flagPrifix) strFilePath = strOutputFolder + "/" + strPrifix;
        else strFilePath = strOutputFolder + "/";
        QDateTime dt = QDateTime::currentDateTime();
        QString strd;

        // ファイル名をYYYYMMDD、COUNTNO、COUNTNO0で作成
        switch (enumFilename) {
            case YYYYMMDD:                                          // enumFilename=YYYYMMDDの場合
                strFilename = dt.toString("yyyyMMddhhmmss.z");
                qDebug("YYYYMMDDhhmmss.d: %s", strFilename.toLocal8Bit().data());
                strFilename += ".jpg";
                break;

            case COUNTNO:                                           // enumFilename=COUNTNOの場合
                strFilename.setNum(count);
                qDebug("COUNTNO: %s", strFilename.toLocal8Bit().data());
                strFilename += ".jpg";
                count++;
                strd.setNum(count);
                ui->lineEdit_CountNo->setText(strd.toLocal8Bit().data());
                break;

            case COUNTNO0:                                          // enumFilename=COUNTNO0の場合
                strd.setNum(digit);
                QString strdigit = "%0" + strd + "d";
                strFilename.sprintf(strdigit.toLocal8Bit().data(), count);
                qDebug("COUNTNO0: %s", strFilename.toLocal8Bit().data());
                strFilename += ".jpg";
                count++;
                strd.sprintf(strdigit.toLocal8Bit().data(), count);
                ui->lineEdit_CountNo_0filled->setText(strd);
                break;
        }

        // ファイルパスを作成
        strFilePath += strFilename;
        qDebug("strFilePath: %s", strFilePath.toUtf8().data());

        // jpgファイルを作成
        QFile file(strFilePath);
        file.open(QIODevice::WriteOnly);
        pixmap.save(&file, "JPG");
    }
}

void MainWindow::initSettingsFile(QString dirPath)
{
    if (!QFile::exists(iniFilePath))                                // iniファイルの存在確認
    {
        settings->setValue("OutputFolderPath", dirPath);
        settings->setValue("flagFilename-prifix", "false");
        settings->setValue("Filename-prifix", "");
        settings->setValue("enumFilename", "0");
        settings->setValue("CountNo", "");
        settings->setValue("CountNo_0filled", "");
        settings->setValue("Countno_0filled_digit", "1");
        QString viewerpath = dirPath + "/TestListWidget.exe";
        settings->setValue("ViewerPath", viewerpath);
        settings->setValue("ViewerFolder", dirPath);

        settings->sync();
    }
}

void MainWindow::readSettingsFile(QString dirPath)
{
    ////////////////////////////////////////////////////////////////// Folderタブの設定読み出し
    strOutputFolder = settings->value("OutputFolderPath", dirPath).toString();
    ui->lineEdit_FolderPath->setText(strOutputFolder);

    ////////////////////////////////////////////////////////////////// Filenameタブの設定読み出し
    strPrifix = settings->value("Filename-prifix", "").toString();
    ui->lineEdit_Prifix->setText(strPrifix);

    ui->lineEdit_CountNo->setText(settings->value("CountNo", "").toString());
    ui->lineEdit_CountNo_0filled->setText(settings->value("CountNo_0filled", "").toString());

    digit = settings->value("Countno_0filled_digit", 1).toInt();
    ui->spinBox->setValue(digit);

    if (settings->value("flagFilename-prifix", false).toBool())     // flagFilename-prifix=trueの場合
    {
        ui->checkBox->setChecked(true);
        ui->lineEdit_Prifix->setEnabled(true);
    }
    else {
        ui->checkBox->setChecked(false);
        ui->lineEdit_Prifix->setEnabled(false);
    }

    enumFilename = static_cast<typeFilename>(settings->value("enumFilename", 0).toInt());
    switch (enumFilename) {
        case YYYYMMDD:
            ui->radioButton_1->setChecked(true);
            on_radioButton_1_toggled(true);
            break;

        case COUNTNO:
            ui->radioButton_2->setChecked(true);
            on_radioButton_2_toggled(true);
            count = settings->value("CountNo", "").toUInt();
            break;

        case COUNTNO0:
            ui->radioButton_3->setChecked(true);
            on_radioButton_3_toggled(true);
            QString strCount = settings->value("CountNo_0filled", "").toString();
            QString strd;
            strd.setNum(digit);
            QString strdigit = "%0" + strd + "d";
            sscanf(strCount.toLocal8Bit().data(),
                   strdigit.toLocal8Bit().data(),
                   &count);
            qDebug("count: %d", count);
            break;
    }

    ////////////////////////////////////////////////////////////////// Viewerタブの設定読み出し
    strViewerPath = settings->value("ViewerPath", dirPath + "/TestListWidget.exe").toString();
    ui->lineEdit_ViewerPath->setText(strViewerPath);
    strViewerFolder = settings->value("ViewerFolder", dirPath).toString();
}

void MainWindow::writeSettingsFile()
{
    ////////////////////////////////////////////////////////////////// Folderタブの設定書き出し
    strOutputFolder = ui->lineEdit_FolderPath->text();
    settings->setValue("OutputFolderPath", strOutputFolder);

    ////////////////////////////////////////////////////////////////// Filenameタブの設定書き出し
    strPrifix = ui->lineEdit_Prifix->text();
    settings->setValue("Filename-prifix", strPrifix);

    if (ui->checkBox->checkState() == Qt::Checked)
        settings->setValue("flagFilename-prifix", "true");
    else if (ui->checkBox->checkState() == Qt::Unchecked)
        settings->setValue("flagFilename-prifix", "false");

    settings->setValue("CountNo", ui->lineEdit_CountNo->text());
    settings->setValue("CountNo_0filled", ui->lineEdit_CountNo_0filled->text());

    digit = ui->spinBox->value();
    settings->setValue("Countno_0filled_digit", digit);

    if (ui->radioButton_1->isChecked())                             // enumFilename=YYYYMMDDの場合
    {
        enumFilename = YYYYMMDD;
        settings->setValue("enumFilename", "0");
    }
    else if (ui->radioButton_2->isChecked())                        // enumFilename=COUNTNOの場合
    {
        enumFilename = COUNTNO;
        settings->setValue("enumFilename", "1");
        count = ui->lineEdit_CountNo->text().toUInt();
    }
    else if (ui->radioButton_3->isChecked())                        // enumFilename=COUNTNO0の場合
    {
        enumFilename = COUNTNO0;
        settings->setValue("enumFilename", "2");
        QString strd;
        strd.setNum(digit);
        QString strdigit = "%0" + strd + "d";
        sscanf(ui->lineEdit_CountNo_0filled->text().toLocal8Bit().data(),
               strdigit.toLocal8Bit().data(),
               &count);
        qDebug("count: %d", count);
    }

    ////////////////////////////////////////////////////////////////// Viewerタブの設定書き出し
    strViewerPath = ui->lineEdit_ViewerPath->text();
    QFileInfo fileinfo(strViewerPath);
    if(fileinfo.isFile()) {
        QString strcmp = fileinfo.suffix();
        if(strcmp.compare("jpg", Qt::CaseInsensitive)) {
            settings->setValue("ViewerPath", strViewerPath);
            strViewerFolder = fileinfo.dir().path();
            settings->setValue("ViewerFolder", strViewerFolder);
        }
    }

    settings->sync();
}

void MainWindow::on_checkBox_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked) ui->lineEdit_Prifix->setEnabled(true);
    else if (arg1 == Qt::Unchecked) ui->lineEdit_Prifix->setEnabled(false);
}

void MainWindow::on_radioButton_1_toggled(bool checked)
{
    if (checked) {
        ui->radioButton_2->setChecked(false);
        ui->radioButton_3->setChecked(false);
        ui->lineEdit_CountNo->setEnabled(false);
        ui->lineEdit_CountNo_0filled->setEnabled(false);
        ui->spinBox->setEnabled(false);
    }
}

void MainWindow::on_radioButton_2_toggled(bool checked)
{
    if (checked) {
        ui->radioButton_1->setChecked(false);
        ui->radioButton_3->setChecked(false);
        ui->lineEdit_CountNo->setEnabled(true);
        ui->lineEdit_CountNo_0filled->setEnabled(false);
        ui->spinBox->setEnabled(false);
    }
}

void MainWindow::on_radioButton_3_toggled(bool checked)
{
    if (checked) {
        ui->radioButton_1->setChecked(false);
        ui->radioButton_2->setChecked(false);
        ui->lineEdit_CountNo->setEnabled(false);
        ui->lineEdit_CountNo_0filled->setEnabled(true);
        ui->spinBox->setEnabled(true);
    }
}

void MainWindow::on_pushButton_OK_clicked()
{
    setVisible(false);
    writeSettingsFile();
}

void MainWindow::on_pushButton_Cansel_clicked()
{
    setVisible(false);
}

void MainWindow::on_pushButton_Apply_clicked()
{
    writeSettingsFile();
}

void MainWindow::on_pushButton_clicked()
{
    // ファイルダイアログを表示
    QString strDir = QFileDialog::getExistingDirectory(this, "Choose Folder", ui->lineEdit_FolderPath->text());
    // ファイルダイアログで正常な戻り値が出力された場合
    if (!strDir.isEmpty()) ui->lineEdit_FolderPath->setText(strDir);
}

void MainWindow::on_pushButton_3_clicked()
{
    // ファイルダイアログを表示
    QString strExefile = QFileDialog::getOpenFileName(this, "Choose exe File", strViewerFolder, tr("exe file(*.exe)"));
    // ファイルダイアログで正常な戻り値が出力された場合
    if (!strExefile.isEmpty()) ui->lineEdit_ViewerPath->setText(strExefile);
}
