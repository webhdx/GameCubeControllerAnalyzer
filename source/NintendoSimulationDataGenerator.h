#ifndef NINTENDO_SIMULATION_DATA_GENERATOR
#define NINTENDO_SIMULATION_DATA_GENERATOR

#include <SimulationChannelDescriptor.h>
#include <string>
class NintendoAnalyzerSettings;

class NintendoSimulationDataGenerator {
  public:
    NintendoSimulationDataGenerator();
    ~NintendoSimulationDataGenerator();

    void Initialize(U32 simulation_sample_rate, NintendoAnalyzerSettings* settings);
    U32 GenerateSimulationData(
      U64 newest_sample_requested,
      U32 sample_rate,
      SimulationChannelDescriptor** simulation_channel);

  protected:
    NintendoAnalyzerSettings* mSettings;
    U32 mSimulationSampleRateHz;

  protected:
    enum class NintendoGenerationState {
        IdCmd,
        IdResp,
        OriginCmd,
        OriginResp,
        PollCmd,
        PollResp,
        DelayShort,
        DelayLong,
    };

    static const int ID_CMDS = 5;
    static const int POLL_CMDS = 20;

    U64 NsToSamples(U64 ns);

    void GenerateOne();
    void GenerateZero();
    void GenerateByte(U8 byte);
    void GenerateStopBit();
    void GenerateDelayShort();
    void GenerateDelayLong();

    void GenerateIdCmd();
    void GenerateIdResp();

    void GenerateOriginCmd();
    void GenerateOriginResp();

    void GeneratePollCmd();
    void GeneratePollResp();

    void RunStateMachine();

    NintendoGenerationState mNintendoGenerationState, mNintendoGenerationLastState;
    int mIdCmds = ID_CMDS;
    int mPollCmds = POLL_CMDS;

    SimulationChannelDescriptor mNintendoSimulationData;
};
#endif // NINTENDO_SIMULATION_DATA_GENERATOR