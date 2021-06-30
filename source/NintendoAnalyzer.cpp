#include "NintendoAnalyzer.h"

#include "NintendoAnalyzerSettings.h"

#include <AnalyzerChannelData.h>
#include <vector>

NintendoAnalyzer::NintendoAnalyzer() :
    Analyzer2(), mSettings(new NintendoAnalyzerSettings()), mSimulationInitilized(false) {
    SetAnalyzerSettings(mSettings.get());
}

NintendoAnalyzer::~NintendoAnalyzer() {
    KillThread();
}

void NintendoAnalyzer::SetupResults() {
    mResults.reset(new NintendoAnalyzerResults(this, mSettings.get()));
    SetAnalyzerResults(mResults.get());
    mResults->AddChannelBubblesWillAppearOn(mSettings->mInputChannel);
}

void NintendoAnalyzer::WorkerThread() {
    mSampleRateHz = GetSampleRate();

    mData = GetAnalyzerChannelData(mSettings->mInputChannel);

    // start at the first falling edge
    if (mData->GetBitState() == BIT_HIGH) mData->AdvanceToNextEdge();

    while (true) {
        bool idle = false;
        bool error = false;
        U64 error_frame_start_sample, error_frame_end_sample;

        while (!idle) {
            U64 frame_start_sample = mData->GetSampleNumber();
            ByteDecodeStatus result = DecodeByte();
            U64 frame_end_sample = mData->GetSampleNumber();

            Frame frame;
            frame.mStartingSampleInclusive = frame_start_sample;
            frame.mEndingSampleInclusive = frame_end_sample;
            frame.mData1 = result.byte;
            frame.mFlags = 0;

            if (result.error) {
                if (!error) {
                    error_frame_start_sample = frame_start_sample;
                }
                error = true;
            }
            if (result.idle) {
                if (error) {
                    error_frame_end_sample = frame_start_sample;
                }
                idle = true;
            }

            if (!idle && !error) {
                mResults->AddFrame(frame);
                mResults->CommitResults();
            } else if (idle && error) {
                frame.mStartingSampleInclusive = error_frame_start_sample;
                frame.mEndingSampleInclusive = error_frame_end_sample;
                frame.mData1 = 0;
                frame.mFlags |= DISPLAY_AS_ERROR_FLAG;
                mResults->AddFrame(frame);
                mResults->CommitResults();
            }

            ReportProgress(frame_end_sample);
        }

        if (!error) {
            mResults->CommitPacketAndStartNewPacket();
        } else {
            mResults->CancelPacketAndStartNewPacket();
        }

        CheckIfThreadShouldExit();
    }
}

bool NintendoAnalyzer::NeedsRerun() {
    return false;
}

U32 NintendoAnalyzer::GenerateSimulationData(
  U64 minimum_sample_index,
  U32 device_sample_rate,
  SimulationChannelDescriptor** simulation_channels) {
    if (mSimulationInitilized == false) {
        mSimulationDataGenerator.Initialize(GetSimulationSampleRate(), mSettings.get());
        mSimulationInitilized = true;
    }

    return mSimulationDataGenerator.GenerateSimulationData(
      minimum_sample_index, device_sample_rate, simulation_channels);
}

U32 NintendoAnalyzer::GetMinimumSampleRateHz() {
    return 2000000;
}

const char* NintendoAnalyzer::GetAnalyzerName() const {
    return "Nintendo";
}

const char* GetAnalyzerName() {
    return "Nintendo";
}

Analyzer* CreateAnalyzer() {
    return new NintendoAnalyzer();
}

void DestroyAnalyzer(Analyzer* analyzer) {
    delete analyzer;
}

U64 NintendoAnalyzer::GetPulseWidthNs(U64 start_edge, U64 end_edge) {
    return (end_edge - start_edge) * 1e9 / mSampleRateHz;
}

NintendoAnalyzer::ByteDecodeStatus NintendoAnalyzer::DecodeByte() {
    ByteDecodeStatus status;
    U64 starting_sample, ending_sample, falling_edge_sample, rising_edge_sample;

    for (U8 bit = 0; bit < 8; bit++) {
        // compute low time
        starting_sample = falling_edge_sample = mData->GetSampleNumber();
        mData->AdvanceToNextEdge();
        rising_edge_sample = mData->GetSampleNumber();
        U64 low_time = GetPulseWidthNs(falling_edge_sample, rising_edge_sample);

        if (750 <= low_time && low_time <= 1500) {
            // detected a 1
            status.byte |= 1 << (7 - bit);
        } else if (2750 <= low_time && low_time <= 4000) {
            // detected a 0
        } else {
            // data is corrupt
            status.error = true;
            mData->AdvanceToNextEdge();
            break;
        }

        // compute high time
        mData->AdvanceToNextEdge();
        ending_sample = falling_edge_sample = mData->GetSampleNumber();
        U64 high_time = GetPulseWidthNs(rising_edge_sample, falling_edge_sample);

        // the line is idle - packet complete
        if (high_time > 4000) {
            if (status.byte != 0x80 || bit != 0) {
                // not a stop bit
                status.error = true;
            }
            status.idle = true;
            break;
        } else {
            U64 middle_sample = (starting_sample + ending_sample) / 2;
            mResults->AddMarker(middle_sample, AnalyzerResults::Dot, mSettings->mInputChannel);
        }
    }

    return status;
}
