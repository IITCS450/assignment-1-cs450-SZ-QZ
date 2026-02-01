#include "common.h"
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
static void usage(const char *a){fprintf(stderr,"Usage: %s <pid>\n",a); exit(1);}
static int isnum(const char*s){for(;*s;s++) if(!isdigit((unsigned char)*s)) return 0; return 1;}
int main(int c,char**v){
 if(c!=2||!isnum(v[1])) usage(v[0]);
 char path[256], buf[4096];
 snprintf(path,sizeof(path), "/proc/%s/stat",v[1]);
 FILE *f = fopen(path, "r");
 if(!f){
    if(errno==ENOENT) fprintf(stderr,"PID not found: %s\n", v[1]);
    else if(errno==EACCES) fprintf(stderr,"Permission denied: %s\n", v[1]);
    else perror(path);
    return 1;
    }
if(!fgets(buf, sizeof(buf), f)){
    perror("fgets");
    fclose(f);
    return 1;
}
    fclose(f);
char *lp = strchr(buf,'(');
char *rp = strrchr(buf,')');
char comm[256];
comm[0] = 0;
if(lp && rp && rp >lp+1){
    size_t n = (size_t)(rp - lp -1);
    if(n >= sizeof(comm)) n  = sizeof(comm)-1;
    memcpy(comm, lp+1,n);
    comm[n] = 0;
}else{
    strcpy(comm,"?");
}
char state='?';
int ppid=0;
unsigned long utime=0, stime=0;
int pgrp=0, session=0, tty_nr=0, tpgid=0;
unsigned long flags=0;
unsigned long minflt=0, cminflt=0, majflt=0, cmajflt=0;
const char *rest = (rp && rp[1]==' ')? (rp+2) : buf;
if(sscanf(rest,"%c %d %d %d %d %d %lu %lu %lu %lu %lu %lu %lu",
    &state, &ppid, &pgrp, &session, &tty_nr, &tpgid,
    &flags, &minflt, &cminflt, &majflt, &cmajflt,
    &utime, &stime) != 13){
    fprintf(stderr,"Failed to parse: %s\n", path);
    return 1;
}
unsigned long ticks = utime + stime;
long hz = sysconf(_SC_CLK_TCK);
double cpu_sec = (hz>0)? (double)ticks/(double)hz : 0.0;
snprintf(path, sizeof(path), "/proc/%s/cmdline", v[1]);
f = fopen(path, "r");
char cmd[1024];
cmd[0] = 0;
if(f){
    size_t n = fread(cmd, 1, sizeof(cmd)-1,f);
    fclose(f);
    cmd[n] = 0;
    if(n==0){
        strncpy(cmd,comm,sizeof(cmd)-1);
        cmd[sizeof(cmd)-1] = 0;
    }else{
        for(size_t i = 0; i < n;i++){
            if(cmd[i] == 0) cmd[i] = ' ';
        }
         while(n>0 && cmd[n-1]==' '){
            cmd[n-1]=0;
            n--;
        }
    }
}
 snprintf(path, sizeof(path), "/proc/%s/status", v[1]);
    f = fopen(path, "r");
    if(!f){
        if(errno==ENOENT) fprintf(stderr,"PID not found: %s\n",v[1]);
        else if(errno==EACCES) fprintf(stderr,"Permission denied: %s\n",v[1]);
        else perror(path);
        return 1;
    }
long vmrss = 0;
while(fgets(buf, sizeof(buf), f)){
    if(!strncmp(buf, "VmRSS:", 6)){
        sscanf(buf+6, "%ld", &vmrss);
        break;
    }
    }
    fclose(f);
int pid = atoi(v[1]);
printf("PID:%d\n", pid);
printf("State:%c\n", state);
printf("PPID:%d\n", ppid);
printf("Cmd:%s\n", cmd);
printf("CPU:%lu %.3f\n", ticks, cpu_sec);
printf("VmRSS:%ld\n", vmrss);
 return 0;
}
