#ifndef _DEBRUIJN_GRAPH__
#define _DEBRUIJN_GRAPH__

#include <stdio.h>
#include <stdlib.h>

#include "defines.h"
#include "structure.h"
#include "utils.h"

#define DEBRUIJN_VERBOSE(func) \
  if (DEBRUIJN_VERBOSE_) {     \
    func                       \
  }

#define GET_SYMBOL_FROM_VALUE(symb__) \
      (((symb__) == VALUE_A) ? 'A'    \
    : ((symb__) == VALUE_Ax) ? 'A'    \
    : ((symb__) == VALUE_C)  ? 'C'    \
    : ((symb__) == VALUE_Cx) ? 'C'    \
    : ((symb__) == VALUE_G)  ? 'G'    \
    : ((symb__) == VALUE_Gx) ? 'G'    \
    : ((symb__) == VALUE_T)  ? 'T'    \
    : ((symb__) == VALUE_Tx) ? 'T' : '$')

#define GET_VALUE_FROM_SYMBOL(symb__) \
      (((symb__) == 'A') ? VALUE_A    \
    : ((symb__) == 'C') ? VALUE_C     \
    : ((symb__) == 'G') ? VALUE_G     \
    : ((symb__) == 'T') ? VALUE_T : VALUE_$)

#define GET_VALUE_FROM_IDX(idx__, dB__) \
      ((idx__ < dB__->F_[0]) ? VALUE_$  \
    : (idx__ < dB__->F_[1]) ? VALUE_A   \
    : (idx__ < dB__->F_[2]) ? VALUE_C   \
    : (idx__ < dB__->F_[3]) ? VALUE_G : VALUE_T)

typedef struct {
  int32_t F_[SYMBOL_COUNT];
  Graph_Struct Graph_;

  int32_t depth;
} deBruijn_graph;

#define deBruijnRef deBruijn_graph*

/*
 * Initialize deBruijn_graph object.
 *
 * @param  dB__  Reference to deBruijn_graph object.
 */
void deBruijn_Init(deBruijnRef dB__);

/*
 * Free all memory associated with deBruijn graph object.
 *
 * @param  dB__  Reference to deBruijn_graph object.
 */
void deBruijn_Free(deBruijnRef dB__);

/*
 * Get number of outgoing edges from given node.
 *
 * @param  dB__  Reference to deBruijn_graph object.
 * @param  idx__  Edge index (line) in deBruijn graph.
 *
 * @return  A number of outgoing edges.
 */
int32_t deBruijn_Outdegree(deBruijnRef dB__, int32_t idx__);

/*
 * Get position of given edge symbol in given node.
 *
 * @param  dB__  Reference to deBruijn_graph object.
 * @param  idx__  Edge index (line) in deBruijn graph.
 * @param  gval__  Symbol (Graph_value) to find.
 *
 * @return  Index of edge in given node.
 */
int32_t deBruijn_Find_Edge(deBruijnRef dB__, int32_t idx__, Graph_value gval__);

/*
 * From given node follow edge labeled by given symbol.
 *
 * @param  dB__  Reference to deBruijn_graph object.
 * @param  idx__  Edge index (line) in deBruijn graph.
 * @param  gval__  Symbol (Graph_value) to follow.
 *
 * @return  Index of new node.
 */
int32_t deBruijn_Outgoing(deBruijnRef dB__, int32_t idx__, Graph_value gval__);

/*
 * Get number of edges that point to current node.
 *
 * As this is not important for compression and it's not super straightforward,
 * it is not implemented.
 */
int32_t deBruijn_Indegree(deBruijnRef dB__, int32_t idx__);

/*
 * Get node starting with given symbol that has an edge to given node.
 *
 * As this is not important for compression and it's not super straightforward,
 * it is not implemented.
 */
int32_t deBruijn_Incomming(deBruijnRef dB__, int32_t idx__, Graph_value gval__);

/*
 * Get label of a node corresponding to given line.
 *
 * Output buffer MUST have a size of at least CONTEXT_LENGTH + 1 otherwise buffer
 * overflow can occur. Output buffer doesn't have terminating null byte after
 * the last character and therefore it cannot be automatically printed out as a
 * string.
 *
 * @param  dB__  Reference to deBruijn_graph object.
 * @param  idx__  Edge index (line) in deBruijn graph.
 * @param  buffer__  [out] Output buffer with label in symbols.
 */
void deBruijn_Label(deBruijnRef dB__, int32_t idx__, char *buffer__);

/*
 * Print whole deBruijn graph struct.
 *
 * Printing graph with labels is much slower because they are not explicitly
 * stored and for each edge, they must be calculated by going back all the way
 * to the root.
 *
 * @param  dB__  Reference to deBruijn_graph object.
 * @param  labels__  Node labels should be printed as well.
 */
void deBruijn_Print(deBruijnRef dB__, bool labels__);

/*
 * Get lower and upper limits for given context length.
 *
 * @param  dB__  Reference to deBruijn_graph object.
 * @param  idx__  Edge index (line) in deBruijn graph.
 * @param  ctx_len__ Desired new context length.
 *
 * @return  Index of first lower/upper line with given context length.
 */
int32_t deBruijn_shorten_lower(deBruijnRef dB__, int32_t idx__, int32_t ctx_len__);
int32_t deBruijn_shorten_upper(deBruijnRef dB__, int32_t idx__, int32_t ctx_len__);

/*
 * Update longest common suffix length with its neighbours.
 *
 * This function updates cls both with higher and lower neighbour.
 *
 * @param  dB__  Reference to deBruijn_graph object.
 * @param  target__  Edge index (line) in deBruijn graph.
 */
void deBruijn_update_csl(deBruijnRef dB__, int32_t target__);

/*
 * Get symbol frequencies from node pointed to by given index.
 *
 * @param  dB__  Reference to deBruijn_graph object.
 * @param  idx__  Edge index (line) in deBruijn graph.
 * @param  freq__  [Out] Frequency count structure.
 */
void deBruijn_Get_symbol_frequency(deBruijnRef dB__, uint32_t idx__, cfreq *freq__);

/*
 * Get symbol frequencies from given range.
 *
 * @param  dB__  Reference to deBruijn_graph object.
 * @param  lo__  Lower bound of given range
 * @param  up__  Upper bound of given range
 * @param  freq__  [Out] Frequency count structure.
 */
void deBruijn_Get_symbol_frequency_range(deBruijnRef dB__, int32_t lo__, int32_t up__, cfreq* freq__);

/*
 * Move to next node pointed to by given edge (line) index.
 *
 * @param  dB__  Reference to deBruijn_graph object.
 * @param  idx__  Edge index (line) in deBruijn graph.
 *
 * @return  Index of last edge of the next node pointed to by given edge or -1
 * if there is no next node.
 */
int32_t deBruijn_Forward_(deBruijnRef dB__, int32_t idx__);

/*
 * Move to parent node of given one.
 *
 * @param  dB__  Reference to deBruijn_graph object.
 * @param  idx__  Edge index (line) in deBruijn graph.
 *
 * @return  Index of parent node (its edge pointing to given one) or -1 if there
 * is no parent node.
 */
int32_t deBruijn_Backward_(deBruijnRef dB__, int32_t idx__);

/*
 * Get length of common suffix of given line and line above.
 *
 * @param  dB__  Reference to deBruijn_graph object.
 * @param  idx1__  First edge index (line) in deBruijn graph.
 * @param  idx2__  Second edge index (line) in deBruijn graph.
 *
 * @return  Length of longest common suffix
 */
int32_t deBruijn_Get_common_suffix_len_(deBruijnRef dB__, int32_t idx1__, int32_t idx2__);

/*
 * Initialize structure with given test data.
 *
 * This function is for testing purposes only. It simply fills all the
 * structures with given data. There is no error checking (except for checks of
 * underlying structures) and if used incorrectly, all other functions can have
 * wrong results.
 *
 * Structure should not be initialized whne this is called!
 *
 * @param  dB__  Reference to deBruijn_graph object.
 * @param  L__  L array (last edge of corresponding node).
 * @param  W__  W array (label of given outgoing edge).
 * @param  P__  P array (frequencies of each symbol in context).
 * @param  F__  F array (base positions of symbols)
 * @param  size__  Size of arrays L and W.
 */
void deBruijn_Insert_test_data(deBruijnRef dB__, const Graph_value *L__, const Graph_value *W__,
                               const int32_t *P__, const int32_t F__[SYMBOL_COUNT],
                               const int32_t size__);

#endif
