#include "VCAUtility.h"
#include "daisysp-lgpl.h"

extern DaisyPatchSM patch;
extern uint16_t     CV_OUT_LOWPRIORITY;

VCAUtility::VCAUtility() {}
VCAUtility::~VCAUtility() {}

void VCAUtility::Init() {}

void VCAUtility::AudioCallback(AudioHandle::InputBuffer  in,
                               AudioHandle::OutputBuffer out,
                               size_t                    size)
{
    // Process audio
    float lvl_audio_L = patch.GetAdcValue(CV_1); // range is (0, 1)
    float lvl_audio_R = patch.GetAdcValue(CV_2); // range is (0, 1)
    float cv_amt_L    = fmap(patch.GetAdcValue(CV_3), -1, 1);
    float cv_amt_R    = fmap(patch.GetAdcValue(CV_4), -1, 1);
    float cv_ctrl_L   = patch.GetAdcValue(CV_5); // range is (-1, 1)
    float cv_ctrl_R   = patch.GetAdcValue(CV_6); // range is (-1, 1)

    float scaleL = (cv_amt_L * cv_ctrl_L) + lvl_audio_L;
    float scaleR = (cv_amt_R * cv_ctrl_R) + lvl_audio_R;

    scaleL = DSY_CLAMP(scaleL, 0.f, 1.f);
    scaleR = DSY_CLAMP(scaleR, 0.f, 1.f);
    for(size_t i = 0; i < size; i++)
    {
        OUT_L[i] = IN_L[i] * scaleL;
        OUT_R[i] = IN_R[i] * scaleR;
    }

    // set CV output (inputs are bipolar, outputs are unipolar ugh)
    float cv_signal_in = fmap(patch.GetAdcValue(CV_7), 0.f, 5.f);
    float cv_ctrl_CV   = patch.GetAdcValue(CV_8);

    CV_OUT_LOWPRIORITY = VoltageToCvValue(cv_signal_in * cv_ctrl_CV);
}