/***************************************************************************
    File                 : Plot2D.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach, Alexander Semke
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de
    Description          : 2d plot class

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
#ifndef PLOT2D_H
#define PLOT2D_H

#include "Plot.h"
#include "../elements/Range.h"
class Plot2D : public Plot{
public:
	Plot2D(AbstractScriptingEngine*, const QString &name);
	void draw(QPainter *p, const int w, const int h);

/*	~Plot2D();
	void saveXML(QDomDocument doc, QDomElement plottag);
	void openXML(QDomElement e);
	void saveAxes(QTextStream *t);
	void openAxes(QTextStream *t, int version);
	Axis *getAxis(int i) { return &axis[i]; }
*/
private:
	void setPlotRanges(const QList<Range>&);
// 	void setRanges(Range* r) {range[0]=r[0];range[1]=r[1];}
// 	void setActRanges(Range* r);
// 	void setRange(Range* r,int i) {range[i]=*r;}
// 	void setActRange(Range* r,int i);

	void drawAxes(QPainter *p, const int w, const int h);
	void drawAxesTicks(QPainter *p, const int w, const int h, const int k);
	void drawBorder(QPainter *p, const int w, const int h);
	void drawLegend(QPainter *p, const int w, const int h);
	virtual void drawCurves(QPainter *p, int w, int h) = 0;
	virtual void drawFill(QPainter *p, int w, int h) = 0;
};

#endif // PLOT2D_H
