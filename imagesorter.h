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

#ifndef IMAGESORTER_H
#define IMAGESORTER_H

#include <QListWidget>

/*
 * Interface for merging algorithms to be used in photosorter
 */
class ImageSorter {
protected:
    QListWidget* list;
public:
    ImageSorter(QListWidget* _list) : list(_list) {}

    virtual ~ImageSorter() {}

    // the item we want to display after we're done sorting.
    virtual QListWidgetItem* getFirstChoice() {
        if (list->count() > 0) {
            return list->item(0);
        }
        return nullptr;
    }

    // item must be returned by the last call to 'nextToSelectFrom'
    virtual void setCurrentBest(QListWidgetItem* item) = 0;
    // item must be returned by the last call to 'nextToSelectFrom'
    virtual void discardItem(QListWidgetItem* item) = 0;

    virtual QVector<QListWidgetItem*> nextToSelectFrom() = 0;
};

/*
 * Implements basic merge sort
 */
class MergeSortImageSorter : public ImageSorter {
    // The current elements to sort from
    QVector<QVector<QListWidgetItem*>> items;
    // Where to put the result of the sorting of 'items'
    QVector<QVector<QListWidgetItem*>> itemResults;

    /* Maybe not the best names
     * items[currentA1][currentA2] and
     * items[currentB1][currentB2]
     * are the two current item options to choose from.
     */
    int currentA1 = 0;
    int currentA2 = 0;
    int currentB1 = 1;
    int currentB2 = 0;

    // The position of the newly selected item in the final list
    int currentListPos = 0;

    // move item to the currentListPos in item's ListWidget
    void moveItem(QListWidgetItem* item);
    // remove item from item's ListWidget
    void removeItem(QListWidgetItem* item);

    void finishMerge();

public:
    MergeSortImageSorter(QListWidget* _list) : ImageSorter(_list) {
        for (int i = 0; i < _list->count(); i++) {
            items.push_back({_list->item(i)});
        }
        itemResults.push_back(QVector<QListWidgetItem*>());
    }

    void setCurrentBest(QListWidgetItem* item) final;
    void discardItem(QListWidgetItem* item) final;
    QVector<QListWidgetItem*> nextToSelectFrom() final;

};

#endif // IMAGESORTER_H
