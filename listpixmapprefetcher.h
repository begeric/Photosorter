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

#ifndef LISTPIXMAPPREFETCHER_H
#define LISTPIXMAPPREFETCHER_H

#include <QQueue>
#include <QMap>
#include <QMutex>
#include <QSemaphore>
#include <QThread>
#include <QVector>

class QListWidgetItem;

// Consumer thread that waits for 'orders' from the main thread about which image to load
class Prefetcher : public QThread {
    Q_OBJECT

    QSemaphore& nonEmptyToProcess;
    QMutex& mutexFetch;
    QQueue<QListWidgetItem*>& toProcess;
public:
    Prefetcher(QSemaphore& _nonEmptyToProcess,
               QMutex& _mutexFetch,
               QQueue<QListWidgetItem*>& _toProcess
               ) : nonEmptyToProcess(_nonEmptyToProcess),
                   mutexFetch(_mutexFetch),
                   toProcess(_toProcess) {}

public slots:
    void run() Q_DECL_OVERRIDE;

signals:
    void loaded(QListWidgetItem* item, QImage* pixmap);
};

// Implements an image prefetcher as well as a LRU cache for said images
class ListPixmapPrefetcher : public QObject {
    Q_OBJECT

    QSemaphore nonEmptyToProcess;
    QMutex mutexFetch;
    QQueue<QListWidgetItem*> toProcess;

    constexpr static int nbPrefetch = 1;
    constexpr static int cacheSize = 5;

    QMutex mutexCache;
    QVector<QListWidgetItem*> lruCache;
    QMap<QListWidgetItem*, QImage*> itemToPix;
    Prefetcher thread;

public:
    ListPixmapPrefetcher();
    ~ListPixmapPrefetcher();

    QPixmap get(QListWidgetItem* item);

public slots:
    void newInCache(QListWidgetItem* item, QImage* pixmap);

private:
    void putInCache(QListWidgetItem* item, QImage* pixmap);
    void freeCache(QListWidgetItem* item);
};


#endif // LISTPIXMAPPREFETCHER_H
