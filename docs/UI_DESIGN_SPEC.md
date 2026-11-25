
# DRMTRGRR — UI Recreation & Development Specification

## 1. Objective

Recreate the supplied DRMTRGRR UI reference image as closely as technically possible.

This is an exact visual recreation task, not an opportunity to reinterpret, modernize, simplify, or redesign the reference.

The reference image is the primary visual authority.

The objective is to make the working plugin interface look as close to the supplied image as possible while preserving all existing DRMTRGRR functionality.

## 2. Reference Materials

### Primary Visual Reference

`references/DRMTRGRR-facelift-reference.png`

This image defines the intended:
- Overall interface layout.
- Panel proportions and positioning.
- Control placement and dimensions.
- Typography and hierarchy.
- Color palette.
- Knob and switch appearance.
- Meter and waveform presentation.
- Background, borders, textures, and hardware details.

The agent MUST locate and visually inspect the actual image before making UI changes.

Do not rely exclusively on this Markdown description.

### Development Reference

Use the following repository as a reference for UI development practices:

https://github.com/elvicw/DRM-SMRTVRB

In particular, study:

- `UI_DESIGN_MAKEOVER.md`
- `UI_SPEC.md`
- `AGENT_HANDOVER.md`
- `ARCHITECTURE.md`
- `BUILD_INSTRUCTIONS.md`
- `TESTING.md`

The most important reference is:

`UI_DESIGN_MAKEOVER.md`

It establishes the UI development workflow, visual inspection requirements, implementation discipline, functional preservation requirements, and completion criteria.

Apply those engineering practices to DRMTRGRR.

IMPORTANT:

The DRM-SMRTVRB repository is a development-process reference, NOT a source of DRMTRGRR layout specifications.

Do not copy its interface layout or its plugin-specific parameter definitions.

DRMTRGRR must follow its own supplied visual reference and existing implementation.

## 3. Visual Reference Priority

When making visual decisions, follow this priority:

1. Actual DRMTRGRR reference image.
2. This document's exact-recreation requirements.
3. Existing DRMTRGRR functionality and architecture.
4. Technical limitations of the existing UI framework.

If a written description differs from the reference image, follow the image unless doing so would break existing functionality.

If something cannot be reproduced exactly, implement the closest practical equivalent and document the difference.

Do not silently substitute your own design.

## 4. Mandatory Reference Inspection

Before modifying code:

1. Locate the reference image.
2. Open and inspect the complete image.
3. Examine its overall proportions and layout.
4. Examine every major panel and its positioning.
5. Examine knob dimensions, appearance, and indicators.
6. Examine switches, buttons, and their labels.
7. Examine the sample rack and its channel strips.
8. Examine the oscilloscope and meter presentation.
9. Examine colors, typography, borders, textures, and spacing.
10. Keep the image available as a visual reference throughout implementation.

Do not begin implementation based solely on this document.

## 5. Exact Layout Recreation

Reproduce the reference image's layout as closely as possible.

### Header
- Branding and plugin identity.
- Preset browser.
- Input, Mix, and Output controls.
- Existing meters and indicators.

### Upper Section
- Trigger Detection.
- Oscilloscope / waveform display.
- Threshold and hit indicators.
- Metering and status indicators.

### Middle Section
Reproduce the four-module arrangement:

- Transient Engine
- Bandpass Filter
- Transient Gate
- Bleed & MIDI

### Bottom Section
Reproduce the six-channel Sample Rack:

- KICK
- SNARE
- RIM
- ROOM
- WIRES
- AUX

Preserve the reference's relative proportions, spacing, alignment, and visual hierarchy.

Do not rearrange sections simply because another layout seems more convenient.

## 6. Visual Implementation Requirements

### Chassis and Panels
- Dark brushed-metal hardware appearance.
- Charcoal and gunmetal surfaces.
- Subtle panel divisions and borders.
- Restrained hardware details.
- Warm cream typography.
- Muted amber illumination.

### Rotary Controls
- Reproduce the reference's knob appearance.
- Use clear position indicators.
- Keep knob proportions consistent.
- Use subtle tactile shading and highlights.
- Ensure values remain readable.
- Do not substitute sliders for knobs where the reference shows rotary controls.

### Switches and Buttons
- Reproduce the physical-looking switches and buttons.
- Keep labels integrated into or clearly associated with their controls.
- Make ON/OFF states immediately recognizable.
- Do not substitute generic checkboxes for the reference's switches.

### Displays and Meters
- Reproduce the oscilloscope and waveform appearance.
- Preserve the existing waveform interactions.
- Reproduce the meter styling and indicators.
- Do not invent visualizations or fake signal behavior.

### Typography
- Reproduce the reference's text hierarchy.
- Use compact, readable labels.
- Maintain consistent font sizes and alignment.
- Do not introduce decorative fonts that compromise readability.

### Spacing and Proportions
Pay particular attention to:
- Panel dimensions.
- Relative control sizes.
- Margins and padding.
- Alignment.
- Section spacing.
- Border thickness.
- Label positioning.

Do not add unnecessary whitespace or decorative elements.

## 7. Functional Preservation

The existing DRMTRGRR implementation is the source of truth for functionality.

Before implementing any visual control, inspect the existing code and identify its actual parameter and interaction.

Preserve all existing:

- DSP processing.
- Parameter IDs.
- Parameter names and ranges.
- Parameter defaults and scaling.
- Host automation.
- Preset behavior.
- Plugin state serialization and restoration.
- Sample loading and sample management.
- Waveform interactions.
- MIDI functionality.
- Routing.
- Host integration.

Do not remove, replace, or invent functionality to make the reference image easier to reproduce.

Do not introduce placeholder controls.

Do not change DSP behavior or parameter semantics as part of this UI task.

Reuse existing parameter attachments and bindings wherever possible.

If a visual requirement appears to require changes to DSP or parameter architecture, stop and report the issue before proceeding.

## 8. Development Workflow

Follow the same disciplined UI development workflow established in DRM-SMRTVRB.

### Before Implementation

1. Read the target repository's `AGENTS.md`.
2. Read relevant project documentation.
3. Inspect the supplied visual reference.
4. Inspect the current DRMTRGRR UI implementation.
5. Identify the UI framework and rendering architecture.
6. Identify all existing parameters and their bindings.
7. Identify preset, automation, and state pathways.
8. Identify files expected to change.
9. Prepare a short implementation plan.

Do not modify code before understanding the existing implementation.

### Branching

Work on a dedicated feature branch.

Suggested branch:

`feature/drmtrgrr-ui-recreation`

Do not implement this directly on the release branch.

### Implementation

- Work in small, reviewable steps.
- Reuse existing components and parameter bindings where practical.
- Keep DSP and parameter-definition files untouched.
- Avoid unrelated refactoring.
- Do not introduce unnecessary dependencies.
- Build and test after meaningful changes.

### Visual Refinement

This is a mandatory part of the task.

After implementing the initial UI:

1. Build the plugin.
2. Launch it in the target DAW or standalone application.
3. Capture an actual screenshot.
4. Compare the screenshot directly against the reference image.
5. Identify differences in proportions, positioning, controls, colors, and typography.
6. Correct those differences.
7. Capture another screenshot.
8. Repeat until the closest practical visual match is achieved.

A successful build does NOT mean the UI recreation is complete.

Do not stop after the first implementation.

## 9. Testing and Compatibility

Verify that:

- The plugin builds successfully.
- The plugin loads in REAPER.
- All existing controls operate correctly.
- Parameter changes update the actual plugin parameters.
- Host automation remains functional.
- Presets continue to work.
- Plugin state saves and restores correctly.
- Sample loading works.
- Waveform interactions work.
- MIDI functionality remains intact.
- Resizing does not cause overlapping or clipped controls.
- UI interaction does not introduce unnecessary audio-thread work.

Do not claim tests passed unless they were actually performed.

## 10. Final Acceptance Criteria

The task is complete only when:

- The interface closely matches the supplied reference image.
- Major sections occupy the same relative positions.
- Knobs, switches, buttons, meters, and displays resemble the reference.
- Colors, typography, proportions, and spacing are consistent.
- All original DRMTRGRR functionality is preserved.
- The plugin builds and loads successfully.
- The actual rendered interface has been visually compared against the reference.
- At least one refinement pass has been completed after the initial implementation.

## 11. Completion Report

When finished, provide:

1. Summary of implemented UI changes.
2. Files modified.
3. Build and test commands executed.
4. Host and plugin formats tested.
5. Confirmation that DSP and parameter behavior were preserved.
6. Any known limitations.
7. Screenshot of the final interface.
8. Remaining differences between the final UI and the reference image.

Do not claim exact visual fidelity if significant differences remain.

## 12. Final Design Principle

The supplied DRMTRGRR reference image is the design.

Do not reinterpret it.

Do not replace it with your own idea of what a vintage plugin should look like.

The task is to turn the supplied visual reference into a working DRMTRGRR interface while maintaining the existing plugin's functionality and engineering standards.

**Recreate the reference. Preserve the instrument. Verify the result.**



## DO NOT READ THROUGH AFTER THIS LINE. THIS HAS ALREADY BEEN ACHIEVED. THE ABSOLUTE INSTRUCTIONS ARE WRITTEN ABOVE THIS



# UI DESIGN DIRECTION: VINTAGE STUDIO HARDWARE

## Primary Design Goal

The initial visual direction for DRMTRGRR should resemble a piece of vintage professional studio hardware rather than a modern software plugin.

The interface should feel like something you would find in a recording studio rack from the 1970s, 1980s, or early 1990s.

Two possible visual inspirations:

1. **Vintage FET Compressor / CLA-76-inspired aesthetic**
   - Industrial, compact, and purposeful.
   - Dark or brushed-metal faceplate.
   - Warm cream, grey, or black labeling.
   - Mechanical-looking knobs and switches.
   - Vintage VU-meter styling.
   - Amber or warm-yellow indicator lights.
   - Strong, recognizable hardware character.

2. **Vintage SSL Console-inspired aesthetic**
   - Professional large-format mixing console appearance.
   - Muted grey, charcoal, and subtle blue/green accents.
   - Clearly separated control sections.
   - Dense but organized control layout.
   - Traditional console-style knobs, buttons, switches, and meters.
   - Functional labeling and a technical, professional appearance.

Choose ONE direction as the primary visual identity. Do not combine both styles indiscriminately.

## Visual Principles

- Prioritize the appearance of physical studio equipment.
- Use tactile-looking knobs, switches, buttons, meters, and engraved-style labels.
- Use subtle gradients, highlights, shadows, and material textures to suggest metal, plastic, and hardware components.
- Avoid excessive glow, neon colors, futuristic styling, and generic modern plugin aesthetics.
- Avoid oversized rounded cards and excessive empty space.
- Use typography that feels appropriate for professional audio hardware.
- Maintain strong readability and clear control hierarchy.
- Use restrained colors for status indicators.
- Make the interface look professionally manufactured rather than artificially distressed.

## Functional Design Requirements

The vintage aesthetic must not compromise usability.

- Keep the transient detection waveform prominent.
- Make threshold and detection controls easy to locate.
- Clearly distinguish detection, sample layers, and output/routing sections.
- Preserve clear visual feedback for accepted and rejected triggers.
- Maintain readable meters and trigger indicators.
- Make sample slots easy to identify and operate.
- Use consistent knob behavior and labeling.
- Support window resizing and sensible UI scaling.
- Provide tooltips for less obvious controls.

## Implementation Guidelines

Before implementing visual changes:

1. Inspect the existing JUCE interface and component architecture.
2. Identify reusable components for knobs, switches, buttons, meters, and labels.
3. Establish a consistent design system before redesigning individual sections.
4. Implement the visual changes incrementally.
5. Keep DSP processing completely independent of UI changes.
6. Preserve existing parameter IDs, automation, preset compatibility, and state serialization.
7. Avoid unnecessary audio-thread communication or expensive GUI repaint operations.

Do not rewrite working DSP code to accommodate visual changes.

## Originality and Branding

The CLA-76 and SSL references are visual inspiration only.

Do not reproduce their exact faceplates, logos, trademarks, proprietary artwork, or distinctive commercial interface layouts.

DRMTRGRR must retain its own recognizable identity.

The final result should look like a believable vintage studio instrument designed and manufactured specifically for DRMTRGRR.

## Design Evaluation

Evaluate the interface against these questions:

- Does it look like physical studio equipment?
- Can the user understand the signal flow immediately?
- Can the user configure a basic kick or snare trigger within 15 seconds?
- Are the most important controls immediately accessible?
- Does the interface remain readable at different window sizes?
- Does the vintage appearance enhance the experience rather than obstruct it?




