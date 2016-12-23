/* Copyright (C) 2014  Béguet Eric
Photosorter is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Photosorter is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Photosorter. If not, see <http://www.gnu.org/licenses/>.
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCheckBox>
#include <QDirModel>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QGraphicsPixmapItem>
#include <QGraphicsView>
#include <QListWidget>
#include <QShortcut>
#include <QStandardPaths>
#include <QVector>
#include <iostream>

#include "clickableqgraphicsscene.h"
#include "imagesorter.h"
#include "listpixmapprefetcher.h"

#include "epeg.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);
    // some setup that somehow don't work with the UI designer
    ui->inputListWidget->setAcceptDrops(false);
    ui->inputListWidget->setTextElideMode(Qt::ElideRight);

    // Add a few shortcuts
    QShortcut* shortcutDelete = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    connect(shortcutDelete, SIGNAL(activated()), this, SLOT(onKeyDelete_Pressed()));

    QShortcut* shortcutSpace = new QShortcut(QKeySequence(Qt::Key_Space), this);
    connect(shortcutSpace, SIGNAL(activated()), this, SLOT(onKeySpace_Pressed()));

    QShortcut *shortcutFS = new QShortcut(QKeySequence(Qt::Key_W), this);
    connect(shortcutFS, SIGNAL(activated()), this, SLOT(toggleFullScreen()));

    // the main graphic scene, as well as the signal/slots when clicked on
    scene = new ClickableQGraphicsScene;
    connect(scene, SIGNAL(clicked(QGraphicsItem*)), this, SLOT(on_selectedSorterChoice(QGraphicsItem*)));
    connect(scene, SIGNAL(rightClicked(QGraphicsItem*)), this, SLOT(on_discardSorterChoice(QGraphicsItem*)));

    // We don't want scrollbars on the graphic view, it's ugly.
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // init the prefetcher to load images
    prefetcher = new ListPixmapPrefetcher;

    on_newListButton_clicked();
}

MainWindow::~MainWindow() {
    delete ui;
    delete scene;
    delete prefetcher;
}

void MainWindow::toggleFullScreen()
{
    if (!this->isFullScreen()) {
        // get the current listWidget we should act on when a shortcut is hit
        if (ui->inputListWidget->hasFocus()) {
            focusListWidget = ui->inputListWidget;
        } else if (ui->listsTabWidget->currentWidget()->hasFocus()) {
            focusListWidget = static_cast<QListWidget*>(ui->listsTabWidget->currentWidget());
        }

        stateBeforeFS = this->windowState();
        this->setWindowState(Qt::WindowFullScreen);
        ui->statusBar->setVisible(false);
        ui->centralWidget->setVisible(false);

        // easier to create a new graphic view than moving ui->graphicsView around.
        graphicViewFS = new QGraphicsView(scene,this);
        graphicViewFS->resize(this->size());
        graphicViewFS->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        graphicViewFS->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        updateGraphicView();
    } else {
        // reset the previous state.
        this->setWindowState(stateBeforeFS);
        ui->statusBar->setVisible(true);
        ui->centralWidget->setVisible(true);
        delete graphicViewFS;
        graphicViewFS = nullptr;
        updateGraphicView();
        focusListWidget = nullptr;
    }

}

void MainWindow::on_openButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, "Open folder with JPG", QDir::currentPath(), QFileDialog::ShowDirsOnly);
    fillListWidgetIconFromFolder(path, ui->inputListWidget);
}

void MainWindow::fillListWidgetIconFromFolder(const QString path, QListWidget* listWidget){
    if (!path.isEmpty()) {
        QDir dir(path);
        dir.setFilter(QDir::Files);
        QFileInfoList list = dir.entryInfoList();
        int loadedItems = 0;
        listWidget->clear();
        for (int i = 0; i < list.size(); ++i) {
            QFileInfo fileInfo = list.at(i);
            bool loaded;
            QIcon thumbnail = icon(fileInfo, loaded);
            if (loaded) {
                QListWidgetItem* item = new QListWidgetItem(thumbnail, "");
                item->setToolTip(fileInfo.filePath());
                item->setData(Qt::UserRole, fileInfo.filePath());
                listWidget->addItem(item);
                ui->statusBar->showMessage(QString("%1/%2 Loading thumbnail").arg(++loadedItems).arg(list.size()));
            }
            // So that it isn't "hung" while loading the icons, as it can take a while.
            QCoreApplication::processEvents();
        }
       ui->statusBar->showMessage("");
    }
}

QIcon MainWindow::icon(const QFileInfo & info, bool& loaded) const {
    // load the icons with epeg and cache them in photoselectorThumbnails
    // TODO change the path.
    loaded = false;
    QString fileName = info.fileName();
    if (info.isFile() && fileName.endsWith("jpg", Qt::CaseInsensitive)) {
        QString path = info.absoluteFilePath();

        // get thumbnail path
        QString pathr = path; // why on hearth does replace is inplace_
        QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);

        QDir myDir;
        if(!myDir.exists(cacheDir)) {
            myDir.mkpath(cacheDir);
        }

        QString tpath = cacheDir + QDir::separator() + pathr.replace("/", "_").replace(":", "_");
        tpath = QDir::toNativeSeparators(tpath);
        QFileInfo checkFile(tpath);
        if (!checkFile.exists()) {
            Epeg_Image* image = epeg_file_open(path.toStdString().c_str());
            if (!image) return QIcon();
            epeg_decode_size_set(image, 200, 200);
            epeg_quality_set(image, 50);
            epeg_file_output_set(image, tpath.toStdString().c_str());
            epeg_encode(image);
            epeg_close(image);
        }
        loaded = true;
        return QIcon(tpath);
    }
    else
        return QIcon();
}

QGraphicsView* MainWindow::getGView() {
    QGraphicsView* gview = ui->graphicsView;
    if (this->isFullScreen() && graphicViewFS) {
        gview = graphicViewFS;
    }
    return gview;
}


void MainWindow::updateGraphicView() {
    if (isSortingProcessRunning()) {
        setSorterCurrentChoice();
    } else {
        setCenterImage();
    }
}

void MainWindow::setCenterImage(){
    if (pxmap.isNull()) return;
    QGraphicsView* gview = getGView();

    QPixmap pxmapScaled = pxmap;
    float ratio = this->isFullScreen() ? 1 : 0.99;
    float offsetX = 0;
    float offsetY = 0;
    if (pxmap.size().width() > gview->size().width()) {
        pxmapScaled = pxmap.scaled(gview->size() * ratio, Qt::KeepAspectRatio);
    }
    offsetX = (gview->size().width() - pxmapScaled.size().width()) / 2.f;
    offsetY = (gview->size().height() - pxmapScaled.size().height()) / 2.f;

    scene->clear();
    scene->setSceneRect(0, 0, gview->size().width(), gview->size().height());
    QGraphicsPixmapItem* item = new QGraphicsPixmapItem(pxmapScaled);
    item->setOffset(offsetX, offsetY);
    scene->addItem(item);
    gview->setScene(scene);
    gview->show();
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    updateGraphicView();
}

void MainWindow::on_saveButton_clicked()
{
    QListWidget* list = static_cast<QListWidget*>(ui->listsTabWidget->currentWidget());
    saveListsTo({list});
}

void MainWindow::on_saveAllButton_clicked()
{
    QVector<QListWidget*> allLists;
    for (int i = 0; i < ui->listsTabWidget->count(); ++i) {
        allLists.push_back(static_cast<QListWidget*>(ui->listsTabWidget->widget(i)));
    }
    saveListsTo(allLists);
}

void MainWindow::saveListsTo(QVector<QListWidget*> lists) {

    QString saveToFolder = QFileDialog::getExistingDirectory(this,
                                                             "Open folder to save to",
                                                             QDir::currentPath(),
                                                             QFileDialog::ShowDirsOnly);
    if (saveToFolder.isEmpty()) {
        return;
    }

    int index = 1;
    for (QListWidget* list : lists) {
        for(int i = 0; i < list->count(); ++i){
            QListWidgetItem* item = list->item(i);
            QString path = item->data(Qt::UserRole).toString();
            QString baseName = QFileInfo(path).fileName();
            QString prefix = ui->sortedPrefixCheckBox->isChecked() ?
                                QString("S_%1_").arg(index++, 3, 10, QLatin1Char('0')) : "";
            QString destPath = saveToFolder + "/" + prefix + baseName;
            std::cout << "copy : " << path.toStdString() << " to " << destPath.toStdString() << std::endl;
            if (!QFile::copy(path, destPath)) {
                std::cout << "failed copy : file " << destPath.toStdString() << std::endl;
            }
        }
    }
}

void MainWindow::onKeyDelete_Pressed() {
    if ((isFullScreen() && focusListWidget && focusListWidget != ui->inputListWidget) ||
        (!isFullScreen() && ui->listsTabWidget->currentWidget()->hasFocus())) {
        QListWidget* list = static_cast<QListWidget*>(ui->listsTabWidget->currentWidget());
        if (list) {
            delete list->currentItem();
        }
    }
}

void MainWindow::onKeySpace_Pressed() {
    if (isFullScreen()) {
        if (!focusListWidget) {
            return;
        }
        if (focusListWidget != ui->inputListWidget) {
            sendImage();
        } else {
            chooseImage();
        }
    } else {
        if (ui->inputListWidget->hasFocus()) {
            chooseImage();
        } else if (ui->listsTabWidget->currentWidget()->hasFocus()) {
            sendImage();
        }
    }
}

void MainWindow::chooseImage() {
    QListWidget* list = static_cast<QListWidget*>(ui->listsTabWidget->currentWidget());
    if (list) {
        QListWidget* fromList = ui->inputListWidget;
        for (QListWidgetItem* item : fromList->selectedItems()) {
            list->addItem(item->clone());
        }
        fromList->setCurrentRow(fromList->currentRow() + 1);
    }
}

void MainWindow::sendImage() {
    QListWidget* source = static_cast<QListWidget*>(ui->listsTabWidget->currentWidget());
    if (source) {
        QListWidget* dest = ui->sendToComboBox->currentData().value<QListWidget*>();
        if (dest != source) {
            for (QListWidgetItem* item : source->selectedItems()) {
                dest->addItem(item->clone());
            }
            source->setCurrentRow(source->currentRow() + 1);
        }
    }
}

void MainWindow::on_inputListWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem * previous) {
    onCurrentListWidget_ItemChanged(current, previous);
}

void MainWindow::on_newListButton_clicked() {
    QListWidget* list = new QListWidget(ui->listsTabWidget);
    list->setViewMode(QListView::IconMode);
    list->setWrapping(false);
    list->setIconSize(QSize(200, 200));
    list->setDefaultDropAction(Qt::DropAction::CopyAction);
    list->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    list->setSelectionMode(QAbstractItemView::ExtendedSelection);

    QString itemName = QString("list %1").arg(++listCounter);
    ui->listsTabWidget->addTab(list, itemName);
    ui->listsTabWidget->setCurrentWidget(list);
    ui->sendToComboBox->addItem(itemName, QVariant::fromValue(list));

    connect(list,SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(onCurrentListWidget_ItemChanged(QListWidgetItem*,QListWidgetItem*)));
}

void MainWindow::on_listsTabWidget_tabCloseRequested(int index) {
    ui->sendToComboBox->removeItem(index);
    ui->listsTabWidget->removeTab(index);
    if (ui->listsTabWidget->count() == 0) {
        on_newListButton_clicked();
    }
}

void MainWindow::onCurrentListWidget_ItemChanged(QListWidgetItem *current, QListWidgetItem *){
    if (!current) {
        scene->clear();
        pxmap = QPixmap();
        return;
    }
    pxmap = prefetcher->get(current);
    setCenterImage();
    setSortingProcessStatus(true);
}

void MainWindow::on_sortButton_clicked() {
    if (isSortingProcessRunning()) {
        doneSorting();
    } else {
        if (!sorter) {
            QListWidget* list = static_cast<QListWidget*>(ui->listsTabWidget->currentWidget());
            if (!list || list->count() <= 1) {
                return;
            }
            sorter = new MergeSortImageSorter(list);
        }
        isSortingProcessPaused = false;
        setSorterCurrentChoice();
        ui->sortButton->setText("Stop");
    }
}

void MainWindow::doneSorting() {
    ui->sortButton->setText("Sort");
    QListWidgetItem* itemToShow = sorter->getFirstChoice();
    delete sorter;
    sorter = nullptr;
    if (itemToShow) {
        // to force the call to onChangeItem...
        itemToShow->listWidget()->setCurrentItem(nullptr);
        itemToShow->listWidget()->setCurrentItem(itemToShow);
    }
}

void MainWindow::setSorterCurrentChoice(){
    if (sorter) {
        QGraphicsView* gview = getGView();
        scene->clear();

        QVector<QListWidgetItem*> items = sorter->nextToSelectFrom();
        if (items.size() == 0) {
            doneSorting();
            return;
        }

        float ratio = (this->isFullScreen() ? 1.f : 0.99f) / items.size();
        int currentAccWidth = 0;
        for(auto item : items) {
            // TODO there must be a more efficient way of doing that
            QPixmap pxmap = prefetcher->get(item);// QPixmap::fromImage(QImage(item->data(Qt::UserRole).toString()));
            if (pxmap.size().width() > gview->size().width() * ratio) {
                pxmap = pxmap.scaledToWidth(static_cast<int>(gview->size().width() * ratio));
            }
            QGraphicsPixmapItem* gpxItem = new QGraphicsPixmapItem(pxmap);
            gpxItem->setData(Qt::UserRole, qVariantFromValue((void *) item));
            gpxItem->setOffset(currentAccWidth, 0);
            currentAccWidth += pxmap.width();

            scene->addItem(gpxItem);
        }
        scene->setSceneRect(0,0,0,0);
        gview->setScene(scene);
        gview->show();
    }
}


bool MainWindow::isSortingProcessRunning() {
    return !isSortingProcessPaused && sorter;
}

void MainWindow::setSortingProcessStatus(bool paused) {
    isSortingProcessPaused = paused;
    if (sorter && isSortingProcessPaused) {
        ui->sortButton->setText("Resume");
    }
}

void MainWindow::on_selectedSorterChoice(QGraphicsItem* item) {
    if (isSortingProcessRunning()) {
        QListWidgetItem* litem = (QListWidgetItem*)item->data(Qt::UserRole).value<void*>();
        sorter->setCurrentBest(litem);
        setSorterCurrentChoice();
    }
}

void MainWindow::on_discardSorterChoice(QGraphicsItem* item) {
    if (isSortingProcessRunning()) {
        QListWidgetItem* litem = (QListWidgetItem*)item->data(Qt::UserRole).value<void*>();
        sorter->discardItem(litem);
        setSorterCurrentChoice();
    }
}
