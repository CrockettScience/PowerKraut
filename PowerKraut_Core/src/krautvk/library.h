#ifndef PKVK_LIBRARY_H
#define PKVK_LIBRARY_H

int Init(int w, int h, char *title, int f);
int WindowShouldClose();
void PollEvents();
void Terminate();

#endif