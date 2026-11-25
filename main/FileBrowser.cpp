#include "FileBrowser.h"
#include "ClockModel.h"
#include "CardStorage.h"
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <vector>
#include <string>
namespace rlcd_clock {
namespace {
std::vector<long> offsets;
void message(Model &m,const char *text){snprintf(m.files.message,sizeof(m.files.message),"%s",text);}
std::string full(const char *relative){return std::string(cardRoot())+relative;}
void list(Model &m){
 auto &s=m.files;s.count=0;s.more=false;message(m,"");
 if(!cardReady()){message(m,"TF 卡未就绪，请插卡后重启");return;}
 DIR *d=opendir(full(s.directory).c_str());if(!d){message(m,"目录读取失败");return;}
 unsigned skipped=0;dirent *entry;
 while((entry=readdir(d))){
  if(strcmp(entry->d_name,".")==0||strcmp(entry->d_name,"..")==0)continue;
  if(skipped++<s.offset)continue;
  if(s.count==5){s.more=true;break;}
  auto &row=s.entries[s.count++];snprintf(row.name,sizeof(row.name),"%s",entry->d_name);
  std::string path=full(s.directory);if(path.back()!='/')path+='/';path+=row.name;
  struct stat st={};row.directory=stat(path.c_str(),&st)==0&&S_ISDIR(st.st_mode);
 }
 closedir(d);if(m.selected>=s.count+2)m.selected=0;
 if(!s.count)message(m,"此目录为空");
}
// Validate one UTF-8 code point, rejecting binary/control data and invalid encodings.
int character(FILE *f,char *out){
 int c=fgetc(f);if(c==EOF)return 0;out[0]=char(c);
 if(c<128){out[1]=0;return (c>=32||c=='\n'||c=='\r'||c=='\t')?1:-1;}
 int n=c>=0xc2&&c<=0xdf?2:c>=0xe0&&c<=0xef?3:c>=0xf0&&c<=0xf4?4:0;if(!n)return -1;
 unsigned cp=c&((1<<(7-n))-1);
 for(int i=1;i<n;i++){int b=fgetc(f);if(b<0x80||b>0xbf)return -1;out[i]=char(b);cp=(cp<<6)|(b&63);}
 if((n==3&&cp<0x800)||(n==4&&cp<0x10000)||cp>0x10ffff||(cp>=0xd800&&cp<=0xdfff))return -1;
 out[n]=0;return n;
}
void readPage(Model &m){
 auto &s=m.files;memset(s.lines,0,sizeof(s.lines));s.next=false;message(m,"");
 if(!cardReady()){message(m,"TF 卡已移除或读取失败");return;}
 FILE*f=fopen(full(s.file).c_str(),"rb");if(!f){message(m,"文件无法打开");return;}
 if(s.page>=offsets.size()||fseek(f,offsets[s.page],SEEK_SET)){fclose(f);message(m,"页面定位失败");return;}
 bool invalid=false;
 for(int row=0;row<10&&!feof(f);row++){
  int columns=0;size_t used=0;
  while(true){
   long pos=ftell(f);char c[5];int n=character(f,c);if(!n)break;if(n<0){invalid=true;break;}
   if(c[0]=='\r'){int next=fgetc(f);if(next!='\n'&&next!=EOF)ungetc(next,f);break;}
   if(c[0]=='\n')break;
   int width=n==1?(c[0]=='\t'?4:1):2;
   if(columns+width>60){fseek(f,pos,SEEK_SET);break;}
   if(c[0]=='\t'){memcpy(s.lines[row]+used,"    ",4);used+=4;}else{memcpy(s.lines[row]+used,c,n);used+=n;}
   columns+=width;
  }
  if(invalid)break;
 }
 long pos=ftell(f);int peek=fgetc(f);bool io=ferror(f);s.next=peek!=EOF&&!invalid&&!io;
 if(s.next&&s.page>=8191){s.next=false;message(m,"已达 8192 页上限，请拆分文件");}
 if(s.next){if(offsets.size()==s.page+1)offsets.push_back(pos);else offsets[s.page+1]=pos;}
 fclose(f);
 if(invalid||io){memset(s.lines,0,sizeof(s.lines));message(m,invalid?"仅支持 UTF-8 文本，文件含无效字符":"文件读取失败");}
}
void parent(Model &m){
 auto &s=m.files;if(strcmp(s.directory,"/")==0){m.page=Page::Home;m.selected=1;return;}
 char *slash=strrchr(s.directory,'/');if(slash==s.directory)s.directory[1]=0;else if(slash)*slash=0;
 s.offset=0;m.selected=0;list(m);
}
}
bool textFileSupported(const char *name){
 const char *ext=strrchr(name,'.');if(!ext)return false;
 const char *types[]={".txt",".json",".log",".xml",".csv",".md",".ini",".conf",".cfg",".yaml",".yml",".toml",".html",".css",".js",".py",".c",".cpp",".h",".sh",".sql",".properties"};
 for(auto t:types){if(strcasecmp(ext,t)==0)return true;}
 return false;
}
void openFiles(Model &m){m.page=Page::Files;m.selected=0;m.files=FileState{};list(m);}
bool handleFiles(Model &m,bool boot,Press press){
 if(m.page!=Page::Files&&m.page!=Page::Text)return false;
 if(press==Press::None)return true;
 auto &s=m.files;
 if(!boot&&press==Press::Long){if(m.page==Page::Text){m.page=Page::Files;m.selected=0;offsets.clear();list(m);}else parent(m);return true;}
 if(press!=Press::Short)return true;
 if(m.page==Page::Text){if(boot&&s.next){s.page++;readPage(m);}else if(!boot&&s.page){s.page--;readPage(m);}return true;}
 if(boot){m.selected=(m.selected+1)%(s.count+2);return true;}
 if(m.selected==s.count){if(s.offset>=5){s.offset-=5;m.selected=0;list(m);}return true;}
 if(m.selected==s.count+1){if(s.more){s.offset+=5;m.selected=0;list(m);}return true;}
 const auto &entry=s.entries[m.selected];std::string path=s.directory;if(path.back()!='/')path+='/';path+=entry.name;
 if(entry.directory){if(path.size()>=sizeof(s.directory)){message(m,"目录层级过深");return true;}snprintf(s.directory,sizeof(s.directory),"%s",path.c_str());s.offset=0;m.selected=0;list(m);return true;}
 if(!textFileSupported(entry.name)){message(m,"仅支持纯文本文件，暂不支持此格式");return true;}
 struct stat st={};if(stat(full(path.c_str()).c_str(),&st)||!S_ISREG(st.st_mode)||st.st_size>16*1024*1024){message(m,"文件不可读或超过 16 MB");return true;}
 snprintf(s.file,sizeof(s.file),"%s",path.c_str());offsets.clear();long start=0;
 FILE*f=fopen(full(s.file).c_str(),"rb");if(f){unsigned char bom[3]={};if(fread(bom,1,3,f)==3&&bom[0]==239&&bom[1]==187&&bom[2]==191)start=3;fclose(f);}
 offsets.push_back(start);s.page=0;m.page=Page::Text;readPage(m);return true;
}
}
