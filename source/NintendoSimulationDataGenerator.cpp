#include "NintendoSimulationDataGenerator.h"

#include "NintendoAnalyzerSettings.h"

#include <AnalyzerHelpers.h>

NintendoSimulationDataGenerator::NintendoSimulationDataGenerator() {
    mNintendoGenerationState = mNintendoGenerationLastState = NintendoGenerationState::IdCmd;
}

NintendoSimulationDataGenerator::~NintendoSimulationDataGenerator() {
}

void NintendoSimulationDataGenerator::Initialize(
  U32 simulation_sample_rate, NintendoAnalyzerSettings* settings) {
    mSimulationSampleRateHz = simulation_sample_rate;
    mSettings = settings;

    mNintendoSimulationData.SetChannel(mSettings->mInputChannel);
    mNintendoSimulationData.SetSampleRate(simulation_sample_rate);
    mNintendoSimulationData.SetInitialBitState(BIT_HIGH);
}

U32 NintendoSimulationDataGenerator::GenerateSimulationData(
  U64 largest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel) {
    U64 adjusted_largest_sample_requested = AnalyzerHelpers::AdjustSimulationTargetSample(
      largest_sample_requested, sample_rate, mSimulationSampleRateHz);

    while (mNintendoSimulationData.GetCurrentSampleNumber() < adjusted_largest_sample_requested) {
        RunStateMachine();
    }

    *simulation_channel = &mNintendoSimulationData;
    return 1;
}

U64 NintendoSimulationDataGenerator::NsToSamples(U64 ns) {
    return mSimulationSampleRateHz * ns / 1e9;
}

void NintendoSimulationDataGenerator::GenerateOne() {
    mNintendoSimulationData.Transition();
    mNintendoSimulationData.Advance(NsToSamples(1250));
    mNintendoSimulationData.Transition();
    mNintendoSimulationData.Advance(NsToSamples(3750));
}

void NintendoSimulationDataGenerator::GenerateZero() {
    mNintendoSimulationData.Transition();
    mNintendoSimulationData.Advance(NsToSamples(3750));
    mNintendoSimulationData.Transition();
    mNintendoSimulationData.Advance(NsToSamples(1250));
}

void NintendoSimulationDataGenerator::GenerateByte(U8 byte) {
    U8 mask = 1 << 7;
    for (int i = 0; i < 8; i++) {
        if (byte & mask) {
            GenerateOne();
        } else {
            GenerateZero();
        }
        mask >>= 1;
    }
}

void NintendoSimulationDataGenerator::GenerateStopBit() {
    GenerateOne();
}

void NintendoSimulationDataGenerator::GenerateDelayShort() {
    mNintendoSimulationData.Advance(NsToSamples(50000));
}
void NintendoSimulationDataGenerator::GenerateDelayLong() {
    mNintendoSimulationData.Advance(NsToSamples(1000000));
}

void NintendoSimulationDataGenerator::GenerateIdCmd() {
    // cmd
    GenerateByte(0x00);
    GenerateStopBit();
}

void NintendoSimulationDataGenerator::GenerateIdResp() {
    // controller info
    GenerateByte(0x09);
    GenerateByte(0x00);
    GenerateByte(0x20);
    GenerateStopBit();
}

void NintendoSimulationDataGenerator::GenerateOriginCmd() {
    // cmd
    GenerateByte(0x41);
    GenerateStopBit();
}

void NintendoSimulationDataGenerator::GenerateOriginResp() {
    // buttons0
    GenerateByte(0x00);
    // buttons1
    GenerateByte(0x80);
    // joystick x
    GenerateByte(0x80);
    // joystick y
    GenerateByte(0x80);
    // c stick x
    GenerateByte(0x80);
    // c stick y
    GenerateByte(0x80);
    // l analog
    GenerateByte(0x20);
    // r analog
    GenerateByte(0x20);
    // analog a
    GenerateByte(0x02);
    // analog b
    GenerateByte(0x02);
    GenerateStopBit();
}

void NintendoSimulationDataGenerator::GeneratePollCmd() {
    // cmd
    GenerateByte(0x40);
    // args
    GenerateByte(0x03);
    GenerateByte(0x00);
    GenerateStopBit();
}

void NintendoSimulationDataGenerator::GeneratePollResp() {
    // buttons0
    GenerateByte(0x00);
    // buttons1
    GenerateByte(0x80);
    // joystick x
    GenerateByte(0x80);
    // joystick y
    GenerateByte(0x80);
    // c stick x
    GenerateByte(0x80);
    // c stick y
    GenerateByte(0x80);
    // l analog
    GenerateByte(0x20);
    // r analog
    GenerateByte(0x20);
    GenerateStopBit();
}

void NintendoSimulationDataGenerator::RunStateMachine() {
    // generates GameCube controller data
    switch (mNintendoGenerationState) {
        case NintendoGenerationState::IdCmd:
            GenerateIdCmd();
            mNintendoGenerationLastState = NintendoGenerationState::IdCmd;
            mNintendoGenerationState = NintendoGenerationState::DelayShort;
            break;
        case NintendoGenerationState::IdResp:
            GenerateIdResp();
            mNintendoGenerationLastState = NintendoGenerationState::IdResp;
            mNintendoGenerationState = NintendoGenerationState::DelayLong;
            break;
        case NintendoGenerationState::OriginCmd:
            GenerateOriginCmd();
            mNintendoGenerationLastState = NintendoGenerationState::OriginCmd;
            mNintendoGenerationState = NintendoGenerationState::DelayShort;
            break;
        case NintendoGenerationState::OriginResp:
            GenerateOriginResp();
            mNintendoGenerationLastState = NintendoGenerationState::OriginResp;
            mNintendoGenerationState = NintendoGenerationState::DelayLong;
            break;
        case NintendoGenerationState::PollCmd:
            GeneratePollCmd();
            mNintendoGenerationLastState = NintendoGenerationState::PollCmd;
            mNintendoGenerationState = NintendoGenerationState::DelayShort;
            break;
        case NintendoGenerationState::PollResp:
            GeneratePollResp();
            mNintendoGenerationLastState = NintendoGenerationState::PollResp;
            mNintendoGenerationState = NintendoGenerationState::DelayLong;
            break;
        case NintendoGenerationState::DelayShort:
            GenerateDelayShort();
            switch (mNintendoGenerationLastState) {
                case NintendoGenerationState::IdCmd:
                    if (mIdCmds--) {
                        mNintendoGenerationState = NintendoGenerationState::DelayLong;
                    } else {
                        mNintendoGenerationState = NintendoGenerationState::IdResp;
                        mIdCmds = ID_CMDS;
                    }
                    break;
                case NintendoGenerationState::OriginCmd:
                    mNintendoGenerationState = NintendoGenerationState::OriginResp;
                    break;
                case NintendoGenerationState::PollCmd:
                    if (mPollCmds--) {
                        mNintendoGenerationState = NintendoGenerationState::PollResp;
                    } else {
                        mNintendoGenerationState = NintendoGenerationState::DelayLong;
                        mPollCmds = POLL_CMDS;
                    }
                    break;
                default:
                    break;
            }
            mNintendoGenerationLastState = NintendoGenerationState::DelayShort;
            break;
        case NintendoGenerationState::DelayLong:
            GenerateDelayLong();
            switch (mNintendoGenerationLastState) {
                case NintendoGenerationState::DelayShort:
                case NintendoGenerationState::DelayLong:
                    mNintendoGenerationState = NintendoGenerationState::IdCmd;
                    break;
                case NintendoGenerationState::IdResp:
                    mNintendoGenerationState = NintendoGenerationState::OriginCmd;
                    break;
                case NintendoGenerationState::OriginResp:
                    mNintendoGenerationState = NintendoGenerationState::PollCmd;
                    break;
                case NintendoGenerationState::PollResp:
                    mNintendoGenerationState = NintendoGenerationState::PollCmd;
                    break;
                default:
                    break;
            }
            mNintendoGenerationLastState = NintendoGenerationState::DelayLong;
            break;
        default:
            break;
    }
}
