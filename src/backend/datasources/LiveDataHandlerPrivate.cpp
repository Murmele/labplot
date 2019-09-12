#include "LiveDataHandlerPrivate.h"
#include "LiveDataHandler.h"

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

#include <QIODevice>
#include <QDir>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QSerialPort>
#include <QLocalSocket>


//##############################################################################
//######################### Private implementation #############################
//##############################################################################
LiveDataHandlerPrivate::LiveDataHandlerPrivate(QObject *parent, LiveDataHandler* owner): QObject(owner), q(owner){
	changeFileFilter(); // initalize filter
}

LiveDataHandlerPrivate::~LiveDataHandlerPrivate() {

}

void LiveDataHandlerPrivate::changeSettings() {
#ifndef HAVE_WINDOWS
	if (!m_fileName.isEmpty() && m_fileName.at(0) != QDir::separator())
		m_fileName = QDir::homePath() + QDir::separator() + m_fileName;
#endif

	changeFileFilter();

	if (m_device)
		m_device->close();

	//DEBUG("Data Source Type: " << ENUM_TO_STRING(LiveDataHandlerPrivate, SourceType, m_sourceType));
	switch (m_sourceType) {
	case LiveDataHandler::SourceType::FileOrPipe: {
		DEBUG("fileName = " << m_fileName.toUtf8().constData());
		const bool enable = QFile::exists(m_fileName);
		if (enable)
			return emit q->feedback(enable, "");
		else
			return emit q->feedback(enable, i18n("Provide an existing file."));

		break;
	}
	case LiveDataHandler::SourceType::LocalSocket: {
		const bool enable = QFile::exists(m_fileName);
		if (enable) {

			QLocalSocket* lsocket = new QLocalSocket(q);
			DEBUG("CONNECT");
			lsocket->connectToServer(m_fileName, QLocalSocket::ReadOnly);
			if (lsocket->waitForConnected()) {

				// this is required for server that send data as soon as connected
				lsocket->waitForReadyRead();

				// read-only socket is disconnected immediately (no waitForDisconnected())
				m_device = static_cast<QIODevice*>(lsocket);
				connect(m_device, &QIODevice::readyRead, this, &LiveDataHandlerPrivate::readyRead);
				return emit q->feedback(true, "");
			} else {
				DEBUG("failed connect to local socket - " << lsocket->errorString().toStdString());
				delete lsocket;
				return emit q->feedback(false, i18n("Could not connect to the provided local socket."));
			}
		} else {
			return emit q->feedback(false, i18n("Selected local socket does not exist."));
		}

		break;
	}
	case LiveDataHandler::SourceType::NetworkTcpSocket: {
		const bool enable = !m_host.isEmpty() && !m_port.isEmpty();
		if (enable) {
			QTcpSocket* tcpSocket = new QTcpSocket(q);
			tcpSocket->connectToHost(m_host, m_port.toInt(), QTcpSocket::ReadOnly);

			if (tcpSocket->waitForConnected()) {
				m_device = static_cast<QIODevice*>(tcpSocket);
				connect(m_device, &QIODevice::readyRead, this, &LiveDataHandlerPrivate::readyRead);
				return emit q->feedback(true, "");
			} else {
				DEBUG("failed to connect to TCP socket - " << tcpSocket->errorString().toStdString());
				delete tcpSocket;
				return emit q->feedback(false, i18n("Could not connect to the provided TCP socket."));
			}
		} else {
			return emit q->feedback(false, i18n("Either the host name or the port number is missing."));
		}
		break;
	}
	case LiveDataHandler::SourceType::NetworkUdpSocket: {
		const bool enable = !m_host.isEmpty() && !m_port.isEmpty();
		if (enable) {

			QUdpSocket* socket = new QUdpSocket(q);
			socket->bind(QHostAddress(m_host), m_port.toUShort());
			socket->connectToHost(m_host, 0, QUdpSocket::ReadOnly);
			if (socket->waitForConnected()) {
				m_device = static_cast<QIODevice*>(socket);
				connect(m_device, &QIODevice::readyRead, this, &LiveDataHandlerPrivate::readyRead);
				return emit q->feedback(true, "");
			} else {
				DEBUG("failed to connect to UDP socket - " << socket->errorString().toStdString());
				delete socket;
				return emit q->feedback(false, i18n("Could not connect to the provided UDP socket."));
			}
		} else {
			return emit q->feedback(false, i18n("Either the host name or the port number is missing."));
		}

		break;
	}
	case LiveDataHandler::SourceType::SerialPort: {

		if (!m_serialPort.isEmpty()) {

			QSerialPort* serialPort = new QSerialPort(q);

			DEBUG("	Port: " << m_serialPort.toStdString() << ", Settings: " << m_baudrate << ',' << serialPort->dataBits()
					<< ',' << serialPort->parity() << ',' << serialPort->stopBits());
			serialPort->setPortName(m_serialPort);
			serialPort->setBaudRate(m_baudrate);

			const bool serialPortOpened = serialPort->open(QIODevice::ReadOnly);
			if (serialPortOpened) {
				m_device = static_cast<QIODevice*>(serialPort);
				connect(m_device, &QIODevice::readyRead, this, &LiveDataHandlerPrivate::readyRead);
				return emit q->feedback(true, "");
			} else {
				DEBUG("Could not connect to the provided serial port");
				delete serialPort;
				return emit q->feedback(false, i18n("Could not connect to the provided serial port."));
			}
		} else {
			return emit q->feedback(false, i18n("Serial port number is missing."));
		}
		break;
	}
	case LiveDataHandler::SourceType::MQTT: {
#ifdef HAVE_MQTT
		const bool enable = m_importFileWidget->isMqttValid();
		if (enable) {
			okButton->setEnabled(true);
			okButton->setToolTip(i18n("Close the dialog and import the data."));
		}
		else {
			okButton->setEnabled(false);
			okButton->setToolTip(i18n("Either there is no connection, or no subscriptions were made, or the file filter is not ASCII."));
		}
#endif
		break;
	}
	}
}

void LiveDataHandlerPrivate::changeFileFilter() {
	DEBUG("ImportFileWidget::currentFileFilter()");
	if (m_currentFilter && m_currentFilter->type() == m_fileType)
		return;

	delete m_currentFilter;

	switch (m_fileType) {
	case AbstractFileFilter::Ascii: {
		DEBUG("	ASCII");
		if (!m_currentFilter)
			m_currentFilter = new AsciiFilter; // will it work?
			auto filter = static_cast<AsciiFilter*>(m_currentFilter);
			filter->setStartRow(m_startRow);
			filter->setEndRow(m_endRow);
			filter->setStartColumn(m_startColumn);
			filter->setEndColumn(m_endColumn);
			filter->setAutoModeEnabled(m_autoModeEnabled);
		break;
	}
	case AbstractFileFilter::Binary: {
		DEBUG("	Binary");
		if (!m_currentFilter)
			m_currentFilter = new BinaryFilter;

		auto filter = static_cast<BinaryFilter*>(m_currentFilter);
		filter->setStartRow(m_startRow);
		filter->setEndRow(m_endRow);
		filter->setAutoModeEnabled(m_autoModeEnabled);
		break;
	}
	case AbstractFileFilter::Image: {
		DEBUG("	Image");
		if (!m_currentFilter)
			m_currentFilter = new ImageFilter;

		auto filter = static_cast<ImageFilter*>(m_currentFilter);
		filter->setStartRow(m_startRow);
		filter->setEndRow(m_endRow);
		filter->setStartColumn(m_startColumn);
		filter->setEndColumn(m_endColumn);
		filter->setImportFormat(m_format);
		break;
	}
	case AbstractFileFilter::HDF5: {
		DEBUG("ImportFileWidget::currentFileFilter(): HDF5");
		if (!m_currentFilter)
			m_currentFilter = new HDF5Filter;

		auto filter = static_cast<HDF5Filter*>(m_currentFilter);
		filter->setStartRow(m_startRow);
		filter->setEndRow(m_endRow);
		filter->setStartColumn(m_startColumn);
		filter->setEndColumn(m_endColumn);
		if (!m_dataSetName.isEmpty())
			filter->setCurrentDataSetName(m_dataSetName[0]);
		break;
	}
	case AbstractFileFilter::NETCDF: {
		DEBUG("	NETCDF");
		if (!m_currentFilter)
			m_currentFilter = new NetCDFFilter;

		auto filter = static_cast<NetCDFFilter*>(m_currentFilter);
		filter->setStartRow(m_startRow);
		filter->setEndRow(m_endRow);
		filter->setStartColumn(m_startColumn);
		filter->setEndColumn(m_endColumn);
		if (!m_varName.isEmpty())
			filter->setCurrentVarName(m_varName[0]);

		break;
	}
	case AbstractFileFilter::FITS: {
		DEBUG("	FITS");
		if (!m_currentFilter)
			m_currentFilter = new FITSFilter;

		auto filter = static_cast<FITSFilter*>(m_currentFilter);
		filter->setStartRow(m_startRow);
		filter->setEndRow(m_endRow);
		filter->setStartColumn(m_startColumn);
		filter->setEndColumn(m_endColumn);
		break;
	}
	case AbstractFileFilter::JSON: {
		DEBUG("	JSON");
		if (!m_currentFilter)
			m_currentFilter = new JsonFilter;

		auto filter = static_cast<JsonFilter*>(m_currentFilter);
		filter->setStartRow(m_startRow);
		filter->setEndRow(m_endRow);
		filter->setStartColumn(m_startColumn);
		filter->setEndColumn(m_endColumn);

		break;
	}
	case AbstractFileFilter::ROOT: {
		DEBUG("	ROOT");
		if (!m_currentFilter)
			m_currentFilter = new ROOTFilter;

		auto filter = static_cast<ROOTFilter*>(m_currentFilter);
		filter->setStartRow(m_startRow);
		filter->setEndRow(m_endRow);
		if (!m_object.isEmpty())
			filter->setCurrentObject(m_object[0]);
		filter->setColumns(m_columns);
		break;
	}
	case AbstractFileFilter::NgspiceRawAscii: {
		DEBUG("	NgspiceRawAscii");
		if (!m_currentFilter)
			m_currentFilter = new NgspiceRawAsciiFilter;

		break;
	}
	case AbstractFileFilter::NgspiceRawBinary: {
		DEBUG("	NgspiceRawBinary");
		if (!m_currentFilter)
			m_currentFilter = new NgspiceRawBinaryFilter;
		auto filter = static_cast<NgspiceRawBinaryFilter*>(m_currentFilter);
		filter->setStartRow(m_startRow);
		filter->setEndRow(m_endRow);

		break;
	}
	}
}

QVector<QStringList>  LiveDataHandlerPrivate::preview(int nbrOfLines) {
	QVector<QStringList>  importedStrings;

	if (!m_device)
		changeSettings();

	switch (m_fileType) {
	case AbstractFileFilter::Ascii: {

		auto filter = static_cast<AsciiFilter*>(m_currentFilter);

		//DEBUG("Data Source Type: " << ENUM_TO_STRING(LiveDataHandlerPrivate, SourceType, sourceType));
		switch (m_sourceType) {
		case LiveDataHandler::SourceType::FileOrPipe: {
			importedStrings = filter->preview(m_fileName, nbrOfLines);
			break;
		}
		case LiveDataHandler::SourceType::LocalSocket: {
			if (m_device)
				importedStrings = filter->preview(q, nbrOfLines);
			break;
		}
		case LiveDataHandler::SourceType::NetworkTcpSocket: {
			if (m_device)
				importedStrings = filter->preview(q, nbrOfLines);
			break;
		}
		case LiveDataHandler::SourceType::NetworkUdpSocket: {
			if (m_device)
				importedStrings = filter->preview(q, nbrOfLines);
			break;
		}
		case LiveDataHandler::SourceType::SerialPort: {
			if (m_device)
				importedStrings = filter->preview(q, nbrOfLines);
			break;
		}
		case LiveDataHandler::SourceType::MQTT: {
#ifdef HAVE_MQTT
			//show the preview for the currently selected topic
			auto* item = m_subscriptionWidget->currentItem();
			if (item && item->childCount() == 0) { //only preview if the lowest level (i.e. a topic) is selected
				const QString& topicName = item->text(0);
				auto i = m_lastMessage.find(topicName);
				if (i != m_lastMessage.end())
					importedStrings = filter->preview(i.value().payload().data());
				else
					importedStrings << QStringList{i18n("No data arrived yet for the selected topic")};
			}
#endif
			break;
		}
		}
		break;
	}
	case AbstractFileFilter::Binary: {

		auto filter = static_cast<BinaryFilter*>(m_currentFilter);
		importedStrings = filter->preview(m_fileName, nbrOfLines);
		break;
	}
	case AbstractFileFilter::Image: {
		break;
	}
	case AbstractFileFilter::HDF5: {
		DEBUG("ImportFileWidget::refreshPreview: HDF5");
		auto filter = static_cast<HDF5Filter*>(m_currentFilter);
		bool ok; // where using?
		importedStrings = filter->readCurrentDataSet(m_fileName, nullptr, ok, AbstractFileFilter::Replace, nbrOfLines);
		break;
	}
	case AbstractFileFilter::NETCDF: {
		auto filter = static_cast<NetCDFFilter*>(m_currentFilter);

		importedStrings = filter->readCurrentVar(m_fileName, nullptr, AbstractFileFilter::Replace, nbrOfLines);
		break;
	}
	case AbstractFileFilter::FITS: {
		auto filter = static_cast<FITSFilter*>(m_currentFilter);

		bool readFitsTableToMatrix;
		importedStrings = filter->readChdu(m_fileName, &readFitsTableToMatrix, nbrOfLines);

		break;
	}
	case AbstractFileFilter::JSON: {
		auto filter = static_cast<JsonFilter*>(m_currentFilter);
		importedStrings = filter->preview(m_fileName);
		break;
	}
	case AbstractFileFilter::ROOT: {
		auto filter = static_cast<ROOTFilter*>(m_currentFilter);
		importedStrings = filter->previewCurrentObject(
							  m_fileName,
							  m_startRow,
							  qMin(m_startRow + nbrOfLines - 1,
								   m_endRow)
						  );
		// the last vector element contains the column names
		importedStrings.removeLast();
		break;
	}
	case AbstractFileFilter::NgspiceRawAscii: {
		auto filter = static_cast<NgspiceRawAsciiFilter*>(m_currentFilter);
		importedStrings = filter->preview(m_fileName, nbrOfLines);
		break;
	}
	case AbstractFileFilter::NgspiceRawBinary: {
		auto filter = static_cast<NgspiceRawBinaryFilter*>(m_currentFilter);
		importedStrings = filter->preview(m_fileName, nbrOfLines);
		break;
	}
	}
	return importedStrings;
}

void LiveDataHandlerPrivate::readyRead() {

	m_deviceBuffer.append(m_device->readAll());
	int bufferSize = m_deviceBuffer.size();
	if (bufferSize > maxBufferSize) // limit buffer to a specific size
		m_deviceBuffer.remove(0, bufferSize - maxBufferSize);
}

/*!
 * Get new line from the device buffer. The \n at the end is removed. So the line is plain data
 * \brief LiveDataHandlerPrivate::getLine
 * \param string
 * \return
 */
bool LiveDataHandlerPrivate::getLine(QString& string) {
	int begin = m_deviceBuffer.indexOf('\n');
	if (begin < 0)
		return false;

	int end = m_deviceBuffer.indexOf('\n', begin+1);
	if (end < 0)
		return false;

	string = m_deviceBuffer.mid(begin+1,end-begin);
	m_deviceBuffer.remove(0, end); // don't remove last \n because it's the next start character
	return true;
}
