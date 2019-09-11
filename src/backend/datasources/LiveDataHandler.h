#ifndef LIVEDATAHANDLER_H
#define LIVEDATAHANDLER_H

#include <QObject>

#include "filters/AbstractFileFilter.h"
#include "filters/ImageFilter.h"

class LiveDataHandlerPrivate;
class ImageFilter;

/*!
 * \brief The LiveDataHandler class handles the connection a different devices
 * and parses the received data. This function can be in it's own thread
 */
class LiveDataHandler : public QObject
{
    Q_OBJECT
public:
	LiveDataHandler(QObject* parent);
	~LiveDataHandler();

	typedef LiveDataHandlerPrivate Private;
	enum class ErrorCodes {
		// TCP/UDP
		host,
		port,
		// serial connection
		serialPort,
		baudRate,
	};

	enum class SourceType {
		FileOrPipe = 0,		// regular file or pipe
		NetworkTcpSocket,	// TCP socket
		NetworkUdpSocket,	// UDP socket
		LocalSocket,		// local socket
		SerialPort,		// serial port
		MQTT
	};

	bool getLine(QString& line);

signals:
    void host(QString host) const;
    void port(QString port) const;
    void serialPort(QString serialPort) const;
    void baudRate(int baudRate) const;
	void sourceType(SourceType sourceType) const;
    void errorCode(ErrorCodes errorCode); // connect to this signal to get a message, when an error occured
    void feedback(bool error, ErrorCodes errorCode); // connect to this signal to get a message when a set function was called with the status
    void feedback(bool error, QString errorMessage); // connect to this signal to get a message when a set function was called with the error message
    void previewData(QVector<QStringList> importedStrings, QStringList vectorNameList, QVector<AbstractColumn::ColumnMode> columnModes);
public slots: // don't access this functions directly when in an own thread! Use signal/slots
    void preview(int nbrOfLines);
    void preview();

    void fileTypeToAscii(int index, int startRow, int endRow, int startColumn, int endColumn);
    void fileTypeToBinary(int index, int startRow, int endRow);
    void fileTypeToROOT(int startRow, int endRow, QVector<QStringList> columns);
    void fileTypeToHDF5(int startRow, int endRow, int startColumn, int endColumn);
    void fileTypeToNETCDF(int startRow, int endRow, int startColumn, int endColumn);
    void fileTypeToFITS(int startRow, int endRow, int startColumn, int endColumn);
    void fileTypeToImage(int startRow, int endRow, int startColumn, int endColumn);
    void fileTypeToNgspiceRawAscii(int startRow, int endRow);
    void fileTypeToNgspiceRawBinary(int startRow, int endRow);
    void filterTypeToJSON(int startRow, int endRow, int startColumn, int endColumn);

    // setter
        // device
    void setHost(QString host);
    void setPort(QString port);
    void setSerialPort(QString serialPort);
    void setBaudRate(QString baudrate);
    void setFileName(QString fileName);
    void setSourceType(int sourceType);
    void setFileType(int fileType);
        // filter
    void setAutoModeEnabled(int disable);
    void setStartRow(int row);
    void setEndRow(int row);
    void setStartColumn(int column);
    void setEndColumn(int column);
    void setImportFormat(const ImageFilter::ImportFormat format);
    void setCurrentDataSetName(const QStringList);
    void setCurrentVarName(const QStringList);
    void setCurrentObject(const QStringList);


protected:
	Private* d_ptr{nullptr};
private:

private:
	Q_DECLARE_PRIVATE(LiveDataHandler)
};

#endif // LIVEDATAHANDLER_H
