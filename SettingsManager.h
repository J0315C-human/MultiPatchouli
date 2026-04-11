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

extern int currentMode;

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

    std::string getOutputValue()
    {
        // populate these 0s with other settings values
        return zeroPadNumber(currentMode) + ";" + zeroPadNumber(0) + ";"
               + zeroPadNumber(0) + ";" + zeroPadNumber(0) + ";"
               + zeroPadNumber(0) + ";" + zeroPadNumber(0) + ";"
               + zeroPadNumber(0) + ";" + zeroPadNumber(0);
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


    bool Save()
    {
        f_close(&SDFile); // close in case it was left open

        char   outbuff[512];
        size_t len;
        size_t byteswritten = 0;

        snprintf(outbuff, sizeof(outbuff), "%s", getOutputValue().c_str());

        len = strlen(outbuff);

        // Open and write the file to the SD Card.
        if(f_open(&SDFile, FILE_NAME, (FA_CREATE_ALWAYS) | (FA_WRITE)) == FR_OK)
        {
            f_write(&SDFile, outbuff, len, &byteswritten);
            f_close(&SDFile);
        }

        return (byteswritten > 0);
    }

    bool Load()
    {
        f_close(&SDFile); // close in case it was left open

        char   inbuff[512];
        size_t len       = getOutputValue().length();
        size_t bytesread = 0;

        memset(inbuff, 0, 512);

        // Read back the file from the SD Card.
        if(f_open(&SDFile, FILE_NAME, FA_READ) == FR_OK)
        {
            f_read(&SDFile, inbuff, len, &bytesread);
            f_close(&SDFile);
        }

        // Read the input buffer into a vector of ints
        std::string asString = inbuff;

        if(!bytesread)
        {
            // default to a clean data state - maybe we have nothing saved yet
            asString = getOutputValue();
        }

        std::vector<int> result;

        std::stringstream data(asString);

        std::string val;
        while(std::getline(data, val, ';'))
        {
            try
            {
                int intVal = std::stoi(val);
                result.push_back(intVal);
            }
            catch(const std::invalid_argument&)
            {
                result.push_back(0);
            }
            catch(const std::out_of_range&)
            {
                result.push_back(0);
            }
        }
        // we should have 8 int values
        if(result.size() < 8)
        {
            return false;
        }

        // write values

        // VALUE 1: mode
        currentMode = result[0];
        return true;
    }
};
