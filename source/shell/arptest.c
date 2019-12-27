#include "types.h"
#include "user.h"

int main(int argc,char** argv) {
  int MAC_SIZE = 18;
//  char* ip = "104.236.20.60";
  char* mac = malloc(MAC_SIZE);
  if(arp(argv[1]) < 0) {
    printf(1, "ARP for IP:%s Failed.\n", argv[1]);
  }
  exit();
}
