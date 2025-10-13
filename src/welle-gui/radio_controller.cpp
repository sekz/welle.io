/*
 *    Copyright (C) 2018
 *    Matthias P. Braendli (matthias.braendli@mpb.li)
 *
 *    Copyright (C) 2017
 *    Albrecht Lohofener (albrechtloh@gmx.de)
 *
 *    This file is based on SDR-J
 *    Copyright (C) 2010, 2011, 2012
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *
 *    This file is part of the welle.io.
 *    Many of the ideas as implemented in welle.io are derived from
 *    other work, made available through the GNU general Public License.
 *    All copyrights of the original authors are recognized.
 *
 *    welle.io is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    welle.io is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with welle.io; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <QCoreApplication>
#include <QDebug>
#include <QSettings>
#include <QStandardPaths>
#include <QTimeZone>
#include <stdexcept>

#include "radio_controller.h"
#ifdef HAVE_SOAPYSDR
#include "soapy_sdr.h"
#endif /* HAVE_SOAPYSDR */
#include "input_factory.h"
#include "raw_file.h"
#include "rtl_tcp.h"

#define AUDIOBUFFERSIZE 32768

static QString serialise_serviceid(quint32 serviceid) {
    return QString::asprintf("%x", serviceid);
}

static quint32 deserialise_serviceid(const char *input)
{
    long value = 0;
    errno = 0;

    char* endptr = nullptr;
    value = strtol(input, &endptr, 16);

    if ((value == LONG_MIN or value == LONG_MAX) and errno == ERANGE) {
        return 0;
    }
    else if (value == 0 and errno != 0) {
        return 0;
    }
    else if (input == endptr) {
        return 0;
    }
    else if (*endptr != '\0') {
        return 0;
    }

    return value;
}

CRadioController::CRadioController(QVariantMap& commandLineOptions, QObject *parent)
    : QObject(parent)
    , commandLineOptions(commandLineOptions)
    , audioBuffer(2 * AUDIOBUFFERSIZE)
    , audio(audioBuffer)
    , originalServiceId_(0)
    , originalSubchannelId_(0)
{
    // Init the technical data
    resetTechnicalData();

    // Init timers
    connect(&labelTimer, &QTimer::timeout, this, &CRadioController::labelTimerTimeout);
    connect(&stationTimer, &QTimer::timeout, this, &CRadioController::stationTimerTimeout);
    connect(&channelTimer, &QTimer::timeout, this, &CRadioController::channelTimerTimeout);
    connect(&announcementDurationTimer, &QTimer::timeout, this, &CRadioController::announcementDurationTimerTimeout);

    // Use the signal slot mechanism is necessary because the backend runs in a different thread
    connect(this, &CRadioController::switchToNextChannel,
            this, &CRadioController::nextChannel);

    connect(this, &CRadioController::ensembleIdUpdated,
            this, &CRadioController::ensembleId);

    qRegisterMetaType<DabLabel>("DabLabel&");
    connect(this, &CRadioController::ensembleLabelUpdated,
            this, &CRadioController::ensembleLabel);

    connect(this, &CRadioController::serviceDetected,
            this, &CRadioController::serviceId);

    qRegisterMetaType<dab_date_time_t>("dab_date_time_t");
    connect(this, &CRadioController::dateTimeUpdated,
            this, &CRadioController::displayDateTime);

    connect(this, &CRadioController::restartServiceRequested,
            this, &CRadioController::restartService);

    // Initialize announcement manager
    announcementManager_ = std::make_unique<AnnouncementManager>();

    // P1-3 FIX (IMPROVE-002): Load announcement settings in constructor
    // Previously: Constructor never called loadAnnouncementSettings(), but destructor saved them
    // Fix: Ensure settings are loaded during initialization
    loadAnnouncementSettings();

    qDebug() << "RadioController: AnnouncementManager initialized with loaded settings";
}

CRadioController::~CRadioController()
{
    // P1-2 FIX (BUG-004): Resource cleanup in destructor
    // Previously: Destructor only called closeDevice() without cleanup
    // Fix: Add comprehensive announcement state cleanup before device closure

    qDebug() << "RadioController: Destroying CRadioController - starting cleanup";

    // Stop announcement duration timer if active
    if (announcementDurationTimer.isActive()) {
        announcementDurationTimer.stop();
        qDebug() << "RadioController: Announcement duration timer stopped";
    }

    // Save announcement settings before shutdown
    if (announcementManager_) {
        saveAnnouncementSettings();
        qDebug() << "RadioController: Announcement settings saved to QSettings";
    }

    // Clear announcement history (std::deque destructor handles memory)
    {
        std::lock_guard<std::mutex> lock(m_announcementHistoryMutex);
        size_t historySize = m_announcementHistory.size();
        m_announcementHistory.clear();
        qDebug() << "RadioController: Announcement history cleared ("
                 << historySize << " entries removed)";
    }

    // Close device and clean up radio resources
    closeDevice();

    qDebug() << "RadioController: CRadioController destroyed (announcement state saved)";
}

void CRadioController::closeDevice()
{
    qDebug() << "RadioController:" << "Close device";

    radioReceiver.reset();
    device.reset();
    audio.reset();

    // Reset the technical data
    resetTechnicalData();

    emit deviceClosed();
}

CDeviceID CRadioController::openDevice(CDeviceID deviceId, bool force, QVariant param1, QVariant param2)
{
    if(this->deviceId != deviceId || force) {
        closeDevice();
        device.reset(CInputFactory::GetDevice(*this, deviceId));

        // Set rtl_tcp settings
        if (device->getID() == CDeviceID::RTL_TCP) {
            CRTL_TCP_Client* RTL_TCP_Client = static_cast<CRTL_TCP_Client*>(device.get());

            RTL_TCP_Client->setServerAddress(param1.toString().toStdString());
            RTL_TCP_Client->setPort(param2.toInt());
        }

        // Set rtl_tcp settings
        if (device->getID() == CDeviceID::RAWFILE) {
            CRAWFile* rawFile = static_cast<CRAWFile*>(device.get());
#ifdef __ANDROID__
            // Using QFile is necessary to get access to com.android.externalstorage.ExternalStorageProvider
            rawFileAndroid.reset(new QFile(param1.toString()));
            bool ret = rawFileAndroid->open(QIODevice::ReadOnly);
            if(!ret) {
                setErrorMessage(tr("Error while opening file ") + param1.toString());
            }
            else {
                rawFile->setFileHandle(rawFileAndroid->handle(), param2.toString().toStdString());
            }
#else
            rawFile->setFileName(param1.toString().toStdString(), param2.toString().toStdString());
#endif
        }

        initialise();
    }

    return device->getID();
}

CDeviceID CRadioController::openDevice()
{
    closeDevice();
    device.reset(CInputFactory::GetDevice(*this, "auto"));
    initialise();

    return device->getID();
}

void CRadioController::setDeviceParam(QString param, int value)
{
    if (param == "biastee") {
        deviceParametersInt[DeviceParam::BiasTee] = value;
    }
    else {
        qDebug() << "Invalid device parameter setting: " << param;
    }

    if (device) {
        device->setDeviceParam(DeviceParam::BiasTee, value);
    }
}

void CRadioController::setDeviceParam(QString param, QString value)
{
    DeviceParam dp;
    bool deviceParamChanged = false;

    if (param == "SoapySDRAntenna") {
        dp = DeviceParam::SoapySDRAntenna;
    }
    else if (param == "SoapySDRDriverArgs") {
        dp = DeviceParam::SoapySDRDriverArgs;
    }
    else if (param == "SoapySDRClockSource") {
        dp = DeviceParam::SoapySDRClockSource;
    }
    else {
        qDebug() << "Invalid device parameter setting: " << param;
        return;
    }

    std::string v = value.toStdString();

    if (deviceParametersString[dp] != v) {
        deviceParamChanged = true;
        deviceParametersString[dp] = v;
    }

    if (device && deviceParamChanged) {
        device->setDeviceParam(dp, v);
        if (dp == DeviceParam::SoapySDRDriverArgs)
            openDevice(CDeviceID::SOAPYSDR,1);
    }
}

void CRadioController::play(QString channel, QString title, quint32 service)
{
    if (channel == "") {
        return;
    }

    currentTitle = title;
    emit titleChanged();


    qDebug() << "RadioController:" << "Play:" << title << serialise_serviceid(service) << "on channel" << channel;

    if (isChannelScan == true) {
        stopScan();
    }

    bool isRestartOk = deviceRestart();
    setChannel(channel, false);
    setService(service);

    currentLastChannel = QStringList() << serialise_serviceid(service) << channel;
    QSettings settings;
    settings.setValue("lastchannel", currentLastChannel);

    if (isRestartOk) {
        isPlaying = true;
        emit isPlayingChanged(isPlaying);

        // Store original service for announcement switching
        if (announcementManager_ && !m_isInAnnouncement) {
            originalServiceId_ = service;
            // Note: originalSubchannelId_ will be set when subchannel info is available
            announcementManager_->setOriginalService(service, 0);  // Subchannel ID will be updated later
        }
    } else {
        resetTechnicalData();
        currentTitle = title;
        emit titleChanged();
        currentText = tr("Playback failed");
        emit textChanged();
    }
}

void CRadioController::stop()
{
    if (radioReceiver) {
        radioReceiver->stop();
    }

    if (device) {
        device->stop();
    }
    else
        throw std::runtime_error("device is null in file " + std::string(__FILE__) +":"+ std::to_string(__LINE__));

    QString title = currentTitle;
    resetTechnicalData();
    currentTitle = title;
    emit titleChanged();
    currentText = tr("Stopped");
    emit textChanged();

    audio.stop();
    labelTimer.stop();

    // Stop announcement duration timer
    announcementDurationTimer.stop();
}

void CRadioController::setService(uint32_t service, bool force)
{
    if (currentService != service or force or isPlaying == false) {
        currentService = service;
        autoService = service;
        emit stationChanged();
        emit autoServiceChanged(autoService);

        // Wait if we found the station inside the signal
        stationTimer.start(1000);

        // Clear old data
        currentStationType = "";
        emit stationTypChanged();

        currentLanguageType = "";
        emit languageTypeChanged();

        currentText = "";
        emit textChanged();

        audioMode = "";
        emit audioModeChanged(audioMode);

        emit motReseted();
    }
}

void CRadioController::setAutoPlay(bool isAutoPlayValue, QString channel, QString service)
{
    isAutoPlay = isAutoPlayValue;
    autoChannel = channel;
    emit autoChannelChanged(autoChannel);
    autoService = deserialise_serviceid(service.toStdString().c_str());
    emit autoServiceChanged(autoService);
    currentLastChannel = QStringList() << service << channel;
}

void CRadioController::setVolume(qreal Volume)
{
    currentVolume = Volume;
    audio.setVolume(Volume);
    emit volumeChanged(currentVolume);
}

void CRadioController::setChannel(QString Channel, bool isScan, bool Force)
{
    if (currentChannel != Channel || Force == true || isPlaying == false) {
        if (device && device->getID() == CDeviceID::RAWFILE) {
            currentChannel = "File";
            if (!isScan)
                autoChannel = currentChannel;
            currentEId = 0;
            currentEnsembleLabel = "";
            currentFrequency = 0;
        }
        else { // A real device
            if(radioReceiver)
                radioReceiver->stop(); // Stop the demodulator in order to avoid working with old data
            currentChannel = Channel;
            if (!isScan)
                autoChannel = currentChannel;
            currentEId = 0;
            currentEnsembleLabel = "";

            // Convert channel into a frequency
            currentFrequency = channels.getFrequency(Channel.toStdString());

            if(currentFrequency != 0 && device) {
                qDebug() << "RadioController: Tune to channel" <<  Channel << "->" << currentFrequency/1e6 << "MHz";
                device->setFrequency(currentFrequency);
                device->reset(); // Clear buffer
            }
        }

        // Restart demodulator and decoder
        if(device) {
            radioReceiver = std::make_unique<RadioReceiver>(*this, *device, rro, 1);
            radioReceiver->setReceiverOptions(rro);
            radioReceiver->restart(isScan);
        }

        emit channelChanged();
        if (!isScan)
            emit autoChannelChanged(autoChannel);
        emit ensembleChanged();
        emit ensembleIdChanged();
        emit frequencyChanged();
    }
}

void CRadioController::setManualChannel(QString Channel)
{
    // Otherwise tune to channel and play first found station
    qDebug() << "RadioController: Tune to channel" <<  Channel;

    deviceRestart();

    currentTitle = Channel;
    emit titleChanged();

    currentService = 0;
    emit stationChanged();

    currentStationType = "";
    emit stationTypChanged();

    currentLanguageType = "";
    emit languageTypeChanged();

    currentText = "";
    emit textChanged();

    emit motReseted();

    // Switch channel
    setChannel(Channel, false, true);
}

void CRadioController::startScan(void)
{
    qDebug() << "RadioController:" << "Start channel scan";

    stop();

    deviceRestart();

    if(device && device->getID() == CDeviceID::RAWFILE) {
        currentTitle = tr("RAW File");
        const auto FirstChannel = QString::fromStdString(Channels::firstChannel);
        setChannel(FirstChannel, false); // Just a dummy
        emit scanStopped();
    }
    else
    {
        // Start with lowest frequency
        QString Channel = QString::fromStdString(Channels::firstChannel);
        setChannel(Channel, true);

        isChannelScan = true;
        emit isChannelScanChanged(isChannelScan);
        stationCount = 0;
        currentTitle = tr("Scanning") + " ... " + Channel
                + " (" + QString::number((1 * 100 / NUMBEROFCHANNELS)) + "%)";
        emit titleChanged();

        currentText = tr("Found channels") + ": " + QString::number(stationCount);
        emit textChanged();

        currentService = 0;
        emit stationChanged();

        currentStationType = "";
        emit stationTypChanged();

        currentLanguageType = "";
        emit languageTypeChanged();

        emit scanProgress(0);
    }
}

void CRadioController::stopScan(void)
{
    qDebug() << "RadioController:" << "Stop channel scan";

    currentTitle = tr("No Station");
    emit titleChanged();

    currentText = "";
    emit textChanged();

    isChannelScan = false;
    emit isChannelScanChanged(isChannelScan);
    emit scanStopped();

    stop();
}

void CRadioController::setAGC(bool isAGC)
{
    this->isAGC = isAGC;

    if (device) {
        device->setAgc(isAGC);

        if (!isAGC) {
            device->setGain(currentManualGain);
            qDebug() << "RadioController:" << "AGC off";
        }
        else {
            qDebug() << "RadioController:" <<  "AGC on";
        }
    }

    emit agcChanged(isAGC);
}

void CRadioController::disableCoarseCorrector(bool disable)
{
    rro.disableCoarseCorrector = disable;
    if (radioReceiver) {
        radioReceiver->setReceiverOptions(rro);
    }
}

void CRadioController::enableTIIDecode(bool enable)
{
    rro.decodeTII = enable;
    if (radioReceiver) {
        radioReceiver->setReceiverOptions(rro);
    }
}

void CRadioController::selectFFTWindowPlacement(int fft_window_placement_ix)
{
    if (fft_window_placement_ix == 0) {
        rro.fftPlacementMethod = FFTPlacementMethod::StrongestPeak;
    }
    else if (fft_window_placement_ix == 1) {
        rro.fftPlacementMethod = FFTPlacementMethod::EarliestPeakWithBinning;
    }
    else if (fft_window_placement_ix == 2) {
        rro.fftPlacementMethod = FFTPlacementMethod::ThresholdBeforePeak;
    }
    else {
        std::clog << "Invalid FFT window placement " <<
            fft_window_placement_ix << " chosen" << std::endl;
        return;
    }

    if (radioReceiver) {
        radioReceiver->setReceiverOptions(rro);
    }
}

void CRadioController::setFreqSyncMethod(int fsm_ix)
{
    rro.freqsyncMethod = static_cast<FreqsyncMethod>(fsm_ix);

    if (radioReceiver) {
        radioReceiver->setReceiverOptions(rro);
    }
}

void CRadioController::setGain(int Gain)
{
    currentManualGain = Gain;
    emit gainChanged(currentManualGain);

    if (device) {
        currentManualGainValue = device->setGain(Gain);
        emit gainValueChanged(currentManualGainValue);

        int32_t gainCount_tmp = device->getGainCount();

        if(gainCount != gainCount_tmp) {
            gainCount = gainCount_tmp;
            emit gainCountChanged(gainCount);
        }
    }
}

void CRadioController::initRecorder(int size)
{
    device->initRecordBuffer(size);
}

void CRadioController::triggerRecorder(QString filename)
{
    // TODO just for testing
    filename = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/welle-io-record.iq";
    std::string filename_tmp = filename.toStdString();
    device->writeRecordBufferToFile(filename_tmp);
}

DABParams& CRadioController::getParams()
{
    static DABParams dummyParams(1);

    if(radioReceiver)
        return radioReceiver->getParams();
    else
        return dummyParams;
}

int CRadioController::getCurrentFrequency()
{
    return currentFrequency;
}

std::vector<float> CRadioController::getImpulseResponse()
{
    std::lock_guard<std::mutex> lock(impulseResponseBufferMutex);
    auto buf = std::move(impulseResponseBuffer);
    return buf;
}

std::vector<DSPCOMPLEX> CRadioController::getSignalProbe()
{
    int16_t T_u = getParams().T_u;

    if (device) {
        return device->getSpectrumSamples(T_u);
    }
    else {
        std::vector<DSPCOMPLEX> dummyBuf(static_cast<std::vector<DSPCOMPLEX>::size_type>(T_u));
        return dummyBuf;
    }
}

std::vector<DSPCOMPLEX> CRadioController::getNullSymbol()
{
    std::lock_guard<std::mutex> lock(nullSymbolBufferMutex);
    auto buf = std::move(nullSymbolBuffer);
    return buf;
}

std::vector<DSPCOMPLEX> CRadioController::getConstellationPoint()
{
    std::lock_guard<std::mutex> lock(constellationPointBufferMutex);
    auto buf = std::move(constellationPointBuffer);
    return buf;
}

/********************
 * Private methods  *
 ********************/
void CRadioController::initialise(void)
{
    for (const auto& param_value : deviceParametersString) {
        device->setDeviceParam(param_value.first, param_value.second);
    }

    for (const auto param_value : deviceParametersInt) {
        device->setDeviceParam(param_value.first, param_value.second);
    }

    gainCount = device->getGainCount();
    emit gainCountChanged(gainCount);
    emit deviceReady();

    if (!isAGC) { // Manual AGC
        device->setAgc(false);
        currentManualGainValue = device->setGain(currentManualGain);
        emit gainValueChanged(currentManualGainValue);

        qDebug() << "RadioController:" << "AGC off";
    }
    else {
        device->setAgc(true);
        qDebug() << "RadioController:" << "AGC on";
    }

    audio.setVolume(currentVolume);

    deviceName = QString::fromStdString(device->getDescription());
    emit deviceNameChanged();

    deviceId = device->getID();
    emit deviceIdChanged();

    if(isAutoPlay) {
        play(autoChannel, tr("Playing last station"), autoService);
    }
}

void CRadioController::resetTechnicalData(void)
{
    currentChannel = tr("Unknown");
    emit channelChanged();

    currentEId = 0;
    currentEnsembleLabel = "";
    emit ensembleChanged();
    emit ensembleIdChanged();

    currentFrequency = 0;
    emit frequencyChanged();

    currentService = 0;
    emit stationChanged();

    currentStationType = "";
    emit stationTypChanged();

    currentLanguageType = "";
    emit languageTypeChanged();

    currentTitle = tr("No Station");
    emit titleChanged();

    currentText = "";
    emit textChanged();

    isPlaying = false;
    emit isPlayingChanged(isPlaying);

    errorMsg = "";
    isSync = false;
    emit isSyncChanged(isSync);
    isFICCRC = false;
    emit isFICCRCChanged(isFICCRC);
    isSignal = false;
    emit isSignalChanged(isSignal);
    snr = 0;
    emit snrChanged(snr);
    frequencyCorrection = 0;
    emit frequencyCorrectionChanged(frequencyCorrection);
    frequencyCorrectionPpm = NAN;
    emit frequencyCorrectionPpmChanged(frequencyCorrectionPpm);
    bitRate = 0;
    emit bitRateChanged(bitRate);
    audioSampleRate = 0;
    isDAB = true;
    emit isDABChanged(isDAB);
    frameErrors = 0;
    emit frameErrorsChanged(frameErrors);
    rsUncorrectedErrors = 0;
    emit rsUncorrectedErrorsChanged(this->rsUncorrectedErrors);
    emit rsCorrectedErrorsChanged(this->rsCorrectedErrors);
    aaErrors = 0;
    emit aacErrorsChanged(aaErrors);

    emit motReseted();
}

bool CRadioController::deviceRestart()
{
    bool isPlay = false;

    if(device) {
        isPlay = device->restart();
    } else {
        return false;
    }

    if(!isPlay) {
        qDebug() << "RadioController:" << "Radio device is not ready or does not exist.";
        emit showErrorMessage(tr("Radio device is not ready or does not exist."));
        return false;
    }

    labelTimer.start(40);
    return true;
}

/*****************
 * Public slots *
 *****************/
void CRadioController::ensembleId(quint16 eId)
{
    qDebug() << "RadioController: ID of ensemble:" << Qt::hex << eId;

    if (currentEId == eId)
        return;

    currentEId = eId;
    emit ensembleIdChanged();

    //auto label = radioReceiver->getEnsembleLabel();
    //currentEnsembleLabel = QString::fromStdString(label.utf8_label());

    //emit ensembleChanged();
}

void CRadioController::ensembleLabel(DabLabel& label)
{
    QString newLabel = QString::fromStdString(label.utf8_label());

    if (currentEnsembleLabel == newLabel)
        return;

    qDebug() << "RadioController: Label of ensemble:" << newLabel;
    currentEnsembleLabel = newLabel;

    emit ensembleChanged();
}

void CRadioController::setErrorMessage(QString Text)
{
    errorMsg = Text;
    emit showErrorMessage(Text);
}

void CRadioController::setErrorMessage(const std::string& head, const std::string& text)
{
    if (text.empty()) {
        setErrorMessage(tr(head.c_str()));
    }
    else {
        setErrorMessage(tr(head.c_str()) + ": " + QString::fromStdString(text));
    }
}

void CRadioController::setInfoMessage(QString Text)
{
    emit showInfoMessage(Text);
}

/********************
 * private slots *
 ********************/
void CRadioController::labelTimerTimeout()
{
    if (radioReceiver and not pendingLabels.empty()) {
        const auto sId = pendingLabels.front();
        pendingLabels.pop_front();

        std::string label;

        auto srv = radioReceiver->getService(sId);
        if (srv.serviceId != 0) {
            label = srv.serviceLabel.utf8_label();
        }

        if (not label.empty()) {
            const auto qlabel = QString::fromStdString(label);
            emit newStationNameReceived(qlabel, sId, currentChannel);
            qDebug() << "RadioController: Found service " << qPrintable(QString::number(sId, 16).toUpper()) << qlabel;

            if (currentService == sId) {
                currentTitle = qlabel;
                emit titleChanged();
            }
        }
        else {
            // Rotate pending labels to avoid getting stuck on a failing one
            pendingLabels.push_back(sId);
        }
    }
}

void CRadioController::stationTimerTimeout()
{
    if (!radioReceiver)
        return;

    const auto services = radioReceiver->getServiceList();

    for (const auto& s : services) {
        if (s.serviceId == currentService) {
            const auto comps = radioReceiver->getComponents(s);
            for (const auto& sc : comps) {
                if (sc.transportMode() == TransportMode::Audio && (
                        sc.audioType() == AudioServiceComponentType::DAB ||
                        sc.audioType() == AudioServiceComponentType::DABPlus) ) {
                    const auto& subch = radioReceiver->getSubchannel(sc);

                    if (not subch.valid()) {
                        return;
                    }

                    // We found the station inside the signal, lets stop the timer
                    stationTimer.stop();

                    std::string dumpFileName;
                    if (commandLineOptions["dumpFileName"] != "") {
                        dumpFileName = commandLineOptions["dumpFileName"].toString().toStdString();
                    }

                    bool success = radioReceiver->playSingleProgramme(*this, dumpFileName, s);
                    if (!success) {
                        qDebug() << "Selecting service failed";
                    }
                    else {
                        currentStationType = DABConstants::getProgramTypeName(s.programType);
                        emit stationTypChanged();

                        currentLanguageType = DABConstants::getLanguageName(s.language);
                        emit languageTypeChanged();

                        bitRate = subch.bitrate();
                        emit bitRateChanged(bitRate);

                        if (sc.audioType() == AudioServiceComponentType::DABPlus)
                            isDAB = false;
                        else
                            isDAB = true;
                        emit isDABChanged(isDAB);

                        // Update original subchannel ID for announcement switching
                        if (announcementManager_ && !m_isInAnnouncement) {
                            originalSubchannelId_ = subch.subChId;
                            announcementManager_->setOriginalService(currentService, subch.subChId);
                        }
                    }

                    return;
                }
            }
        }
    }
}

void CRadioController::channelTimerTimeout(void)
{
    channelTimer.stop();

    if(isChannelScan)
        nextChannel(false);
}

void CRadioController::announcementDurationTimerTimeout(void)
{
    // Update announcement duration every second
    if (m_isInAnnouncement && announcementManager_) {
        updateAnnouncementDuration();
    }
}

void CRadioController::displayDateTime(const dab_date_time_t& dateTime)
{
    QDate Date;
    QTime Time;

    Time.setHMS(dateTime.hour, dateTime.minutes, dateTime.seconds);
    currentDateTime.setTime(Time);

    Date.setDate(dateTime.year, dateTime.month, dateTime.day);
    currentDateTime.setDate(Date);

    // int OffsetFromUtc = dateTime.hourOffset * 3600 +
    //                     dateTime.minuteOffset * 60;
    currentDateTime.setTimeZone(QTimeZone::utc());

    emit dateTimeChanged(currentDateTime);
}

void CRadioController::nextChannel(bool isWait)
{
    if (isWait) { // It might be a channel, wait 10 seconds
        channelTimer.start(10000);
    }
    else {
        auto Channel = QString::fromStdString(channels.getNextChannel());

        if(!Channel.isEmpty()) {
            setChannel(Channel, true);

            int index = channels.getCurrentIndex() + 1;

            currentTitle = tr("Scanning") + " ... " + Channel
                    + " (" + QString::number(index * 100 / NUMBEROFCHANNELS) + "%)";
            emit titleChanged();

            emit scanProgress(index);
        }
        else {
            stopScan();
        }
    }
}

/*********************
 * Backend callbacks *
 *********************/
void CRadioController::onServiceDetected(uint32_t sId)
{
    // you may not call radioReceiver->getService() because it internally holds the FIG mutex.
    emit serviceDetected(sId);
}

void CRadioController::serviceId(quint32 sId)
{
    if (isChannelScan == true) {
        stationCount++;
        currentText = tr("Found channels") + ": " + QString::number(stationCount);
        emit textChanged();
    }

    if (sId <= 0xFFFF) {
        // Exclude data services from the list
        pendingLabels.push_back(sId);
    }
}

void CRadioController::onNewEnsemble(quint16 eId)
{
    emit ensembleIdUpdated(eId);
}

void CRadioController::onSetEnsembleLabel(DabLabel& label)
{
    emit ensembleLabelUpdated(label);
}

void CRadioController::onDateTimeUpdate(const dab_date_time_t& dateTime)
{
    emit dateTimeUpdated(dateTime);
}

void CRadioController::onFIBDecodeSuccess(bool crcCheckOk, const uint8_t* fib)
{
    (void)fib;
    if (isFICCRC == crcCheckOk)
        return;
    isFICCRC = crcCheckOk;
    emit isFICCRCChanged(isFICCRC);
}

void CRadioController::onNewImpulseResponse(std::vector<float>&& data)
{
    std::lock_guard<std::mutex> lock(impulseResponseBufferMutex);
    impulseResponseBuffer = std::move(data);
}

void CRadioController::onConstellationPoints(std::vector<DSPCOMPLEX>&& data)
{
    std::lock_guard<std::mutex> lock(constellationPointBufferMutex);
    constellationPointBuffer = std::move(data);
}

void CRadioController::onNewNullSymbol(std::vector<DSPCOMPLEX>&& data)
{
    std::lock_guard<std::mutex> lock(nullSymbolBufferMutex);
    nullSymbolBuffer = std::move(data);
}

void CRadioController::onTIIMeasurement(tii_measurement_t&& m)
{
    qDebug().noquote() << "TII comb " << m.comb <<
        " pattern " << m.pattern <<
        " delay " << m.delay_samples <<
        "= " << m.getDelayKm() << " km" <<
        " with error " << m.error;
}

void CRadioController::onMessage(message_level_t level, const std::string& text, const std::string& text2)
{
    QString fullText;
    if (text2.empty())
      fullText = tr(text.c_str());
    else
      fullText = tr(text.c_str()) + QString::fromStdString(text2);

    switch (level) {
        case message_level_t::Information:
            emit showInfoMessage(fullText);
            break;
        case message_level_t::Error:
            emit showErrorMessage(fullText);
            break;
    }
}

void CRadioController::onSNR(float snr)
{
    if (this->snr == snr)
        return;
    this->snr = snr;
    emit snrChanged(this->snr);
}

void CRadioController::onFrequencyCorrectorChange(int fine, int coarse)
{
    if (frequencyCorrection == coarse + fine)
        return;
    frequencyCorrection = coarse + fine;
    emit frequencyCorrectionChanged(frequencyCorrection);

    if (currentFrequency != 0)
        frequencyCorrectionPpm = -1000000.0f * static_cast<float>(frequencyCorrection) / static_cast<float>(currentFrequency);
    else
        frequencyCorrectionPpm = NAN;
    emit frequencyCorrectionPpmChanged(frequencyCorrectionPpm);
}

void CRadioController::onSyncChange(char isSync)
{
    bool sync = (isSync == SYNCED) ? true : false;
    if (this->isSync == sync)
        return;
    this->isSync = sync;
    emit isSyncChanged(isSync);
}

void CRadioController::onSignalPresence(bool isSignal)
{
    if (this->isSignal != isSignal) {
        this->isSignal = isSignal;
        emit isSignalChanged(isSignal);
    }

    if (isChannelScan)
        emit switchToNextChannel(isSignal);
}

void CRadioController::onNewAudio(std::vector<int16_t>&& audioData, int sampleRate, const std::string& mode)
{
    audioBuffer.putDataIntoBuffer(audioData.data(), static_cast<int32_t>(audioData.size()));

    if (audioSampleRate != sampleRate) {
        qDebug() << "RadioController: Audio sample rate" <<  sampleRate << "Hz, mode=" <<
            QString::fromStdString(mode);
        audioSampleRate = sampleRate;

        audio.setRate(sampleRate);
    }

    if (audioMode != QString::fromStdString(mode)) {
        audioMode = QString::fromStdString(mode);
        emit audioModeChanged(audioMode);
    }
}

void CRadioController::onFrameErrors(int frameErrors)
{
    if (this->frameErrors == frameErrors)
        return;
    this->frameErrors = frameErrors;
    emit frameErrorsChanged(this->frameErrors);
}

void CRadioController::onRsErrors(bool uncorrectedErrors, int numCorrectedErrors)
{
    if (this->rsUncorrectedErrors != uncorrectedErrors)
    {
        this->rsUncorrectedErrors = uncorrectedErrors;
        emit rsUncorrectedErrorsChanged(this->rsUncorrectedErrors);
    }

    if (this->rsCorrectedErrors != numCorrectedErrors)
    {
        this->rsCorrectedErrors = numCorrectedErrors;
        emit rsCorrectedErrorsChanged(this->rsCorrectedErrors);
    }
}

void CRadioController::onAacErrors(int aacErrors)
{
    if (this->aaErrors == aacErrors)
        return;
    this->aaErrors = aacErrors;
    emit aacErrorsChanged(this->aaErrors);
}

void CRadioController::onNewDynamicLabel(const std::string& label)
{
    auto qlabel = QString::fromUtf8(label.c_str());
    if (this->currentText != qlabel) {
        this->currentText = qlabel;
        emit textChanged();
    }
}

void CRadioController::onMOT(const mot_file_t& mot_file)
{
    emit motChanged(mot_file);
}

void CRadioController::onPADLengthError(size_t announced_xpad_len, size_t xpad_len)
{
    qDebug() << "X-PAD length mismatch, expected:" << announced_xpad_len << " effective:" << xpad_len;
}

void CRadioController::onInputFailure()
{
    stop();
}

void CRadioController::onRestartService()
{
    emit restartServiceRequested();
}

void CRadioController::restartService(void)
{
    setService(currentService, true);
}

// ============================================================================
// ANNOUNCEMENT BACKEND INTEGRATION
// ============================================================================

AnnouncementManager* CRadioController::getAnnouncementManager()
{
    return announcementManager_.get();
}

void CRadioController::onAnnouncementSupportUpdate(const ServiceAnnouncementSupport& support)
{
    // Called when FIG 0/18 is received (announcement support information)
    // This updates which services support which announcement types

    if (!announcementManager_) {
        return;
    }

    // Update announcement manager with support data
    announcementManager_->updateAnnouncementSupport(support);

    // Check if any announcements are supported in the ensemble
    bool hasSupport = support.support_flags.hasAny();
    if (m_announcementSupported != hasSupport) {
        m_announcementSupported = hasSupport;
        emit announcementSupportedChanged(hasSupport);
    }

    qDebug() << "RadioController: Announcement support updated for service"
             << QString::number(support.service_id, 16).toUpper()
             << "- flags:" << QString::number(support.support_flags.flags, 16);
}

void CRadioController::onAnnouncementSwitchingUpdate(
    const std::vector<ActiveAnnouncement>& announcements)
{
    // Called when FIG 0/19 is received (active announcement information)

    if (!announcementManager_ || !m_announcementEnabled) {
        return;
    }

    // Update announcement manager
    announcementManager_->updateActiveAnnouncements(announcements);

    for (const auto& ann : announcements) {
        if (!ann.isActive()) {
            // Announcement ended (ASw = 0x0000)
            if (m_isInAnnouncement) {
                ActiveAnnouncement current = announcementManager_->getCurrentAnnouncement();
                if (ann.cluster_id == current.cluster_id) {
                    handleAnnouncementEnded(ann);
                }
            }
            continue;
        }

        // Check if we should switch to this announcement
        if (announcementManager_->shouldSwitchToAnnouncement(ann)) {
            handleAnnouncementStarted(ann);
        }
    }
}

void CRadioController::handleAnnouncementStarted(const ActiveAnnouncement& ann)
{
    // Safety checks
    if (!radioReceiver || !isPlaying) {
        qWarning() << "RadioController: Cannot switch - radio not playing";
        return;
    }

    if (ann.subchannel_id == 0 || ann.subchannel_id > 63) {
        qWarning() << "RadioController: Invalid announcement subchannel ID" << ann.subchannel_id;
        return;
    }

    // Check priority if already in announcement
    if (m_isInAnnouncement) {
        ActiveAnnouncement current = announcementManager_->getCurrentAnnouncement();
        int currentPriority = getAnnouncementPriority(current.getHighestPriorityType());
        int newPriority = getAnnouncementPriority(ann.getHighestPriorityType());

        if (newPriority >= currentPriority) {
            // Don't switch to lower/equal priority announcement
            qDebug() << "RadioController: Ignoring lower/equal priority announcement"
                     << "(current priority:" << currentPriority
                     << ", new priority:" << newPriority << ")";
            return;
        }

        // Higher priority announcement - end current announcement first
        qDebug() << "RadioController: Switching to higher priority announcement"
                 << "(current priority:" << currentPriority
                 << ", new priority:" << newPriority << ")";
    }

    // Save current service/subchannel if not already in announcement
    if (!m_isInAnnouncement) {
        originalServiceId_ = currentService;
        originalSubchannelId_ = 0;  // Will be updated if available

        // Try to get current subchannel ID
        const auto services = radioReceiver->getServiceList();
        for (const auto& s : services) {
            if (s.serviceId == currentService) {
                const auto comps = radioReceiver->getComponents(s);
                for (const auto& sc : comps) {
                    if (sc.transportMode() == TransportMode::Audio) {
                        const auto& subch = radioReceiver->getSubchannel(sc);
                        if (subch.valid()) {
                            originalSubchannelId_ = subch.subChId;
                            break;
                        }
                    }
                }
                break;
            }
        }

        qDebug() << "RadioController: Saving original service"
                 << QString::number(originalServiceId_, 16).toUpper()
                 << "subchannel" << originalSubchannelId_;
    }

    // Find service that uses announcement subchannel
    uint32_t target_service_id = 0;
    QString target_service_name;
    const auto services = radioReceiver->getServiceList();

    for (const auto& s : services) {
        const auto comps = radioReceiver->getComponents(s);
        for (const auto& sc : comps) {
            if (sc.transportMode() == TransportMode::Audio && sc.subchannelId == ann.subchannel_id) {
                const auto& subch = radioReceiver->getSubchannel(sc);
                if (subch.valid() && subch.subChId == ann.subchannel_id) {
                    target_service_id = s.serviceId;
                    target_service_name = QString::fromStdString(s.serviceLabel.utf8_label());
                    break;
                }
            }
        }
        if (target_service_id != 0) break;
    }

    if (target_service_id == 0) {
        qWarning() << "RadioController: No service found for announcement subchannel"
                   << ann.subchannel_id;
        return;
    }

    // Switch to announcement in backend state
    announcementManager_->switchToAnnouncement(ann);

    // ACTUAL SERVICE SWITCHING - Call existing setService() method
    qDebug() << "RadioController: Switching to announcement service"
             << QString::number(target_service_id, 16).toUpper()
             << "(" << target_service_name << ")"
             << "on subchannel" << ann.subchannel_id;

    setService(target_service_id, true);  // Force=true to ensure switch

    // Update UI state
    m_isInAnnouncement = true;
    m_activeAnnouncementType = static_cast<int>(ann.getHighestPriorityType());
    m_announcementDuration = 0;
    m_announcementServiceName = target_service_name.isEmpty()
        ? QString("Announcement SubCh %1").arg(ann.subchannel_id)
        : target_service_name;

    emit isInAnnouncementChanged(true);
    emit activeAnnouncementTypeChanged(m_activeAnnouncementType);
    emit announcementDurationChanged(0);
    emit announcementServiceNameChanged(m_announcementServiceName);

    // Start duration timer (update every second)
    announcementDurationTimer.start(1000);

    // Add to history
    AnnouncementHistoryEntry entry;
    entry.startTime = QDateTime::currentDateTime();
    entry.type = m_activeAnnouncementType;
    entry.serviceName = m_announcementServiceName;
    entry.durationSeconds = 0;
    addAnnouncementToHistory(entry);

    qDebug() << "RadioController: Successfully switched to announcement type"
             << getAnnouncementTypeName(ann.getHighestPriorityType())
             << "on subchannel" << ann.subchannel_id;
}

void CRadioController::handleAnnouncementEnded(const ActiveAnnouncement& ann)
{
    if (!m_isInAnnouncement) {
        return;
    }

    qDebug() << "RadioController: Announcement ended, returning to original service";

    // Return to original service in backend state
    announcementManager_->returnToOriginalService();

    // Stop duration timer
    announcementDurationTimer.stop();

    // ACTUAL SERVICE RESTORATION - Call existing setService() method
    if (originalServiceId_ != 0 && radioReceiver) {
        qDebug() << "RadioController: Restoring original service"
                 << QString::number(originalServiceId_, 16).toUpper()
                 << "subchannel" << originalSubchannelId_;

        setService(originalServiceId_, true);  // Force=true to ensure switch

        // Reset saved state
        originalServiceId_ = 0;
        originalSubchannelId_ = 0;
    } else {
        qWarning() << "RadioController: No original service to restore";
    }

    // Update history with end time
    if (!m_announcementHistory.empty()) {
        std::lock_guard<std::mutex> lock(m_announcementHistoryMutex);
        auto& lastEntry = m_announcementHistory.back();
        lastEntry.endTime = QDateTime::currentDateTime();
        lastEntry.durationSeconds = m_announcementDuration;
        emit announcementHistoryChanged();
    }

    // Update UI state
    m_isInAnnouncement = false;
    m_activeAnnouncementType = -1;
    m_announcementDuration = 0;
    m_announcementServiceName.clear();

    emit isInAnnouncementChanged(false);
    emit activeAnnouncementTypeChanged(-1);
    emit announcementDurationChanged(0);
    emit announcementServiceNameChanged("");

    qDebug() << "RadioController: Successfully returned from announcement";
}

void CRadioController::updateAnnouncementDuration()
{
    if (!m_isInAnnouncement || !announcementManager_) {
        return;
    }

    int duration = announcementManager_->getAnnouncementDuration();

    if (m_announcementDuration != duration) {
        m_announcementDuration = duration;
        emit announcementDurationChanged(duration);

        // Check timeout
        if (duration >= m_maxAnnouncementDuration) {
            qDebug() << "RadioController: Announcement timeout exceeded, forcing return";
            ActiveAnnouncement ann;  // Dummy announcement for handleAnnouncementEnded
            handleAnnouncementEnded(ann);
        }
    }
}

uint8_t CRadioController::getCurrentClusterId() const
{
    // TODO: Get cluster ID for current service from FIBProcessor
    // This requires querying announcement support data
    // For now, return 0 (default cluster)
    return 0;
}

void CRadioController::loadAnnouncementSettings()
{
    QSettings settings;

    // P1-5 FIX (IMPROVE-004): Error handling in QSettings load
    // Previously: No error checking if QSettings fails
    // Fix: Add comprehensive error handling with fallback to defaults

    if (settings.status() != QSettings::NoError) {
        qWarning() << "RadioController: QSettings error on load, status:" << settings.status()
                   << "- using default announcement settings";
        // Continue with default values (will be set below)
    }

    settings.beginGroup("Announcements");

    m_announcementEnabled = settings.value("enabled", true).toBool();
    m_minAnnouncementPriority = settings.value("minPriority", 1).toInt();
    m_maxAnnouncementDuration = settings.value("maxDuration", 300).toInt();
    m_allowManualReturn = settings.value("allowManualReturn", true).toBool();

    // Load enabled types
    QStringList enabledTypes = settings.value("enabledTypes").toStringList();
    if (!enabledTypes.isEmpty()) {
        m_enabledAnnouncementTypes.clear();
        for (const QString& typeStr : enabledTypes) {
            bool ok = false;
            int typeValue = typeStr.toInt(&ok);
            if (ok && typeValue >= 0 && typeValue <= 10) {
                m_enabledAnnouncementTypes.insert(typeValue);
            } else {
                qWarning() << "RadioController: Invalid announcement type in settings:" << typeStr;
            }
        }
    } else {
        // Default: enable all types (P1-1 FIX: Use centralized MAX_TYPE)
        m_enabledAnnouncementTypes.clear();
        for (int i = 0; i <= static_cast<int>(AnnouncementType::MAX_TYPE); i++) {
            m_enabledAnnouncementTypes.insert(i);
        }
    }

    settings.endGroup();

    // Apply settings to announcement manager
    if (announcementManager_) {
        AnnouncementPreferences prefs;
        prefs.enabled = m_announcementEnabled;
        prefs.priority_threshold = m_minAnnouncementPriority;
        prefs.max_announcement_duration = std::chrono::seconds(m_maxAnnouncementDuration);
        prefs.allow_manual_return = m_allowManualReturn;

        // Copy enabled types
        for (int type : m_enabledAnnouncementTypes) {
            prefs.type_enabled[static_cast<AnnouncementType>(type)] = true;
        }

        announcementManager_->setUserPreferences(prefs);
    }

    qDebug() << "RadioController: Loaded announcement settings (enabled:"
             << m_announcementEnabled << ", priority:" << m_minAnnouncementPriority
             << ", enabled types:" << m_enabledAnnouncementTypes.size() << ")";
}

// ============================================================================
// ANNOUNCEMENT UI METHODS
// ============================================================================

QVariantList CRadioController::announcementHistory()
{
    std::lock_guard<std::mutex> lock(m_announcementHistoryMutex);
    QVariantList result;
    for (const auto& entry : m_announcementHistory) {
        result.append(entry.toVariantMap());
    }
    return result;
}

void CRadioController::addAnnouncementToHistory(const AnnouncementHistoryEntry& entry)
{
    std::lock_guard<std::mutex> lock(m_announcementHistoryMutex);
    m_announcementHistory.push_back(entry);

    // Enforce max size (FIFO - remove oldest entries)
    while (m_announcementHistory.size() > MAX_HISTORY_SIZE) {
        m_announcementHistory.pop_front();
    }

    emit announcementHistoryChanged();
}

void CRadioController::setAnnouncementEnabled(bool enabled)
{
    if (m_announcementEnabled != enabled) {
        m_announcementEnabled = enabled;
        emit announcementEnabledChanged(enabled);

        // Update backend
        if (announcementManager_) {
            AnnouncementPreferences prefs = announcementManager_->getUserPreferences();
            prefs.enabled = enabled;
            announcementManager_->setUserPreferences(prefs);
        }
    }
}

void CRadioController::setMinAnnouncementPriority(int priority)
{
    // Validate range: 1-11 (ETSI EN 300 401 announcement priorities)
    if (priority < 1 || priority > 11) {
        qDebug() << "RadioController: Invalid announcement priority" << priority
                 << "- must be in range 1-11. Ignoring.";
        return;
    }

    if (m_minAnnouncementPriority != priority) {
        m_minAnnouncementPriority = priority;
        emit minAnnouncementPriorityChanged(priority);

        // Update backend
        if (announcementManager_) {
            AnnouncementPreferences prefs = announcementManager_->getUserPreferences();
            prefs.priority_threshold = priority;
            announcementManager_->setUserPreferences(prefs);
        }
    }
}

void CRadioController::setMaxAnnouncementDuration(int duration)
{
    // Validate range: 30-600 seconds (30s to 10 minutes)
    if (duration < 30 || duration > 600) {
        qDebug() << "RadioController: Invalid announcement duration" << duration
                 << "seconds - must be in range 30-600 seconds. Ignoring.";
        return;
    }

    if (m_maxAnnouncementDuration != duration) {
        m_maxAnnouncementDuration = duration;
        emit maxAnnouncementDurationChanged(duration);

        // Update backend
        if (announcementManager_) {
            AnnouncementPreferences prefs = announcementManager_->getUserPreferences();
            prefs.max_announcement_duration = std::chrono::seconds(duration);
            announcementManager_->setUserPreferences(prefs);
        }
    }
}

void CRadioController::setAllowManualAnnouncementReturn(bool allow)
{
    if (m_allowManualReturn != allow) {
        m_allowManualReturn = allow;
        emit allowManualAnnouncementReturnChanged(allow);

        // Update backend
        if (announcementManager_) {
            AnnouncementPreferences prefs = announcementManager_->getUserPreferences();
            prefs.allow_manual_return = allow;
            announcementManager_->setUserPreferences(prefs);
        }
    }
}

void CRadioController::returnFromAnnouncement()
{
    if (!m_isInAnnouncement) {
        qDebug() << "RadioController: Not in announcement, cannot return";
        return;
    }

    // Check if manual return is allowed
    if (!m_allowManualReturn) {
        qDebug() << "RadioController: Manual return from announcement is disabled";
        return;
    }

    qDebug() << "RadioController: Manual return from announcement requested";

    // Get current announcement
    if (announcementManager_) {
        ActiveAnnouncement ann = announcementManager_->getCurrentAnnouncement();
        handleAnnouncementEnded(ann);
    }
}

bool CRadioController::isAnnouncementTypeEnabled(int type)
{
    // If no types explicitly set, all are enabled by default
    if (m_enabledAnnouncementTypes.empty()) {
        return true;
    }
    return m_enabledAnnouncementTypes.find(type) != m_enabledAnnouncementTypes.end();
}

void CRadioController::setAnnouncementTypeEnabled(int type, bool enabled)
{
    bool wasEnabled = isAnnouncementTypeEnabled(type);

    if (enabled) {
        m_enabledAnnouncementTypes.insert(type);
    } else {
        m_enabledAnnouncementTypes.erase(type);
    }

    if (wasEnabled != enabled) {
        qDebug() << "RadioController: Announcement type" << type << (enabled ? "enabled" : "disabled");

        // Update backend
        if (announcementManager_) {
            announcementManager_->enableAnnouncementType(static_cast<AnnouncementType>(type), enabled);
        }

        // Notify QML that announcement types changed
        emit announcementTypesChanged();
    }
}

void CRadioController::saveAnnouncementSettings()
{
    QSettings settings;

    // P1-5 FIX (IMPROVE-004): Error handling in QSettings save
    // Previously: No error checking if QSettings fails to save
    // Fix: Add comprehensive error handling with sync and status checks

    // Check settings accessibility before save
    if (settings.status() != QSettings::NoError) {
        qWarning() << "RadioController: QSettings error before save, status:" << settings.status();
        emit showErrorMessage(tr("Failed to access settings storage"));
        return;
    }

    settings.beginGroup("Announcements");

    settings.setValue("enabled", m_announcementEnabled);
    settings.setValue("minPriority", m_minAnnouncementPriority);
    settings.setValue("maxDuration", m_maxAnnouncementDuration);
    settings.setValue("allowManualReturn", m_allowManualReturn);

    // Save enabled types
    QStringList enabledTypes;
    for (int type : m_enabledAnnouncementTypes) {
        enabledTypes.append(QString::number(type));
    }
    settings.setValue("enabledTypes", enabledTypes);

    settings.endGroup();

    // Ensure sync to disk
    settings.sync();

    // Check for errors after sync
    if (settings.status() == QSettings::NoError) {
        qDebug() << "RadioController: Announcement settings saved successfully"
                 << "(enabled:" << m_announcementEnabled
                 << ", priority:" << m_minAnnouncementPriority
                 << ", types:" << enabledTypes.size() << ")";
    } else {
        qWarning() << "RadioController: Failed to sync announcement settings, status:"
                   << settings.status();
        emit showErrorMessage(tr("Failed to save announcement settings"));
    }
}

void CRadioController::resetAnnouncementSettings()
{
    // Reset to defaults
    setAnnouncementEnabled(true);
    setMinAnnouncementPriority(1);
    setMaxAnnouncementDuration(300);
    setAllowManualAnnouncementReturn(true);

    // Enable all types by default (P1-1 FIX: Use centralized MAX_TYPE)
    m_enabledAnnouncementTypes.clear();
    for (int i = 0; i <= static_cast<int>(AnnouncementType::MAX_TYPE); i++) {
        m_enabledAnnouncementTypes.insert(i);
    }

    saveAnnouncementSettings();

    qDebug() << "RadioController: Announcement settings reset to defaults";
}
