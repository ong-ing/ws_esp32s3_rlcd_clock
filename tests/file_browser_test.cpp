#include "ClockModel.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
namespace rlcd_clock {static bool present=true;static std::string root;bool cardReady(){return present;}const char *cardRoot(){return root.c_str();}}
int main(){
 char temp[]="/tmp/rlcd-files-XXXXXX";rlcd_clock::root=mkdtemp(temp);assert(!rlcd_clock::root.empty());
 auto write=[](const char *name,const char *body){FILE *f=fopen((rlcd_clock::root+name).c_str(),"wb");assert(f);fwrite(body,1,strlen(body),f);fclose(f);};
 write("/readme.TXT","\xef\xbb\xbf第一行\r\nsecond\n");
 rlcd_clock::Model m;rlcd_clock::openFiles(m);assert(m.page==rlcd_clock::Page::Files&&m.files.count==1);
 rlcd_clock::handleFiles(m,false,rlcd_clock::Press::Short);assert(m.page==rlcd_clock::Page::Text);assert(strcmp(m.files.lines[0],"第一行")==0);assert(strcmp(m.files.lines[1],"second")==0);assert(!m.files.next);
 rlcd_clock::handleFiles(m,false,rlcd_clock::Press::Long);assert(m.page==rlcd_clock::Page::Files);
 write("/readme.TXT","0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11");rlcd_clock::handleFiles(m,false,rlcd_clock::Press::Short);assert(m.files.next);
 rlcd_clock::handleFiles(m,true,rlcd_clock::Press::Short);assert(m.files.page==1&&strcmp(m.files.lines[0],"10")==0);
 rlcd_clock::handleFiles(m,false,rlcd_clock::Press::Short);assert(m.files.page==0&&strcmp(m.files.lines[0],"0")==0);
 rlcd_clock::handleFiles(m,false,rlcd_clock::Press::Long);write("/readme.TXT","\xff\xfe");rlcd_clock::handleFiles(m,false,rlcd_clock::Press::Short);assert(m.files.message[0]&&m.files.lines[0][0]==0);
 assert(rlcd_clock::textFileSupported("abc.JSON"));assert(!rlcd_clock::textFileSupported("photo.jpg"));
 rlcd_clock::present=false;rlcd_clock::openFiles(m);assert(m.files.count==0&&m.files.message[0]);
 unlink((rlcd_clock::root+"/readme.TXT").c_str());rmdir(rlcd_clock::root.c_str());puts("File browsing: UTF-8, BOM, pagination, invalid encoding and card removal passed");
}
