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

#include "imagesorter.h"

void MergeSortImageSorter::moveItem(QListWidgetItem* item) {
    int row = list->row(item);
    if (row != currentListPos) {
        list->takeItem(row);
        list->insertItem(currentListPos, item);
    }
    currentListPos++;
}

void MergeSortImageSorter::removeItem(QListWidgetItem* item) {
    list->takeItem(list->row(item));
}

void MergeSortImageSorter::finishMerge() {
    // note: merge in 'bfs-fashion'
    /* if one list to merge is 'empty' (i.e there is nothing more to select from) then
     *  - put the rest of other list in the result list
     *  - select the next two lists to merge either
     */
    if (currentB2 == items[currentB1].size() || currentA2 == items[currentA1].size()) {
        for (int i = currentA2; i < items[currentA1].size(); ++i) {
            itemResults.back().push_back(items[currentA1][currentA2]);
            moveItem(itemResults.back().back());
        }
        for (int i = currentB2; i < items[currentB1].size(); ++i) {
            itemResults.back().push_back(items[currentB1][currentB2]);
            moveItem(itemResults.back().back());
        }
        // move to the next items
        currentA1 += 2;
        currentB1 += 2;
        /* if there is a 'lone list' (like when there are 3 lists to merge and we have already
         * merged the first 2) then add this list to the results.
         */
        if (currentB1 >= items.size() && currentA1 < items.size()) {
            itemResults.push_back(QVector<QListWidgetItem*>(items.back()));
            currentA1 += 2;
            currentB1 += 2;
        }
        if (currentA1 >= items.size()) {
            items = itemResults;
            itemResults.clear();
            currentA1 = 0;
            currentB1 = 1;
        }
        currentA2 = 0;
        currentB2 = 0;
        currentListPos = 0;
        itemResults.push_back(QVector<QListWidgetItem*>());
    }
}

void MergeSortImageSorter::setCurrentBest(QListWidgetItem* item) {
    if (items.size() == 1) {
        return;
    }
    if (items[currentB1][currentB2] == item) {
        itemResults.back().push_back(items[currentB1][currentB2]);
        moveItem(item);
        currentB2++;
    } else {
        itemResults.back().push_back(items[currentA1][currentA2]);
        moveItem(item);
        currentA2++;
    }

    finishMerge();
}

void MergeSortImageSorter::discardItem(QListWidgetItem* item) {

    if (items.size() == 1) {
        return;
    }
    if (items[currentB1][currentB2] == item) {
        removeItem(item);
        currentB2++;
    } else {
        removeItem(item);
        currentA2++;
    }

    finishMerge();
}

QVector<QListWidgetItem*> MergeSortImageSorter::nextToSelectFrom() {
    QVector<QListWidgetItem*> tmp;
    if (items.size() > 1) {
        tmp.push_back(items[currentA1][currentA2]);
        tmp.push_back(items[currentB1][currentB2]);
    }
    return tmp;
}
