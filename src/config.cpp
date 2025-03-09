#include "config.h"
#include "globals.h"
#include "utils.h"

void InitializeConfig()
{
    ::KeycastConfig.maxSize = 83;

    // Set up font
    if (!IsFontExists(::defaultFont))
    {
        ::fontName = ::backupFont;
    }
    else
    {
        ::fontName = ::defaultFont;
    }
}