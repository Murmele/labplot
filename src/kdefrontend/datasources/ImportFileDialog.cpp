/***************************************************************************
    File                 : ImportDialog.cc
    Project              : LabPlot
    Description          : import file data dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2019 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2008-2015 by Stefan Gerlach (stefan.gerlach@uni.kn)

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

#include "ImportFileDialog.h"
#include "ImportFileWidget.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/datasources/LiveDataSource.h"
#include "backend/datasources/filters/AbstractFileFilter.h"
#include "backend/datasources/filters/filters.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/matrix/Matrix.h"
#include "backend/core/Workbook.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/MainWin.h"

#ifdef HAVE_MQTT
#include "backend/datasources/MQTTClient.h"
#endif

#include <KMessageBox>
#include <KSharedConfig>
#include <KWindowConfig>
#include <KLocalizedString>

#include <QDialogButtonBox>
#include <QProgressBar>
#include <QTcpSocket>
#include <QLocalSocket>
#include <QUdpSocket>
#include <QStatusBar>
#include <QDir>
#include <QInputDialog>
#include <QMenu>
#include <QWindow>

/*!
	\class ImportFileDialog
	\brief Dialog for importing data from a file. Embeds \c ImportFileWidget and provides the standard buttons.

	\ingroup kdefrontend
 */

ImportFileDialog::ImportFileDialog(MainWin* parent, bool liveDataSource, const QString& fileName) : ImportDialog(parent),
	m_importFileWidget(new ImportFileWidget(this, liveDataSource, fileName)) {

	vLayout->addWidget(m_importFileWidget);

	//dialog buttons
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Reset |QDialogButtonBox::Cancel);
	okButton = buttonBox->button(QDialogButtonBox::Ok);
	m_optionsButton = buttonBox->button(QDialogButtonBox::Reset); //we highjack the default "Reset" button and use if for showing/hiding the options
	okButton->setEnabled(false); //ok is only available if a valid container was selected
	vLayout->addWidget(buttonBox);

	//hide the data-source related widgets
	if (!liveDataSource)
		setModel();

	//Signals/Slots
	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	if (!liveDataSource)
		setWindowTitle(i18nc("@title:window", "Import Data to Spreadsheet or Matrix"));
	else
		setWindowTitle(i18nc("@title:window", "Add New Live Data Source"));

	setWindowIcon(QIcon::fromTheme("document-import-database"));

	//restore saved settings if available
	create(); // ensure there's a window created

	QApplication::processEvents(QEventLoop::AllEvents, 0);
	m_importFileWidget->loadSettings();

	KConfigGroup conf(KSharedConfig::openConfig(), "ImportFileDialog");
	if (conf.exists()) {
		m_showOptions = conf.readEntry("ShowOptions", false);
		m_showOptions ? m_optionsButton->setText(i18n("Hide Options")) : m_optionsButton->setText(i18n("Show Options"));
		m_importFileWidget->showOptions(m_showOptions);

		//do the signal-slot connections after all settings were loaded in import file widget and check the OK button after this
		connect(m_importFileWidget, &ImportFileWidget::checkedFitsTableToMatrix, this, &ImportFileDialog::checkOnFitsTableToMatrix);
		connect(m_importFileWidget, &ImportFileWidget::checkOKButton, this, QOverload<bool, QString>::of(&ImportFileDialog::checkOkButton)); // with C++14: connect(m_importFileWidget, &ImportFileWidget::checkOKButton, this, qOverload<bool, QString>::(&ImportFileDialog::checkOkButton));

		//TODO: do we really need to check the ok button when the preview was refreshed?
		//If not, remove this together with the previewRefreshed signal in ImportFileWidget
		//connect(m_importFileWidget, &ImportFileWidget::previewRefreshed, this, &ImportFileDialog::checkOkButton);
#ifdef HAVE_MQTT
		connect(m_importFileWidget, &ImportFileWidget::subscriptionsChanged, this, &ImportFileDialog::checkOkButton);
		connect(m_importFileWidget, &ImportFileWidget::checkFileType, this, &ImportFileDialog::checkOkButton);
#endif
		connect(m_optionsButton, &QPushButton::clicked, this, &ImportFileDialog::toggleOptions);

		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(0, 0).expandedTo(minimumSize()));
}

ImportFileDialog::~ImportFileDialog() {
	//save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportFileDialog");
	conf.writeEntry("ShowOptions", m_showOptions);
	if (cbPosition)
		conf.writeEntry("Position", cbPosition->currentIndex());

	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

int ImportFileDialog::sourceType() const {
	return static_cast<int>(m_importFileWidget->currentSourceType());
}

/*!
  triggers data import to the live data source \c source
*/
void ImportFileDialog::importToLiveDataSource(LiveDataSource* source, QStatusBar* statusBar) const {
	DEBUG("ImportFileDialog::importToLiveDataSource()");
	m_importFileWidget->saveSettings(source);

	//show a progress bar in the status bar
	auto* progressBar = new QProgressBar();
	progressBar->setRange(0, 100);
	connect(source->filter(), &AbstractFileFilter::completed, progressBar, &QProgressBar::setValue);

	statusBar->clearMessage();
	statusBar->addWidget(progressBar, 1);
	WAIT_CURSOR;

	QTime timer;
	timer.start();
	DEBUG("	Initial read()");
	source->read();
	statusBar->showMessage( i18n("Live data source created in %1 seconds.", (float)timer.elapsed()/1000) );

	RESET_CURSOR;
	statusBar->removeWidget(progressBar);
	source->ready();
}

#ifdef HAVE_MQTT
/*!
  triggers data import to the MQTTClient \c client
*/
void ImportFileDialog::importToMQTT(MQTTClient* client) const{
	m_importFileWidget->saveMQTTSettings(client);
	client->read();
	client->ready();
}
#endif

/*!
  triggers data import to the currently selected data container
*/
void ImportFileDialog::importTo(QStatusBar* statusBar) const {
	DEBUG("ImportFileDialog::importTo()");
	QDEBUG("	cbAddTo->currentModelIndex() =" << cbAddTo->currentModelIndex());
	AbstractAspect* aspect = static_cast<AbstractAspect*>(cbAddTo->currentModelIndex().internalPointer());
	if (!aspect) {
		DEBUG("ERROR in importTo(): No aspect available");
		DEBUG("	cbAddTo->currentModelIndex().isValid() = " << cbAddTo->currentModelIndex().isValid());
		DEBUG("	cbAddTo->currentModelIndex() row/column = " << cbAddTo->currentModelIndex().row() << ' ' << cbAddTo->currentModelIndex().column());
		return;
	}

	if (m_importFileWidget->isFileEmpty()) {
		KMessageBox::information(nullptr, i18n("No data to import."), i18n("No Data"));
		return;
	}

	QString fileName = m_importFileWidget->fileName();
	auto filter = m_importFileWidget->currentFileFilter();
	auto mode = AbstractFileFilter::ImportMode(cbPosition->currentIndex());

	//show a progress bar in the status bar
	auto* progressBar = new QProgressBar();
	progressBar->setRange(0, 100);
	connect(filter, &AbstractFileFilter::completed, progressBar, &QProgressBar::setValue);

	statusBar->clearMessage();
	statusBar->addWidget(progressBar, 1);

	WAIT_CURSOR;
	QApplication::processEvents(QEventLoop::AllEvents, 100);

	QTime timer;
	timer.start();

	if (aspect->inherits("Matrix")) {
		DEBUG("	to Matrix");
		auto* matrix = qobject_cast<Matrix*>(aspect);
		filter->readDataFromFile(fileName, matrix, mode);
	} else if (aspect->inherits("Spreadsheet")) {
		DEBUG("	to Spreadsheet");
		auto* spreadsheet = qobject_cast<Spreadsheet*>(aspect);
		DEBUG(" Calling readDataFromFile() with spreadsheet " << spreadsheet);
		filter->readDataFromFile(fileName, spreadsheet, mode);
	} else if (aspect->inherits("Workbook")) {
		DEBUG("	to Workbook");
		auto* workbook = qobject_cast<Workbook*>(aspect);
		QVector<AbstractAspect*> sheets = workbook->children<AbstractAspect>();

		AbstractFileFilter::FileType fileType = m_importFileWidget->currentFileType();
		// multiple data sets/variables for HDF5, NetCDF and ROOT
		if (fileType == AbstractFileFilter::HDF5 || fileType == AbstractFileFilter::NETCDF ||
			fileType == AbstractFileFilter::ROOT) {
			QStringList names;
			if (fileType == AbstractFileFilter::HDF5)
				names = m_importFileWidget->selectedHDF5Names();
			else if (fileType == AbstractFileFilter::NETCDF)
				names = m_importFileWidget->selectedNetCDFNames();
			else
				names = m_importFileWidget->selectedROOTNames();

			int nrNames = names.size(), offset = sheets.size();

			//TODO: think about importing multiple sets into one sheet

			int start = 0;	// add nrNames sheets (0 to nrNames)
			if (mode == AbstractFileFilter::Replace) // add only missing sheets (from offset to nrNames)
				start = offset;

			// add additional sheets
			for (int i = start; i < nrNames; ++i) {
				Spreadsheet *spreadsheet = new Spreadsheet(i18n("Spreadsheet"));
				if (mode == AbstractFileFilter::Prepend)
					workbook->insertChildBefore(spreadsheet, sheets[0]);
				else
					workbook->addChild(spreadsheet);
			}

			// start at offset for append, else at 0
			if (mode != AbstractFileFilter::Append)
				offset = 0;

			// import all sets to a different sheet
			sheets = workbook->children<AbstractAspect>();
			for (int i = 0; i < nrNames; ++i) {
				if (fileType == AbstractFileFilter::HDF5)
					static_cast<HDF5Filter*>(filter)->setCurrentDataSetName(names[i]);
				else if (fileType == AbstractFileFilter::NETCDF)
					static_cast<NetCDFFilter*>(filter)->setCurrentVarName(names[i]);
				else
					static_cast<ROOTFilter*>(filter)->setCurrentObject(names[i]);

				int index = i + offset;
				if (sheets[index]->inherits("Matrix"))
					filter->readDataFromFile(fileName, qobject_cast<Matrix*>(sheets[index]));
				else if (sheets[index]->inherits("Spreadsheet"))
					filter->readDataFromFile(fileName, qobject_cast<Spreadsheet*>(sheets[index]));
			}
		} else { // single import file types
			// use active spreadsheet/matrix if present, else new spreadsheet
			Spreadsheet* spreadsheet = workbook->currentSpreadsheet();
			Matrix* matrix = workbook->currentMatrix();
			if (spreadsheet)
				filter->readDataFromFile(fileName, spreadsheet, mode);
			else if (matrix)
				filter->readDataFromFile(fileName, matrix, mode);
			else {
				spreadsheet = new Spreadsheet(i18n("Spreadsheet"));
				workbook->addChild(spreadsheet);
				filter->readDataFromFile(fileName, spreadsheet, mode);
			}
		}
	}
	statusBar->showMessage(i18n("File %1 imported in %2 seconds.", fileName, (float)timer.elapsed()/1000));

	RESET_CURSOR;
	statusBar->removeWidget(progressBar);
}

void ImportFileDialog::toggleOptions() {
	m_importFileWidget->showOptions(!m_showOptions);
	m_showOptions = !m_showOptions;
	m_showOptions ? m_optionsButton->setText(i18n("Hide Options")) : m_optionsButton->setText(i18n("Show Options"));

	//resize the dialog
	layout()->activate();
	resize( QSize(this->width(), 0).expandedTo(minimumSize()) );
}

void ImportFileDialog::checkOnFitsTableToMatrix(const bool enable) {
	if (cbAddTo) {
		QDEBUG("cbAddTo->currentModelIndex() = " << cbAddTo->currentModelIndex());
		AbstractAspect* aspect = static_cast<AbstractAspect*>(cbAddTo->currentModelIndex().internalPointer());
		if (!aspect) {
			DEBUG("ERROR: no aspect available.");
			return;
		}

		if (aspect->inherits("Matrix")) {
			okButton->setEnabled(enable);
			if (enable)
				okButton->setToolTip(i18n("Close the dialog and import the data."));
			else
				okButton->setToolTip(i18n("Cannot import into a matrix since the data contains non-numerical data."));
		}
	}
}

void ImportFileDialog::checkOkButton() {
	checkOkButton(true, "");
}

void ImportFileDialog::checkOkButton(bool deviceOK, QString toolTip) {
	DEBUG("ImportFileDialog::checkOkButton()");
	if (cbAddTo) { //only check for the target container when no file data source is being added
		QDEBUG(" cbAddTo->currentModelIndex() = " << cbAddTo->currentModelIndex());
		AbstractAspect* aspect = static_cast<AbstractAspect*>(cbAddTo->currentModelIndex().internalPointer());
		if (!aspect || !deviceOK) {
			okButton->setEnabled(false);
			okButton->setToolTip(i18n("Select a data container where the data has to be imported into."));
			lPosition->setEnabled(false);
			cbPosition->setEnabled(false);
			okButton->setToolTip(i18n("Close the dialog and import the data."));
			return;
		} else {
			lPosition->setEnabled(true);
			cbPosition->setEnabled(true);
			okButton->setToolTip(toolTip);
			//when doing ASCII import to a matrix, hide the options for using the file header (first line)
			//to name the columns since the column names are fixed in a matrix
			const auto* matrix = dynamic_cast<const Matrix*>(aspect);
			m_importFileWidget->showAsciiHeaderOptions(matrix == nullptr);
		}
	}
}

QString ImportFileDialog::selectedObject() const {
	return m_importFileWidget->selectedObject();
}
