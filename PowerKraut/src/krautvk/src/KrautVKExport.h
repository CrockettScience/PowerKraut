//
// Created by John Crockett on 11/14/2018.
//

#ifndef KRAUTVK_KRAUTVKEXPORT_H
#define KRAUTVK_KRAUTVKEXPORT_H

#include "KrautVK.cpp"

extern "C"{

__declspec(dllexport) int KrautInit(int w, int h, char* title, int f, char* dllPath);

__declspec(dllexport) int KrautWindowShouldClose();

__declspec(dllexport) void KrautPollEvents();

__declspec(dllexport) void KrautTerminate();

__declspec(dllexport) void KrautDraw();
}

#endif //KRAUTVK_KRAUTVKEXPORT_H
