#include <QDebug>
#include "searcherdialog.h"
#include "ui_searcherdialog.h"

#define WORK_THREAD_NUM 8

QQueue<QString> fileQueue;
QMutex mutex;       // protect fileQueue
QSemaphore sem(0);  // sem.available() should be equal to fileQueue.count()
QString StopSearchString("StopSearchString");

SearcherDialog::SearcherDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearcherDialog)
{
    ui->setupUi(this);

    /* rename TBW_Result's horizontal header */
    QStringList headers;
    headers << tr("File") << tr("Line") << tr("Context");
    ui->TBW_Result->setHorizontalHeaderLabels(headers);
    ui->TBW_Result->setColumnWidth(0, 300);
    ui->TBW_Result->setColumnWidth(1, 60);
    ui->TBW_Result->setTextElideMode(Qt::ElideLeft);

    readSettings();
}

SearcherDialog::~SearcherDialog()
{
    on_BTN_Stop_clicked();
    saveSettings();
    delete ui;
}

void SearcherDialog::readSettings()
{

    QSettings settings("StringSearcher.ini", QSettings::IniFormat);
    QStringList textList;
    int i;

    settings.beginGroup("History");

    /* restore Keyword history */
    textList = settings.value("key").toStringList();
    for (i = 0; i < textList.size(); i++)
        ui->CMB_Keyword->addItem(textList[i]);

    /* restore Replace word history */
    textList = settings.value("replace").toStringList();
    for (i = 0; i < textList.size(); i++)
        ui->CMB_Replace->addItem(textList[i]);
    if (textList.size() > 0 && textList[0].isEmpty()) {
        ui->CMB_Replace->removeItem(0);
        ui->CMB_Replace->setEditText("");
    }

    /* restore Filter history */
    textList = settings.value("filter").toStringList();
    for (i = 0; i < textList.size(); i++)
        ui->CMB_Filter->addItem(textList[i]);
    if (textList.size() > 0 && textList[0].isEmpty()) {
        ui->CMB_Filter->removeItem(0);
        ui->CMB_Filter->setEditText("");
    }

    /* restore searching dir history */
    textList = settings.value("dir").toStringList();
    if (textList.isEmpty())
        textList.append(QDir::currentPath());
    for (i = 0; i < textList.size(); i++)
        ui->CMB_DirPath->addItem(textList[i]);

    settings.endGroup();
}

void SearcherDialog::saveSettings()
{
    QSettings settings("StringSearcher.ini", QSettings::IniFormat);
    QStringList textList;
    int i;

    settings.beginGroup("History");

    /* save Keyword history */
    for (i = 0; i < ui->CMB_Keyword->count(); i++)
        textList.append(ui->CMB_Keyword->itemText(i));
    settings.setValue("key", textList);

    /* save Replace history */
    textList.clear();
    for (i = 0; i < ui->CMB_Replace->count(); i++)
        textList.append(ui->CMB_Replace->itemText(i));
    if (ui->CMB_Replace->currentText().isEmpty() && ui->CMB_Replace->count() > 0)
        textList.prepend("");
    settings.setValue("replace", textList);

    /* save Filter history */
    textList.clear();
    for (i = 0; i < ui->CMB_Filter->count(); i++)
        textList.append(ui->CMB_Filter->itemText(i));
    if (ui->CMB_Filter->currentText().isEmpty() && ui->CMB_Filter->count() > 0)
        textList.prepend("");
    settings.setValue("filter", textList);

    /* save Directory history */
    textList.clear();
    for (i = 0; i < ui->CMB_DirPath->count(); i++)
        textList.append(ui->CMB_DirPath->itemText(i));
    settings.setValue("dir", textList);

    settings.endGroup();
}

void SearcherDialog::updateComboBox(QComboBox *comboBox, QString &text)
{
    if (text.isEmpty())
        return;

    int index = comboBox->findText(text);

    if (index == -1) {
        comboBox->insertItem(0, text);
    } else {
        comboBox->removeItem(index);
        comboBox->insertItem(0, text);
        comboBox->setCurrentIndex(0);
    }
}

void SearcherDialog::on_BTN_Browse_clicked()
{
    QFileDialog::Options options =
                QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;

    QString dirPath = QFileDialog::getExistingDirectory(this,
                                tr("Please Select Path:"),
                                ui->CMB_DirPath->currentText(),
                                options);
    if (!dirPath.isEmpty()) {
        QString oldDirPath = ui->CMB_DirPath->currentText();
        if (oldDirPath.endsWith(";"))
            dirPath.prepend(oldDirPath);
        ui->CMB_DirPath->setEditText(dirPath);
    }
}

void SearcherDialog::on_BTN_Search_clicked()
{
    QString key = ui->CMB_Keyword->currentText();
    if (key.isEmpty()) {
        QMessageBox::warning(this, tr("No KeyWord"),
                         tr("Please specify the Keyword to search!"));
        return;
    }

    /* save history for each ComboBox */
    updateComboBox(ui->CMB_Keyword, key);
    QString replace = ui->CMB_Replace->currentText();
    updateComboBox(ui->CMB_Replace, replace);
    QString filter = ui->CMB_Filter->currentText();
    updateComboBox(ui->CMB_Filter, filter);
    QString path = ui->CMB_DirPath->currentText();
    updateComboBox(ui->CMB_DirPath, path);

    QStringList filterInList;
    QStringList filterOutList;
    QStringList filterList = filter.split(QRegExp("[ ,;]"), QString::SkipEmptyParts);
    filterList.removeDuplicates();
    for (int i = 0; i < filterList.count(); i++) {
        if (filterList[i].startsWith("-"))
            filterOutList.append(filterList[i].remove(0, 1));   /* remove '-' */
        else
            filterInList.append(filterList[i]);
    }

    QStringList filePathList = path.split(";", QString::SkipEmptyParts);
    filePathList.removeDuplicates();

    /* truncate previous results, and init result label */
    ui->TBW_Result->clearContents();
    ui->TBW_Result->setRowCount(0);
    ui->LBL_result->setText(
                tr("Result: 0 file(s) scanned, 0 file(s) matched, 0 file(s) failed."));

    ui->BTN_Search->setEnabled(false);
    ui->BTN_Stop->setEnabled(true);
    ui->LBL_Time->setText(tr("Elapsed Time: 0 ms"));
    ElapsedTime.start();

    fileScanned = 0;
    fileMatched = 0;
    fileFailed = 0;

    /* make sure to clear working status */
    mutex.lock();
    sem.acquire(sem.available());
    fileQueue.clear();
    searchThreads.clear();
    mutex.unlock();

    WorkingThreadNum = WORK_THREAD_NUM;
    for (int i = 0; i < WorkingThreadNum; i++) {
        SearchThread *thread = new SearchThread(key);
        searchThreads.append(thread);
        connect(thread, SIGNAL(signal_show_result(QString,int,QString)),
                this, SLOT(slot_show_result(QString,int,QString)));
        connect(thread, SIGNAL(signal_update_label(int, int)),
                this, SLOT(slot_update_label(int, int)));
        connect(thread, SIGNAL(finished()), this, SLOT(slot_thread_stopped()));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        thread->start();
    }

    QFuture<void> cntFuture = QtConcurrent::run(this, &SearcherDialog::collectFiles,
                                 filePathList, filterInList, filterOutList, key);
    cntWatcher.setFuture(cntFuture);
}

void SearcherDialog::on_BTN_Stop_clicked()
{
    stopScanDir = true;
    cntWatcher.waitForFinished();

    for (int i = 0; i < WorkingThreadNum; i++) {
        SearchThread *thread = searchThreads[i];
        thread->stop();
        if (!thread->wait(1000)) {
            qWarning() << i << ": Dead Search Thread, force to stop it!";
            thread->terminate();
            thread->wait();
        }
    }
    searchThreads.clear();
}

void SearcherDialog::scanDirectory(QDir &dir, QStringList &filterInList,
                                     QStringList &filterOutList, QString &key)
{
    if (stopScanDir)
        return;

    QStringList fileList = dir.entryList(filterInList,
                       QDir::Files | QDir::NoSymLinks | QDir::AllDirs | QDir::NoDotAndDotDot);

    /* filter-out files that don't want to search */
    if (!filterOutList.isEmpty()) {
        QStringList fileOutList = dir.entryList(filterOutList,
                           QDir::Files | QDir::NoSymLinks | QDir::Dirs | QDir::NoDotAndDotDot);

        for (int i = 0; i < fileOutList.count(); i++) {
            fileList.removeAll(fileOutList[i]);
        }
    }

    for (int i = 0; i < fileList.size(); i++) {
        QString filePath = dir.absoluteFilePath(fileList[i]);
        QFileInfo info(filePath);
        if (info.isDir()) {
            QDir subdir(filePath);
            scanDirectory(subdir, filterInList, filterOutList, key);
        } else {
            mutex.lock();
            fileQueue.enqueue(filePath);
            sem.release();
            mutex.unlock();
        }
    }
}

void SearcherDialog::collectFiles(QStringList filePathList, QStringList &filterInList,
                                     QStringList &filterOutList, QString &key)
{
    stopScanDir = false;
    for(int i = 0; i < filePathList.size(); i++) {
        QDir dir(filePathList[i]);
        scanDirectory(dir, filterInList, filterOutList, key);
    }

    mutex.lock();
    for (int i = 0; i < searchThreads.size(); i++) {
        fileQueue.enqueue(StopSearchString);
        sem.release();
    }
    mutex.unlock();
}

void SearcherDialog::slot_show_result(const QString filePath,
                                      const int line,
                                      const QString context)
{
    int row = ui->TBW_Result->rowCount();
    ui->TBW_Result->insertRow(row);

    QTableWidgetItem *item0 = new QTableWidgetItem(filePath);
    item0->setFlags(item0->flags() ^ Qt::ItemIsEditable);
    ui->TBW_Result->setItem(row, 0, item0);

    QTableWidgetItem *item1 = new QTableWidgetItem(tr("%1").arg(line));
    item1->setTextAlignment(Qt::AlignCenter);
    item1->setFlags(item1->flags() ^ Qt::ItemIsEditable);
    ui->TBW_Result->setItem(row, 1, item1);

    QTableWidgetItem *item2 = new QTableWidgetItem(context);
    item2->setFlags(item2->flags() ^ Qt::ItemIsEditable);
    ui->TBW_Result->setItem(row, 2, item2);
}

void SearcherDialog::slot_update_label(const int Matched, const int Failed)
{
    mutex.lock();
    fileScanned++;
    fileMatched += Matched;
    fileFailed += Failed;
    ui->LBL_result->setText(tr(
            "Result: %1 file(s) scanned, %2 file(s) matched, %3 file(s) failed.")
            .arg(fileScanned).arg(fileMatched).arg(fileFailed));
    mutex.unlock();
}

void SearcherDialog::slot_thread_stopped()
{
    WorkingThreadNum--;
    if (WorkingThreadNum == 0) {
        ui->BTN_Stop->setEnabled(false);
        ui->BTN_Search->setEnabled(true);
        ui->LBL_Time->setText(tr("Elapsed Time: %1 ms").arg(ElapsedTime.elapsed()));
    }
}

SearchThread::SearchThread(QString k)
{
    key = k;
    stopSearchStr = false;
}

SearchThread::~SearchThread()
{
}

int SearchThread::searchString(const QString &filePath, const QString &key)
{
    int found = 0;
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly)) {
        return -1;
    }

    QTextStream in(&file);
    QString text;
    int line = 0;

    while(!in.atEnd() && !stopSearchStr) {
        line++;
        text = in.readLine();
        if (text.contains(key)) {
            int index = text.indexOf(key);
            QString context = text.mid(index - 15);
            emit signal_show_result(filePath, line, context);
            found = 1;
            break;
        }
    }

    return found;
}

void SearchThread::run()
{
    int found, Matched, Failed;
    QString filePath;

    forever {
        sem.acquire();
        mutex.lock();
        filePath = fileQueue.dequeue();
        mutex.unlock();

        if (filePath == StopSearchString)
            break;

        found = searchString(filePath, key);
        if (found > 0) {
            Matched = 1;
            Failed = 0;
        } else if (found == 0) {
            Matched = 0;
            Failed = 0;
        } else {
            Matched = 0;
            Failed = 1;
        }

        emit signal_update_label(Matched, Failed);
    }
}

void SearchThread::stop()
{
    QMutexLocker locker(&mutex);

    /* stop thread as quick as possible, so pop up all of legal filePath */
    forever {
        if (fileQueue.isEmpty())
            break;
        if (fileQueue.dequeue() == StopSearchString) {
            fileQueue.enqueue(StopSearchString);
            break;
        }
        sem.acquire();
    }
    stopSearchStr = true;
}

