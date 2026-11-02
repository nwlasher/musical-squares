/* Source for the tone generator */

#include "synth.hpp"
#include <Arduino.h>
#include <Arduino_AdvancedAnalog.h>
#include <cstdint>
#include <math.h>

// Assign pins to dac object
AdvancedDAC dac0(A12);
//AdvancedDAC dac1(A13); Hardware will use one signal to both speakers

int16_t SINE_TABLE[TABLE_SIZE]; // One cyle centered on zero

// List of tracks
Track TRACKS[NUM_TRACKS];

// Calculates the `step` field value for a given frequency
float stepFor(float hz){return hz * TABLE_SIZE / SAMPLE_RATE;}

// Move every gliding track a little closer to its target. Runs once per buffer.
void advanceGlides(){
    for (uint8_t i = 0; i < NUM_TRACKS; i++){
        Track &t = TRACKS[i];
        if (t.hz_per_buffer == 0.0f) continue; // if target has been hit
        t.hz += t.hz_per_buffer; // Increment frequency
        // Check if have distance to travel, if not -> arrived!
        bool arrived = (t.hz_per_buffer > 0.0f) ? (t.hz >= t.target_hz) : (t.hz <= t.target_hz);
        if (arrived){
            t.hz = t.target_hz; // Match target
            t.hz_per_buffer = 0.0f; // clear increment
        }
        t.step = stepFor(t.hz);
    }
}

// Initialize and start the DACs. Returns false on failed start.
bool synthInit(){
    for (int i = 0; i < TABLE_SIZE; i++)
        SINE_TABLE[i] = (int16_t)(AMPLITUDE * sinf(2.0f * PI * i / TABLE_SIZE));
    for (uint8_t t = 0; t < NUM_TRACKS; t++) TRACKS[t] = {false, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    // resolution, sample rate, samples per buffer, queue depth
    return dac0.begin(AN_RESOLUTION_12, SAMPLE_RATE, BUF_SAMPLES, BUF_COUNT);
}

// Refills audio buffer for DMA chip to send to DAC
void synthUpdate(){
    if (!dac0.available()) return;       // Hardware still busy with queued audio
    SampleBuffer buf = dac0.dequeue();   // Grab a free buffer
    advanceGlides();
    for (size_t i = 0; i < buf.size(); i++){
        int32_t sum = 0;
        for (uint8_t t = 0; t < NUM_TRACKS; t++){
            if (!TRACKS[t].active) continue;
            sum += SINE_TABLE[(int)TRACKS[t].phase];
            TRACKS[t].phase += TRACKS[t].step;
            if (TRACKS[t].phase >= TABLE_SIZE) TRACKS[t].phase -= TABLE_SIZE;
        }
        // Fixed divisor keeps loudness steady as notes start and stop
        buf[i] = (uint16_t)(2048 + sum / NUM_TRACKS);
    }
    dac0.write(buf);                     // Queue it for playback
}

// Clamps the applied frequency between MIN and MAX FREQ
float clampFreq(float hz){
    if(hz > MAX_FREQ) {
        return MAX_FREQ;
    }else if (hz < MIN_FREQ){
        return MIN_FREQ;
    }else{
        return hz;
    }
}

// Start a track playing an arbitrary frequency in Hz
void synthPlayHz(uint8_t track, float hz){
    hz = clampFreq(hz);
    if (track >= NUM_TRACKS) return;
    Track &t = TRACKS[track];
    t.hz = t.target_hz = hz;
    t.hz_per_buffer = 0.0f;                // Cancels any glide in progress
    t.step = stepFor(hz);
    t.active = true;
}

// Start a track playing a descrete tone
void synthPlay(uint8_t track, Tone tone){
    if (tone >= NUM_TONES) return;
    synthPlayHz(track, TONE_HZ[tone]);
}

// Slide a playing track to a new note/frequency over a duration in ms
void synthGlideToHz(uint8_t track, float hz, uint32_t ms){
    hz = clampFreq(hz);
    if (track >= NUM_TRACKS) return;
    Track &t = TRACKS[track];
    if (!t.active || ms == 0){ synthPlayHz(track, hz); return; }  // Nothing to glide from
    float buffers = ms / BUFFER_MS;
    if (buffers < 1.0f) buffers = 1.0f;
    t.target_hz = hz;
    t.hz_per_buffer = (hz - t.hz) / buffers;
}
 
void synthGlideTo(uint8_t track, Tone tone, uint32_t ms){
    if (tone >= NUM_TONES) return;
    synthGlideToHz(track, TONE_HZ[tone], ms);
}

// Silence a track
void synthStop(uint8_t track){
    if (track >= NUM_TRACKS) return;
    TRACKS[track].active = false;
    TRACKS[track].hz_per_buffer = 0.0f;
}

