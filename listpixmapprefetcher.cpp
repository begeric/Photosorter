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

#include "listpixmapprefetcher.h"

#include <QListWidgetItem>

void Prefetcher::run() {
    while(true) {
        nonEmptyToProcess.acquire();
        mutexFetch.lock();
        QListWidgetItem* item = toProcess.dequeue();
        mutexFetch.unlock();
        if (!item) { // nullptr is a sentinel value to stop the thread
            break;
        }
        emit loaded(item, new QImage(item->data(Qt::UserRole).toString()));
    }
}

ListPixmapPrefetcher::ListPixmapPrefetcher() : thread(nonEmptyToProcess, mutexFetch, toProcess) {
    connect(&thread, SIGNAL(loaded(QListWidgetItem*, QImage*)),
            this,SLOT(newInCache(QListWidgetItem*,QImage*)));
    thread.start();
}

ListPixmapPrefetcher::~ListPixmapPrefetcher() {
    mutexFetch.lock();
    toProcess.push_back(nullptr);
    mutexFetch.unlock();
    nonEmptyToProcess.release();
    thread.wait();
}

QPixmap ListPixmapPrefetcher::get(QListWidgetItem* item) {

    // ask to prefetch the next image
    QListWidget* list = item->listWidget();
    int nextRow = list->row(item) + 1;
    mutexFetch.lock();
    for (int i = nextRow; i < nextRow + nbPrefetch && i < list->count(); i++) {
        if (itemToPix.count(list->item(i)) == 0) {
            toProcess.enqueue(list->item(i));
            nonEmptyToProcess.release();
        }
    }
    mutexFetch.unlock();

    QMutexLocker locker(&mutexCache);
    if (itemToPix.count(item) == 0 || itemToPix[item]->isNull()) {
        // dont bother with the prefetcher for this one, it would only take more time
        putInCache(item, new QImage(item->data(Qt::UserRole).toString()));
    } else {
        for (int i = 0; i < lruCache.size(); ++i) {
            if (lruCache[i] == item) {
                lruCache.remove(i);
                break;
            }
        }
        lruCache.push_back(item);
    }

    return QPixmap::fromImage(*itemToPix[item]);
}

void ListPixmapPrefetcher::newInCache(QListWidgetItem* item, QImage* pixmap) {
    QMutexLocker locker(&mutexCache);
    putInCache(item, pixmap);
}

void ListPixmapPrefetcher::putInCache(QListWidgetItem* item, QImage* pixmap) {
    freeCache(item);
    itemToPix[item] = pixmap;
}

void ListPixmapPrefetcher::freeCache(QListWidgetItem* item) {
    if (lruCache.size() >= cacheSize) {
        delete itemToPix[lruCache[0]];
        itemToPix.remove(lruCache[0]);
        lruCache.remove(0);
    }
    lruCache.push_back(item);
}
