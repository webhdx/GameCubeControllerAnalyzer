#ifndef NINTENDO_ANALYZER_H
#define NINTENDO_ANALYZER_H

#include "NintendoAnalyzerResults.h"
#include "NintendoSimulationDataGenerator.h"

#include <Analyzer.h>

class NintendoAnalyzerSettings;
class ANALYZER_EXPORT NintendoAnalyzer : public Analyzer2 {
  public:
    NintendoAnalyzer();
    virtual ~NintendoAnalyzer();

    virtual void SetupResults();
    virtual void WorkerThread();

    virtual U32 GenerateSimulationData(
      U64 newest_sample_requested,
      U32 sample_rate,
      SimulationChannelDescriptor** simulation_channels);
    virtual U32 GetMinimumSampleRateHz();

    virtual const char* GetAnalyzerName() const;
    virtual bool NeedsRerun();

  protected: // vars
    struct ByteDecodeStatus {
        bool error = false;
        bool idle = false;
        U8 byte = 0x00;
    };

    std::auto_ptr<NintendoAnalyzerSettings> mSettings;
    std::auto_ptr<NintendoAnalyzerResults> mResults;
    AnalyzerChannelData* mData;

    NintendoSimulationDataGenerator mSimulationDataGenerator;
    bool mSimulationInitilized;

    U32 mSampleRateHz;
    U32 mStartOfStopBitOffset;
    U32 mEndOfStopBitOffset;

    U64 GetPulseWidthNs(U64 start_edge, U64 end_edge);
    ByteDecodeStatus DecodeByte();
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer();
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer(Analyzer* analyzer);

#endif // NINTENDO_ANALYZER_H
