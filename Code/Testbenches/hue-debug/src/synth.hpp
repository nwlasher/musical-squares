/* Tone generator for the music box. Plays up to NUM_TRACKS notes at once,
mixed together and sent out DAC0 (A12). Call chooses notes; this module
owns everything about how sound is made. */

#pragma once
#include "TCS3472x.hpp"
#include <cstddef>
#include <cstdint>
#include <stdint.h>

constexpr uint8_t NUM_TRACKS = NUM_SENSORS; // One track per color sensor

// Enumeration for the playable tones
enum Tone : uint8_t {
    TONE_C4, // Frequencies are defined in synth.cpp
    TONE_D4,
    TONE_E4,
    TONE_F4,
    TONE_G4,
    TONE_A4,
    TONE_B4,
    TONE_C5,
    NUM_TONES
};

// Frequency of each Tone, in order
const float TONE_HZ[NUM_TONES] = {
    261.63f, 293.66f, 329.63f, 349.23f, 392.00f, 440.00f, 493.88f, 523.25f
};

// String names associated with Tones
const char* const TONE_NAMES[NUM_TONES] = {"C4", "D4", "E4", "F4", "G4", "A4", "B4", "C5"};

struct Track {
    bool active;
    float phase; // Position in sine table, 0 up to TABLE_SIZE
    float step; // Table entries to advance per output sample
    float hz; // Current frequency 
    float target_hz; // Glide destination (same as hz when not gliding)
    float hz_per_buffer; // Frequency change applied each buffer while gliding
    //          ^^ (0 when not gliding)
};

const float MIN_FREQ = 200.0f; // Frequency range for my shitty mazon speakers
const float MAX_FREQ = 20000.0f;
const uint32_t SAMPLE_RATE = 32000; // Output samples per second
const int TABLE_SIZE = 256; // Entries in the sine cycle table
const int AMPLITUDE = 100; // Peak table value (amplitude of sine cycle)
const size_t BUF_SAMPLES = 128; // Samples per DMA buffer
const size_t BUF_COUNT = 16; // Number of buffers queued
const float BUFFER_MS = 1000.0f * BUF_SAMPLES / SAMPLE_RATE; // Duration of one buffer

extern Track TRACKS[NUM_TRACKS]; // List of tracks

// Main interface

// Initialize and start the DACs. Returns false on failed start.
bool synthInit();
// Refills audio buffer for DMA chip to send to DAC
void synthUpdate();
// Start a track playing a descrete tone
void synthPlay(uint8_t track, Tone tone);
// Start a track playing an arbitrary frequency in Hz
void synthPlayHz(uint8_t track, float hz);
// Slide a playing track to a new note/frequency over a duration in ms
void synthGlideTo(uint8_t track, Tone tone, uint32_t ms);
void synthGlideToHz(uint8_t track, float hz, uint32_t ms);
// Silence a track
void synthStop(uint8_t track);
