#pragma once

typedef enum
{
    midiFeatureSection,
    midiChannelSection,
    MIDI_SECTIONS
} midiSection;

typedef enum
{
    noteChannel,
    programChangeChannel,
    CCchannel,
    inputChannel,
    MIDI_CHANNELS
} midiChannels;

typedef enum
{
    midiFeatureStandardNoteOff,
    midiFeatureRunningStatus,
    midiFeatureUSBconvert,
    MIDI_FEATURES
} midiFeatures;
