#ifndef NINTENDO_ANALYZER_SETTINGS
#define NINTENDO_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class NintendoAnalyzerSettings : public AnalyzerSettings {
  public:
    NintendoAnalyzerSettings();
    virtual ~NintendoAnalyzerSettings();

    virtual bool SetSettingsFromInterfaces();
    void UpdateInterfacesFromSettings();
    virtual void LoadSettings(const char* settings);
    virtual const char* SaveSettings();

    Channel mInputChannel;
    U32 mBitRate;

  protected:
    std::auto_ptr<AnalyzerSettingInterfaceChannel> mInputChannelInterface;
};

#endif // NINTENDO_ANALYZER_SETTINGS
