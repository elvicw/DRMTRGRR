Act as a senior JUCE UI/UX designer and audio plugin developer.

Review the existing DRMTRGRR repository and propose a UI/UX modernization plan.

PROJECT:
DRMTRGRR is a JUCE-based VST3/AU drum replacement and transient triggering plugin.

IMPORTANT:
Preserve all existing DSP functionality, parameter IDs, automation compatibility, preset compatibility, and plugin state serialization. Do not rewrite working DSP simply to accommodate visual changes.

DESIGN OBJECTIVES:

1. VISUAL HIERARCHY
Organize the interface into three primary sections:
- Transient detection and waveform visualization.
- Six sample slots arranged in a 3x2 grid.
- Output, gating, bleed suppression, and MIDI controls.

2. TRANSIENT VISUALIZATION
Improve the waveform display with:
- Accepted trigger markers.
- Rejected trigger markers.
- Velocity visualization.
- Draggable threshold line.
- Waveform zoom.
- Freeze display.
- Clear trigger activity indicators.

3. SAMPLE SLOT REDESIGN
Create compact sample cards with:
- Sample name.
- Mini waveform.
- Volume.
- Pan.
- Tune.
- Phase inversion.
- Solo and mute.
- Attack and decay.
- Velocity range.

Provide an expandable advanced settings view for each slot.

4. WORKFLOW IMPROVEMENTS
Implement:
- Independent sample audition.
- Manual trigger test.
- Detection signal audition.
- Trigger statistics.
- Drag-and-drop sample loading.
- Slot duplication and reordering.

5. VISUAL DESIGN
Maintain the dark brushed-slate aesthetic.
Use restrained cyan accents, amber threshold controls, and clear accepted/rejected trigger indicators.
Avoid excessive glow and unnecessary animations.

6. USABILITY
Implement:
- Resizable plugin interface.
- Multiple UI scaling levels.
- Double-click parameter reset.
- Mouse-wheel parameter adjustment.
- Fine adjustment using modifier keys.
- Contextual tooltips.
- Consistent control sizing.

7. ENGINEERING REQUIREMENTS
- Keep DSP processing independent from GUI code.
- Never perform blocking file operations in the audio thread.
- Keep waveform visualization communication thread-safe.
- Avoid unnecessary repainting.
- Maintain low GUI CPU usage.
- Preserve existing automation and preset behavior.
- Maintain cross-platform compatibility.

DEVELOPMENT APPROACH:
First inspect the existing implementation and identify which components can be improved without changing DSP behavior.

Present:
1. Current UI architecture.
2. Proposed layout.
3. Required component changes.
4. Potential regression risks.
5. Incremental implementation plan.

Do not immediately rewrite the entire interface. Implement improvements in small, independently testable stages.