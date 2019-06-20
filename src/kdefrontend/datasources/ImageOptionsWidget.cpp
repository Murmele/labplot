/***************************************************************************
File                 : ImageOptionsWidget.cpp
Project              : LabPlot
Description          : widget providing options for the import of image data
--------------------------------------------------------------------
Copyright            : (C) 2015-2017 Stefan Gerlach (stefan.gerlach@uni.kn)
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
#include "ImageOptionsWidget.h"

#include <KSharedConfig>
#include <KConfigGroup>

 /*!
	\class ImageOptionsWidget
	\brief Widget providing options for the import of image data

	\ingroup kdefrontend
 */

ImageOptionsWidget::ImageOptionsWidget(QWidget* parent) : QWidget(parent) {
	ui.setupUi(parent);

	ui.cbImportFormat->addItems(ImageFilter::importFormats());

	const QString textImageFormatShort = i18n("This option determines how the image is converted when importing.");

	ui.lImportFormat->setToolTip(textImageFormatShort);
	ui.lImportFormat->setWhatsThis(textImageFormatShort);
	ui.cbImportFormat->setToolTip(textImageFormatShort);
	ui.cbImportFormat->setWhatsThis(textImageFormatShort);

	connect(ui.cbImportFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ImageOptionsWidget::cbImportFormatIndexChanged);
}

void ImageOptionsWidget::cbImportFormatIndexChanged() {
	emit currentFormatChanged(static_cast<ImageFilter::ImportFormat>(ui.cbImportFormat->currentIndex()));
}

void ImageOptionsWidget::loadSettings() const {
	KConfigGroup conf(KSharedConfig::openConfig(), "Import");

	ui.cbImportFormat->setCurrentIndex(conf.readEntry("ImportFormat", 0));
}

void ImageOptionsWidget::saveSettings() {
	KConfigGroup conf(KSharedConfig::openConfig(), "Import");

	conf.writeEntry("ImportFormat", ui.cbImportFormat->currentIndex());
}
