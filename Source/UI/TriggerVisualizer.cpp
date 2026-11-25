#include "TriggerVisualizer.h"
#include "SlateLookAndFeel.h"
#include <cmath>

namespace yeahwaah
{

namespace
{
constexpr float kXStart = 14.0f;
constexpr float kXEnd(float w) { return w - 14.0f; }
constexpr float kVMargin = 12.0f;
constexpr int64_t kHitWindowMs = 800;

const juce::Colour kScopeAmber(0xffffb43a);
} // namespace

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

// The scope is symmetric: 0 dB sits at the top edge and -60 dB on the centre line.
float TriggerVisualizer::dbToY(float dB, float height) const
{
    const float norm = juce::jlimit(0.0f, 1.0f, (dB + 60.0f) / 60.0f);
    const float half = height * 0.5f - kVMargin;
    return height * 0.5f - norm * half;
}

float TriggerVisualizer::yToDb(float y, float height) const
{
    const float half = height * 0.5f - kVMargin;
    const float norm = juce::jlimit(0.0f, 1.0f, std::abs(height * 0.5f - y) / half);
    return -60.0f + norm * 60.0f;
}

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

int TriggerVisualizer::getActiveHitCount() const
{
    const int64_t now = juce::Time::currentTimeMillis();
    int n = 0;
    for (const auto& h : hitMarkers)
        if (now - h.timestampMs < kHitWindowMs)
            ++n;
    return n;
}

void TriggerVisualizer::updateWaveform()
{
    visualizer.getWaveform(waveformData);
    hitMarkers = visualizer.getRecentHits();

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
    const float fh = static_cast<float>(height);
    const float xStart = kXStart;
    const float xEnd = kXEnd(static_cast<float>(width));
    const float midY = fh * 0.5f;

    // CRT glass
    juce::ColourGradient bg(juce::Colour(0xff1a1206), static_cast<float>(width) * 0.5f, midY,
                            juce::Colour(0xff060402), 0.0f, 0.0f, true);
    g.setGradientFill(bg);
    g.fillRect(0, 0, width, height);

    const float lowCutX = hzToX(lowCutHz, xStart, xEnd);
    const float highCutX = hzToX(highCutHz, xStart, xEnd);

    // Band-pass attenuation zones
    g.setColour(juce::Colours::black.withAlpha(0.45f));
    if (lowCutX > xStart)
        g.fillRect(xStart, 6.0f, lowCutX - xStart, fh - 12.0f);
    if (highCutX < xEnd)
        g.fillRect(highCutX, 6.0f, xEnd - highCutX, fh - 12.0f);

    // Graticule
    g.setColour(kScopeAmber.withAlpha(0.07f));
    for (float x = xStart; x < xEnd; x += (xEnd - xStart) / 16.0f)
        g.drawVerticalLine(static_cast<int>(x), 6.0f, fh - 6.0f);
    for (float db : { -6.0f, -12.0f, -24.0f, -36.0f, -48.0f })
    {
        const float y = dbToY(db, fh);
        g.drawHorizontalLine(static_cast<int>(y), xStart, xEnd);
        g.drawHorizontalLine(static_cast<int>(fh - y), xStart, xEnd);
    }
    g.setColour(kScopeAmber.withAlpha(0.22f));
    g.drawHorizontalLine(static_cast<int>(midY), xStart, xEnd);

    // Waveform, mirrored around the centre line
    if (!waveformData.empty())
    {
        const int numPoints = static_cast<int>(waveformData.size());
        const int pixelWidth = std::max(1, static_cast<int>(xEnd - xStart));

        juce::Path top, bottom, fill;
        for (int px = 0; px < pixelWidth; ++px)
        {
            const int startIdx = px * numPoints / pixelWidth;
            const int endIdx = std::min(numPoints, (px + 1) * numPoints / pixelWidth);
            float maxAmp = 0.0f;
            for (int s = startIdx; s < endIdx; ++s)
                maxAmp = std::max(maxAmp, std::abs(waveformData[static_cast<size_t>(s)]));

            const float y = dbToY(juce::Decibels::gainToDecibels(maxAmp + 1e-4f), fh);
            const float x = xStart + static_cast<float>(px);
            if (px == 0)
            {
                top.startNewSubPath(x, y);
                bottom.startNewSubPath(x, fh - y);
            }
            else
            {
                top.lineTo(x, y);
                bottom.lineTo(x, fh - y);
            }
        }

        fill = top;
        fill.lineTo(xEnd, midY);
        fill.lineTo(xStart, midY);
        fill.closeSubPath();
        juce::Path fillBottom;
        fillBottom.addPath(bottom);
        fillBottom.lineTo(xEnd, midY);
        fillBottom.lineTo(xStart, midY);
        fillBottom.closeSubPath();

        g.setColour(kScopeAmber.withAlpha(0.22f));
        g.fillPath(fill);
        g.fillPath(fillBottom);

        for (auto* p : { &top, &bottom })
        {
            g.setColour(kScopeAmber.withAlpha(0.30f));
            g.strokePath(*p, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.setColour(juce::Colour(0xffffd98a));
            g.strokePath(*p, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
    }

    // Hit markers scroll right to left over the hit window
    const int64_t now = juce::Time::currentTimeMillis();
    for (const auto& hit : hitMarkers)
    {
        const int64_t elapsed = now - hit.timestampMs;
        if (elapsed >= kHitWindowMs)
            continue;

        const float progress = static_cast<float>(elapsed) / static_cast<float>(kHitWindowMs);
        const float alpha = 1.0f - progress;
        const float x = xEnd - progress * (xEnd - xStart);

        g.setColour(kScopeAmber.withAlpha(alpha * 0.35f));
        g.drawLine(x, 14.0f, x, fh - 6.0f, 1.0f);

        juce::Path tri;
        tri.addTriangle(x - 6.0f, 4.0f, x + 6.0f, 4.0f, x, 16.0f);
        g.setColour(kScopeAmber.withAlpha(alpha));
        g.fillPath(tri);
    }

    // Band-pass handles
    auto handle = [&](float x, const juce::String& text, juce::Colour c) {
        g.setColour(c.withAlpha(0.35f));
        g.drawLine(x, 6.0f, x, fh - 6.0f, 3.0f);
        g.setColour(c);
        g.drawLine(x, 6.0f, x, fh - 6.0f, 1.2f);
        juce::Rectangle<float> badge(x - 34.0f, fh - 24.0f, 68.0f, 16.0f);
        g.setColour(juce::Colour(0xff120c04));
        g.fillRoundedRectangle(badge, 3.0f);
        g.setColour(c);
        g.drawRoundedRectangle(badge, 3.0f, 1.0f);
        g.setFont(SlateLookAndFeel::boldFont(11.0f));
        g.drawText(text, badge.toNearestInt(), juce::Justification::centred);
    };
    handle(lowCutX, "HP " + juce::String(static_cast<int>(lowCutHz)) + " Hz", juce::Colour(0xffd9d2bf));
    handle(highCutX,
           highCutHz >= 1000.0f ? "LP " + juce::String(highCutHz / 1000.0f, 1) + " kHz"
                                : "LP " + juce::String(static_cast<int>(highCutHz)) + " Hz",
           juce::Colour(0xffd9d2bf));

    // Threshold: dashed, mirrored
    const float detailY = dbToY(detailDb, fh);
    juce::Path dash;
    dash.startNewSubPath(xStart, detailY);
    dash.lineTo(xEnd, detailY);
    juce::Path dashed;
    const float pattern[] = { 7.0f, 5.0f };
    juce::PathStrokeType(1.4f).createDashedStroke(dashed, dash, pattern, 2);
    g.setColour(kScopeAmber);
    g.fillPath(dashed);
    g.setColour(kScopeAmber.withAlpha(0.5f));
    g.fillPath(dashed, juce::AffineTransform::scale(1.0f, -1.0f, 0.0f, midY));

    g.setFont(SlateLookAndFeel::boldFont(12.0f));
    g.setColour(kScopeAmber);
    g.drawText(juce::String(detailDb, 1) + " dB", static_cast<int>(xEnd) - 90, static_cast<int>(detailY) - 17, 88, 14,
               juce::Justification::centredRight);

    if (isAuditioning)
    {
        juce::Rectangle<int> badge(static_cast<int>(xStart) + 4, 6, 118, 18);
        g.setColour(kScopeAmber.withAlpha(0.25f));
        g.fillRoundedRectangle(badge.toFloat(), 3.0f);
        g.setColour(kScopeAmber);
        g.drawRoundedRectangle(badge.toFloat(), 3.0f, 1.0f);
        g.setFont(SlateLookAndFeel::boldFont(11.0f));
        g.drawText("AUDITION FILTER", badge, juce::Justification::centred);
    }

    // Glass vignette
    juce::ColourGradient vig(juce::Colours::transparentBlack, static_cast<float>(width) * 0.5f, midY,
                             juce::Colours::black.withAlpha(0.55f), 0.0f, 0.0f, true);
    g.setGradientFill(vig);
    g.fillRect(0, 0, width, height);

    if (isDragHovered)
    {
        g.setColour(kScopeAmber.withAlpha(0.2f));
        g.fillRect(0, 0, width, height);
        g.setColour(kScopeAmber);
        g.drawRect(2, 2, width - 4, height - 4, 2);
        g.setFont(SlateLookAndFeel::boldFont(18.0f));
        g.setColour(juce::Colours::white);
        g.drawText("DROP AUDIO SAMPLE TO LOAD INTO CHANNEL 1", 0, height / 2 - 12, width, 24, juce::Justification::centred);
    }
}

void TriggerVisualizer::resized() {}

void TriggerVisualizer::mouseMove(const juce::MouseEvent& e)
{
    const float xEnd = kXEnd(static_cast<float>(getWidth()));
    const float lowCutX = hzToX(lowCutHz, kXStart, xEnd);
    const float highCutX = hzToX(highCutHz, kXStart, xEnd);
    const float detailY = dbToY(detailDb, static_cast<float>(getHeight()));
    const float mirrored = static_cast<float>(getHeight()) - detailY;

    if (std::abs(e.position.y - detailY) < 10.0f || std::abs(e.position.y - mirrored) < 10.0f)
        setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
    else if (std::abs(e.position.x - lowCutX) < 10.0f || std::abs(e.position.x - highCutX) < 10.0f)
        setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
    else
        setMouseCursor(juce::MouseCursor::NormalCursor);
}

void TriggerVisualizer::mouseDown(const juce::MouseEvent& e)
{
    const float xEnd = kXEnd(static_cast<float>(getWidth()));
    const float lowCutX = hzToX(lowCutHz, kXStart, xEnd);
    const float highCutX = hzToX(highCutHz, kXStart, xEnd);
    const float detailY = dbToY(detailDb, static_cast<float>(getHeight()));
    const float mirrored = static_cast<float>(getHeight()) - detailY;

    if (std::abs(e.position.y - detailY) < 10.0f || std::abs(e.position.y - mirrored) < 10.0f)
        activeDragTarget = DragTarget::Detail;
    else if (std::abs(e.position.x - lowCutX) < 12.0f)
        activeDragTarget = DragTarget::LowCut;
    else if (std::abs(e.position.x - highCutX) < 12.0f)
        activeDragTarget = DragTarget::HighCut;
    else
        activeDragTarget = DragTarget::None;
}

void TriggerVisualizer::mouseDrag(const juce::MouseEvent& e)
{
    const float xEnd = kXEnd(static_cast<float>(getWidth()));

    if (activeDragTarget == DragTarget::Detail)
    {
        detailDb = juce::jlimit(-60.0f, 0.0f, yToDb(e.position.y, static_cast<float>(getHeight())));
        if (detailParam != nullptr)
            detailParam->setValueNotifyingHost(detailParam->convertTo0to1(detailDb));
        repaint();
    }
    else if (activeDragTarget == DragTarget::LowCut)
    {
        lowCutHz = xToHz(e.position.x, kXStart, xEnd);
        lowCutHz = juce::jlimit(20.0f, std::min(2000.0f, highCutHz - 50.0f), lowCutHz);
        if (lowCutParam != nullptr)
            lowCutParam->setValueNotifyingHost(lowCutParam->convertTo0to1(lowCutHz));
        repaint();
    }
    else if (activeDragTarget == DragTarget::HighCut)
    {
        highCutHz = xToHz(e.position.x, kXStart, xEnd);
        highCutHz = juce::jlimit(std::max(200.0f, lowCutHz + 50.0f), 20000.0f, highCutHz);
        if (highCutParam != nullptr)
            highCutParam->setValueNotifyingHost(highCutParam->convertTo0to1(highCutHz));
        repaint();
    }
}

void TriggerVisualizer::mouseUp(const juce::MouseEvent&)
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

void TriggerVisualizer::fileDragEnter(const juce::StringArray&, int, int)
{
    isDragHovered = true;
    repaint();
}

void TriggerVisualizer::fileDragExit(const juce::StringArray&)
{
    isDragHovered = false;
    repaint();
}

void TriggerVisualizer::filesDropped(const juce::StringArray& files, int, int)
{
    isDragHovered = false;
    if (!files.isEmpty() && onSampleDropped)
        onSampleDropped(juce::File(files[0]));
    repaint();
}

} // namespace yeahwaah
