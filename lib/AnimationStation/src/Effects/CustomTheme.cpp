#include "CustomTheme.hpp"

#define PRESS_COOLDOWN_INCREMENT   500
#define PRESS_COOLDOWN_MAX         5000
#define PRESS_COOLDOWN_MIN         0

std::map<uint32_t, RGB> CustomTheme::theme;
std::map<uint32_t, int32_t> CustomTheme::times = {};
std::map<uint32_t, RGB> CustomTheme::hitColor = {};

CustomTheme::CustomTheme(PixelMatrix &matrix) : Animation(matrix) {
    for (size_t r = 0; r != matrix.pixels.size(); r++) {
        for (size_t c = 0; c != matrix.pixels[r].size(); c++) {
            if (matrix.pixels[r][c].index == NO_PIXEL.index)
                continue;
            times.insert_or_assign(matrix.pixels[r][c].index, 0);
            hitColor.insert_or_assign(matrix.pixels[r][c].index, defaultColor);            
        }
    }
}

void CustomTheme::UpdatePixels(std::vector<Pixel> inpixels) {
  this->pixels = inpixels;
}

void CustomTheme::Animate(RGB (&frame)[100]) {
    coolDownTimeInMs = AnimationStation::options.customThemeCooldownTimeInMs;

    absolute_time_t currentTime = get_absolute_time();
    int64_t updateTimeInMs = absolute_time_diff_us(lastUpdateTime, currentTime) / 1000;
    lastUpdateTime = currentTime;

    for (size_t p = 0; p < pixels.size(); p++) {
        if (pixels[p].index != NO_PIXEL.index) {
            times[pixels[p].index] = coolDownTimeInMs;               
            hitColor[pixels[p].index] = frame[pixels[p].positions[0]];       
        }
    }

    for (size_t r = 0; r != matrix->pixels.size(); r++) {
        for (size_t c = 0; c != matrix->pixels[r].size(); c++) {
            if (matrix->pixels[r][c].index == NO_PIXEL.index)
                continue;

            // Count down the timer
            times[matrix->pixels[r][c].index] -= updateTimeInMs;
            if (times[matrix->pixels[r][c].index] < 0) {
                times[matrix->pixels[r][c].index] = 0;
            };

            auto itr = theme.find(matrix->pixels[r][c].mask);
            if (itr != theme.end()) {
                for (size_t p = 0; p != matrix->pixels[r][c].positions.size(); p++) {
                    // Interpolate from hitColor (color the button was assigned when pressed) back to the theme color
                    frame[matrix->pixels[r][c].positions[p]] = BlendColor(hitColor[matrix->pixels[r][c].index], itr->second, times[matrix->pixels[r][c].index]);
                }
            } else {
                for (size_t p = 0; p != matrix->pixels[r][c].positions.size(); p++) {
                    frame[matrix->pixels[r][c].positions[p]] = defaultColor;
                }
            }
        }
    }
}

bool CustomTheme::HasTheme() {
    return CustomTheme::theme.size() > 0;
}

void CustomTheme::SetCustomTheme(std::map<uint32_t, RGB> customTheme) {
    CustomTheme::theme = customTheme;
    AnimationStation::effectCount = TOTAL_EFFECTS + 1;
}

void CustomTheme::ParameterUp() {
    AnimationStation::options.customThemeCooldownTimeInMs = AnimationStation::options.customThemeCooldownTimeInMs + PRESS_COOLDOWN_INCREMENT;

    if (AnimationStation::options.customThemeCooldownTimeInMs > PRESS_COOLDOWN_MAX) {
        AnimationStation::options.customThemeCooldownTimeInMs = PRESS_COOLDOWN_MAX;
    }
}

void CustomTheme::ParameterDown() {
    AnimationStation::options.customThemeCooldownTimeInMs = AnimationStation::options.customThemeCooldownTimeInMs - PRESS_COOLDOWN_INCREMENT;

    if (AnimationStation::options.customThemeCooldownTimeInMs > PRESS_COOLDOWN_MAX) {
        AnimationStation::options.customThemeCooldownTimeInMs = PRESS_COOLDOWN_MIN;
    }
}

RGB CustomTheme::BlendColor(RGB start, RGB end, uint32_t timeRemainingInMs) {
    RGB result = ColorBlack;

    if (timeRemainingInMs <= 0) {
        return end;
    }

    float progress = 1.0f - (static_cast<float>(timeRemainingInMs) / static_cast<float>(coolDownTimeInMs));
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    result.r = static_cast<uint32_t>(static_cast<float>(start.r + (end.r - start.r) * progress));
    result.g = static_cast<uint32_t>(static_cast<float>(start.g + (end.g - start.g) * progress));
    result.b = static_cast<uint32_t>(static_cast<float>(start.b + (end.b - start.b) * progress));

    return result;
}
