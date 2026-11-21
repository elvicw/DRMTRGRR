#include "TriggerVisualizer.h"
#include "SlateLookAndFeel.h"
#include <cmath>

namespace yeahwaah
{

TriggerVisualizer::TriggerVisualizer(VisualizerBuffer& vb,
                                     juce::RangedAudioParameter* dp,
                                     juce::RangedAudioParameter* lcp,
                                     juce::RangedAudioParameter* hcp,
                                     juce::RangedAudioParameter* rp)
    : visualizer(vb),
      detailParam(dp),
      lowCutParam(lcp),
      highCutParam(hcp),
      retriggerParam(rp)
{
    setOpaque(true);

    if (detailParam != nullptr)
        detailDb = detailParam->convertFrom0to1(detailParam->getValue());
    if (lowCutParam != nullptr)
        lowCutHz = lowCutParam->convertFrom0to1(lowCutParam->getValue());
    if (highCutParam != nullptr)
        highCutHz = highCutParam->convertFrom0to1(highCutParam->getValue());
    if (retriggerParam != nullptr)
        retriggerMs = retriggerParam->convertFrom0to1(retriggerParam->getValue());
}

float TriggerVisualizer::dbToY(float dB, float height) const
{
    float norm = (dB + 60.0f) / 60.0f;
    norm = juce::jlimit(0.0f, 1.0f, norm);
    return height - (norm * (height - 24.0f) + 12.0f);
}

float TriggerVisualizer::yToDb(float y, float height) const
{
    float norm = (height - 12.0f - y) / (height - 24.0f);
    norm = juce::jlimit(0.0f, 1.0f, norm);
    return -60.0f + norm * 60.0f;
}

// Logarithmic frequency mapping across display: 20 Hz to 20,000 Hz
float TriggerVisualizer::hzToX(float hz, float startX, float endX) const
{
    const float minHz = 20.0f;
    const float maxHz = 20000.0f;
    float clampedHz = juce::jlimit(minHz, maxHz, hz);
    float norm = std::log(clampedHz / minHz) / std::log(maxHz / minHz);
    return startX + norm * (endX - startX);
}

float TriggerVisualizer::xToHz(float x, float startX, float endX) const
{
    const float minHz = 20.0f;
    const float maxHz = 20000.0f;
    float norm = juce::jlimit(0.0f, 1.0f, (x - startX) / (endX - startX));
    return minHz * std::pow(maxHz / minHz, norm);
}

void TriggerVisualizer::updateWaveform()
{
    visualizer.getWaveform(waveformData);
    hitMarkers = visualizer.getRecentHits();

    int64_t now = juce::Time::currentTimeMillis();

    // Spawn new shockwave ripples for newly registered hits
    for (const auto& hit : hitMarkers)
    {
        if (hit.timestampMs > lastSeenHitTimestamp)
        {
            float hitY = dbToY(juce::Decibels::gainToDecibels(hit.velocity + 1e-4f), static_cast<float>(getHeight()));
            shockwaves.push_back({ static_cast<float>(getWidth()) - 25.0f, hitY, hit.velocity, now });
            if (hit.timestampMs > lastSeenHitTimestamp)
                lastSeenHitTimestamp = hit.timestampMs;
        }
    }

    // Clean up shockwaves older than 500ms
    shockwaves.erase(
        std::remove_if(shockwaves.begin(), shockwaves.end(),
                       [now](const HitShockwave& sw) { return (now - sw.timestamp) > 500; }),
        shockwaves.end());

    if (detailParam != nullptr && activeDragTarget != DragTarget::Detail)
        detailDb = detailParam->convertFrom0to1(detailParam->getValue());
    if (lowCutParam != nullptr && activeDragTarget != DragTarget::LowCut)
        lowCutHz = lowCutParam->convertFrom0to1(lowCutParam->getValue());
    if (highCutParam != nullptr && activeDragTarget != DragTarget::HighCut)
        highCutHz = highCutParam->convertFrom0to1(highCutParam->getValue());
    if (retriggerParam != nullptr)
        retriggerMs = retriggerParam->convertFrom0to1(retriggerParam->getValue());

    repaint();
}

void TriggerVisualizer::paint(juce::Graphics& g)
{
    const int width = getWidth();
    const int height = getHeight();

    const float xStart = 44.0f;
    const float xEnd = static_cast<float>(width) - 10.0f;

    // 1. CRT Oscilloscope Deep Dark Background with subtle glass curvature
    juce::ColourGradient bgGrad(juce::Colour(0xff090c12), static_cast<float>(width) * 0.5f, static_cast<float>(height) * 0.5f,
                                juce::Colour(0xff040508), 0.0f, 0.0f, true);
    g.setGradientFill(bgGrad);
    g.fillRect(0, 0, width, height);

    // 2. Bandpass Filter Shaded Attenuation Zones (Slate Trigger 2 Signature Filter Overlay)
    float lowCutX = hzToX(lowCutHz, xStart, xEnd);
    float highCutX = hzToX(highCutHz, xStart, xEnd);

    // Attenuation shade on left (Low-Cut / High-Pass side)
    if (lowCutX > xStart)
    {
        juce::ColourGradient lowCutShade(juce::Colour(0xffff2d55).withAlpha(0.18f), xStart, 0.0f,
                                         juce::Colour(0xff06080d).withAlpha(0.40f), lowCutX, 0.0f, false);
        g.setGradientFill(lowCutShade);
        g.fillRect(xStart, 8.0f, lowCutX - xStart, static_cast<float>(height) - 16.0f);
    }

    // Attenuation shade on right (High-Cut / Low-Pass side)
    if (highCutX < xEnd)
    {
        juce::ColourGradient highCutShade(juce::Colour(0xff06080d).withAlpha(0.40f), highCutX, 0.0f,
                                          juce::Colour(0xffff2d55).withAlpha(0.18f), xEnd, 0.0f, false);
        g.setGradientFill(highCutShade);
        g.fillRect(highCutX, 8.0f, xEnd - highCutX, static_cast<float>(height) - 16.0f);
    }

    // 3. High-Tech Reticle Grid Lines & dB Scales
    const float dbLevels[] = { 0.0f, -6.0f, -12.0f, -24.0f, -36.0f, -48.0f };
    g.setFont(juce::Font(9.5f, juce::Font::bold));

    for (float db : dbLevels)
    {
        float y = dbToY(db, static_cast<float>(height));
        // Grid line
        g.setColour(juce::Colour(0xff161c28).withAlpha(0.85f));
        g.drawHorizontalLine(static_cast<int>(y), xStart - 4.0f, xEnd);

        // Accent tick at left
        g.setColour(juce::Colour(0xff222c3d));
        g.drawHorizontalLine(static_cast<int>(y), xStart - 8.0f, xStart);

        g.setColour(juce::Colour(0xff687a95));
        g.drawText(juce::String(static_cast<int>(db)) + " dB", 2, static_cast<int>(y) - 6, 38, 12, juce::Justification::centredRight);
    }

    // Vertical time division grid lines
    g.setColour(juce::Colour(0xff121720));
    for (float x = xStart; x < xEnd; x += 55.0f)
        g.drawVerticalLine(static_cast<int>(x), 8.0f, static_cast<float>(height) - 8.0f);

    // 4. Render Incoming Audio Waveform (Multi-Stop Electric Neon Glow)
    if (!waveformData.empty())
    {
        const int numPoints = static_cast<int>(waveformData.size());
        const float xRange = xEnd - xStart;
        const float step = xRange / static_cast<float>(numPoints);

        juce::Path wavePath;
        juce::Path fillPath;

        float firstY = dbToY(juce::Decibels::gainToDecibels(std::abs(waveformData[0]) + 1e-5f), static_cast<float>(height));
        wavePath.startNewSubPath(xStart, firstY);
        fillPath.startNewSubPath(xStart, static_cast<float>(height) - 12.0f);
        fillPath.lineTo(xStart, firstY);

        for (int i = 1; i < numPoints; ++i)
        {
            float curX = xStart + static_cast<float>(i) * step;
            float absAmp = std::abs(waveformData[static_cast<size_t>(i)]) + 1e-5f;
            float curDb = juce::Decibels::gainToDecibels(absAmp);
            float curY = dbToY(curDb, static_cast<float>(height));

            wavePath.lineTo(curX, curY);
            fillPath.lineTo(curX, curY);
        }

        fillPath.lineTo(xStart + xRange, static_cast<float>(height) - 12.0f);
        fillPath.closeSubPath();

        // Multi-stop neon wave fill gradient
        juce::ColourGradient waveGrad(juce::Colour(0xff00f0ff).withAlpha(0.32f), 0.0f, 12.0f,
                                      juce::Colour(0xff180045).withAlpha(0.08f), 0.0f, static_cast<float>(height), false);
        waveGrad.addColour(0.35, juce::Colour(0xff0066ff).withAlpha(0.20f));
        g.setGradientFill(waveGrad);
        g.fillPath(fillPath);

        // Luminous Bloom Outline
        g.setColour(SlateLookAndFeel::cyanGlow.withAlpha(0.35f));
        g.strokePath(wavePath, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Crisp Inner Neon Core
        g.setColour(juce::Colour(0xffb3ffff));
        g.strokePath(wavePath, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 5. Expanding Impact Shockwave Rings (Ripples) on Drum Hits
    int64_t now = juce::Time::currentTimeMillis();
    for (const auto& sw : shockwaves)
    {
        int64_t age = now - sw.timestamp;
        if (age < 500)
        {
            float progress = static_cast<float>(age) / 500.0f;
            float alpha = (1.0f - progress) * (0.3f + 0.7f * sw.velocity);
            float radius = 8.0f + progress * 55.0f;

            g.setColour(juce::Colour(0xff00ffee).withAlpha(alpha * 0.7f));
            g.drawEllipse(sw.x - radius, sw.y - radius, radius * 2.0f, radius * 2.0f, 1.8f);

            if (radius > 16.0f)
            {
                float innerR = radius * 0.6f;
                g.setColour(juce::Colour(0xff00c8ff).withAlpha(alpha * 0.45f));
                g.drawEllipse(sw.x - innerR, sw.y - innerR, innerR * 2.0f, innerR * 2.0f, 1.2f);
            }
        }
    }

    // 6. Hit Markers & Retrigger Lockout Zone Visualization (Slate Trigger 2 Style)
    for (const auto& hit : hitMarkers)
    {
        int64_t elapsed = now - hit.timestampMs;
        if (elapsed < 800)
        {
            float alpha = 1.0f - static_cast<float>(elapsed) / 800.0f;
            float progress = static_cast<float>(elapsed) / 800.0f;
            float markerX = (static_cast<float>(width) - 25.0f) - progress * (static_cast<float>(width) - 65.0f);

            // Retrigger Lockout Shaded Duration Window
            float lockoutPx = (retriggerMs / 800.0f) * (static_cast<float>(width) - 65.0f);
            if (lockoutPx > 2.0f)
            {
                juce::Rectangle<float> lockoutBox(markerX, 10.0f, lockoutPx, static_cast<float>(height) - 20.0f);
                g.setColour(SlateLookAndFeel::amberGlow.withAlpha(alpha * 0.14f));
                g.fillRect(lockoutBox);
                g.setColour(SlateLookAndFeel::amberGlow.withAlpha(alpha * 0.35f));
                g.drawHorizontalLine(static_cast<int>(lockoutBox.getBottom() - 2.0f), lockoutBox.getX(), lockoutBox.getRight());
            }

            // Glowing trigger beacon line
            g.setColour(juce::Colour(0xff00ffc8).withAlpha(alpha * 0.45f));
            g.drawLine(markerX, 10.0f, markerX, static_cast<float>(height) - 10.0f, 3.0f);

            g.setColour(juce::Colour(0xff00ffc8).withAlpha(alpha));
            g.drawLine(markerX, 10.0f, markerX, static_cast<float>(height) - 10.0f, 1.2f);

            // Diamond Hit Beacon Head at top
            juce::Path diamond;
            diamond.startNewSubPath(markerX, 8.0f);
            diamond.lineTo(markerX + 4.5f, 13.0f);
            diamond.lineTo(markerX, 18.0f);
            diamond.lineTo(markerX - 4.5f, 13.0f);
            diamond.closeSubPath();

            g.setColour(juce::Colour(0xff00ffea).withAlpha(alpha * 0.8f));
            g.fillPath(diamond);
            g.setColour(juce::Colours::white.withAlpha(alpha));
            g.strokePath(diamond, juce::PathStrokeType(1.0f));

            // Velocity dynamic bar at bottom
            float velH = hit.velocity * (static_cast<float>(height) - 35.0f);
            g.setColour(juce::Colour(0xff00e5ff).withAlpha(alpha * 0.5f));
            g.fillRoundedRectangle(markerX - 1.5f, static_cast<float>(height) - 12.0f - velH, 3.0f, velH, 1.5f);
        }
    }

    // 7. Interactive High-Pass (Low-Cut) Vertical Boundary Handle Line
    g.setColour(juce::Colour(0xffff3366).withAlpha(0.4f));
    g.drawLine(lowCutX, 8.0f, lowCutX, static_cast<float>(height) - 8.0f, 3.0f);
    g.setColour(juce::Colour(0xffff3366));
    g.drawLine(lowCutX, 8.0f, lowCutX, static_cast<float>(height) - 8.0f, 1.5f);

    // HP Cutoff Frequency Handle Badge
    juce::Rectangle<float> hpBadge(lowCutX - 32.0f, 10.0f, 64.0f, 16.0f);
    g.setColour(juce::Colour(0xff20050c));
    g.fillRoundedRectangle(hpBadge, 3.0f);
    g.setColour(juce::Colour(0xffff3366));
    g.drawRoundedRectangle(hpBadge, 3.0f, 1.0f);
    g.setFont(juce::Font(9.0f, juce::Font::bold));
    g.drawText("HP: " + juce::String(static_cast<int>(lowCutHz)) + " Hz", hpBadge.toNearestInt(), juce::Justification::centred);

    // 8. Interactive Low-Pass (High-Cut) Vertical Boundary Handle Line
    g.setColour(juce::Colour(0xff33ccff).withAlpha(0.4f));
    g.drawLine(highCutX, 8.0f, highCutX, static_cast<float>(height) - 8.0f, 3.0f);
    g.setColour(juce::Colour(0xff33ccff));
    g.drawLine(highCutX, 8.0f, highCutX, static_cast<float>(height) - 8.0f, 1.5f);

    // LP Cutoff Frequency Handle Badge
    juce::Rectangle<float> lpBadge(highCutX - 32.0f, 10.0f, 64.0f, 16.0f);
    g.setColour(juce::Colour(0xff051622));
    g.fillRoundedRectangle(lpBadge, 3.0f);
    g.setColour(juce::Colour(0xff33ccff));
    g.drawRoundedRectangle(lpBadge, 3.0f, 1.0f);
    g.setFont(juce::Font(9.0f, juce::Font::bold));
    juce::String lpText = highCutHz >= 1000.0f
        ? "LP: " + juce::String(highCutHz / 1000.0f, 1) + " kHz"
        : "LP: " + juce::String(static_cast<int>(highCutHz)) + " Hz";
    g.drawText(lpText, lpBadge.toNearestInt(), juce::Justification::centred);

    // 9. Active Draggable DETAIL / THRESHOLD Laser Line (Amber / Gold)
    float detailY = dbToY(detailDb, static_cast<float>(height));

    // Outer laser glow
    g.setColour(SlateLookAndFeel::amberGlow.withAlpha(0.28f));
    g.drawLine(xStart - 4.0f, detailY, xEnd, detailY, 4.0f);

    // Laser beam core
    g.setColour(SlateLookAndFeel::amberGlow);
    g.drawLine(xStart - 4.0f, detailY, xEnd, detailY, 1.5f);

    // Laser end markers
    g.setColour(SlateLookAndFeel::amberGlow.brighter(0.4f));
    g.fillEllipse(xStart - 6.0f, detailY - 2.5f, 5.0f, 5.0f);

    // Tactile Draggable Handle Badge
    juce::Rectangle<float> handleRect(static_cast<float>(width) - 118.0f, detailY - 10.0f, 108.0f, 20.0f);
    juce::ColourGradient handleGrad(juce::Colour(0xff352308), handleRect.getX(), handleRect.getY(),
                                    juce::Colour(0xff1a1103), handleRect.getX(), handleRect.getBottom(), false);
    g.setGradientFill(handleGrad);
    g.fillRoundedRectangle(handleRect, 3.5f);

    g.setColour(SlateLookAndFeel::amberGlow);
    g.drawRoundedRectangle(handleRect, 3.5f, 1.2f);

    g.setFont(juce::Font(10.5f, juce::Font::bold));
    g.setColour(juce::Colour(0xffffdd88));
    juce::String detailText = "DETAIL: " + juce::String(detailDb, 1) + " dB";
    g.drawText(detailText, handleRect.toNearestInt(), juce::Justification::centred);

    // 10. Auditioning Warning Badge
    if (isAuditioning)
    {
        juce::Rectangle<int> badge(width - 245, 9, 120, 20);
        g.setColour(SlateLookAndFeel::amberGlow.withAlpha(0.25f));
        g.fillRoundedRectangle(badge.toFloat(), 3.5f);
        g.setColour(SlateLookAndFeel::amberGlow);
        g.drawRoundedRectangle(badge.toFloat(), 3.5f, 1.2f);
        g.setFont(juce::Font(9.5f, juce::Font::bold));
        g.drawText("AUDITION FILTER", badge, juce::Justification::centred);
    }

    // 11. Outer CRT Vignette & Screen Frame
    g.setColour(juce::Colour(0xff07090d).withAlpha(0.6f));
    g.drawRect(0, 0, width, height, 2);

    g.setColour(SlateLookAndFeel::slateBorder.brighter(0.1f));
    g.drawRect(0, 0, width, height, 1);

    // 12. Drag-and-Drop Overlay Highlight
    if (isDragHovered)
    {
        g.setColour(SlateLookAndFeel::cyanGlow.withAlpha(0.25f));
        g.fillRect(0, 0, width, height);

        g.setColour(SlateLookAndFeel::cyanGlow);
        g.drawRect(2, 2, width - 4, height - 4, 2);

        g.setFont(juce::Font(15.0f, juce::Font::bold));
        g.setColour(juce::Colours::white);
        g.drawText("DROP AUDIO SAMPLE TO LOAD INTO CHANNEL 1", 0, height / 2 - 12, width, 24, juce::Justification::centred);
    }
}

void TriggerVisualizer::resized()
{
}

void TriggerVisualizer::mouseMove(const juce::MouseEvent& e)
{
    const float xStart = 44.0f;
    const float xEnd = static_cast<float>(getWidth()) - 10.0f;
    float lowCutX = hzToX(lowCutHz, xStart, xEnd);
    float highCutX = hzToX(highCutHz, xStart, xEnd);
    float detailY = dbToY(detailDb, static_cast<float>(getHeight()));

    if (std::abs(e.position.y - detailY) < 10.0f)
        setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
    else if (std::abs(e.position.x - lowCutX) < 10.0f || std::abs(e.position.x - highCutX) < 10.0f)
        setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
    else
        setMouseCursor(juce::MouseCursor::NormalCursor);
}

void TriggerVisualizer::mouseDown(const juce::MouseEvent& e)
{
    const float xStart = 44.0f;
    const float xEnd = static_cast<float>(getWidth()) - 10.0f;
    float lowCutX = hzToX(lowCutHz, xStart, xEnd);
    float highCutX = hzToX(highCutHz, xStart, xEnd);
    float detailY = dbToY(detailDb, static_cast<float>(getHeight()));

    // Check hit on Low-Cut handle
    if (std::abs(e.position.x - lowCutX) < 12.0f)
    {
        activeDragTarget = DragTarget::LowCut;
    }
    // Check hit on High-Cut handle
    else if (std::abs(e.position.x - highCutX) < 12.0f)
    {
        activeDragTarget = DragTarget::HighCut;
    }
    // Check hit on Detail handle or horizontal line
    else if (std::abs(e.position.y - detailY) < 12.0f || e.position.x > static_cast<float>(getWidth() - 125))
    {
        activeDragTarget = DragTarget::Detail;
    }
    else
    {
        activeDragTarget = DragTarget::None;
    }
}

void TriggerVisualizer::mouseDrag(const juce::MouseEvent& e)
{
    const float xStart = 44.0f;
    const float xEnd = static_cast<float>(getWidth()) - 10.0f;

    if (activeDragTarget == DragTarget::Detail)
    {
        detailDb = yToDb(e.position.y, static_cast<float>(getHeight()));
        detailDb = juce::jlimit(-60.0f, 0.0f, detailDb);
        if (detailParam != nullptr)
            detailParam->setValueNotifyingHost(detailParam->convertTo0to1(detailDb));
        repaint();
    }
    else if (activeDragTarget == DragTarget::LowCut)
    {
        lowCutHz = xToHz(e.position.x, xStart, xEnd);
        lowCutHz = juce::jlimit(20.0f, std::min(2000.0f, highCutHz - 50.0f), lowCutHz);
        if (lowCutParam != nullptr)
            lowCutParam->setValueNotifyingHost(lowCutParam->convertTo0to1(lowCutHz));
        repaint();
    }
    else if (activeDragTarget == DragTarget::HighCut)
    {
        highCutHz = xToHz(e.position.x, xStart, xEnd);
        highCutHz = juce::jlimit(std::max(200.0f, lowCutHz + 50.0f), 20000.0f, highCutHz);
        if (highCutParam != nullptr)
            highCutParam->setValueNotifyingHost(highCutParam->convertTo0to1(highCutHz));
        repaint();
    }
}

void TriggerVisualizer::mouseUp(const juce::MouseEvent& /*e*/)
{
    activeDragTarget = DragTarget::None;
}

bool TriggerVisualizer::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (const auto& f : files)
    {
        if (f.endsWithIgnoreCase(".wav") || f.endsWithIgnoreCase(".aif") ||
            f.endsWithIgnoreCase(".aiff") || f.endsWithIgnoreCase(".mp3") ||
            f.endsWithIgnoreCase(".flac"))
            return true;
    }
    return false;
}

void TriggerVisualizer::fileDragEnter(const juce::StringArray& /*files*/, int /*x*/, int /*y*/)
{
    isDragHovered = true;
    repaint();
}

void TriggerVisualizer::fileDragExit(const juce::StringArray& /*files*/)
{
    isDragHovered = false;
    repaint();
}

void TriggerVisualizer::filesDropped(const juce::StringArray& files, int /*x*/, int /*y*/)
{
    isDragHovered = false;
    if (!files.isEmpty() && onSampleDropped)
    {
        onSampleDropped(juce::File(files[0]));
    }
    repaint();
}

} // namespace yeahwaah
