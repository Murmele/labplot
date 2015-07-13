
/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#ifndef SEGMENT_H
#define SEGMENT_H

#include "backend/lib/macros.h"

class QGraphicsItem;
class QLine;

class SegmentPrivate;
class Image;

class Segment {

	public:
        explicit Segment(Image*);
		~Segment();

        QList<QLine*> path;
        int yLast;
        int length;

        QGraphicsItem *graphicsItem() const;
		void setParentGraphicsItem(QGraphicsItem*);

        bool isVisible() const;
        void setVisible(bool);

		typedef SegmentPrivate Private;

    public slots:
        void retransform();

	protected:
		SegmentPrivate* const d_ptr;

	private:
    	Q_DECLARE_PRIVATE(Segment)
        void init();
        Image* m_image;
};

#endif
