#ifndef SOUND_INCLUDED
#define SOUND_INCLUDED

#define WAVEDATA_LENGTH 16

typedef enum {
    SoundConsecutive = 0,
    SoundWait = 1
} SoundTimingMode;

typedef enum {
    SoundMute = 0,
    SoundNoShift = 1,
    SoundHalfShift = 2,
    SoundQuarterShift = 3
} WavePatternShift;

typedef natural Frequency;

typedef struct {
    natural BufferLength;
    union {
        struct {
            byte Volume;
            byte StereoControl;
            byte ChannelControl;
        } Bytes;
        struct {
            bool IsLeftTerminalEnabled:1; // S02
            byte LeftTerminalVolume:3;
            bool IsRightTerminalEnabled:1; //S01
            byte RightTerminalVolume:3;

            bool IsChannel4OnLeftTerminal:1;
            bool IsChannel3OnLeftTerminal:1;
            bool IsChannel2OnLeftTerminal:1;
            bool IsChannel1OnLeftTerminal:1;
            bool IsChannel4OnRightTerminal:1;
            bool IsChannel3OnRightTerminal:1;
            bool IsChannel2OnRightTerminal:1;
            bool IsChannel1OnRightTerminal:1;

            bool IsSoundEnabled:1;
            byte pad:3;
            bool IsChannel4Enabled:1;
            bool IsChannel3Enabled:1;
            bool IsChannel2Enabled:1;
            bool IsChannel1Enabled:1;
        } Bits;
    };
} GbSoundControl;

typedef struct {
    byte ChannelNumber;
    union {
        struct {
            byte SweepRegister;
            byte SoundLength;
            byte Envelope;
            byte Frequency1;
            byte FrequencyAndTiming;
        } Bytes;

        struct {
            byte pad1:1;
            byte SweepTime:3;
            bool IsFrequencyDecrease:1;
            byte SweepShift:3;
            byte pad2:8;
            byte pad3:8;
            byte pad4:8;
            byte pad5:8;
        } Channel1;

        struct {
            byte pad1:8;
            byte WavePatternDuty:2;
            byte pad2:6;
            byte pad3:8;
            byte pad4:8;
            byte pad5:8;
        } Channel1And2;

        struct {
            byte pad1:8;
            byte pad2:8;
            byte InitialVolume:4;
            bool IsVolumeIncreasing:1;
            byte EnvelopeSteps:3;
            byte pad3:8;
            byte pad4:8;
        } Envelope;

        struct {
            byte pad1:8;
            byte pad2:8;
            byte pad3:8;
            byte Frequency1:8;
            bool IsSoundReset:1;
            byte pad4:4;
            byte Frequency2:3;
        } Frequency;

        struct {
            byte pad1:8;
            byte pad2:2;
            byte SoundLength:6;
            byte pad3:8;
            byte pad4:8;
            byte pad5:8;
            SoundTimingMode TimingMode:1;
            byte pad6:7;
        } AllChannels;

        struct {
            bool IsEnabled:1;
            byte pad1:7;
            byte pad2:8;
            byte pad3:1;
            WavePatternShift PatternShift:2;
            byte pad5:5;
            byte pad6:8;
            byte pad7:8;
        } Channel3;

        struct {
            byte pad7:8;
            byte pad8:8;
            byte pad9:8;
            byte NoiseRatioShift:4;
            byte NoiseStepsCode: 1;
            byte NoiseRatioFactor:3;
            byte pad10: 8;
        } Channel4;
    };

    byte WaveData[WAVEDATA_LENGTH];

} GbSoundChannel;

void prepareSoundBuffer(const GbSoundControl* control, const GbSoundChannel* channel, sShort* buffer);
void getSoundControl(const GbState* gbState, GbSoundControl* result);
void getSoundChannel(const GbState* gbState, const byte channelNumber, GbSoundChannel* result);

#endif
