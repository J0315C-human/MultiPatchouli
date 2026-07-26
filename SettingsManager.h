#pragma once
#include <stdio.h>
#include <string.h>
#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "fatfs.h"
#include "Constants.h"

#define FILE_NAME "settings.txt"

using namespace daisy;
using namespace patch_sm;

struct Settings
{
    int mode;
    int effectMode;
    int superSawMode;
    int quantizePage;
    int favoriteMode;
    int favoriteSubMode;
    int reserved6;
    int reserved7;

    // default values
    Settings()
    : mode(1),
      effectMode(0),
      superSawMode(0),
      quantizePage(0),
      favoriteMode(1),
      favoriteSubMode(0),
      reserved6(0),
      reserved7(0)
    {
    }
};

class SettingsManager
{
  private:
    SdmmcHandler   sd;
    FatFSInterface fsi;
    FIL            SDFile;

    void Serialize(const Settings& s, char* buf, size_t bufSize)
    {
        snprintf(buf,
                 bufSize,
                 "%03d;%03d;%03d;%03d;%03d;%03d;%03d;%03d",
                 s.mode,
                 s.effectMode,
                 s.superSawMode,
                 s.quantizePage,
                 s.favoriteMode,
                 s.favoriteSubMode,
                 s.reserved6,
                 s.reserved7);
    }

    bool Deserialize(const char* data, Settings& out)
    {
        int vals[8] = {0};
        int parsed  = sscanf(data,
                             "%d;%d;%d;%d;%d;%d;%d;%d",
                             &vals[0],
                             &vals[1],
                             &vals[2],
                             &vals[3],
                             &vals[4],
                             &vals[5],
                             &vals[6],
                             &vals[7]);

        if(parsed < 8)
            return false;

        out.mode            = vals[0];
        out.effectMode      = vals[1];
        out.superSawMode    = vals[2];
        out.quantizePage    = vals[3];
        out.favoriteMode    = vals[4];
        out.favoriteSubMode = vals[5];
        out.reserved6       = vals[6];
        out.reserved7       = vals[7];

        if(out.mode < 1 || out.mode > NUM_MODES)
            out.mode = 1;
        if(out.favoriteMode < 1 || out.favoriteMode > NUM_MODES)
            out.favoriteMode = 1;
        return true;
    }

  public:
    SettingsManager() {}
    ~SettingsManager() {}

    bool Init()
    {
        // Init SD Card
        SdmmcHandler::Config sd_cfg;
        sd_cfg.Defaults();
        sd.Init(sd_cfg);

        // Links libdaisy i/o to fatfs driver.
        fsi.Init(FatFSInterface::Config::MEDIA_SD);

        // Mount SD Card
        FRESULT mountResult = f_mount(&fsi.GetSDFileSystem(), "/", 1);
        return mountResult == FR_OK;
    }

    bool Save(const Settings& settings)
    {
        f_close(&SDFile); // close in case it was left open

        char   outbuff[128];
        size_t byteswritten = 0;

        Serialize(settings, outbuff, sizeof(outbuff));
        size_t len = strlen(outbuff);

        // Open and write the file to the SD Card.
        if(f_open(&SDFile, FILE_NAME, (FA_CREATE_ALWAYS) | (FA_WRITE)) == FR_OK)
        {
            f_write(&SDFile, outbuff, len, &byteswritten);
            f_close(&SDFile);
        }

        return (byteswritten > 0);
    }

    bool Load(Settings& settings)
    {
        f_close(&SDFile); // close in case it was left open

        char   inbuff[128];
        size_t bytesread = 0;

        memset(inbuff, 0, sizeof(inbuff));

        // Read back the file from the SD Card.
        if(f_open(&SDFile, FILE_NAME, FA_READ) == FR_OK)
        {
            f_read(&SDFile, inbuff, sizeof(inbuff) - 1, &bytesread);
            f_close(&SDFile);
        }

        if(!bytesread)
        {
            // default to a clean data state - maybe we have nothing saved yet
            Settings defaults;
            Serialize(defaults, inbuff, sizeof(inbuff));
        }

        return Deserialize(inbuff, settings);
    }
};