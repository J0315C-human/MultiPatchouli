#pragma once
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include <iomanip>
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
    Settings() : mode(0), reserved1(0), reserved2(0), reserved3(0),
                 reserved4(0), reserved5(0), reserved6(0), reserved7(0) {}
};

class SettingsManager
{
  private:
    SdmmcHandler   sd;
    FatFSInterface fsi;
    FIL            SDFile;

    std::string zeroPadNumber(int num)
    {
        std::ostringstream ss;
        ss << std::setw(3) << std::setfill('0') << num;
        return ss.str();
    }

    std::string Serialize(const Settings& s)
    {
        return zeroPadNumber(s.mode)       + ";"
             + zeroPadNumber(s.reserved1)  + ";"
             + zeroPadNumber(s.reserved2)  + ";"
             + zeroPadNumber(s.reserved3)  + ";"
             + zeroPadNumber(s.reserved4)  + ";"
             + zeroPadNumber(s.reserved5)  + ";"
             + zeroPadNumber(s.reserved6)  + ";"
             + zeroPadNumber(s.reserved7);
    }

    bool Deserialize(const std::string& data, Settings& out)
    {
        std::vector<int> result;
        std::stringstream ss(data);
        std::string val;

        while(std::getline(ss, val, ';'))
        {
            try
            {
                result.push_back(std::stoi(val));
            }
            catch(const std::invalid_argument&) { result.push_back(0); }
            catch(const std::out_of_range&)     { result.push_back(0); }
        }

        if(result.size() < 8)
            return false;

        out.mode      = result[0];
        out.reserved1 = result[1];
        out.reserved2 = result[2];
        out.reserved3 = result[3];
        out.reserved4 = result[4];
        out.reserved5 = result[5];
        out.reserved6 = result[6];
        out.reserved7 = result[7];

        return true;
    }

  public:
    SettingsManager() {}
    ~SettingsManager() {}

    void Init()
    {
        // Init SD Card
        SdmmcHandler::Config sd_cfg;
        sd_cfg.Defaults();
        sd.Init(sd_cfg);

        // Links libdaisy i/o to fatfs driver.
        fsi.Init(FatFSInterface::Config::MEDIA_SD);

        // Mount SD Card
        f_mount(&fsi.GetSDFileSystem(), "/", 1);
    }

    bool Save(const Settings& settings)
    {
        f_close(&SDFile); // close in case it was left open

        char   outbuff[512];
        size_t byteswritten = 0;

        snprintf(outbuff, sizeof(outbuff), "%s", Serialize(settings).c_str());

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

        memset(inbuff, 0, 512);

        // Read back the file from the SD Card.
        if(f_open(&SDFile, FILE_NAME, FA_READ) == FR_OK)
        {
            f_read(&SDFile, inbuff, Serialize(settings).length(), &bytesread);
            f_close(&SDFile);
        }

        std::string asString = bytesread ? inbuff : Serialize(Settings());

        return Deserialize(asString, settings);
    }
};