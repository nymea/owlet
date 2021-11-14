#ifndef GPIOCONTROLLER_H
#define GPIOCONTROLLER_H

#include <ustd_map.h>
#include <ustd_functional.h>

typedef ustd::function<void(int id, bool power)> PowerChangedHandlerFunction;

#ifdef USE_WS2812FX
#include <WS2812FX.h>
typedef ustd::function<void(int id, int brightness)> BrightnessChangedHandlerFunction;
typedef ustd::function<void(int id, int color)> ColorChangedHandlerFunction;
typedef ustd::function<void(int id, int color)> EffectChangedHandlerFunction;
#endif


class GPIOController
{
public:
    enum GPIOError {
        GPIOErrorNoError,
        GPIOErrorUnconfigured,
        GPIOErrorConfigurationMismatch
    };

    enum PinMode{
        PinModeUnconfigured = 0,
        PinModeGPIOInput,
        PinModeGPIOOutput,
#ifdef USE_WS2812FX
        PinModeWS2812,
#endif
    };

#ifdef USE_WS2812FX
    enum WS2812Mode {
        WS2812ModeRGB = NEO_RGB,
        WS2812ModeRBG = NEO_RBG,
        WS2812ModeGRB = NEO_GRB,
        WS2812ModeGBR = NEO_GBR,
        WS2812ModeBRG = NEO_BRG,
        WS2812ModeBGR = NEO_BGR,

        WS2812ModeWRGB = NEO_WRGB,
        WS2812ModeWRBG = NEO_WRBG,
        WS2812ModeWGRB = NEO_WGRB,
        WS2812ModeWGBR = NEO_WGBR,
        WS2812ModeWBRG = NEO_WBRG,
        WS2812ModeWBGR = NEO_WBGR,

        WS2812ModeRWGB = NEO_RWGB,
        WS2812ModeRWBG = NEO_RWBG,
        WS2812ModeRGWB = NEO_RGWB,
        WS2812ModeRGBW = NEO_RGBW,
        WS2812ModeRBWG = NEO_RBWG,
        WS2812ModeRBGW = NEO_RBGW,

        WS2812ModeGWRB = NEO_GWRB,
        WS2812ModeGWBR = NEO_GWBR,
        WS2812ModeGRWB = NEO_GRWB,
        WS2812ModeGRBW = NEO_GRBW,
        WS2812ModeGBWR = NEO_GBWR,
        WS2812ModeGBRW = NEO_GBRW,

        WS2812ModeBWRG = NEO_BWRG,
        WS2812ModeBWGR = NEO_BWGR,
        WS2812ModeBRWG = NEO_BRWG,
        WS2812ModeBRGW = NEO_BRGW,
        WS2812ModeBGWR = NEO_BGWR,
        WS2812ModeBGRW = NEO_BGRW
    };
    enum WS2812Clock {
        WS2812Clock400kHz = NEO_KHZ400,
        WS2812Clock800kHz = NEO_KHZ800
    };
#endif

    GPIOController();

    int gpioCount() const;

    GPIOError configurePin(int id, PinMode mode);
    PinMode getPinMode(int id) const;

    GPIOError setGPIOPower(int id, bool power);
    void onPowerChanged(PowerChangedHandlerFunction callback);

#if USE_WS2812FX
    GPIOError configureWS2812(int id, int ledCount, WS2812Mode mode, WS2812Clock clock);
    GPIOError setWs2812Power(int id, bool power);
    GPIOError setWs2812Brightness(int id, int brightness);
    GPIOError setWs2812Color(int id, int color);
    GPIOError setWs2812Effect(int id, int effect);

    void onWs2812BrightnessChanged(BrightnessChangedHandlerFunction callback);
    void onWs2812ColorChanged(ColorChangedHandlerFunction callback);
    void onWs2812EffectChanged(EffectChangedHandlerFunction callback);
#endif

    void loop();

private:

    ustd::map<int, PinMode> m_pinModes;
    ustd::map<int, int> m_inputStates;

    ustd::array<PowerChangedHandlerFunction> m_powerChangedHandlers;

#ifdef USE_WS2812FX
    ustd::map<int, WS2812FX*> m_ws2812map;
    ustd::array<BrightnessChangedHandlerFunction> m_brightnessChangedHandlers;
    ustd::array<ColorChangedHandlerFunction> m_colorChangedHandlers;
    ustd::array<EffectChangedHandlerFunction> m_effectChangedHandlers;
#endif
};


#endif
