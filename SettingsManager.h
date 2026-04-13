#pragma once
#include <stdio.h>
#include <string.h>
#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "fatfs.h"

#define FILE_NAME "settings.txt"

using namespace daisy;
using namespace patch_sm;

struct Settings
{
    int mode;
    int reserved1;
    int reserved2;
    int reserved3;
    int reserved4;
    int reserved5;
    int reserved6;
    int reserved7;

    // default values
    Settings()
    : mode(0),
      reserved1(0),
      reserved2(0),
      reserved3(0),
      reserved4(0),
      reserved5(0),
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
                 s.reserved1,
                 s.reserved2,
                 s.reserved3,
                 s.reserved4,
                 s.reserved5,
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

        out.mode      = vals[0];
        out.reserved1 = vals[1];
        out.reserved2 = vals[2];
        out.reserved3 = vals[3];
        out.reserved4 = vals[4];
        out.reserved5 = vals[5];
        out.reserved6 = vals[6];
        out.reserved7 = vals[7];

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

        char   outbuff[512];
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

        char   inbuff[512];
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