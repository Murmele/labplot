#ifndef LIVEDATAHANDLERPRIVATE_H
#define LIVEDATAHANDLERPRIVATE_H

#include "filters/AbstractFileFilter.h"
#include "filters/ImageFilter.h"
#include "LiveDataHandler.h"

#include <memory>
#include <QString>

class AbstractFileFilter;
class LiveDataHandler;
class QIODevice;

class LiveDataHandlerPrivate : public QObject{
	Q_OBJECT
public:
	LiveDataHandlerPrivate(QObject* parent, LiveDataHandler* owner);
	~LiveDataHandlerPrivate();

	void changeSettings();
	QVector<QStringList>  preview(int nbrOfLines);

	bool getLine(QString& string);
	void changeFileFilter();

private slots:
	void readyRead();


public:

	mutable AbstractFileFilter* m_currentFilter; // was macht mutable nochmals?

	QIODevice* m_device{nullptr};
	LiveDataHandler* const q {nullptr};
	QString m_host{""};
	QString m_port{""};
	int m_baudrate{9200};
	QString m_serialPort{""};
	AbstractFileFilter::FileType m_fileType{AbstractFileFilter::FileType::Ascii};
	LiveDataHandler::SourceType m_sourceType{LiveDataHandler::SourceType::FileOrPipe};
	QString m_fileName{""};

	// filter
	bool m_autoModeEnabled{false};
	int m_startRow{-1};
	int m_endRow{-1};
	int m_startColumn{-1};
	int m_endColumn{-1};
	ImageFilter::ImportFormat m_format;
	QStringList m_dataSetName;
	QStringList m_varName;
	QStringList m_object;
	QVector<QStringList> m_columns;

private:
	const int maxBufferSize{1000000};
	QString m_deviceBuffer; // better would be to implement a ring buffer with a maximum size
	int m_deviceBufferIndex{0}; // when using a buffer

};

#endif // LIVEDATAHANDLERPRIVATE_H
