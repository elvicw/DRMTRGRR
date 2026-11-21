#include "TriggerVisualizer.h"
#include "SlateLookAndFeel.h"
#include <cmath>

namespace yeahwaah
{

TriggerVisualizer::TriggerVisualizer(VisualizerBuffer& vb, juce::RangedAudioParameter* dp)
    : visualizer(vb), detailParam(dp)
{
    setOpaque(true);
    if (detailParam != nullptr)
        detailDb = detailParam->convertFrom0to1(detailParam->getValue());
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
            float hitY = dbToY(juce::Decibels::gainToDecibels(hit.velocity + 1e-4f), (float)getHeight());
            shockwaves.push_back({ (float)getWidth() - 25.0f, hitY, hit.velocity, now });
            if (hit.timestampMs > lastSeenHitTimestamp)
                lastSeenHitTimestamp = hit.timestampMs;
        }
    }

    // Clean up shockwaves older than 500ms
    shockwaves.erase(
        std::remove_if(shockwaves.begin(), shockwaves.end(),
                       [now](const HitShockwave& sw) { return (now - sw.timestamp) > 500; }),
        shockwaves.end());

    if (detailParam != nullptr && !isDraggingDetail)
        detailDb = detailParam->convertFrom0to1(detailParam->getValue());

    repaint();
}

void TriggerVisualizer::paint(juce::Graphics& g)
{
    const int width = getWidth();
    const int height = getHeight();

    // 1. Deep Oscilloscope CRT Dark Background
    juce::ColourGradient bgGrad(juce::Colour(0xff090c12), width * 0.5f, height * 0.5f,
                                juce::Colour(0xff05070a), 0.0f, 0.0f, true);
    g.setGradientFill(bgGrad);
    g.fillRect(0, 0, width, height);

    // 2. High-Tech Reticle Grid Lines & dB Scales
    const float dbLevels[] = { 0.0f, -6.0f, -12.0f, -24.0f, -36.0f, -48.0f };
    g.setFont(juce::Font(9.5f, juce::Font::bold));

    for (float db : dbLevels)
    {
        float y = dbToY(db, (float)height);
        // Subtle dotted or solid grid line
        g.setColour(juce::Colour(0xff161c28));
        g.drawHorizontalLine(static_cast<int>(y), 38.0f, (float)width - 8.0f);

        // Accent tick at left
        g.setColour(juce::Colour(0xff222c3d));
        g.drawHorizontalLine(static_cast<int>(y), 35.0f, 42.0f);

        g.setColour(juce::Colour(0xff62728c));
        g.drawText(juce::String(static_cast<int>(db)) + " dB", 2, static_cast<int>(y) - 6, 34, 12, juce::Justification::centredRight);
    }

    // Vertical time division grid lines
    g.setColour(juce::Colour(0xff121720));
    for (int x = 40; x < width; x += 55)
        g.drawVerticalLine(x, 8.0f, (float)height - 8.0f);

    // 3. Render Incoming Audio Waveform (Multi-Stop Electric Neon Glow)
    if (!waveformData.empty())
    {
        const int numPoints = (int)waveformData.size();
        const float xStart = 40.0f;
        const float xRange = width - xStart - 8.0f;
        const float step = xRange / static_cast<float>(numPoints);

        juce::Path wavePath;
        juce::Path fillPath;

        float firstY = dbToY(juce::Decibels::gainToDecibels(std::abs(waveformData[0]) + 1e-5f), (float)height);
        wavePath.startNewSubPath(xStart, firstY);
        fillPath.startNewSubPath(xStart, (float)height - 12.0f);
        fillPath.lineTo(xStart, firstY);

        for (int i = 1; i < numPoints; ++i)
        {
            float curX = xStart + i * step;
            float absAmp = std::abs(waveformData[i]) + 1e-5f;
            float curDb = juce::Decibels::gainToDecibels(absAmp);
            float curY = dbToY(curDb, (float)height);

            wavePath.lineTo(curX, curY);
            fillPath.lineTo(curX, curY);
        }

        fillPath.lineTo(xStart + xRange, (float)height - 12.0f);
        fillPath.closeSubPath();

        // Multi-stop neon wave fill gradient
        juce::ColourGradient waveGrad(juce::Colour(0xff00f0ff).withAlpha(0.35f), 0.0f, 12.0f,
                                      juce::Colour(0xff200055).withAlpha(0.08f), 0.0f, (float)height, false);
        waveGrad.addColour(0.35, juce::Colour(0xff0066ff).withAlpha(0.22f));
        g.setGradientFill(waveGrad);
        g.fillPath(fillPath);

        // Luminous Bloom Outline
        g.setColour(SlateLookAndFeel::cyanGlow.withAlpha(0.35f));
        g.strokePath(wavePath, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Crisp Inner Neon Core
        g.setColour(juce::Colour(0xffb3ffff));
        g.strokePath(wavePath, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 4. Expanding Impact Shockwave Rings (Ripples) on Drum Hits
    int64_t now = juce::Time::currentTimeMillis();
    for (const auto& sw : shockwaves)
    {
        int64_t age = now - sw.timestamp;
        if (age < 500)
        {
            float progress = static_cast<float>(age) / 500.0f; // 0.0 to 1.0
            float alpha = (1.0f - progress) * (0.3f + 0.7f * sw.velocity);
            float radius = 8.0f + progress * 55.0f;

            // Expanding glowing ripple circle
            g.setColour(juce::Colour(0xff00ffee).withAlpha(alpha * 0.7f));
            g.drawEllipse(sw.x - radius, sw.y - radius, radius * 2.0f, radius * 2.0f, 1.8f);

            // Inner secondary ring
            if (radius > 16.0f)
            {
                float innerR = radius * 0.6f;
                g.setColour(juce::Colour(0xff00c8ff).withAlpha(alpha * 0.45f));
                g.drawEllipse(sw.x - innerR, sw.y - innerR, innerR * 2.0f, innerR * 2.0f, 1.2f);
            }
        }
    }

    // 5. Hit Markers (Flashing Transient Indicators)
    for (const auto& hit : hitMarkers)
    {
        int64_t elapsed = now - hit.timestampMs;
        if (elapsed < 800)
        {
            float alpha = 1.0f - static_cast<float>(elapsed) / 800.0f;
            float progress = static_cast<float>(elapsed) / 800.0f;
            float markerX = (width - 25.0f) - progress * (width - 65.0f);

            // Glowing trigger beacon line
            g.setColour(juce::Colour(0xff00ffc8).withAlpha(alpha * 0.4f));
            g.drawLine(markerX, 10.0f, markerX, (float)height - 10.0f, 3.0f);

            g.setColour(juce::Colour(0xff00ffc8).withAlpha(alpha));
            g.drawLine(markerX, 10.0f, markerX, (float)height - 10.0f, 1.2f);

            // Glowing hit beacon orb at top
            g.setColour(juce::Colour(0xff00ffea).withAlpha(alpha * 0.5f));
            g.fillEllipse(markerX - 4.5f, 9.0f, 9.0f, 9.0f);
            g.setColour(juce::Colours::white.withAlpha(alpha));
            g.fillEllipse(markerX - 2.0f, 11.5f, 4.0f, 4.0f);

            // Velocity dynamic bar at bottom
            float velH = hit.velocity * (height - 35.0f);
            g.setColour(juce::Colour(0xff00e5ff).withAlpha(alpha * 0.5f));
            g.fillRoundedRectangle(markerX - 1.5f, height - 12.0f - velH, 3.0f, velH, 1.5f);
        }
    }

    // 6. Active Draggable DETAIL / THRESHOLD Laser Line (Amber / Gold)
    float detailY = dbToY(detailDb, (float)height);

    // Outer laser glow
    g.setColour(SlateLookAndFeel::amberGlow.withAlpha(0.28f));
    g.drawLine(38.0f, detailY, (float)width - 8.0f, detailY, 4.0f);

    // Laser beam core
    g.setColour(SlateLookAndFeel::amberGlow);
    g.drawLine(38.0f, detailY, (float)width - 8.0f, detailY, 1.5f);

    // Laser end markers
    g.setColour(SlateLookAndFeel::amberGlow.brighter(0.4f));
    g.fillEllipse(36.0f, detailY - 2.5f, 5.0f, 5.0f);

    // Tactile Draggable Handle Badge
    juce::Rectangle<float> handleRect(width - 114.0f, detailY - 10.0f, 106.0f, 20.0f);
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

    // 7. Auditioning Warning Badge
    if (isAuditioning)
    {
        juce::Rectangle<int> badge(width - 235, 9, 115, 20);
        g.setColour(SlateLookAndFeel::amberGlow.withAlpha(0.25f));
        g.fillRoundedRectangle(badge.toFloat(), 3.5f);
        g.setColour(SlateLookAndFeel::amberGlow);
        g.drawRoundedRectangle(badge.toFloat(), 3.5f, 1.2f);
        g.setFont(juce::Font(9.5f, juce::Font::bold));
        g.drawText("AUDITION FILTER", badge, juce::Justification::centred);
    }

    // 8. Outer CRT Vignette & Screen Frame
    g.setColour(juce::Colour(0xff07090d).withAlpha(0.6f));
    g.drawRect(0, 0, width, height, 2);

    g.setColour(SlateLookAndFeel::slateBorder.brighter(0.1f));
    g.drawRect(0, 0, width, height, 1);

    // 9. Drag-and-Drop Overlay Highlight
    if (isDragHovered)
    {
        g.setColour(SlateLookAndFeel::cyanGlow.withAlpha(0.25f));
        g.fillRect(0, 0, width, height);
        g.setColour(SlateLookAndFeel::cyanGlow);
        g.drawRect(1, 1, width - 2, height - 2, 2);

        g.setFont(juce::Font(16.0f, juce::Font::bold));
        g.setColour(juce::Colours::white);
        g.drawText("DROP SAMPLE AUDIO FILE TO LOAD INTO SLOT 1", 0, 0, width, height, juce::Justification::centred);
    }
}

void TriggerVisualizer::resized()
{
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
    if (onSampleDropped && !files.isEmpty())
    {
        onSampleDropped(juce::File(files[0]));
    }
    repaint();
}

void TriggerVisualizer::mouseDown(const juce::MouseEvent& e)
{
    float detailY = dbToY(detailDb, (float)getHeight());
    if (std::abs(e.position.y - detailY) < 14.0f || e.position.x > getWidth() - 120.0f)
    {
        isDraggingDetail = true;
        mouseDrag(e);
    }
}

void TriggerVisualizer::mouseDrag(const juce::MouseEvent& e)
{
    if (isDraggingDetail && detailParam != nullptr)
    {
        float newDb = yToDb(e.position.y, (float)getHeight());
        newDb = juce::jlimit(-60.0f, 0.0f, newDb);
        detailDb = newDb;

        float normVal = detailParam->convertTo0to1(newDb);
        detailParam->setValueNotifyingHost(normVal);
        repaint();
    }
}

void TriggerVisualizer::mouseUp(const juce::MouseEvent& /*e*/)
{
    isDraggingDetail = false;
}

} // namespace yeahwaah
