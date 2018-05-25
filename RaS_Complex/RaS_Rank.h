#ifndef _RAS_RANK__
#define _RAS_RANK__

#include "memory.h"


int32_t RaS_Rank_32_(memory_32b* mem__, int32_t root__, uint32_t pos__) {
  int32_t i, temp, limit;

  int32_t current = root__;
  int32_t rank = 0;

  Node_32b* node_ref;
  Leaf_32b* leaf_ref;

  node_ref = MEMORY_GET_ANY(mem__, root__);
  if (pos__ >= node_ref->p_)
    return node_ref->r_;

  // traverse the tree and enter correct leaf
  while (!IS_LEAF(current)) {
    node_ref = MEMORY_GET_NODE(mem__, current);

    // get p_ counter of left child and act accordingly
    temp = MEMORY_GET_ANY(mem__, node_ref->left_)->p_;
    if ((uint32_t)temp >= pos__) {
      current = node_ref->left_;
    } else {
      pos__ -= temp;
      rank += MEMORY_GET_ANY(mem__, node_ref->left_)->r_;
      current = node_ref->right_;
    }
  }

  leaf_ref = MEMORY_GET_LEAF(mem__, current);

  // handle last leaf of this query
  limit = (pos__ <= 32) ? pos__ : 32;
  for (i = 0; i < limit; i++)
    rank += (leaf_ref->vector_ >> (31 - i)) & 0x1;

  return rank;
}


#endif  // _RAS_RANK__
