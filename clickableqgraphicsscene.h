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

#ifndef CLICKABLEQGRAPHICSSCENE_H
#define CLICKABLEQGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

class ClickableQGraphicsScene : public QGraphicsScene {
    Q_OBJECT
public:

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) {
        QGraphicsItem* item = itemAt(event->scenePos(), QTransform());
        if (item) {
            if (event->button() == Qt::LeftButton) {
                emit clicked(item);
            } else if (event->button() == Qt::RightButton) {
                emit rightClicked(item);
            }
        }
    }

signals:

    void clicked(QGraphicsItem* item);
    void rightClicked(QGraphicsItem* item);

};
#endif // CLICKABLEQGRAPHICSSCENE_H
