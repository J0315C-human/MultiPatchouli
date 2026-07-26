#pragma once

// MODE ORDER:
enum GlobalMode
{
    SUPERSAW = 1,
    MULTIFX,
    VCAUTILITY,
    ENVFOLLOWER,
    ADSR,
    QUANTIZER,
    GATEKEEPER,
};

static constexpr int NUM_MODES = 7;

/* this was found with trial-and-error with my particular unit. */
static constexpr float CALIBRATE_VOCT = 0.9790673f;

// Good "average" values to use for enveloper followers
static constexpr float ENV_ATT   = 20.f;
static constexpr float ENV_REL   = 90.f;
static constexpr float ENV_SCALE = 2.f;

