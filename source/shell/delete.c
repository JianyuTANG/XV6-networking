#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    printf(2, "Usage: delete files...\n");
    exit();
  }

  for(i = 1; i < argc; i++){
    if(hide(argv[i]) < 0){
      printf(2, "delete: %s failed to delete\n", argv[i]);
      break;
    }
  }

  exit();
}
