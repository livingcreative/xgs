#pragma once

// application interface
void initialize(void *hwnd); // set up everything before start
void step();       // called once per frame
void finalize();   // shut everything down
