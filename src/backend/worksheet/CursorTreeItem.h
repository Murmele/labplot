#ifndef CURSORTREEMODEL_H
#define CURSORTREEMODEL_H

#include <cmath>
#include "backend/worksheet/TreeModel.h"

class XYCurve;
class CartesianPlot;

namespace CursorTreeItem {
	enum Column {PLOTNAME = 0, SIGNALNAME = 0, CURSOR0, CURSOR1, CURSORDIFF};
}

class CursorCurveTreeItem : public TreeItem
{
public:
	CursorCurveTreeItem(const XYCurve* curve, TreeItem* parent = nullptr);
	bool updateValue(double xpos, int cursor);
	QVariant data(int column) const;
	QVariant backgroundColor() const;
	bool setData(int column, const QVariant &value);
	bool setBackgroundColor(int column, const QVariant &value);
	int columnCount() const;

private:
	const XYCurve* m_curve;
	double m_valCursor0{NAN}; // TODO: use c++ NAN
	double m_valCursor1{NAN};
	const int alpha{50}; // transparancy of the background color [%]
};

class CursorPlotTreeItem : public TreeItem
{
public:
	int columnCount() const;
	CursorPlotTreeItem(const CartesianPlot* plot, TreeItem* parent = nullptr);
	QVariant data(int column) const;
private:
	const CartesianPlot* m_plot;
};

#endif // CURSORTREEMODEL_H
