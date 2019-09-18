/***************************************************************************
File                 : BinaryFilterPrivate.h
Project              : LabPlot
Description          : Private implementation class for BinaryFilter.
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
#ifndef BINARYFILTERPRIVATE_H
#define BINARYFILTERPRIVATE_H

#include <QDataStream>
#include <QVector>
#include "backend/datasources/filters/AbstractFileFilter.h"

class AbstractDataSource;
class AbstractColumn;
class BinaryFilter;

class BinaryFilterSettings : public AbstractFileFilterSettings {

public:
	BinaryFilterSettings() {}
	enum class DataType {INT8, INT16, INT32, INT64, UINT8, UINT16, UINT32, UINT64, REAL32, REAL64};
public:
	size_t vectors{2};
	DataType dataType{DataType::INT8};
	QDataStream::ByteOrder byteOrder{QDataStream::LittleEndian};
	size_t skipBytes{0}; // bytes to skip after each value
	size_t skipStartBytes{0}; // bytes to skip at start
	bool createIndexEnabled{false}; // if create index column
};

class BinaryFilterPrivate {

public:
	explicit BinaryFilterPrivate(BinaryFilter*);

	int prepareStreamToRead(QDataStream&);
	void readDataFromDevice(QIODevice& device, AbstractDataSource* = nullptr,
			AbstractFileFilter::ImportMode = AbstractFileFilter::Replace, int lines = -1);
	void readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr,
			AbstractFileFilter::ImportMode = AbstractFileFilter::Replace);
	void write(const QString& fileName, AbstractDataSource*);
	QVector<QStringList> preview(const QString& fileName, int lines);

	const BinaryFilter* q;


	BinaryFilterSettings settings;
	QVector<AbstractColumn::ColumnMode> columnModes;

	int startRow{1};		// start row (value*vectors) to read (can be -1)
	int endRow{-1};			// end row to (value*vectors) read (can be -1)
	size_t numRows{0};		// number of rows

	bool autoModeEnabled{true};

private:
	int m_actualRows{0};
	int m_actualCols{0};
};

#endif
