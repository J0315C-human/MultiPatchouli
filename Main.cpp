#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "GateKeeper.h"
#include "SuperSaw.h"
#include "ADSREnv.h"
#include "Blinker.h"
#include "MultiFX.h"
#include "VCAUtility.h"
#include "EnvFollower.h"
#include "MiniGateKeeper.h"
#include "Quantizer.h"
#include "MiniEnvFollower.h"
#include "ButtonPressHelper.h"
#include "SettingsManager.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

DaisyPatchSM patch;
Switch       toggle;
Switch       button7;
Blinker      blinker;

constexpr int NUM_MODES = 7;

// MODE ORDER:
enum GlobalMode
{
    SUPERSAW,
    MULTIFX,
    VCAUTILITY,
    ENVFOLLOWER,
    ADSR,
    QUANTIZER,
    GATEKEEPER,
};

GateKeeper  gateKeeper;
SuperSaw    superSaw;
MultiFX     multiFX;
VCAUtility  vcaUtility;
Quantizer   quantizer;
EnvFollower envFollower;
ADSREnv     adsrEnv;

// "layered on top of" other modes:
MiniGateKeeper  miniGateKeeper;
MiniEnvFollower miniEnvFollower;

// vars to track CV values to output at DacCallback speed
uint16_t LED_OUT_LOWPRIORITY;
uint16_t CV_OUT_LOWPRIORITY;

// Helpers / global data
ButtonPressHelper btnLongPress;
ButtonPressHelper btnShortPress;
SettingsManager   settingsManager;
Settings          settings;

bool CurrentModeHasMiniGatekeeper()
{
    return settings.mode == GlobalMode::SUPERSAW
           || settings.mode == GlobalMode::MULTIFX
           || settings.mode == GlobalMode::VCAUTILITY;
}

bool CurrentModeHasMiniEnvFollower()
{ return settings.mode == GlobalMode::SUPERSAW; }

IModuleMode *GetCurrentModeInstance()
{
    switch(settings.mode)
    {
        case GlobalMode::GATEKEEPER: return &gateKeeper;
        case GlobalMode::SUPERSAW: return &superSaw;
        case GlobalMode::VCAUTILITY: return &vcaUtility;
        case GlobalMode::ENVFOLLOWER: return &envFollower;
        case GlobalMode::QUANTIZER: return &quantizer;
        case GlobalMode::ADSR: return &adsrEnv;
        default: return &multiFX;
    }
}

void MainAudioCallback(AudioHandle::InputBuffer  in,
                       AudioHandle::OutputBuffer out,
                       size_t                    size)
{
    patch.ProcessAllControls();

    IModuleMode *modeInstance = GetCurrentModeInstance();

    modeInstance->AudioCallback(in, out, size);

    if(CurrentModeHasMiniEnvFollower())
        miniEnvFollower.AudioCallback(in, out, size);

    if(CurrentModeHasMiniGatekeeper())
        miniGateKeeper.AudioCallback(in, out, size);
}

void MainDacCallback(uint16_t **output, size_t size)
{
    patch.ProcessAllControls();
    toggle.Debounce();
    button7.Debounce();

    // long press changes mode
    if(btnLongPress.ProcessAndCheckTrigger())
    {
        settings.mode = (settings.mode + 1) % NUM_MODES;
        blinker.Trigger(settings.mode + 1);
        settings.shouldSave = true;
    }

    IModuleMode *modeInstance = GetCurrentModeInstance();

    if(btnShortPress.ProcessAndCheckTrigger())
        modeInstance->OnSubmodeButtonPress();

    modeInstance->DacCallback(output, size);

    if(CurrentModeHasMiniEnvFollower())
        miniEnvFollower.DacCallback(output, size);

    if(CurrentModeHasMiniGatekeeper())
        miniGateKeeper.DacCallback(output, size);

    // set LED and cv out value, giving "Blinker" higher priority
    for(size_t i = 0; i < size; i++)
    {
        blinker.Process();
        uint16_t ledVal = blinker.IsActive() ? 4095.f * blinker.State()
                                             : LED_OUT_LOWPRIORITY;

        output[0][i] = CV_OUT_LOWPRIORITY;
        output[1][i] = ledVal;
    }
}

int main(void)
{
    patch.Init();
    toggle.Init(patch.B8);
    button7.Init(patch.B7);
    blinker.Init(48000); // MAGIC NUM
    btnLongPress.Init(ButtonPressHelper::LONG_PRESS);
    btnShortPress.Init(ButtonPressHelper::SHORT_PRESS);

    // load saved settings or defaults
    settingsManager.Init();
    settingsManager.Load(settings);

    // Init the Module mode classes
    gateKeeper.Init();
    superSaw.Init();
    multiFX.Init();
    vcaUtility.Init();
    envFollower.Init();
    miniGateKeeper.Init();
    miniEnvFollower.Init();
    quantizer.Init();
    adsrEnv.Init();

    // start patch stuff
    patch.StartAudio(MainAudioCallback);
    patch.StartDac(MainDacCallback);

    // show current mode
    blinker.Trigger(settings.mode + 1);

    while(1)
    {
        if(settings.shouldSave)
        {
            settingsManager.Save(settings);
            settings.shouldSave = false;
        }
    }
}