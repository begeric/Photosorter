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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileInfo>
#include <QMainWindow>

#include <Epeg.h>

namespace Ui {
class MainWindow;
}

class QGraphicsItem;
class QGraphicsView;
class QListWidgetItem;
class QListWidget;
class QShortcut;

class ClickableQGraphicsScene;
class ImageSorter;
class ListPixmapPrefetcher;

/*
 * This class manages the interface of the whole program.
 * The goal is to have a software to easily choose, prune, select and sort
 * a large collection of photograhs.
 *
 * The interface:
 * - One "input list" where we can import pictures from a file.
 * - A collections of "selection lists" where selected pictures from the input list or
 *   another selection list are transfered too. It is possible to then sort, re-prune,
 *   move to other lists and to save to files the pictures in those lists.
 * - A "main" view where the selected pictures from any of the list is shown.
 *
 * Image selection:
 * - Use the space key to 'select' an image and show the next one. Use the arrows to naviguate
 *   in the lists otherwise.
 *   'select' means 'move to another list'
 *
 * Sorting:
 * - When using the sorting mode, the user is prompted to chose the best image among a selection
 *   in the main view. To select the best one, simply click on it.
 *   this will re-iterate until the sorted order has been established.
 * - Clicking on an image on any of the lists will pause the sorting process. It is possible to resume it,
 *   as well as to stop it at anytime.
 *
 * Technicalities:
 *  - Thumbnails are cached, so as to save time upon loading.
 *    Known issue: if the underlying image changes then the thumbnail is not updated.
 *    it shouldn't be very common though.
 *  - Images are cached and prefetched. This allows for more seamless naviguation between pictures.
 *    Known issues: Qt seems wacky in Image loading so the prefetched is not as ambitious as planned.
 *  - We use merge sort for sorting even though it is not really the best for sorting pictures.
 *    (because A > B && B > C does not implies A > C, where '>' is the 'better image than' comparator).
 *    It would be better to use an n^2 algo but then it would be very time consuming.
*/
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void toggleFullScreen();

    void on_openButton_clicked();
    void on_saveButton_clicked();
    void on_saveAllButton_clicked();

    // can't have on_KeyDelete because Qt automatic signals/slots assignments.
    void onKeyDelete_Pressed();
    void onKeySpace_Pressed();

    void onCurrentListWidget_ItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_inputListWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_listsTabWidget_tabCloseRequested(int index);
    void on_newListButton_clicked();

    void on_sortButton_clicked();
    void on_selectedSorterChoice(QGraphicsItem* item);
    void on_discardSorterChoice(QGraphicsItem* item);

private:
    Ui::MainWindow *ui;
    ClickableQGraphicsScene* scene = nullptr;
    ImageSorter* sorter = nullptr;
    ListPixmapPrefetcher* prefetcher = nullptr;
    // the current pixmap that is
    QPixmap pxmap;
    // the graphicsView we use when in fullscreen mode
    QGraphicsView* graphicViewFS = nullptr;
    // the listwidget that had the focus before entering fullscreen mode
    // this is useful to know which listwidget to update/get image from
    // when using shortcuts in fullscreen mode (since we can't rely on hasFocus)
    QListWidget* focusListWidget = nullptr;
    // the state of this window before entering fullscreen mode
    Qt::WindowStates stateBeforeFS;

    bool isSortingProcessPaused = true;

    // the counter of lists in the listsTabWidget. So that we can increment it
    // (and thus have a different name) each time we add a new list.
    unsigned listCounter = 0;

    // get the current graphicsView in fullscreen or 'normal' mode
    QGraphicsView* getGView();

    // put an image from the inputListWidget in the current listwidget.
    void chooseImage();

    // send an image from the current listwidget to the one selected by sendToCombobox
    void sendImage();

    // fill the list witch icons from files in path
    void fillListWidgetIconFromFolder(const QString path, QListWidget* list);

    // load (and cache) an icon from the info jpg file.
    QIcon icon(const QFileInfo & info, bool& loaded) const;

    void updateGraphicView();

    // put pxmap in the center graphicsView
    void setCenterImage();

    // display the current options the the center graphicsView
    void setSorterCurrentChoice();

    // to call when sorting has been completed
    void doneSorting();

    void resizeEvent(QResizeEvent *event) override;

    // save a list of list widgets to a folder (prompted to the user)
    void saveListsTo(QVector<QListWidget*> lists);

    bool isSortingProcessRunning();
    void setSortingProcessStatus(bool paused);
};

#endif // MAINWINDOW_H
