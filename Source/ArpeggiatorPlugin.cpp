#include "DistrhoPlugin.hpp"
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <chrono>

START_NAMESPACE_DISTRHO

class ArpeggiatorPlugin : public Plugin
{
public:
    ArpeggiatorPlugin()
        : Plugin(5, 0, 0), speed(120.0f), speedMultiplier(1.0f), pattern(0), octaveRange(1), noteIndex(0), repeatCount(0), stop(false)
    {
        lastTimePoint = std::chrono::steady_clock::now();
        ascending = true;
    }

protected:
    const char* getLabel() const override { return "Arpeggiator"; }
    const char* getDescription() const override { return "An arpeggiator plugin with customizable speed, pattern, octave range, and a stop button."; }
    const char* getMaker() const override { return "NirBlu"; }
    const char* getHomePage() const override { return "https://github.com/NirBlu/ArpeggiatorPlugin.git"; }
    const char* getLicense() const override { return "ISC"; }
    uint32_t getVersion() const override { return d_version(1, 0, 0); }
    int64_t getUniqueId() const override { return 0x12345678; }

    void initParameter(uint32_t index, Parameter& parameter) override {
        switch (index) {
            case 0: // Speed
                parameter.hints = kParameterIsAutomatable | kParameterIsInteger;
                parameter.name = "Speed";
                parameter.symbol = "speed";
                parameter.ranges.def = 120.0f;
                parameter.ranges.min = 30.0f;
                parameter.ranges.max = 693.0f;
                break;
            case 1: // Speed Multiplier
                parameter.hints = kParameterIsAutomatable;
                parameter.name = "Speed Multiplier";
                parameter.symbol = "speed_multiplier";
                parameter.ranges.def = 1.0f;
                parameter.ranges.min = 0.1f;
                parameter.ranges.max = 4.0f;
                break;
            case 2: // Pattern
                parameter.hints = kParameterIsAutomatable | kParameterIsInteger;
                parameter.name = "Pattern";
                parameter.symbol = "pattern";
                parameter.ranges.def = 0;
                parameter.ranges.min = 0;
                parameter.ranges.max = 6;
                break;
            case 3: // Octave range
                parameter.hints = kParameterIsAutomatable | kParameterIsInteger;
                parameter.name = "Octave Range";
                parameter.symbol = "octave_range";
                parameter.ranges.def = 1;
                parameter.ranges.min = 1;
                parameter.ranges.max = 3;
                break;
            case 4: // Stop button
                parameter.hints = kParameterIsBoolean;
                parameter.name = "Stop";
                parameter.symbol = "stop";
                parameter.ranges.def = 0;
                parameter.ranges.min = 0;
                parameter.ranges.max = 1;
                break;
        }
    }

    float getParameterValue(uint32_t index) const override {
        switch (index) {
            case 0: return speed;
            case 1: return speedMultiplier;
            case 2: return pattern;
            case 3: return octaveRange;
            case 4: return stop ? 1.0f : 0.0f;
        }
        return 0.0f;
    }

    void setParameterValue(uint32_t index, float value) override {
        switch (index) {
            case 0: speed = value; break;
            case 1: speedMultiplier = value; break;
            case 2: pattern = static_cast<int>(value); break;
            case 3: octaveRange = static_cast<int>(value); break;
            case 4: stop = (value > 0.5f); break;
        }
    }

    void run(const float**, float**, uint32_t,
             const MidiEvent* midiEvents, uint32_t midiEventCount) override
    {
        if (stop) {
            clearAllNotes();
            stop = false;  // Reset stop after clearing
            return;
        }

        // Process incoming MIDI events
        for (uint32_t i = 0; i < midiEventCount; ++i) {
            const MidiEvent& event = midiEvents[i];
            uint8_t status = event.data[0] & 0xF0;
            uint8_t note = event.data[1];

            if (status == 0x90 && event.data[2] > 0) { // Note-On
                if (std::find(chordNotes.begin(), chordNotes.end(), note) == chordNotes.end()) {
                    chordNotes.push_back(note);  // Add note if not already present
                }
                activeNotes.insert(note); // Track active notes
            } else if (status == 0x80 || (status == 0x90 && event.data[2] == 0)) { // Note-Off
                activeNotes.erase(note); // Remove from active notes
                chordNotes.erase(std::remove(chordNotes.begin(), chordNotes.end(), note), chordNotes.end());
                sendNoteOff(note);
            }
        }

        // Timing control based on speed and multiplier
        auto now = std::chrono::steady_clock::now();
        double msPerBeat = (60000.0 / (speed * speedMultiplier));
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTimePoint).count() >= msPerBeat) {
            lastTimePoint = now;

            // Arpeggiate if there are active notes in chordNotes
            if (!chordNotes.empty()) {
                uint8_t noteToPlay = getNextNote();
                if (activeNotes.count(noteToPlay) > 0) { // Play only if note is active
                    for (int octave = 0; octave < octaveRange; ++octave) {
                        sendNoteOn(noteToPlay + 12 * octave);
                    }
                }
            }
        }
    }

private:
    float speed;
    float speedMultiplier;
    int pattern;
    int octaveRange;
    bool stop;
    std::vector<uint8_t> chordNotes;
    std::unordered_set<uint8_t> activeNotes; // Track only held notes
    uint32_t noteIndex;
    int repeatCount;
    bool ascending;
    std::chrono::steady_clock::time_point lastTimePoint;

    void clearAllNotes() {
        for (auto note : chordNotes) {
            sendNoteOff(note);
        }
        chordNotes.clear();
        activeNotes.clear();
        noteIndex = 0;
        repeatCount = 0;
        ascending = true;
    }

    void sendNoteOn(uint8_t note) {
        MidiEvent event;
        event.frame = 0;
        event.size = 3;
        event.data[0] = 0x90;
        event.data[1] = note;
        event.data[2] = 100;
        writeMidiEvent(event);
    }

    void sendNoteOff(uint8_t note) {
        MidiEvent event;
        event.frame = 0;
        event.size = 3;
        event.data[0] = 0x80;
        event.data[1] = note;
        event.data[2] = 0;
        writeMidiEvent(event);
    }

    uint8_t getNextNote() {
        if (chordNotes.empty()) return 0;

        switch (pattern) {
            case 0: // Ascending pattern
                noteIndex = (noteIndex + 1) % chordNotes.size();
                break;
            case 1: // Descending pattern
                noteIndex = (noteIndex == 0) ? chordNotes.size() - 1 : noteIndex - 1;
                break;
            case 2: // Ascend-Descend pattern
                if (ascending) {
                    if (noteIndex < chordNotes.size() - 1) ++noteIndex;
                    else { ascending = false; --noteIndex; }
                } else {
                    if (noteIndex > 0) --noteIndex;
                    else { ascending = true; ++noteIndex; }
                }
                break;
            case 3: // Descend-Ascend pattern
                if (!ascending) {
                    if (noteIndex > 0) --noteIndex;
                    else { ascending = true; ++noteIndex; }
                } else {
                    if (noteIndex < chordNotes.size() - 1) ++noteIndex;
                    else { ascending = false; --noteIndex; }
                }
                break;
            case 4: // Repeat Twice pattern
                if (repeatCount < 1) repeatCount++;
                else { repeatCount = 0; noteIndex = (noteIndex + 1) % chordNotes.size(); }
                break;
            case 5: // Repeat Thrice pattern
                if (repeatCount < 2) repeatCount++;
                else { repeatCount = 0; noteIndex = (noteIndex + 1) % chordNotes.size(); }
                break;
            case 6: // Played Order pattern (ascending)
                noteIndex = (noteIndex + 1) % chordNotes.size();
                break;
            default: // Default to ascending if undefined
                noteIndex = (noteIndex + 1) % chordNotes.size();
                break;
        }
        return chordNotes[noteIndex];
    }

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArpeggiatorPlugin)
};

/* ------------------------------------------------------------------------------------------------------------
 * Plugin entry point, called by DPF to create a new plugin instance. */

Plugin* createPlugin()
{
    return new ArpeggiatorPlugin();
}

// -----------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO

