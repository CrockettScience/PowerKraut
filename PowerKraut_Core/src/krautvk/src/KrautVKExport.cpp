//
// Created by John Crockett on 11/14/2018.
//

#include "KrautVKExport.h"

extern __declspec(dllexport) int KrautInit(int width, int height, char *title, int fullScreen, char *dllPath) {
    //dllPath exists so as to let the calling .NET Core decide where the root of the dll is
    //as finding it within execution of the framework in this dll is possible but "janky"

    std::string rootPath = dllPath;
    KVKBase::Tools::findAndReplace(rootPath, std::string("\\"), std::string("/"));

    KVKBase::Tools::rootPath = rootPath;
    return KVKBase::KrautVK::kvkInit(width, height, title, fullScreen);
}

extern __declspec(dllexport) int KrautWindowShouldClose() {
    return KVKBase::KrautVK::kvkWindowShouldClose();
}

extern __declspec(dllexport) void KrautPollEvents() {
    KVKBase::KrautVK::kvkPollEvents();
}

extern __declspec(dllexport) void KrautTerminate() {
    KVKBase::KrautVK::kvkTerminate();
}

extern __declspec(dllexport) void KrautDraw() {
    KVKBase::KrautVK::kvkRenderUpdate();
}