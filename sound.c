#include "sound.h"

#include <string.h>
#include <stdlib.h>
#include "core.h"
#include "state.h"
#include <math.h>


static const byte WaveDuty[4][8] = {
    { 1, 0, 0, 0, 0, 0, 0, 0 },
    { 1, 1, 0, 0, 0, 0, 0, 0 },
    { 1, 1, 1, 1, 0, 0, 0, 0 },
    { 1, 1, 1, 1, 1, 1, 0, 0 }
};

/**
 * @return Duration in milliseconds.
 */
natural getSoundDuration(const GbSoundChannel* channel) {
    return (64 - channel->AllChannels.SoundLength) * 4;
}

Frequency getFrequencyData(const GbSoundChannel* channel) {
    if (channel->ChannelNumber >= 4) {
        return 0;
    }
    natural result =
        0x0000 |
        ((natural)channel->Frequency.Frequency2 & 0x0007) << 8 | // Only the lowest 3 bits
        (channel->Frequency.Frequency1);
    return result;
}

natural getHertz(const GbSoundChannel* channel) {
    if (channel->ChannelNumber >= 4) {
        return 0;
    }

    Frequency data = getFrequencyData(channel);
    return 131072 / (2048 - data);
}

float getNextSweepFrequency(const GbSoundChannel* channel, float currentHz) {
    if (channel->ChannelNumber != 1) {
        return 0;
    }
    sByte dir = 1;
    if (channel->Channel1.IsFrequencyDecrease) {
        dir = -1;
    }

    return currentHz + dir * (currentHz/(2^channel->Channel1.SweepShift));
}

/**
 * Take the data from the SWEEP channel registers and tranform it into a format that consumed by the N64's audio system.
 * @param channel The channel data.
 * @out buffer The transformed audui buffer.
 */
void prepareChannel1(const GbSoundControl* control, const GbSoundChannel* channel, sShort* buffer) {
    if (channel->ChannelNumber != 1) {
        return;
    }

    for(natural i = 0; i < control->BufferLength; i += 2) {
        sShort sample = 0;

        sByte sign = channel->Envelope.IsVolumeIncreasing ? 1 : -1;
        sample = channel->Envelope.InitialVolume + (sign * channel->Envelope.EnvelopeSteps);

        // Follows the square wave pattern.
        if (!WaveDuty[channel->Channel1And2.WavePatternDuty][i % 8]) {
            sample *= -1;
        }

        // Don't know why we do this, but gnuboy does it and it makes sense.
        sample <<= 2;

        if (control->Bits.IsLeftTerminalEnabled && control->Bits.IsChannel1OnLeftTerminal) {
            buffer[i] += sample;
        }
        if (control->Bits.IsRightTerminalEnabled && control->Bits.IsChannel1OnRightTerminal) {
            buffer[i+1] += sample;
        }
    }
}

/**
 * Take the data from the PULSE channel registers and tranform it into a format that consumed by the N64's audio system.
 * @param channel The channel data.
 * @out buffer The transformed audui buffer.
 */
void prepareChannel2(const GbSoundControl* control, const GbSoundChannel* channel, sShort* buffer) {
    if (channel->ChannelNumber != 2) {
        return;
    }

    for(natural i = 0; i < control->BufferLength; i += 2) {
        sShort sample = 0;

        sByte sign = channel->Envelope.IsVolumeIncreasing ? 1 : -1;
        sample = channel->Envelope.InitialVolume + (sign * channel->Envelope.EnvelopeSteps);

        // Follows the square wave pattern.
        if (!WaveDuty[channel->Channel1And2.WavePatternDuty][i % 8]) {
            sample *= -1;
        }

        // Don't know why we do this, but gnuboy does it and it makes sense.
        sample <<= 2;

        if (control->Bits.IsLeftTerminalEnabled && control->Bits.IsChannel2OnLeftTerminal) {
            buffer[i] += sample;
        }
        if (control->Bits.IsRightTerminalEnabled && control->Bits.IsChannel2OnRightTerminal) {
            buffer[i+1] += sample;
        }
    }
}

/**
 * Take the data from the WAVE channel registers and tranform it into a format that consumed by the N64's audio system.
 * @param channel The channel data.
 * @out buffer The transformed audui buffer.
 */
void prepareChannel3(const GbSoundControl* control, const GbSoundChannel* channel, sShort* buffer) {
    if (channel->ChannelNumber != 3) {
        return;
    }

    for (natural i = 0; i < control->BufferLength; i+=4) {
        byte byteNumber = control->BufferLength % WAVEDATA_LENGTH;
        byte data = channel->WaveData[byteNumber];

        // Four bits per sample.  We interleave between the left and right speaker.
        byte nibble1 = data >> 4;
        byte nibble2 = data & 0x0F;

        switch(channel->Channel3.PatternShift) {
            case SoundNoShift: break;
            case SoundHalfShift:
                nibble1 >>= 1;
                nibble2 >>= 1;
            case SoundQuarterShift:
                nibble1 >>= 2;
                nibble2 >>= 2;
            case SoundMute:
            default:
                continue;
        }

        if (control->Bits.IsLeftTerminalEnabled && control->Bits.IsChannel3OnLeftTerminal) {
            buffer[i] += nibble1;
            buffer[i+2] += nibble2;
        }
        if (control->Bits.IsRightTerminalEnabled && control->Bits.IsChannel3OnRightTerminal) {
            buffer[i+1] += nibble1;
            buffer[i+3] += nibble2;
        }
    }
}

/**
 * Take the data from the NOISE channel registers and tranform it into a format that consumed by the N64's audio system.
 * @param channel The channel data.
 * @out buffer The transformed audui buffer.
 */
void prepareChannel4(const GbSoundControl* control, const GbSoundChannel* channel, sShort* buffer) {
    if (channel->ChannelNumber != 4) {
        return;
    }

    for(natural i = 0; i < control->BufferLength; i += 2) {

        // TODO - It seems there's a really complicated way of calculating this, but rand will do for now.
        byte sample = rand() % 2;
        if (sample == 0) {
            sample = -1;
        }

        sByte sign = channel->Envelope.IsVolumeIncreasing ? 1 : -1;
        sample = sample * channel->Envelope.InitialVolume + (sample * sign * channel->Envelope.EnvelopeSteps);

        // Don't know why we do this, but gnuboy does it and it makes sense.
        sample += sample << 1;

        if (control->Bits.IsLeftTerminalEnabled && control->Bits.IsChannel4OnLeftTerminal) {
            buffer[i] += sample;
        }
        if (control->Bits.IsRightTerminalEnabled && control->Bits.IsChannel4OnRightTerminal) {
            buffer[i+1] += sample;
        }
    }
}

/**
 * Adjusts the buffer with data from the given channel.
 * @param control General sound control information.
 * @param channel Sound channel data.
 * @out buffer N64 Audio buffer.
 */
void prepareSoundBuffer(const GbSoundControl* control, const GbSoundChannel* channel, sShort* buffer) {
    switch(channel->ChannelNumber) {
        case 1: prepareChannel1(control, channel, buffer);
        case 2: prepareChannel2(control, channel, buffer);
        case 3: prepareChannel3(control, channel, buffer);
        case 4: prepareChannel4(control, channel, buffer);
        default: return;
    }
}

/**
 * Gets sound data for a specific channel from gameboy registers.
 * @param gameboy state with sound registers
 * @out result The sound control data.
 */
void getSoundControl(const GbState* gbState, GbSoundControl* result) {
    /*
    result->BufferLength = audio_get_buffer_length();
    result->Bytes.Volume = gbState->io_sound_terminal_control;
    result->Bytes.StereoControl = gbState->io_sound_out_terminal;
    result->Bytes.ChannelControl = gbState->AudioChannelSwitch;
    */
}

/**
 * Gets general sound control info from gameboy registers.
 * @param gameboy state with sound registers
 * @param channelNumber channel to get data for.
 * @out result The channel data.
 */
void getSoundChannel(const GbState* gbState, const byte channelNumber, GbSoundChannel* result) {
    /*
    result->ChannelNumber = channelNumber;
    switch(channelNumber) {
        case 1:
            result->Bytes.SweepRegister = gbState->AudioChannel1Sweep;
            result->Bytes.SoundLength = gbState->AudioChannel1PatternAndLength;
            result->Bytes.Envelope = gbState->AudioChannel1Envelope;
            result->Bytes.Frequency1 = gbState->AudioChannel1Frequency;
            result->Bytes.FrequencyAndTiming = gbState->AudioChannel1Flags;
            break;
        case 2:
            result->Bytes.SweepRegister = 0;
            result->Bytes.SoundLength = gbState->AudioChannel2PatternAndLength;
            result->Bytes.Envelope = gbState->AudioChannel2Envelope;
            result->Bytes.Frequency1 = gbState->AudioChannel2FrequencyLow;
            result->Bytes.FrequencyAndTiming = gbState->AudioChannel2Flags;
            break;
        case 3:
            result->Bytes.SweepRegister = gbState->AudioChannel3Control;
            result->Bytes.SoundLength = gbState->AudioChannel3Length;
            result->Bytes.Envelope = gbState->AudioChannel3Level;
            result->Bytes.Frequency1 = gbState->AudioChannel3FrequencyLow;
            result->Bytes.FrequencyAndTiming = gbState->AudioChannel3Flags;

            memcpy(result->WaveData, gbState->SoundWaveData, sizeof(gbState->SoundWaveData));
            break;
        case 4:
            result->Bytes.SweepRegister = 0;
            result->Bytes.SoundLength = gbState->AudioChannel4Length;
            result->Bytes.Envelope = 0;
            result->Bytes.Frequency1 = gbState->AudioChannel4RNGParameters;
            result->Bytes.FrequencyAndTiming = gbState->AudioChannel4Flags;
            break;
        default: return;
    }
    */
}
