#ifndef MIDISETTINGS_H_
#define MIDISETTINGS_H_

//maximum sysex length we can receive
#define MIDI_SYSEX_ARRAY_SIZE   80

//safety masks
#define MAX_MIDI_VALUE_MASK     0x7F
#define MAX_MIDI_CHANNEL_MASK   0x0F

#define normalizeChannel(channel) (((channel - 1) & MAX_MIDI_CHANNEL_MASK))
#define normalizeData(data) (data & MAX_MIDI_VALUE_MASK)

typedef enum {

    midiFeatureConf,
    midiChannelConf,
    MIDI_SUBTYPES

} sysExMessageSubtypeMIDI;

typedef enum {

    noteChannel,
    programChangeChannel,
    CCchannel,
    inputChannel,
    MIDI_CHANNELS

} midiChannels;

typedef enum {

    midiFeatureStandardNoteOff,
    midiFeatureRunningStatus,
    midiFeatureUSBconvert,
    MIDI_FEATURES

} midiFeatures;

#endif