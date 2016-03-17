#include "textdemux.h"

#include "core/subtitle.h"
#include "streamprocessor/streamprocessor.h"

#include <KLocalizedString>

#include <QLabel>
#include <QBoxLayout>
#include <QProgressBar>

using namespace SubtitleComposer;

TextDemux::TextDemux(QWidget *parent)
	: QObject(parent),
	  m_subtitle(NULL),
	  m_streamProcessor(new StreamProcessor(this)),
	  m_progressWidget(new QWidget(parent))
{
	// progress Bar
	m_progressWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	m_progressWidget->hide();

	QLabel *label = new QLabel(i18n("Importing Subtitle Stream"), m_progressWidget);

	m_progressBar = new QProgressBar(m_progressWidget);
	m_progressBar->setMinimumWidth(300);
	m_progressBar->setTextVisible(true);

	QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight, m_progressWidget);
	layout->setContentsMargins(1, 0, 1, 0);
	layout->setSpacing(1);
	layout->addWidget(label);
	layout->addWidget(m_progressBar);

	// stream processor
	connect(m_streamProcessor, &StreamProcessor::streamProgress, this, &TextDemux::onStreamProgress);
	connect(m_streamProcessor, &StreamProcessor::streamError, this, &TextDemux::onStreamError);
	connect(m_streamProcessor, &StreamProcessor::streamFinished, this, &TextDemux::onStreamFinished);
	connect(m_streamProcessor, &StreamProcessor::textDataAvailable, this, &TextDemux::onStreamData);
}

void
TextDemux::demuxFile(Subtitle *subtitle, const QString filename, int textStreamIndex)
{
	if(!subtitle)
		return;

	m_streamProcessor->close();

	m_subtitle = subtitle;

	if(m_streamProcessor->open(filename) && m_streamProcessor->initText(textStreamIndex))
		m_streamProcessor->start();
}

void
TextDemux::onStreamData(const QString &text, quint64 msecStart, quint64 msecDuration)
{
	m_subtitle->insertLine(new SubtitleLine(SString(text), Time(double(msecStart)), Time(double(msecStart) + double(msecDuration))));
}

void
TextDemux::onStreamProgress(quint64 msecPos, quint64 msecLength)
{
	m_progressBar->setRange(0, msecLength / 1000);
	m_progressBar->setValue(msecPos / 1000);
	m_progressWidget->show();
}

void
TextDemux::onStreamError(int code, const QString &message, const QString &debug)
{
	emit onError(i18n("Subtitle demux failed %1: %2\n%3")
				 .arg(code)
				 .arg(message)
				 .arg(debug));
	m_streamProcessor->close();
	m_progressWidget->hide();
}

void
TextDemux::onStreamFinished()
{
	m_streamProcessor->close();
	m_progressWidget->hide();
}

QWidget *
TextDemux::progressWidget()
{
	return m_progressWidget;
}