#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "Constants.h"
#include "GateKeeper.h"
#include "SuperSaw.h"
#include "ADSREnv.h"
#include "Blinker.h"
#include "MultiFX.h"
#include "VCAUtility.h"
#include "EnvFollower.h"
#include "Sequencer.h"
#include "MiniGateKeeper.h"
#include "Quantizer.h"
#include "MiniEnvFollower.h"
#include "MiniReverb.h"
#include "ButtonPressHelper.h"
#include "SettingsManager.h"
#include "SlewLimiter.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

DaisyPatchSM patch;
Switch       toggle;
Switch       button7;
Blinker      blinker;
ReverbSc     globalReverb; // to save buffer space this is shared between modes

// mode instances
GateKeeper  gateKeeper;
SuperSaw    superSaw;
MultiFX     multiFX;
VCAUtility  vcaUtility;
Quantizer   quantizer;
EnvFollower envFollower;
ADSREnv     adsrEnv;
SlewLimiter slewLimiter;
Sequencer   sequencer;

// "layered on top of" other modes:
MiniGateKeeper  miniGateKeeper;
MiniEnvFollower miniEnvFollower;
MiniReverb      miniReverb;

// vars to track CV values to output at DacCallback speed
uint16_t LED_OUT_LOWPRIORITY;
uint16_t CV_OUT_LOWPRIORITY;

// button helpers
ButtonPressHelper btnShortPress;
ButtonPressHelper btnLongPress;
ButtonPressHelper btnExtraLongPress;

// global persistent data
SettingsManager settingsManager;
Settings        settings;
volatile bool   shouldSave;

bool ModeHasMiniGatekeeper(int modeIdx)
{
    return modeIdx == GlobalMode::SUPERSAW || modeIdx == GlobalMode::MULTIFX
           || modeIdx == GlobalMode::VCAUTILITY
           || modeIdx == GlobalMode::ENVFOLLOWER
           || modeIdx == GlobalMode::SLEWLIMITER;
}

bool ModeHasMiniEnvFollower(int modeIdx)
{ return modeIdx == GlobalMode::SUPERSAW; }

bool ModeHasMiniReverb(int modeIdx)
{
    return modeIdx == GlobalMode::QUANTIZER || modeIdx == GlobalMode::GATEKEEPER
           || modeIdx == GlobalMode::SLEWLIMITER
           || modeIdx == GlobalMode::SEQUENCER;
}


IModuleMode *GetModeInstance(int modeIdx)
{
    switch(modeIdx)
    {
        case GlobalMode::GATEKEEPER: return &gateKeeper;
        case GlobalMode::SUPERSAW: return &superSaw;
        case GlobalMode::VCAUTILITY: return &vcaUtility;
        case GlobalMode::ENVFOLLOWER: return &envFollower;
        case GlobalMode::QUANTIZER: return &quantizer;
        case GlobalMode::ADSR: return &adsrEnv;
        case GlobalMode::SLEWLIMITER: return &slewLimiter;
        case GlobalMode::SEQUENCER: return &sequencer;
        case GlobalMode::MULTIFX: return &multiFX;
        default: return &multiFX;
    }
}

void MainAudioCallback(AudioHandle::InputBuffer  in,
                       AudioHandle::OutputBuffer out,
                       size_t                    size)
{
    patch.ProcessAllControls();

    IModuleMode *modeInstance = GetModeInstance(settings.mode);

    if(ModeHasMiniEnvFollower(settings.mode))
        miniEnvFollower.AudioCallback(in, out, size);

    if(ModeHasMiniGatekeeper(settings.mode))
        miniGateKeeper.AudioCallback(in, out, size);

    if(ModeHasMiniReverb(settings.mode))
        miniReverb.AudioCallback(in, out, size);

    modeInstance->AudioCallback(in, out, size);
}

void MainDacCallback(uint16_t **output, size_t size)
{
    patch.ProcessAllControls();
    toggle.Debounce();
    button7.Debounce();

    IModuleMode *modeInstance = GetModeInstance(settings.mode);

    // long press changes mode
    if(btnLongPress.ProcessAndCheckTrigger())
    {
        u_int8_t newMode = GetBinaryValueOfKnobs();

        if(newMode <= NUM_MODES         // skip invalid modes
           && newMode != settings.mode) // skip if it's not different
        {
            if(newMode == 0) // special favorite slot
            {
                settings.mode                 = settings.favoriteMode;
                IModuleMode *favoriteInstance = GetModeInstance(settings.mode);
                favoriteInstance->SetSubMode(settings.favoriteSubMode);
            }
            else
            {
                settings.mode = newMode;
            }

            blinker.Trigger(settings.mode);
            shouldSave = true;
        }
    }
    // extra long press saves favorite
    else if(btnExtraLongPress.ProcessAndCheckTrigger())
    {
        settings.favoriteMode    = settings.mode;
        settings.favoriteSubMode = modeInstance->GetSubMode();
        shouldSave               = true;
        // show mode blinker to confirm
        blinker.Trigger(settings.mode);
    }
    else if(btnShortPress.ProcessAndCheckTrigger())
    {
        modeInstance->OnSubModeButtonPress();
    }

    if(ModeHasMiniEnvFollower(settings.mode))
        miniEnvFollower.DacCallback(output, size);

    if(ModeHasMiniGatekeeper(settings.mode))
        miniGateKeeper.DacCallback(output, size);

    if(ModeHasMiniReverb(settings.mode))
        miniReverb.DacCallback(output, size);

    modeInstance->DacCallback(output, size);

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
    btnShortPress.Init(ButtonPressHelper::SHORT_PRESS);
    btnLongPress.Init(ButtonPressHelper::LONG_PRESS);
    btnExtraLongPress.Init(ButtonPressHelper::EXTRA_LONG_PRESS);

    // load saved settings or defaults
    settingsManager.Init();
    settingsManager.Load(settings);

    // Init the mode instances
    for(int i = 1; i <= NUM_MODES; i++)
    {
        IModuleMode *modeInstance = GetModeInstance(i);
        modeInstance->Init();
    }

    // Init mini mode layers
    miniGateKeeper.Init();
    miniEnvFollower.Init();
    miniReverb.Init();

    // wire up global reverb line
    globalReverb.Init(patch.AudioSampleRate());
    multiFX.AttachReverb(&globalReverb);
    miniReverb.AttachReverb(&globalReverb);

    // start patch stuff
    patch.StartAudio(MainAudioCallback);
    patch.StartDac(MainDacCallback);

    // show current mode
    blinker.Trigger(settings.mode);

    while(1)
    {
        if(shouldSave)
        {
            shouldSave = false;
            settingsManager.Save(settings);
        }
    }
}