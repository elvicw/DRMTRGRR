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




