#include "stdafx.h"

#include "filedialog.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>

#include "logger.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#include "tinyfiledialogs.h"
#endif

namespace pg
{
    namespace filedialog
    {
        namespace
        {
            constexpr const char * const DOM = "File Dialog";

            std::string lastUploadName;

#ifndef __EMSCRIPTEN__
            std::vector<const char*> cStrings(const std::vector<std::string>& strings)
            {
                std::vector<const char*> pointers;

                for (const auto& s : strings)
                    pointers.push_back(s.c_str());

                return pointers;
            }
#endif
        }

        bool hasNativeDialogs()
        {
#ifdef __EMSCRIPTEN__
            return false;
#else
            return true;
#endif
        }

        std::string askSavePath(const std::string& title, const std::string& defaultPath, const std::vector<std::string>& patterns, const std::string& description)
        {
#ifdef __EMSCRIPTEN__
            (void)title; (void)defaultPath; (void)patterns; (void)description;
            return "";
#else
            auto filters = cStrings(patterns);

            const char* chosen = tinyfd_saveFileDialog(title.c_str(), defaultPath.c_str(), static_cast<int>(filters.size()), filters.empty() ? nullptr : filters.data(),
                                                       description.empty() ? nullptr : description.c_str());

            return chosen ? std::string(chosen) : std::string();
#endif
        }

        std::string askOpenPath(const std::string& title, const std::vector<std::string>& patterns, const std::string& description)
        {
#ifdef __EMSCRIPTEN__
            (void)title; (void)patterns; (void)description;
            return "";
#else
            auto filters = cStrings(patterns);

            const char* chosen = tinyfd_openFileDialog(title.c_str(), "", static_cast<int>(filters.size()), filters.empty() ? nullptr : filters.data(),
                                                       description.empty() ? nullptr : description.c_str(), 0);

            return chosen ? std::string(chosen) : std::string();
#endif
        }

        bool offerDownload(const std::string& path, const std::string& name, const std::string& mime)
        {
#ifdef __EMSCRIPTEN__
            std::ifstream file(path, std::ios::binary);

            if (not file)
            {
                LOG_ERROR(DOM, "Cannot read " << path << " to offer it");
                return false;
            }

            const std::vector<char> bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

            // The page's own thread holds the document: the bytes are copied out of the game's
            // memory there (slice: a plain buffer, which a Blob takes, where the memory may be shared)
            MAIN_THREAD_EM_ASM({
                var bytes = HEAPU8.slice($0, $0 + $1);
                var blob = new Blob([bytes], { type: UTF8ToString($3) });
                var url = URL.createObjectURL(blob);
                var link = document.createElement('a');
                link.href = url;
                link.download = UTF8ToString($2);
                link.style.display = 'none';
                document.body.appendChild(link);
                link.click();
                link.remove();
                setTimeout(function() { URL.revokeObjectURL(url); }, 4000);
            }, bytes.data(), static_cast<int>(bytes.size()), name.c_str(), mime.c_str());

            return true;
#else
            (void)path; (void)name; (void)mime;
            return false;
#endif
        }

        bool pickUpload(const std::string& accept)
        {
#ifdef __EMSCRIPTEN__
            lastUploadName.clear();

            // state: 0 nothing, 1 waiting, 2 read (bytes, name), 3 failed
            MAIN_THREAD_EM_ASM({
                var input = document.createElement('input');
                input.type = 'file';
                input.accept = UTF8ToString($0);
                input.style.display = 'none';
                Module.pgUpload = { state: 1 };

                input.addEventListener('change', function() {
                    var file = input.files && input.files[0];
                    input.remove();

                    if (!file) { Module.pgUpload = { state: 0 }; return; }

                    file.arrayBuffer().then(function(buffer) {
                        Module.pgUpload = { state: 2, name: file.name, bytes: new Uint8Array(buffer) };
                    }, function() {
                        Module.pgUpload = { state: 3 };
                    });
                });

                input.addEventListener('cancel', function() { input.remove(); Module.pgUpload = { state: 0 }; });

                document.body.appendChild(input);
                input.click();
            }, accept.c_str());

            return true;
#else
            (void)accept;
            return false;
#endif
        }

        Upload pollUpload(const std::string& into)
        {
#ifdef __EMSCRIPTEN__
            const int state = MAIN_THREAD_EM_ASM_INT({ return Module.pgUpload ? Module.pgUpload.state : 0; });

            if (state == 0)
                return Upload::None;

            if (state == 1)
                return Upload::Waiting;

            if (state == 3)
            {
                MAIN_THREAD_EM_ASM({ Module.pgUpload = null; });
                return Upload::Failed;
            }

            const int size = MAIN_THREAD_EM_ASM_INT({ return Module.pgUpload.bytes.length; });

            std::vector<char> bytes(static_cast<size_t>(std::max(0, size)));

            if (not bytes.empty())
                MAIN_THREAD_EM_ASM({ HEAPU8.set(Module.pgUpload.bytes, $0); }, bytes.data());

            char name[256] = {0};
            MAIN_THREAD_EM_ASM({ stringToUTF8(Module.pgUpload.name || '', $0, $1); }, name, static_cast<int>(sizeof(name)));

            MAIN_THREAD_EM_ASM({ Module.pgUpload = null; });

            std::error_code ec;
            const auto folder = std::filesystem::path(into).parent_path();

            if (not folder.empty())
                std::filesystem::create_directories(folder, ec);

            std::ofstream file(into, std::ios::binary | std::ios::trunc);

            if (not file)
            {
                LOG_ERROR(DOM, "Cannot write the file picked at " << into);
                return Upload::Failed;
            }

            file.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));

            if (not file)
                return Upload::Failed;

            lastUploadName = name;

            return Upload::Ready;
#else
            (void)into;
            return Upload::None;
#endif
        }

        const std::string& uploadName()
        {
            return lastUploadName;
        }
    }
}
