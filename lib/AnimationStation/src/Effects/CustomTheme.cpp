#include "CustomTheme.hpp"

std::map<uint32_t, RGB> CustomTheme::theme;
std::map<uint32_t, int32_t> CustomTheme::times = {};
std::map<uint32_t, RGB> CustomTheme::hitColor = {};

CustomTheme::CustomTheme(PixelMatrix &matrix) : Animation(matrix) {
    for (size_t r = 0; r != matrix.pixels.size(); r++) {
        for (size_t c = 0; c != matrix.pixels[r].size(); c++) {
            if (matrix.pixels[r][c].index == NO_PIXEL.index)
                continue;
            times.insert_or_assign(matrix.pixels[r][c].index, 0);
        }
    }
}

void CustomTheme::UpdatePixels(std::vector<Pixel> inpixels) {
  this->pixels = inpixels;
}

void CustomTheme::Animate(RGB (&frame)[100]) {

    coolDownTimeInMs = AnimationStation::options.customThemeCooldownTimeInMs;

    for (size_t p = 0; p < pixels.size(); p++) {
        if (pixels[p].index != NO_PIXEL.index) {
            hitColor[pixels[p].index] = frame[pixels[p].positions[0]];
            times[pixels[p].index] = coolDownTimeInMs;            
        }
    }

    if (!time_reached(this->nextRunTime)) {
        return;
    }


    for (size_t r = 0; r != matrix->pixels.size(); r++) {
        for (size_t c = 0; c != matrix->pixels[r].size(); c++) {
            if (matrix->pixels[r][c].index == NO_PIXEL.index)
                continue;

            auto itr = theme.find(matrix->pixels[r][c].mask);
            if (itr != theme.end()) {
                for (size_t p = 0; p != matrix->pixels[r][c].positions.size(); p++) {
                    
                    // Count down the timer
                    times[matrix->pixels[r][c].index]-=updateTimeInms;
                    if (times[matrix->pixels[r][c].index] < 0) {
                        times[matrix->pixels[r][c].index] = 0;
                    };

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

    this->nextRunTime = make_timeout_time_ms(updateTimeInms);
}

bool CustomTheme::HasTheme() {
    return CustomTheme::theme.size() > 0;
}

void CustomTheme::SetCustomTheme(std::map<uint32_t, RGB> customTheme) {
    CustomTheme::theme = customTheme;
    AnimationStation::effectCount = TOTAL_EFFECTS + 1;
}

#define PRESS_COOLDOWN_INCREMENT   500
#define PRESS_COOLDOWN_MAX         10000
#define PRESS_COOLDOWN_MIN         10
 
void CustomTheme::ParameterUp() {
    if (AnimationStation::options.customThemeCooldownTimeInMs + PRESS_COOLDOWN_INCREMENT < PRESS_COOLDOWN_MAX) {
        AnimationStation::options.customThemeCooldownTimeInMs = AnimationStation::options.customThemeCooldownTimeInMs + PRESS_COOLDOWN_INCREMENT;
    } else {
        AnimationStation::options.customThemeCooldownTimeInMs = PRESS_COOLDOWN_MAX;
    }
}

void CustomTheme::ParameterDown() {
    if (AnimationStation::options.customThemeCooldownTimeInMs - PRESS_COOLDOWN_INCREMENT > PRESS_COOLDOWN_MIN) {
        AnimationStation::options.customThemeCooldownTimeInMs = AnimationStation::options.customThemeCooldownTimeInMs - PRESS_COOLDOWN_INCREMENT;
    } else {
        AnimationStation::options.customThemeCooldownTimeInMs = PRESS_COOLDOWN_MIN;
    }
}

RGB CustomTheme::BlendColor(RGB start, RGB end, uint32_t frame) {
    RGB result = ColorBlack;
    float progress = 1.0f - (static_cast<float>(frame) / static_cast<float>(coolDownTimeInMs));
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    result.r = static_cast<uint32_t>(static_cast<float>(start.r + (end.r - start.r) * progress));
    result.g = static_cast<uint32_t>(static_cast<float>(start.g + (end.g - start.g) * progress));
    result.b = static_cast<uint32_t>(static_cast<float>(start.b + (end.b - start.b) * progress));

    return result;
}
