#include "types.h"
#include "user.h"

int main(int argc,char** argv) {
  if(argc==1){
    printf(1,"Usage: arptest target_name \n");
    return 0;
  }
  if(arp(argv[1]) < 0) 
  {
    printf(1, "ARP for IP:%s Failed.\n", argv[1]);
  }
  exit();
}