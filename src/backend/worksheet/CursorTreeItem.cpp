/***************************************************************************
File                 : TreeModel.cpp
Project              : LabPlot
Description 	     : This is Treemodel used to show the cursor data in the cursor dock
--------------------------------------------------------------------
Copyright            : (C) 2019 Martin Marmsoler (martin.marmsoler@gmail.com)

***************************************************************************/

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

#include "CursorTreeItem.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"

CursorCurveTreeItem::CursorCurveTreeItem(const XYCurve *curve, TreeItem* parent):
	TreeItem(QVector<QVariant>(), parent),
	m_curve(curve) {
}

bool CursorCurveTreeItem::updateValue(double xpos, int cursor) {
	bool valueFound = false;
	if (cursor == 0)
		m_valCursor0 = m_curve->y(xpos, valueFound);
	else
		m_valCursor1 = m_curve->y(xpos, valueFound);

	return valueFound;
}

QVariant CursorCurveTreeItem::data(int column) const {
	if (column == 0)
		return m_curve->name();
	else if (column == 2)
		return m_valCursor0;
	else if (column == 2)
		return m_valCursor1;
	else
		return m_valCursor1 - m_valCursor0;
}

QVariant CursorCurveTreeItem::backgroundColor() const {
	QColor color = m_curve->linePen().color();
	color.setAlpha(alpha);
	return color;
}

// The data is calculated in update Value and therefore no setData needed
bool CursorCurveTreeItem::setData(int column, const QVariant &value) {
	return true;
}

// The color comes directly from the curve, so no set needed
bool CursorCurveTreeItem::setBackgroundColor(int column, const QVariant &value) {
	return true;
}

int CursorCurveTreeItem::columnCount() const {
	return 4; // name, cursor0, cursor1, diff
}

//###################################################################################################
// CursorPlotTreeItem
//###################################################################################################

CursorPlotTreeItem::CursorPlotTreeItem(const CartesianPlot* plot, TreeItem* parent) :
	TreeItem(QVector<QVariant>(), parent),
	m_plot(plot) {

}

int CursorPlotTreeItem::columnCount() const {
	return 1; // only the name
}

QVariant CursorPlotTreeItem::data(int column) const {
	if (column != 0)
		return QVariant();
	return m_plot->name();
}
