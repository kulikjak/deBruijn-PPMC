#include <stdio.h>
#include <stdlib.h>

#include "structure.h"
#include "../shared/utils.h"

#define PRINT_DEBRUIJN_ true

bool static_basic_test(void) {
  int8_t L[] = {0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1};
  char W[] = {'A','C','G','C','G','C','$','$','$','$','A','G','G','A','$'};
  int32_t F[] = {3, 7, 10, 15};

  int32_t resOutdegree[] = {3, 3, 3, 2, 2, 1, 1, 1, 1, 1, 2, 2, 1, 1, 1};
  int32_t resOutgoingA[] = {4, 4, 4, -1, -1, -1, -1, -1, -1, -1, 5, 5, -1, 6, -1};
  int32_t resOutgoingC[] = {7, 7, 7, 8, 8, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1};
  int32_t resOutgoingG[] = {11, 11, 11, 12, 12, -1, -1, -1, -1, -1, 13, 13, 14, -1, -1};

  int32_t resIndegree[] = {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

  int32_t resShorten1[] = {-1, -1, -1, -1, 3, 3, 3, -1, 7, 7, -1, 10, 10, 10, 10};
  int32_t resShorten2[] = {-1, -1, -1, -1, -1, -1, 5, -1, -1, 8, -1, -1, -1, -1, 13};

  deBruijn_graph dB;
  deBruijn_Init(&dB);

  deBruijn_Insert_test_data_(&dB, L, W, F, 15);

#if PRINT_DEBRUIJN_
  deBruijn_Print(&dB, true);
#endif

  for (int32_t i = 0; i < 15; i++) {
    assert(resOutdegree[i] == deBruijn_Outdegree(&dB, i));

    assert(resOutgoingA[i] == deBruijn_Outgoing(&dB, i, 'A'));
    assert(resOutgoingC[i] == deBruijn_Outgoing(&dB, i, 'C'));
    assert(resOutgoingG[i] == deBruijn_Outgoing(&dB, i, 'G'));
    assert(-1 == deBruijn_Outgoing(&dB, i, 'T'));
    assert(-1 == deBruijn_Outgoing(&dB, i, '$'));

    assert(resIndegree[i] == deBruijn_Indegree(&dB, i));

    assert(resShorten1[i] == deBruijn_Shorten_context(&dB, i, 1));
    assert(resShorten2[i] == deBruijn_Shorten_context(&dB, i, 2));
  }

  deBruijn_Free(&dB);

  printf("Static basic test successfull.\n");
  return true;
}

int main(int argc, char* argv[]) {
  UNUSED(argc);
  UNUSED(argv);

  static_basic_test();

  printf("All tests successfull\n");

  return EXIT_SUCCESS;
}