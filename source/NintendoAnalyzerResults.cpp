#include "NintendoAnalyzerResults.h"

#include "NintendoAnalyzer.h"
#include "NintendoAnalyzerSettings.h"

#include <AnalyzerHelpers.h>
#include <fstream>
#include <iostream>

NintendoAnalyzerResults::NintendoAnalyzerResults(
  NintendoAnalyzer* analyzer, NintendoAnalyzerSettings* settings) :
    AnalyzerResults(),
    mSettings(settings), mAnalyzer(analyzer) {
}

NintendoAnalyzerResults::~NintendoAnalyzerResults() {
}

void NintendoAnalyzerResults::GenerateBubbleText(
  U64 frame_index, Channel& channel, DisplayBase display_base) {
    ClearResultStrings();
    Frame frame = GetFrame(frame_index);

    bool error = frame.mFlags & DISPLAY_AS_ERROR_FLAG;

    if (!error) {
        char number_str[128];
        AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);
        AddResultString(number_str);
    } else {
        // This is not really worth enabling unless you can change the color
        AddResultString("ERROR: Invalid frame");
    }
}

void NintendoAnalyzerResults::GenerateExportFile(
  const char* file, DisplayBase display_base, U32 export_type_user_id) {
    std::ofstream file_stream(file, std::ios::out);

    U64 trigger_sample = mAnalyzer->GetTriggerSample();
    U32 sample_rate = mAnalyzer->GetSampleRate();

    file_stream << "Time [s], Value [0, 1]" << std::endl;

    U64 num_frames = GetNumFrames();
    for (U32 i = 0; i < num_frames; i++) {
        Frame frame = GetFrame(i);

        char time_str[128];
        AnalyzerHelpers::GetTimeString(
          frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, 128);

        char number_str[128];
        AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);

        file_stream << time_str << "," << number_str << std::endl;

        if (UpdateExportProgressAndCheckForCancel(i, num_frames) == true) {
            file_stream.close();
            return;
        }
    }

    file_stream.close();
}

void NintendoAnalyzerResults::GenerateFrameTabularText(U64 frame_index, DisplayBase display_base) {
#ifdef SUPPORTS_PROTOCOL_SEARCH
    Frame frame = GetFrame(frame_index);
    ClearTabularText();

    char number_str[128];
    AnalyzerHelpers::GetNumberString(frame.mData1, display_base, 8, number_str, 128);
    AddTabularText(number_str);
#endif
}

void NintendoAnalyzerResults::GeneratePacketTabularText(U64 packet_id, DisplayBase display_base) {
    // not supported
}

void NintendoAnalyzerResults::GenerateTransactionTabularText(
  U64 transaction_id, DisplayBase display_base) {
    // not supported
}