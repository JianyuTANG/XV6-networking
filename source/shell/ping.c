#include "types.h"
#include "user.h"

int main(int argc,char** argv) {
  if(arp(argv[1]) < 0) 
  {
    printf(1, "ARP for IP:%s Failed.\n", argv[1]);
  }
  exit();
}