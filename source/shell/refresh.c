#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    printf(2, "Usage: refersh files...\n");
    exit();
  }

  for(i = 1; i < argc; i++){
    if(show(argv[i]) < 0){
      printf(2, "refersh: %s failed to refersh\n", argv[i]);
      break;
    }
  }

  exit();
}
