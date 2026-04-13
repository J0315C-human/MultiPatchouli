#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "GateKeeper.h"
#include "SuperSaw.h"
#include "Blinker.h"
#include "MultiFX.h"
#include "VCAUtility.h"
#include "EnvFollower.h"
#include "MiniGateKeeper.h"
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

constexpr int NUM_MODES = 5;

// MODE ORDER:
enum GlobalMode
{
    SUPERSAW,
    MULTIFX,
    VCAUTILITY,
    ENVFOLLOWER,
    GATEKEEPER
};

GateKeeper  gateKeeper;
SuperSaw    superSaw;
MultiFX     multiFX;
VCAUtility  vcaUtility;
EnvFollower envFollower;

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
volatile bool     shouldSave;

void MainAudioCallback(AudioHandle::InputBuffer  in,
                       AudioHandle::OutputBuffer out,
                       size_t                    size)
{
    patch.ProcessAllControls();

    switch(settings.mode)
    {
        case GlobalMode::GATEKEEPER:
        {
            gateKeeper.AudioCallback(in, out, size);
            break;
        }
        case GlobalMode::SUPERSAW:
        {
            miniEnvFollower.AudioCallback(in, out, size);
            miniGateKeeper.AudioCallback(in, out, size);
            superSaw.AudioCallback(in, out, size);
            break;
        }
        case GlobalMode::MULTIFX:
        {
            miniGateKeeper.AudioCallback(in, out, size);
            multiFX.AudioCallback(in, out, size);
            break;
        }
        case GlobalMode::VCAUTILITY:
        {
            miniGateKeeper.AudioCallback(in, out, size);
            vcaUtility.AudioCallback(in, out, size);
            break;
        }
        case GlobalMode::ENVFOLLOWER:
        {
            envFollower.AudioCallback(in, out, size);
            break;
        }
    }
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
        shouldSave = true;
    }

    switch(settings.mode)
    {
        case GlobalMode::GATEKEEPER:
        {
            gateKeeper.DacCallback(output, size);
            break;
        }
        case GlobalMode::SUPERSAW:
        {
            miniEnvFollower.DacCallback(output, size);
            miniGateKeeper.DacCallback(output, size);
            superSaw.DacCallback(output, size);
            break;
        }
        case GlobalMode::MULTIFX:
        {
            // short press changes fx mode when in fx mode
            if(btnShortPress.ProcessAndCheckTrigger())
            {
                settings.effectMode
                    = (settings.effectMode + 1) % MultiFX::NUM_FX_MODES;
                shouldSave = true;
            }

            miniGateKeeper.DacCallback(output, size);
            multiFX.DacCallback(output, size);
            break;
        }
        case GlobalMode::VCAUTILITY:
        {
            miniGateKeeper.DacCallback(output, size);
            vcaUtility.DacCallback(output, size);
            break;
        }
        case GlobalMode::ENVFOLLOWER:
        {
            envFollower.DacCallback(output, size);
            break;
        }
    }

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
    blinker.Init(48000); // MAGIC NUMBER
    btnLongPress.Init(ButtonPressHelper::LONG_PRESS);
    btnShortPress.Init(ButtonPressHelper::SHORT_PRESS);
    settings.mode = 0;

    // Init the Module mode classes
    gateKeeper.Init();
    superSaw.Init();
    multiFX.Init();
    vcaUtility.Init();
    envFollower.Init();
    miniGateKeeper.Init();
    miniEnvFollower.Init();

    // load saved settings or defaults
    settingsManager.Init();
    settingsManager.Load(settings);

    // start patch stuff
    patch.StartAudio(MainAudioCallback);
    patch.StartDac(MainDacCallback);

    // show current mode
    blinker.Trigger(settings.mode + 1);

    while(1)
    {
        if(shouldSave)
        {
            shouldSave = false;
            settingsManager.Save(settings);
        }
    }
}