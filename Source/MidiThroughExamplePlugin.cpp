#include "DistrhoPlugin.hpp"
#include <vector>
#include <algorithm>
#include <chrono>

START_NAMESPACE_DISTRHO

class ArpeggiatorPlugin : public Plugin
{
public:
    ArpeggiatorPlugin()
        : Plugin(4, 0, 0), speed(120.0f), pattern(0), octaveRange(1), noteIndex(0), repeatCount(0), stop(false)
    {
        lastTimePoint = std::chrono::steady_clock::now();
        ascending = true;  // For ascend-descend patterns
    }

protected:
    const char* getLabel() const override { return "Arpeggiator"; }
    const char* getDescription() const override { return "An arpeggiator plugin with customizable speed, pattern, octave range, and a stop button."; }
    const char* getMaker() const override { return "NirBlu"; }
    const char* getHomePage() const override { return "https://github.com/YourRepo"; }
    const char* getLicense() const override { return "ISC"; }
    uint32_t getVersion() const override { return d_version(1, 0, 0); }
    int64_t getUniqueId() const override { return 0x12345678; }

    void initParameter(uint32_t index, Parameter& parameter) override {
        switch (index) {
            case 0: // Speed
                parameter.hints = kParameterIsAutomatable;
                parameter.name = "Speed";
                parameter.symbol = "speed";
                parameter.ranges.def = 120.0f;
                parameter.ranges.min = 30.0f;
                parameter.ranges.max = 693.0f;
                break;
            case 1: // Pattern
                parameter.hints = kParameterIsAutomatable | kParameterIsInteger;
                parameter.name = "Pattern";
                parameter.symbol = "pattern";
                parameter.ranges.def = 0;
                parameter.ranges.min = 0;
                parameter.ranges.max = 5;  // Patterns: 0 = up, 1 = down, 2 = repeat-twice, 3 = ascend-descend, 4 = descend-ascend, 5 = repeat-thrice
                break;
            case 2: // Octave range
                parameter.hints = kParameterIsAutomatable | kParameterIsInteger;
                parameter.name = "Octave Range";
                parameter.symbol = "octave_range";
                parameter.ranges.def = 1;
                parameter.ranges.min = 1;
                parameter.ranges.max = 3;
                break;
            case 3: // Stop button
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
            case 1: return pattern;
            case 2: return octaveRange;
            case 3: return stop ? 1.0f : 0.0f;
        }
        return 0.0f;
    }

    void setParameterValue(uint32_t index, float value) override {
        switch (index) {
            case 0: speed = value; break;
            case 1: pattern = static_cast<int>(value); break;
            case 2: octaveRange = static_cast<int>(value); break;
            case 3: stop = (value > 0.5f); break;
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
                chordNotes.push_back(note);
            } else if (status == 0x80 || (status == 0x90 && event.data[2] == 0)) { // Note-Off
                chordNotes.erase(std::remove(chordNotes.begin(), chordNotes.end(), note), chordNotes.end());
                sendNoteOff(note);
            }
        }

        // Timing control based on speed
        auto now = std::chrono::steady_clock::now();
        double msPerBeat = 60000.0 / speed;
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTimePoint).count() >= msPerBeat) {
            lastTimePoint = now;

            // Arpeggiate if there are notes in chordNotes
            if (!chordNotes.empty()) {
                uint8_t noteToPlay = getNextNote();
                sendNoteOn(noteToPlay);
            }
        }
    }

private:
    float speed;
    int pattern;
    int octaveRange;
    bool stop;
    std::vector<uint8_t> chordNotes;
    uint32_t noteIndex;
    int repeatCount;  // For repeat-twice and repeat-thrice patterns
    bool ascending;   // For ascend-descend and descend-ascend patterns
    std::chrono::steady_clock::time_point lastTimePoint;

    void clearAllNotes() {
        for (auto note : chordNotes) {
            sendNoteOff(note);
        }
        chordNotes.clear();
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
        switch (pattern) {
            case 1: // Down pattern
                noteIndex = (noteIndex == 0) ? chordNotes.size() - 1 : noteIndex - 1;
                break;
            case 2: // Repeat Twice pattern
                if (repeatCount < 1) {
                    repeatCount++;
                } else {
                    repeatCount = 0;
                    noteIndex = (noteIndex + 1) % chordNotes.size();
                }
                break;
            case 3: // Ascend-Descend pattern
                if (ascending) {
                    if (noteIndex < chordNotes.size() - 1) {
                        ++noteIndex;
                    } else {
                        ascending = false;
                        --noteIndex;
                    }
                } else {
                    if (noteIndex > 0) {
                        --noteIndex;
                    } else {
                        ascending = true;
                        ++noteIndex;
                    }
                }
                break;
            case 4: // Descend-Ascend pattern
                if (!ascending) {
                    if (noteIndex > 0) {
                        --noteIndex;
                    } else {
                        ascending = true;
                        ++noteIndex;
                    }
                } else {
                    if (noteIndex < chordNotes.size() - 1) {
                        ++noteIndex;
                    } else {
                        ascending = false;
                        --noteIndex;
                    }
                }
                break;
            case 5: // Repeat Thrice pattern
                if (repeatCount < 2) {
                    repeatCount++;
                } else {
                    repeatCount = 0;
                    noteIndex = (noteIndex + 1) % chordNotes.size();
                }
                break;
            default: // Up pattern (default)
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

