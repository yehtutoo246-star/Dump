#include <cstring>
#include <thread>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cinttypes>
#include "hack.h"
#include "zygisk.hpp"
#include "game.h"
#include "log.h"


#include <jni.h>
#include <pthread.h>
#include <unistd.h>
#include <dlfcn.h>
#include <android/log.h>
#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <sys/ptrace.h>
#include <sys/wait.h>





#include <jni.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <cstring>
#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "libhack", __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, "libhack", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "libhack", __VA_ARGS__)

const char *GamePackageName = "com.OneManEmpire.SurvivalOdyssey";  // <-- change this
const char *GameDataDir     = "/data/data/com.OneManEmpire.SurvivalOdyssey";  // <-- change this

void *data = nullptr;
size_t length = 0;

void *hack_thread(void *) {
    LOGI("Hack thread started");

    // Your logic here. E.g., patch memory, hook function, etc.
    // Maybe wait for libraries to load:
    sleep(2);

    LOGI("Hack thread done");
    return nullptr;
}

void prepare_hack(const char *app_data_dir) {
    LOGI("Preparing hack for: %s", app_data_dir);

#if defined(__i386__)
    const char *path = "/data/local/tmp/armeabi-v7a.so";
#elif defined(__x86_64__)
    const char *path = "/data/local/tmp/arm64-v8a.so";
#else
    const char *path = "/data/local/tmp/libpayload.so";
#endif

    int fd = open(path, O_RDONLY);
    if (fd != -1) {
        struct stat sb{};
        fstat(fd, &sb);
        length = sb.st_size;
        data = mmap(nullptr, length, PROT_READ, MAP_PRIVATE, fd, 0);
        close(fd);
        LOGI("Mapped %zu bytes from %s", length, path);
    } else {
        LOGW("Failed to open arm payload at %s", path);
    }

    pthread_t thread;
    if (pthread_create(&thread, nullptr, hack_thread, nullptr) != 0) {
        LOGE("Failed to create hack thread");
    }
}

__attribute__((constructor))
void lib_main() {
    LOGI("lib_main constructor called");

    // Optional: Check if injected in the correct process
    char procname[256];
    readlink("/proc/self/exe", procname, sizeof(procname) - 1);
    if (strstr(procname, GamePackageName) || access(GameDataDir, F_OK) == 0) {
        prepare_hack(GameDataDir);
    } else {
        LOGI("Not the target app: %s", procname);
    }
}


/*
using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        auto package_name = env->GetStringUTFChars(args->nice_name, nullptr);
        auto app_data_dir = env->GetStringUTFChars(args->app_data_dir, nullptr);
        preSpecialize(package_name, app_data_dir);
        env->ReleaseStringUTFChars(args->nice_name, package_name);
        env->ReleaseStringUTFChars(args->app_data_dir, app_data_dir);
    }

    void postAppSpecialize(const AppSpecializeArgs *) override {
        if (enable_hack) {
            std::thread hack_thread(hack_prepare, game_data_dir, data, length);
            hack_thread.detach();
        }
    }

private:
    Api *api;
    JNIEnv *env;
    bool enable_hack;
    char *game_data_dir;
    void *data;
    size_t length;

    void preSpecialize(const char *package_name, const char *app_data_dir) {
        if (strcmp(package_name, GamePackageName) == 0) {
            LOGI("detect game: %s", package_name);
            enable_hack = true;
            game_data_dir = new char[strlen(app_data_dir) + 1];
            strcpy(game_data_dir, app_data_dir);

#if defined(__i386__)
            auto path = "zygisk/armeabi-v7a.so";
#endif
#if defined(__x86_64__)
            auto path = "zygisk/arm64-v8a.so";
#endif
#if defined(__i386__) || defined(__x86_64__)
            int dirfd = api->getModuleDir();
            int fd = openat(dirfd, path, O_RDONLY);
            if (fd != -1) {
                struct stat sb{};
                fstat(fd, &sb);
                length = sb.st_size;
                data = mmap(nullptr, length, PROT_READ, MAP_PRIVATE, fd, 0);
                close(fd);
            } else {
                LOGW("Unable to open arm file");
            }
#endif
        } else {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
        }
    }
};

REGISTER_ZYGISK_MODULE(MyModule)*/
