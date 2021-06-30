#include "NintendoAnalyzerSettings.h"

#include <AnalyzerHelpers.h>

NintendoAnalyzerSettings::NintendoAnalyzerSettings() : mInputChannel(UNDEFINED_CHANNEL) {
    mInputChannelInterface.reset(new AnalyzerSettingInterfaceChannel());
    mInputChannelInterface->SetTitleAndTooltip("Data", "Nintendo controller data line");
    mInputChannelInterface->SetChannel(mInputChannel);

    AddInterface(mInputChannelInterface.get());

    AddExportOption(0, "Export as text/csv file");
    AddExportExtension(0, "text", "txt");
    AddExportExtension(0, "csv", "csv");

    ClearChannels();
    AddChannel(mInputChannel, "Serial", false);
}

NintendoAnalyzerSettings::~NintendoAnalyzerSettings() {
}

bool NintendoAnalyzerSettings::SetSettingsFromInterfaces() {
    mInputChannel = mInputChannelInterface->GetChannel();

    ClearChannels();
    AddChannel(mInputChannel, "Nintendo", true);

    return true;
}

void NintendoAnalyzerSettings::UpdateInterfacesFromSettings() {
    mInputChannelInterface->SetChannel(mInputChannel);
}

void NintendoAnalyzerSettings::LoadSettings(const char* settings) {
    SimpleArchive text_archive;
    text_archive.SetString(settings);

    text_archive >> mInputChannel;

    ClearChannels();
    AddChannel(mInputChannel, "Nintendo", true);

    UpdateInterfacesFromSettings();
}

const char* NintendoAnalyzerSettings::SaveSettings() {
    SimpleArchive text_archive;

    text_archive << mInputChannel;

    return SetReturnString(text_archive.GetString());
}
