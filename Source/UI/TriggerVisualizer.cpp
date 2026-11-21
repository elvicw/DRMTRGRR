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
    // dB from -60.0 to 0.0 -> normalized 0.0 (bottom) to 1.0 (top)
    float norm = (dB + 60.0f) / 60.0f;
    norm = juce::jlimit(0.0f, 1.0f, norm);
    return height - (norm * (height - 20.0f) + 10.0f);
}

float TriggerVisualizer::yToDb(float y, float height) const
{
    float norm = (height - 10.0f - y) / (height - 20.0f);
    norm = juce::jlimit(0.0f, 1.0f, norm);
    return -60.0f + norm * 60.0f;
}

void TriggerVisualizer::updateWaveform()
{
    visualizer.getWaveform(waveformData);
    hitMarkers = visualizer.getRecentHits();

    if (detailParam != nullptr && !isDraggingDetail)
        detailDb = detailParam->convertFrom0to1(detailParam->getValue());

    repaint();
}

void TriggerVisualizer::paint(juce::Graphics& g)
{
    const int width = getWidth();
    const int height = getHeight();

    // 1. Oscilloscope dark background
    g.setColour(juce::Colour(0xff0b0d10));
    g.fillRect(0, 0, width, height);

    // Subtle horizontal scanline effect
    g.setColour(juce::Colour(0xff080a0c));
    for (int y = 0; y < height; y += 4)
        g.drawHorizontalLine(y, 0.0f, (float)width);

    // 2. dB Grid lines & labels
    const float dbLevels[] = { 0.0f, -6.0f, -12.0f, -24.0f, -36.0f, -48.0f };
    g.setFont(juce::Font(10.0f, juce::Font::bold));

    for (float db : dbLevels)
    {
        float y = dbToY(db, (float)height);
        g.setColour(juce::Colour(0xff1e232c));
        g.drawHorizontalLine(static_cast<int>(y), 38.0f, (float)width - 8.0f);

        g.setColour(juce::Colour(0xff556070));
        g.drawText(juce::String(static_cast<int>(db)) + " dB", 2, static_cast<int>(y) - 6, 34, 12, juce::Justification::centredRight);
    }

    // Vertical time division ticks
    g.setColour(juce::Colour(0xff161920));
    for (int x = 40; x < width; x += 60)
        g.drawVerticalLine(x, 8.0f, (float)height - 8.0f);

    // 3. Draw Incoming Drum Waveform
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
        fillPath.startNewSubPath(xStart, (float)height - 10.0f);
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

        fillPath.lineTo(xStart + xRange, (float)height - 10.0f);
        fillPath.closeSubPath();

        // Waveform gradient fill (slate cyan to transparent dark)
        juce::ColourGradient waveGrad(SlateLookAndFeel::cyanGlow.withAlpha(0.28f), 0.0f, 10.0f,
                                      juce::Colour(0x00000000), 0.0f, (float)height, false);
        g.setGradientFill(waveGrad);
        g.fillPath(fillPath);

        // Waveform curve outline
        g.setColour(SlateLookAndFeel::cyanGlow);
        g.strokePath(wavePath, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 4. Hit Markers (Flashing Transient Indicators)
    int64_t now = juce::Time::currentTimeMillis();
    for (const auto& hit : hitMarkers)
    {
        int64_t elapsed = now - hit.timestampMs;
        if (elapsed < 800)
        {
            float alpha = 1.0f - static_cast<float>(elapsed) / 800.0f;
            // Marker scrolls across from right to left
            float progress = static_cast<float>(elapsed) / 800.0f;
            float markerX = (width - 20.0f) - progress * (width - 60.0f);

            // Flashing vertical trigger line
            g.setColour(juce::Colour(0xff00ffcc).withAlpha(alpha * 0.75f));
            g.drawVerticalLine(static_cast<int>(markerX), 10.0f, (float)height - 10.0f);

            // Hit flag at top
            g.setColour(juce::Colour(0xff00ffcc).withAlpha(alpha));
            g.fillEllipse(markerX - 3.5f, 10.0f, 7.0f, 7.0f);

            // Velocity indicator bar
            float velH = hit.velocity * (height - 30.0f);
            g.setColour(juce::Colour(0xff00e5ff).withAlpha(alpha * 0.4f));
            g.fillRect(markerX - 1.5f, height - 10.0f - velH, 3.0f, velH);
        }
    }

    // 5. Draggable DETAIL / THRESHOLD Line (Amber/Gold Slate Style)
    float detailY = dbToY(detailDb, (float)height);

    // Glowing threshold beam
    g.setColour(SlateLookAndFeel::amberGlow.withAlpha(0.35f));
    g.drawHorizontalLine(static_cast<int>(detailY), 38.0f, (float)width - 8.0f);
    g.setColour(SlateLookAndFeel::amberGlow);
    g.drawHorizontalLine(static_cast<int>(detailY), 38.0f, (float)width - 8.0f);

    // Threshold handle & badge
    juce::Rectangle<float> handleRect(width - 110.0f, detailY - 9.0f, 102.0f, 18.0f);
    g.setColour(juce::Colour(0xff221706));
    g.fillRoundedRectangle(handleRect, 3.0f);
    g.setColour(SlateLookAndFeel::amberGlow);
    g.drawRoundedRectangle(handleRect, 3.0f, 1.2f);

    g.setFont(juce::Font(10.5f, juce::Font::bold));
    g.setColour(juce::Colour(0xffffdd88));
    juce::String detailText = "DETAIL: " + juce::String(detailDb, 1) + " dB";
    g.drawText(detailText, handleRect.toNearestInt(), juce::Justification::centred);

    // 6. Auditioning Badge if filter listen is active
    if (isAuditioning)
    {
        juce::Rectangle<int> badge(width - 230, 8, 110, 20);
        g.setColour(SlateLookAndFeel::amberGlow.withAlpha(0.2f));
        g.fillRoundedRectangle(badge.toFloat(), 3.0f);
        g.setColour(SlateLookAndFeel::amberGlow);
        g.drawRoundedRectangle(badge.toFloat(), 3.0f, 1.2f);
        g.setFont(juce::Font(9.5f, juce::Font::bold));
        g.drawText("AUDITION FILTER", badge, juce::Justification::centred);
    }

    // Frame border
    g.setColour(SlateLookAndFeel::slateBorder);
    g.drawRect(0, 0, width, height, 1);
}

void TriggerVisualizer::resized()
{
}

void TriggerVisualizer::mouseDown(const juce::MouseEvent& e)
{
    float detailY = dbToY(detailDb, (float)getHeight());
    // Click within 14px of line or inside handle
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
