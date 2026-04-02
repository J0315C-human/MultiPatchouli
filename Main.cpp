#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "GateKeeper.h"
#include "SuperSaw.h"
#include "Blinker.h"
#include "Reverb.h"
#include "VCAUtility.h"
#include "EnvFollower.h"
#include "MiniGateKeeper.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

DaisyPatchSM patch;
Switch       toggle;
Switch       button;
Blinker      blinker;

constexpr int NUM_MODES = 5;

// MODE INDEX:
// 0: GateKeeper
GateKeeper gateKeeper;
// 1: SuperSaw
SuperSaw superSaw;
// 2: Reverb
Reverb reverb;
// 3: VCAUtility
VCAUtility vcaUtility;
// 4: EnvFollower
EnvFollower envFollower;

// "layered on top of" other modes:
MiniGateKeeper miniGateKeeper;

int      currentMode;
uint16_t LED_OUT_LOWPRIORITY;
uint16_t CV_OUT_LOWPRIORITY;

void MainAudioCallback(AudioHandle::InputBuffer  in,
                       AudioHandle::OutputBuffer out,
                       size_t                    size)
{
    patch.ProcessAllControls();

    switch(currentMode)
    {
        case 0:
        {
            gateKeeper.AudioCallback(in, out, size);
            break;
        }
        case 1:
        {
            miniGateKeeper.AudioCallback(in, out, size);
            superSaw.AudioCallback(in, out, size);
            break;
        }
        case 2:
        {
            miniGateKeeper.AudioCallback(in, out, size);
            reverb.AudioCallback(in, out, size);
            break;
        }
        case 3:
        {
            miniGateKeeper.AudioCallback(in, out, size);
            vcaUtility.AudioCallback(in, out, size);
            break;
        }
        case 4:
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
    button.Debounce();

    if(button.RisingEdge())
    {
        currentMode = (currentMode + 1) % NUM_MODES;

        blinker.Trigger(currentMode + 1);
    }


    switch(currentMode)
    {
        case 0:
        {
            gateKeeper.DacCallback(output, size);
            break;
        }
        case 1:
        {
            miniGateKeeper.DacCallback(output, size);
            superSaw.DacCallback(output, size);
            break;
        }
        case 2:
        {
            miniGateKeeper.DacCallback(output, size);
            reverb.DacCallback(output, size);
            break;
        }
        case 3:
        {
            miniGateKeeper.DacCallback(output, size);
            vcaUtility.DacCallback(output, size);
            break;
        }
        case 4:
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
    button.Init(patch.B7);
    blinker.Init(48000);

    currentMode = 0;

    // Init the Module mode classes
    gateKeeper.Init();
    superSaw.Init();
    reverb.Init();
    vcaUtility.Init();
    envFollower.Init();
    miniGateKeeper.Init();

    patch.StartAudio(MainAudioCallback);
    patch.StartDac(MainDacCallback);

    blinker.Trigger(currentMode);

    while(1) {}
}