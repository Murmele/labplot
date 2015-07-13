
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

#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include "ui_imagewidget.h"
#include "backend/worksheet/Image.h"

class CustomItem;
class CustomItemWidget;
class QxtSpanSlider;

class ImageWidget: public QWidget{
	Q_OBJECT

public:
    explicit ImageWidget(QWidget *);


    void setImages(QList<Image*>);
	void load();

    QxtSpanSlider* ssIntensity;
    QxtSpanSlider* ssForeground;
    QxtSpanSlider* ssHue;
    QxtSpanSlider* ssSaturation;
    QxtSpanSlider* ssValue;

private:
	Ui::ImageWidget ui;
	Image *m_image;
    QList<CustomItem*> m_itemsList;
    QList<Image*> m_imagesList;
	bool m_initializing;
    CustomItemWidget* customItemWidget;

    void initConnections();

signals:
    //delete it
	void dataChanged(bool);

private slots:
    void rotationChanged(double);
    void fileNameChanged();
    void xErrorTypeChanged(int);
    void yErrorTypeChanged(int);
    void minSegmentLengthChanged(int);
    void pointSeparationChanged(int);
    void selectFile();
    void graphTypeChanged();
    void logicalPositionChanged();

    void imageFileNameChanged(const QString&);
    void imageRotationAngleChanged(float);
    void imageAxisPointsChanged(const Image::ReferencePoints&);
    void imageEditorSettingsChanged(const Image::EditorSettings&);
    void updateCustomItemList();
    void handleAspectAdded();
    void handleWidgetActions();

    void plotImageTypeChanged(int);
    void plotErrorsChanged(Image::Errors);
    void intensitySpanChanged(int, int);
    void foregroundSpanChanged(int, int);
    void hueSpanChanged(int, int);
    void saturationSpanChanged(int, int);
    void valueSpanChanged(int, int);
    void rbClicked();
};

#endif
