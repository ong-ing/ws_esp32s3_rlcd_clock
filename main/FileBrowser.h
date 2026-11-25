#pragma once
#include <stdint.h>
namespace rlcd_clock {
struct Model;enum class Press;
struct FileEntry {char name[256]={};bool directory=false;};
struct FileState {
 char directory[512]="/",file[768]={},message[80]={};
 FileEntry entries[5];unsigned count=0,offset=0,page=0;bool more=false,next=false;
 char lines[10][181]={};
};
bool textFileSupported(const char *name);
void openFiles(Model &m);
bool handleFiles(Model &m,bool boot,Press press);
}
