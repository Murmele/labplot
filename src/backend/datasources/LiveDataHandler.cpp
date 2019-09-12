#include "LiveDataHandler.h"
#include "LiveDataHandlerPrivate.h"

#include <KLocalizedString>
#include "../lib/macros.h"
#include "filters/AsciiFilter.h"
#include "filters/BinaryFilter.h"
#include "filters/HDF5Filter.h"
#include "filters/NetCDFFilter.h"
#include "filters/ImageFilter.h"
#include "filters/NgspiceRawAsciiFilter.h"
#include "filters/NgspiceRawBinaryFilter.h"
#include "filters/JsonFilter.h"
#include "filters/FITSFilter.h"
#include "filters/ROOTFilter.h"

#include <QObject>

// Problem: Undefined reference to vtable



LiveDataHandler::LiveDataHandler(QObject *parent, QString host, QString port, QString serialPort, int baudRate, int sourceType):
	QObject(parent),
	d_ptr(new LiveDataHandlerPrivate(this, this)) {
	d_ptr->m_host = host;
	d_ptr->m_port = port;
	d_ptr->m_serialPort = serialPort;
	d_ptr->m_baudrate = baudRate;
	d_ptr->m_sourceType = static_cast<SourceType>(sourceType);
}

LiveDataHandler::~LiveDataHandler() {
	//delete d_ptr; // needed?
}

/*!
 * \brief Called from Object which wants a preview of the data
 */
void LiveDataHandler::preview(int nbrOfLines) {
	Q_D(LiveDataHandler);
	d->m_host;
	d->preview(nbrOfLines);
	switch(d->m_fileType) {
	case AbstractFileFilter::Ascii: {
		auto* filter = static_cast<AsciiFilter*>(d->m_currentFilter);
		emit previewData(d->preview(nbrOfLines), filter->vectorNames(), filter->columnModes());
		break;
	} case AbstractFileFilter::Binary: {
		auto* filter = static_cast<BinaryFilter*>(d->m_currentFilter);
		QStringList list;
		QVector<AbstractColumn::ColumnMode> columnModes;
		emit previewData(d->preview(nbrOfLines), list, columnModes);
		break;
	} case AbstractFileFilter::Image: {
		break;
	} case AbstractFileFilter::NETCDF: {
		auto* filter = static_cast<NetCDFFilter*>(d->m_currentFilter);
		QStringList list;
		QVector<AbstractColumn::ColumnMode> columnModes;
		emit previewData(d->preview(nbrOfLines), list, columnModes);
		break;
	} case AbstractFileFilter::HDF5: {
		auto* filter = static_cast<HDF5Filter*>(d->m_currentFilter);
		QStringList list;
		QVector<AbstractColumn::ColumnMode> columnModes;
		emit previewData(d->preview(nbrOfLines), list, columnModes);
		break;
	} case AbstractFileFilter::FITS: {
		auto* filter = static_cast<FITSFilter*>(d->m_currentFilter);
		//filter->setStartRow(row);
		break;
	} case AbstractFileFilter::JSON: {
		auto* filter = static_cast<JsonFilter*>(d->m_currentFilter);
		//filter->setStartRow(row);
		break;
	} case AbstractFileFilter::ROOT: {
		auto* filter = static_cast<ROOTFilter*>(d->m_currentFilter);
		//filter->setStartRow(row);
		break;
	} case AbstractFileFilter::NgspiceRawAscii: {
		auto* filter = static_cast<NgspiceRawAsciiFilter*>(d->m_currentFilter);
		//filter->setStartRow(row);
		break;
	} case AbstractFileFilter::NgspiceRawBinary: {
		auto* filter = static_cast<ROOTFilter*>(d->m_currentFilter);
		//filter->setStartRow(row);
		break;
	}

	}
}

void LiveDataHandler::preview() {
}

void LiveDataHandler::fileTypeToAscii(int index, int startRow, int endRow, int startColumn, int endColumn) {
	Q_D(LiveDataHandler);
	d->m_autoModeEnabled = false;
	if (index == 0)
		d->m_autoModeEnabled = true;
	d->m_startRow = startRow;
	d->m_endRow = endRow;
	d->m_startColumn = startColumn;
	d->m_endColumn = endColumn;
}

void LiveDataHandler::fileTypeToBinary(int index, int startRow, int endRow) {
	Q_D(LiveDataHandler);
	d->m_autoModeEnabled = false;
	if (index == 0)
		d->m_autoModeEnabled = true;
	d->m_startRow = startRow;
	d->m_endRow = endRow;
}

void LiveDataHandler::fileTypeToROOT(int startRow, int endRow, QVector<QStringList> columns) {
	Q_D(LiveDataHandler);
	d->m_startRow = startRow;
	d->m_endRow = endRow;
	d->m_columns = columns;
}

void LiveDataHandler::fileTypeToHDF5(int startRow, int endRow, int startColumn, int endColumn) {
	Q_D(LiveDataHandler);
	d->m_startRow = startRow;
	d->m_endRow = endRow;
	d->m_startColumn = startColumn;
	d->m_endColumn = endColumn;
}

void LiveDataHandler::fileTypeToNETCDF(int startRow, int endRow, int startColumn, int endColumn) {
	Q_D(LiveDataHandler);
	d->m_startRow = startRow;
	d->m_endRow = endRow;
	d->m_startColumn = startColumn;
	d->m_endColumn = endColumn;
}

void LiveDataHandler::fileTypeToFITS(int startRow, int endRow, int startColumn, int endColumn) {
	Q_D(LiveDataHandler);
	d->m_startRow = startRow;
	d->m_endRow = endRow;
	d->m_startColumn = startColumn;
	d->m_endColumn = endColumn;
}

void LiveDataHandler::fileTypeToImage(int startRow, int endRow, int startColumn, int endColumn) {
	Q_D(LiveDataHandler);
	d->m_startRow = startRow;
	d->m_endRow = endRow;
	d->m_startColumn = startColumn;
	d->m_endColumn = endColumn;
}

void LiveDataHandler::fileTypeToNgspiceRawAscii(int startRow, int endRow) {
	Q_D(LiveDataHandler);
	d->m_startRow = startRow;
	d->m_endRow = endRow;
}

void LiveDataHandler::fileTypeToNgspiceRawBinary(int startRow, int endRow) {
	Q_D(LiveDataHandler);
	d->m_startRow = startRow;
	d->m_endRow = endRow;
}

void LiveDataHandler::filterTypeToJSON(int startRow, int endRow, int startColumn, int endColumn) {
	Q_D(LiveDataHandler);
	d->m_startRow = startRow;
	d->m_endRow = endRow;
	d->m_startColumn = startColumn;
	d->m_endColumn = endColumn;
}

bool LiveDataHandler::getLine(QString& line) {
	Q_D(LiveDataHandler);
	return d->getLine(line);
}

//############
// SETTER
//############

void LiveDataHandler::setHost(QString host) {
	Q_D(LiveDataHandler);
	d->m_host = host;
	d->changeSettings();
}

void LiveDataHandler::setPort(QString port) {
	Q_D(LiveDataHandler);
	d->m_port = port;
	d->changeSettings();
}

void LiveDataHandler::setSerialPort(QString serialPort) {
	Q_D(LiveDataHandler);
	d->m_serialPort = serialPort;
	d->changeSettings();
}

void LiveDataHandler::setBaudRate(QString baudrate) {
	Q_D(LiveDataHandler);
	d->m_baudrate = baudrate.toInt();
	d->changeSettings();
}

void LiveDataHandler::setFileName(QString fileName) {
	Q_D(LiveDataHandler);
	d->m_fileName = fileName;
	d->changeSettings();
}

void LiveDataHandler::setSourceType(int sourceType) {
	Q_D(LiveDataHandler);
	d->m_sourceType = static_cast<LiveDataHandler::SourceType>(sourceType);
	d->changeSettings();
}

void LiveDataHandler::setFileType(int fileType) {
	Q_D(LiveDataHandler);
	d->m_fileType = static_cast<AbstractFileFilter::FileType>(fileType);
	d->changeSettings();
}

// filter
void LiveDataHandler::setAutoModeEnabled(int value) {
	bool enable = true;
	if (value != 0)
		enable = false;

	Q_D(LiveDataHandler);
	d->m_autoModeEnabled = enable;

	switch(d->m_fileType) {
	case AbstractFileFilter::Ascii: {
		auto* filter = static_cast<AsciiFilter*>(d->m_currentFilter);
		filter->setAutoModeEnabled(enable);
		break;
	} case AbstractFileFilter::Binary: {
		auto* filter = static_cast<BinaryFilter*>(d->m_currentFilter);
		filter->setAutoModeEnabled(enable);
		break;
	}
	}

}

void LiveDataHandler::setStartRow(int row) {
	Q_D(LiveDataHandler);
	d->m_startRow = row;

	switch(d->m_fileType) {
	case AbstractFileFilter::Ascii: {
		auto* filter = static_cast<AsciiFilter*>(d->m_currentFilter);
		filter->setStartRow(row);
		break;
	} case AbstractFileFilter::Binary: {
		auto* filter = static_cast<BinaryFilter*>(d->m_currentFilter);
		filter->setStartRow(row);
		break;
	} case AbstractFileFilter::Image: {
		auto* filter = static_cast<ImageFilter*>(d->m_currentFilter);
		filter->setStartRow(row);
		break;
	} case AbstractFileFilter::NETCDF: {
		auto* filter = static_cast<NetCDFFilter*>(d->m_currentFilter);
		filter->setStartRow(row);
		break;
	} case AbstractFileFilter::HDF5: {
		auto* filter = static_cast<HDF5Filter*>(d->m_currentFilter);
		filter->setStartRow(row);
		break;
	} case AbstractFileFilter::FITS: {
		auto* filter = static_cast<FITSFilter*>(d->m_currentFilter);
		filter->setStartRow(row);
		break;
	} case AbstractFileFilter::JSON: {
		auto* filter = static_cast<JsonFilter*>(d->m_currentFilter);
		filter->setStartRow(row);
		break;
	} case AbstractFileFilter::ROOT: {
		auto* filter = static_cast<ROOTFilter*>(d->m_currentFilter);
		filter->setStartRow(row);
		break;
	} case AbstractFileFilter::NgspiceRawAscii: {
		auto* filter = static_cast<NgspiceRawAsciiFilter*>(d->m_currentFilter);
		filter->setStartRow(row);
		break;
	} case AbstractFileFilter::NgspiceRawBinary: {
		auto* filter = static_cast<ROOTFilter*>(d->m_currentFilter);
		filter->setStartRow(row);
		break;
	}

	}
}

void LiveDataHandler::setEndRow(int row) {
	Q_D(LiveDataHandler);
	d->m_endRow = row;

	switch(d->m_fileType) {
	case AbstractFileFilter::Ascii: {
		auto* filter = static_cast<AsciiFilter*>(d->m_currentFilter);
		filter->setEndRow(row);
		break;
	} case AbstractFileFilter::Binary: {
		auto* filter = static_cast<BinaryFilter*>(d->m_currentFilter);
		filter->setEndRow(row);
		break;
	} case AbstractFileFilter::Image: {
		auto* filter = static_cast<ImageFilter*>(d->m_currentFilter);
		filter->setEndRow(row);
		break;
	} case AbstractFileFilter::NETCDF: {
		auto* filter = static_cast<NetCDFFilter*>(d->m_currentFilter);
		filter->setEndRow(row);
		break;
	} case AbstractFileFilter::HDF5: {
		auto* filter = static_cast<HDF5Filter*>(d->m_currentFilter);
		filter->setEndRow(row);
		break;
	} case AbstractFileFilter::FITS: {
		auto* filter = static_cast<FITSFilter*>(d->m_currentFilter);
		filter->setEndRow(row);
		break;
	} case AbstractFileFilter::JSON: {
		auto* filter = static_cast<JsonFilter*>(d->m_currentFilter);
		filter->setEndRow(row);
		break;
	} case AbstractFileFilter::ROOT: {
		auto* filter = static_cast<ROOTFilter*>(d->m_currentFilter);
		filter->setEndRow(row);
		break;
	} case AbstractFileFilter::NgspiceRawAscii: {
		auto* filter = static_cast<NgspiceRawAsciiFilter*>(d->m_currentFilter);
		filter->setEndRow(row);
		break;
	} case AbstractFileFilter::NgspiceRawBinary: {
		auto* filter = static_cast<ROOTFilter*>(d->m_currentFilter);
		filter->setEndRow(row);
		break;
	}

	}
}

void LiveDataHandler::setStartColumn(int column) {
	Q_D(LiveDataHandler);
	d->m_startColumn = column;

	switch(d->m_fileType) {
	case AbstractFileFilter::Ascii: {
		auto* filter = static_cast<AsciiFilter*>(d->m_currentFilter);
		filter->setStartColumn(column);
		break;
	} case AbstractFileFilter::Image: {
		auto* filter = static_cast<ImageFilter*>(d->m_currentFilter);
		filter->setStartColumn(column);
		break;
	} case AbstractFileFilter::NETCDF: {
		auto* filter = static_cast<NetCDFFilter*>(d->m_currentFilter);
		filter->setStartColumn(column);
		break;
	} case AbstractFileFilter::HDF5: {
		auto* filter = static_cast<HDF5Filter*>(d->m_currentFilter);
		filter->setStartColumn(column);
		break;
	} case AbstractFileFilter::FITS: {
		auto* filter = static_cast<FITSFilter*>(d->m_currentFilter);
		filter->setStartColumn(column);
		break;
	} case AbstractFileFilter::JSON: {
		auto* filter = static_cast<JsonFilter*>(d->m_currentFilter);
		filter->setStartColumn(column);
		break;
	}

	}
}

void LiveDataHandler::setEndColumn(int column) {
	Q_D(LiveDataHandler);
	d->m_endColumn = column;

	switch(d->m_fileType) {
	case AbstractFileFilter::Ascii: {
		auto* filter = static_cast<AsciiFilter*>(d->m_currentFilter);
		filter->setEndColumn(column);
		break;
	} case AbstractFileFilter::Image: {
		auto* filter = static_cast<ImageFilter*>(d->m_currentFilter);
		filter->setEndColumn(column);
		break;
	} case AbstractFileFilter::NETCDF: {
		auto* filter = static_cast<NetCDFFilter*>(d->m_currentFilter);
		filter->setEndColumn(column);
		break;
	} case AbstractFileFilter::HDF5: {
		auto* filter = static_cast<HDF5Filter*>(d->m_currentFilter);
		filter->setEndColumn(column);
		break;
	} case AbstractFileFilter::FITS: {
		auto* filter = static_cast<FITSFilter*>(d->m_currentFilter);
		filter->setEndColumn(column);
		break;
	} case AbstractFileFilter::JSON: {
		auto* filter = static_cast<JsonFilter*>(d->m_currentFilter);
		filter->setEndColumn(column);
		break;
	}

	}
}

void LiveDataHandler::setImportFormat(const ImageFilter::ImportFormat format) {
	Q_D(LiveDataHandler);
	d->m_format = format;

	switch(d->m_fileType) {
	case AbstractFileFilter::Image: {
		auto* filter = static_cast<ImageFilter*>(d->m_currentFilter);
		filter->setImportFormat(format);
		break;
	}
	}
}

void LiveDataHandler::setCurrentDataSetName(const QStringList name) {
	Q_D(LiveDataHandler);
	d->m_dataSetName = name;
	if (name.length() < 1)
		return;
	switch(d->m_fileType) {
	case AbstractFileFilter::HDF5: {
		auto* filter = static_cast<HDF5Filter*>(d->m_currentFilter);
		filter->setCurrentDataSetName(name[0]);
		break;
	}
	}
}

void LiveDataHandler::setCurrentVarName(const QStringList name) {
	Q_D(LiveDataHandler);
	d->m_varName = name;

	if (name.length() < 1)
		return;
	switch(d->m_fileType) {
	case AbstractFileFilter::NETCDF: {
		auto* filter = static_cast<NetCDFFilter*>(d->m_currentFilter);
		filter->setCurrentVarName(name[0]);
		break;
	}
	}
}

void LiveDataHandler::setCurrentObject(const QStringList object) {
	Q_D(LiveDataHandler);
	d->m_object = object;

	if (object.length() < 1)
		return;
	switch(d->m_fileType) {
	case AbstractFileFilter::ROOT: {
		auto* filter = static_cast<ROOTFilter*>(d->m_currentFilter);
		filter->setCurrentObject(object[0]);
		break;
	}
	}
}
