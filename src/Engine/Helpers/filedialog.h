#pragma once

#include <string>
#include <vector>

namespace pg
{
    /**
     * @brief Files the player picks: the OS's dialogs on a desktop, the browser's download and file
     * picker on the web.
     *
     * On a desktop askSavePath / askOpenPath open the OS dialog (tinyfiledialogs) and return once the
     * player has chosen: the game stands still meanwhile, so a caller that counts time should not
     * count the frame after. They return "" when the player cancels, when no dialog could be shown,
     * and always in a browser.
     *
     * In a browser there is no path to ask for: offerDownload hands a file of the game's file system
     * to the browser as a download, and pickUpload opens the browser's file picker. The file chosen
     * comes later, read by the browser: pollUpload writes it into the game's file system once it has
     * come. Elsewhere these three do nothing and say so.
     */
    namespace filedialog
    {
        // True where askSavePath / askOpenPath can show a dialog (not in a browser)
        bool hasNativeDialogs();

        // `patterns` like {"*.sz"}, `description` like "Chronicle saves"
        std::string askSavePath(const std::string& title, const std::string& defaultPath, const std::vector<std::string>& patterns, const std::string& description);

        std::string askOpenPath(const std::string& title, const std::vector<std::string>& patterns, const std::string& description);

        // Browser only: the file at `path` offered as a download named `name`. False elsewhere, or
        // when the file cannot be read
        bool offerDownload(const std::string& path, const std::string& name, const std::string& mime = "application/octet-stream");

        // Browser only: opens the file picker, `accept` like ".sz". False elsewhere
        bool pickUpload(const std::string& accept);

        enum class Upload
        {
            None,      // Nothing asked, or the picker was closed with no file
            Waiting,   // The picker is open, or the browser is still reading the file
            Ready,     // The file is written at `into`: said once, then None again
            Failed     // The browser could not read it, or it could not be written: said once
        };

        // Browser only: where the file picked stands. Once it has come it is written at `into`
        Upload pollUpload(const std::string& into);

        // The name the player's file had, once it is Ready ("" before, and on a desktop)
        const std::string& uploadName();
    }
}
