/***************************************************************************
File                 : BinaryFilter.cpp
Project              : LabPlot
Description          : Binary I/O-filter
--------------------------------------------------------------------
Copyright            : (C) 2015-2018 by Stefan Gerlach (stefan.gerlach@uni.kn)
Copyright            : (C) 2017 Alexander Semke (alexander.semke@web.de)
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
#include "backend/datasources/filters/BinaryFilter.h"
#include "backend/datasources/filters/BinaryFilterPrivate.h"
#include "backend/datasources/AbstractDataSource.h"
#include "backend/core/column/Column.h"

#include <QDataStream>
#include <KLocalizedString>
#include <KFilterDev>
#include <cmath>

/*!
\class BinaryFilter
\brief Manages the import/export of data organized as columns (vectors) from/to a binary file.

\ingroup datasources
*/
BinaryFilter::BinaryFilter():AbstractFileFilter(Binary), d(new BinaryFilterPrivate(this)) {}

BinaryFilter::~BinaryFilter() = default;

/*!
  reads the content of the file \c fileName.
*/
void BinaryFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) {
	d->readDataFromFile(fileName, dataSource, importMode);
}

/*!
  reads the content of the device \c device.
*/
void BinaryFilter::readDataFromDevice(QIODevice& device, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode,  int lines) {
	d->readDataFromDevice(device, dataSource, importMode, lines);
}

QVector<QStringList> BinaryFilter::preview(const QString& fileName, int lines) {
	return d->preview(fileName, lines);
}

/*!
writes the content of the data source \c dataSource to the file \c fileName.
*/
void BinaryFilter::write(const QString & fileName, AbstractDataSource* dataSource) {
	d->write(fileName, dataSource);
// 	emit()
}

/*!
returns the list of all predefined data formats.
*/
QStringList BinaryFilter::dataTypes() {
	return (QStringList()
		<<"int8 (8 bit signed integer)"
		<<"int16 (16 bit signed integer)"
		<<"int32 (32 bit signed integer)"
		<<"int64 (64 bit signed integer)"
	        <<"uint8 (8 bit unsigned integer)"
		<<"uint16 (16 bit unsigned integer)"
		<<"uint32 (32 bit unsigned integer)"
		<<"uint64 (64 bit unsigned integer)"
	        <<"real32 (single precision floats)"
		<<"real64 (double precision floats)"
	);
}

/*!
returns the size of the predefined data types
*/
int BinaryFilter::dataSize(BinaryFilter::DataType type) {
	int sizes[] = {1,2,4,8,1,2,4,8,4,8};

	return sizes[(int)type];
}

/*!
  returns the number of rows (length of vectors) in the file \c fileName.
*/
size_t BinaryFilter::rowNumber(const QString& fileName, const size_t vectors, const BinaryFilter::DataType type) {
	KFilterDev device(fileName);
	if (!device.open(QIODevice::ReadOnly))
		return 0;

	size_t rows = 0;
	while (!device.atEnd()) {
		// one row
		for (size_t i = 0; i < vectors; ++i) {
			for (int j = 0; j < BinaryFilter::dataSize(type); ++j)
				device.read(1);
		}
		rows++;
	}

	return rows;
}

///////////////////////////////////////////////////////////////////////
/*!
  loads the predefined filter settings for \c filterName
*/
void BinaryFilter::loadFilterSettings(const QString& filterName) {
	Q_UNUSED(filterName);
}

/*!
  saves the current settings as a new filter with the name \c filterName
*/
void BinaryFilter::saveFilterSettings(const QString& filterName) const {
	Q_UNUSED(filterName);
}

///////////////////////////////////////////////////////////////////////

void BinaryFilter::setVectors(const size_t v) {
	d->settings.vectors = v;
}

size_t BinaryFilter::vectors() const {
	return d->settings.vectors;
}

void BinaryFilter::setDataType(const BinaryFilter::DataType t) {
	d->settings.dataType = t;
}

BinaryFilter::DataType BinaryFilter::dataType() const {
	return d->settings.dataType;
}

void BinaryFilter::setByteOrder(const QDataStream::ByteOrder b) {
	d->settings.byteOrder = b;
}

QDataStream::ByteOrder BinaryFilter::byteOrder() const {
	return d->settings.byteOrder;
}

void BinaryFilter::setSkipStartBytes(const size_t s) {
	d->settings.skipStartBytes = s;
}

size_t BinaryFilter::skipStartBytes() const {
	return d->settings.skipStartBytes;
}

void BinaryFilter::setStartRow(const int s) {
	d->startRow = s;
}

int BinaryFilter::startRow() const {
	return d->startRow;
}

void BinaryFilter::setEndRow(const int e) {
	d->endRow = e;
}

int BinaryFilter::endRow() const {
	return d->endRow;
}

void BinaryFilter::setSkipBytes(const size_t s) {
	d->settings.skipBytes = s;
}

size_t BinaryFilter::skipBytes() const {
	return d->settings.skipBytes;
}

void BinaryFilter::setCreateIndexEnabled(bool b) {
	d->settings.createIndexEnabled = b;
}

void BinaryFilter::setAutoModeEnabled(bool b) {
	d->autoModeEnabled = b;
}

bool BinaryFilter::isAutoModeEnabled() const {
	return d->autoModeEnabled;
}

QString BinaryFilter::fileInfoString(const QString& fileName) {
	DEBUG("BinaryFilter::fileInfoString()");
	QString info;

	//TODO
	Q_UNUSED(fileName);

	return info;
}

//#####################################################################
//################### Private implementation ##########################
//#####################################################################

BinaryFilterPrivate::BinaryFilterPrivate(BinaryFilter* owner) : q(owner) {}

/*!
    reads the content of the device \c device to the data source \c dataSource or return as string for preview.
    Uses the settings defined in the data source.
*/
void BinaryFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode) {
	DEBUG("readDataFromFile()");

	KFilterDev device(fileName);
	numRows = BinaryFilter::rowNumber(fileName, settings.vectors, settings.dataType);

	if (! device.open(QIODevice::ReadOnly)) {
		DEBUG("	could not open file " << fileName.toStdString());
		return;
	}
	readDataFromDevice(device, dataSource, importMode);
}

/*!
 * returns 1 if the current read position in the device is at the end and 0 otherwise.
 */
int BinaryFilterPrivate::prepareStreamToRead(QDataStream& in) {
	DEBUG("prepareStreamToRead()");

	in.setByteOrder(settings.byteOrder);

	// catch case that skipStartBytes or startRow is bigger than file
	if (settings.skipStartBytes >= BinaryFilter::dataSize(settings.dataType) * settings.vectors * numRows || startRow > (int)numRows)
		return 1;

	// skip bytes at start
	for (size_t i = 0; i < settings.skipStartBytes; ++i) {
		qint8 tmp;
		in >> tmp;
	}

	// skip until start row
	for (size_t i = 0; i < (startRow-1) * settings.vectors; ++i) {
		for (int j = 0; j < BinaryFilter::dataSize(settings.dataType); ++j) {
			qint8 tmp;
			in >> tmp;
		}
	}

	// set range of rows
	if (endRow == -1)
		m_actualRows = (int)numRows - startRow + 1;
	else if (endRow > (int)numRows - startRow + 1)
		m_actualRows = (int)numRows;
	else
		m_actualRows = endRow - startRow + 1;
	m_actualCols = (int)settings.vectors;

	DEBUG("numRows = " << numRows);
	DEBUG("endRow = " << endRow);
	DEBUG("actual rows = " << m_actualRows);
	DEBUG("actual cols = " << m_actualCols);

	return 0;
}

/*!
    reads \c lines lines of the device \c device and return as string for preview.
*/
QVector<QStringList> BinaryFilterPrivate::preview(const QString& fileName, int lines) {
	DEBUG("BinaryFilterPrivate::preview( " << fileName.toStdString() << ", " << lines << ")");
	QVector<QStringList> dataStrings;

	KFilterDev device(fileName);
	if (! device.open(QIODevice::ReadOnly))
		return dataStrings << (QStringList() << i18n("could not open device"));

	numRows = BinaryFilter::rowNumber(fileName, settings.vectors, settings.dataType);

	QDataStream in(&device);
	const int deviceError = prepareStreamToRead(in);

	if (deviceError)
		return dataStrings << (QStringList() << i18n("data selection empty"));

	//TODO: support other modes
	columnModes.resize(m_actualCols);

	//TODO: use given names
	QStringList vectorNames;

	if (settings.createIndexEnabled)
		vectorNames.prepend(i18n("Index"));

	if (lines == -1)
		lines = m_actualRows;

	// read data
	DEBUG("generating preview for " << qMin(lines, m_actualRows)  << " lines");
	for (int i = 0; i < qMin(m_actualRows, lines); ++i) {
		QStringList lineString;

		//prepend the index if required
		if (settings.createIndexEnabled)
			lineString << QString::number(i+1);

		for (int n = 0; n < m_actualCols; ++n) {
			//TODO: use ColumnMode when it supports all types
			switch (settings.dataType) {
			case BinaryFilter::DataType::INT8: {
					qint8 value;
					in >> value;
					lineString << QString::number(value);
					break;
				}
			case BinaryFilter::DataType::INT16: {
					qint16 value;
					in >> value;
					lineString << QString::number(value);
					break;
				}
			case BinaryFilter::DataType::INT32: {
					qint32 value;
					in >> value;
					lineString << QString::number(value);
					break;
				}
			case BinaryFilter::DataType::INT64: {
					qint64 value;
					in >> value;
					lineString << QString::number(value);
					break;
				}
			case BinaryFilter::DataType::UINT8: {
					quint8 value;
					in >> value;
					lineString << QString::number(value);
					break;
				}
			case BinaryFilter::DataType::UINT16: {
					quint16 value;
					in >> value;
					lineString << QString::number(value);
					break;
				}
			case BinaryFilter::DataType::UINT32: {
					quint32 value;
					in >> value;
					lineString << QString::number(value);
					break;
				}
			case BinaryFilter::DataType::UINT64: {
					quint64 value;
					in >> value;
					lineString << QString::number(value);
					break;
				}
			case BinaryFilter::DataType::REAL32: {
					float value;
					in >> value;
					lineString << QString::number(value);
					break;
				}
			case BinaryFilter::DataType::REAL64: {
					double value;
					in >> value;
					lineString << QString::number(value);
					break;
				}
			}
		}
		dataStrings << lineString;
		emit q->completed(100*i/m_actualRows);
	}

	return dataStrings;
}

/*!
reads the content of the file \c fileName to the data source \c dataSource or return as string for preview.
Uses the settings defined in the data source.
*/
void BinaryFilterPrivate::readDataFromDevice(QIODevice& device, AbstractDataSource* dataSource, AbstractFileFilter::ImportMode importMode, int lines) {
	DEBUG("BinaryFilterPrivate::readDataFromDevice()");

	QDataStream in(&device);
	const int deviceError = prepareStreamToRead(in);

	if (deviceError) {
		dataSource->clear();
		DEBUG("device error");
		return;
	}

	if (settings.createIndexEnabled)
		m_actualCols++;

	QVector<void*> dataContainer;
	int columnOffset = 0;

	//TODO: support other modes
	columnModes.resize(m_actualCols);

	//TODO: use given names
	QStringList vectorNames;

	if (settings.createIndexEnabled) {
		vectorNames.prepend(i18n("Index"));
		columnModes[0] = AbstractColumn::Integer;
	}

	columnOffset = dataSource->prepareImport(dataContainer, importMode, m_actualRows, m_actualCols, vectorNames, columnModes);

	if (lines == -1)
		lines = m_actualRows;

	// start column
	int startColumn = 0;
	if (settings.createIndexEnabled)
		startColumn++;

	// read data
	DEBUG("reading " << qMin(lines, m_actualRows)  << " lines");
	for (int i = 0; i < qMin(m_actualRows, lines); ++i) {
		DEBUG("reading row " << i);
		//prepend the index if required
		if (settings.createIndexEnabled)
			static_cast<QVector<int>*>(dataContainer[0])->operator[](i) = i+1;

		for (int n = startColumn; n < m_actualCols; ++n) {
			DEBUG("reading column " << n);
			//TODO: use ColumnMode when it supports all types
			switch (settings.dataType) {
			case BinaryFilter::DataType::INT8: {
					qint8 value;
					in >> value;
					static_cast<QVector<double>*>(dataContainer[n])->operator[](i) = value;
					break;
				}
			case BinaryFilter::DataType::INT16: {
					qint16 value;
					in >> value;
					static_cast<QVector<double>*>(dataContainer[n])->operator[](i) = value;
					break;
				}
			case BinaryFilter::DataType::INT32: {
					qint32 value;
					in >> value;
					static_cast<QVector<double>*>(dataContainer[n])->operator[](i) = value;
					break;
				}
			case BinaryFilter::DataType::INT64: {
					qint64 value;
					in >> value;
					static_cast<QVector<double>*>(dataContainer[n])->operator[](i) = value;
					break;
				}
			case BinaryFilter::DataType::UINT8: {
					quint8 value;
					in >> value;
					static_cast<QVector<double>*>(dataContainer[n])->operator[](i) = value;
					break;
				}
			case BinaryFilter::DataType::UINT16: {
					quint16 value;
					in >> value;
					static_cast<QVector<double>*>(dataContainer[n])->operator[](i) = value;
					break;
				}
			case BinaryFilter::DataType::UINT32: {
					quint32 value;
					in >> value;
					static_cast<QVector<double>*>(dataContainer[n])->operator[](i) = value;
					break;
				}
			case BinaryFilter::DataType::UINT64: {
					quint64 value;
					in >> value;
					static_cast<QVector<double>*>(dataContainer[n])->operator[](i) = value;
					break;
				}
			case BinaryFilter::DataType::REAL32: {
					float value;
					in >> value;
					static_cast<QVector<double>*>(dataContainer[n])->operator[](i) = value;
					break;
				}
			case BinaryFilter::DataType::REAL64: {
					double value;
					in >> value;
					static_cast<QVector<double>*>(dataContainer[n])->operator[](i) = value;
					break;
				}
			}
		}
		if (m_actualRows > 0)
			emit q->completed(100*i/m_actualRows);
	}

	dataSource->finalizeImport(columnOffset, 1, m_actualCols, m_actualRows, QString(), importMode);
}

/*!
    writes the content of \c dataSource to the file \c fileName.
*/
void BinaryFilterPrivate::write(const QString & fileName, AbstractDataSource* dataSource) {
	Q_UNUSED(fileName);
	Q_UNUSED(dataSource);
	//TODO: writing binary files not supported yet
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

/*!
  Saves as XML.
 */
void BinaryFilter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement("binaryFilter");
	writer->writeAttribute("vectors", QString::number(d->settings.vectors) );
	writer->writeAttribute("dataType", QString::number(static_cast<int>(d->settings.dataType)) );
	writer->writeAttribute("byteOrder", QString::number(d->settings.byteOrder) );
	writer->writeAttribute("autoMode", QString::number(d->autoModeEnabled) );
	writer->writeAttribute("startRow", QString::number(d->startRow) );
	writer->writeAttribute("endRow", QString::number(d->endRow) );
	writer->writeAttribute("skipStartBytes", QString::number(d->settings.skipStartBytes) );
	writer->writeAttribute("skipBytes", QString::number(d->settings.skipBytes) );
	writer->writeAttribute( "createIndex", QString::number(d->settings.createIndexEnabled) );
	writer->writeEndElement();
}

/*!
  Loads from XML.
*/
bool BinaryFilter::load(XmlStreamReader* reader) {
	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs = reader->attributes();

	// read attributes
	QString str = attribs.value("vectors").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("vectors").toString());
	else
		d->settings.vectors = (size_t)str.toULong();

	str = attribs.value("dataType").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("dataType").toString());
	else
		d->settings.dataType = (BinaryFilter::DataType) str.toInt();

	str = attribs.value("byteOrder").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("byteOrder").toString());
	else
		d->settings.byteOrder = (QDataStream::ByteOrder) str.toInt();

	str = attribs.value("autoMode").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("autoMode").toString());
	else
		d->autoModeEnabled = str.toInt();

	str = attribs.value("startRow").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("startRow").toString());
	else
		d->startRow = str.toInt();

	str = attribs.value("endRow").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("endRow").toString());
	else
		d->endRow = str.toInt();

	str = attribs.value("skipStartBytes").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("skipStartBytes").toString());
	else
		d->settings.skipStartBytes = (size_t)str.toULong();

	str = attribs.value("skipBytes").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("skipBytes").toString());
	else
		d->settings.skipBytes = (size_t)str.toULong();

	str = attribs.value("createIndex").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.subs("createIndex").toString());
	else
		d->settings.createIndexEnabled = str.toInt();

	return true;
}
