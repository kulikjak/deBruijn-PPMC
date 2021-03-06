#include "deBruijn.h"

void deBruijn_Init(deBruijnRef dB__) {
  Graph_Line line;

  /* initialize all structures */
  Graph_Init(&(dB__->Graph_));

  GLine_Fill(&line, VALUE_1, VALUE_A, 1);
  GLine_Insert(&(dB__->Graph_), 0, &line);
  GLine_Fill(&line, VALUE_1, VALUE_C, 1);
  GLine_Insert(&(dB__->Graph_), 1, &line);
  GLine_Fill(&line, VALUE_1, VALUE_G, 1);
  GLine_Insert(&(dB__->Graph_), 2, &line);
  GLine_Fill(&line, VALUE_1, VALUE_T, 1);
  GLine_Insert(&(dB__->Graph_), 3, &line);
  GLine_Fill(&line, VALUE_1, VALUE_$, 0);
  GLine_Insert(&(dB__->Graph_), 4, &line);

  dB__->F_[0] = 1;
  dB__->F_[1] = 2;
  dB__->F_[2] = 3;
  dB__->F_[3] = 4;

  /* calculate csl for all inserted lines */
  deBruijn_update_csl(dB__, 0);
  deBruijn_update_csl(dB__, 2);
  deBruijn_update_csl(dB__, 4);
}

void deBruijn_Free(deBruijnRef dB__) {
  Graph_Free(&(dB__->Graph_));
}

int32_t deBruijn_Forward_(deBruijnRef dB__, int32_t idx__) {
  int32_t rank, spos, temp;
  Graph_Line line;

  DEBRUIJN_VERBOSE(
    printf("[deBruijn]: Calling Forward on index %d\n", idx__);
  )

  /* find edge label of given edge (outgoing edge symbol) */
  GLine_Get(&(dB__->Graph_), idx__, &line);

  /* if edge label is dollar, there is nowhere to go */
  if (line.W_ == VALUE_$) return -1;

  /* calculate rank of edge label in the W array */
  rank = Graph_Rank(&(dB__->Graph_), idx__ + 1, VECTOR_W, (line.W_ & 0xE));

  /* get starting position of edge label */
  spos = dB__->F_[line.W_ >> 0x1];

  /* get index of the last edge of the node pointed to by given edge */
  temp = Graph_Rank(&(dB__->Graph_), spos, VECTOR_L, VALUE_1);
  return Graph_Select(&(dB__->Graph_), temp + rank, VECTOR_L, VALUE_1) - 1;
}

int32_t deBruijn_Backward_(deBruijnRef dB__, int32_t idx__) {
  int32_t base, temp;
  Graph_value symbol;
  Graph_Line line;

  DEBRUIJN_VERBOSE(
    printf("[deBruijn]: Calling Backward on index %d\n", idx__);
  )

  assert(idx__ < Graph_Size(&(dB__->Graph_)) && idx__ >= 0);

  /* find last symbol of this node */
  symbol = GET_VALUE_FROM_IDX(idx__, dB__);

  /* if last symbol is dollar, there is nowhere to go */
  if (symbol == VALUE_$)
    return -1;

  /* rank to current base */
  base = Graph_Rank(&(dB__->Graph_), dB__->F_[symbol >> 0x1], VECTOR_L, VALUE_1);

  /* rank to given line (including it) */
  temp = Graph_Rank(&(dB__->Graph_), idx__ + 1, VECTOR_L, VALUE_1);

  /* if given line is not last edge of the node, add that node */
  GLine_Get(&(dB__->Graph_), (uint32_t) idx__, &line);
  temp += (line.L_) ? 0 : 1;

  /* get index of the edge leading to given node */
  return Graph_Select(&(dB__->Graph_), temp - base, VECTOR_W, symbol) - 1;
}

int32_t deBruijn_Outdegree(deBruijnRef dB__, int32_t idx__) {
  int32_t node_id;

  DEBRUIJN_VERBOSE(
    printf("[deBruijn]: Calling Outdegree on index %d\n", idx__);
  )

  assert(idx__ < Graph_Size(&(dB__->Graph_)) && idx__ >= 0);

  /* get node index */
  node_id = Graph_Rank(&(dB__->Graph_), idx__, VECTOR_L, VALUE_1);

  /* calculate outdegree itself */
  return Graph_Select(&(dB__->Graph_), node_id + 1, VECTOR_L, VALUE_1) -
         Graph_Select(&(dB__->Graph_), node_id, VECTOR_L, VALUE_1);
}

int32_t deBruijn_Find_Edge(deBruijnRef dB__, int32_t idx__, Graph_value gval__) {
  return Graph_Find_Edge(&(dB__->Graph_), idx__, gval__);
}

int32_t deBruijn_Outgoing(deBruijnRef dB__, int32_t idx__, Graph_value gval__) {
  int32_t edge_idx;

  DEBRUIJN_VERBOSE(
    printf("[deBruijn]: Calling Outgoing on index %d and value %d\n", idx__, gval__);
  )

  /* get index of edge we should follow */
  edge_idx = deBruijn_Find_Edge(dB__, idx__, gval__);

  /* check if such edge exist */
  if (edge_idx == -1)
    return -1;

  /* return index of new node */
  return deBruijn_Forward_(dB__, edge_idx);
}

int32_t deBruijn_Indegree(deBruijnRef dB__, int32_t idx__) {
  UNUSED(dB__);
  UNUSED(idx__);

  FATAL("Not Implemented");
  return 0;
}

int32_t deBruijn_Incomming(deBruijnRef dB__, int32_t idx__, Graph_value gval__) {
  UNUSED(dB__);
  UNUSED(idx__);
  UNUSED(gval__);

  FATAL("Not Implemented");
  return 0;
}

void deBruijn_Label(deBruijnRef dB__, int32_t idx__, char *buffer__) {
  int8_t symbol;
  int32_t pos, i;

  memset(buffer__, '$', CONTEXT_LENGTH + 1);

  pos = CONTEXT_LENGTH;
  for (i = 0; i < CONTEXT_LENGTH; i++) {
    symbol = GET_VALUE_FROM_IDX(idx__, dB__);

    buffer__[pos--] = GET_SYMBOL_FROM_VALUE(symbol);
    idx__ = deBruijn_Backward_(dB__, idx__);
    if (idx__ == -1) break;
  }
}

void deBruijn_Print(deBruijnRef dB__, bool labels__) {
  char label[CONTEXT_LENGTH + 2];
  int32_t i, j, size;
  int32_t nextPos = 0;
  int32_t next = 0;
  Graph_Line line;

  /* print header for main structure */
  if (labels__) {
    printf("      F  L  Label   ");
    for (i = 0; i < CONTEXT_LENGTH - 5; i++)
      printf(" ");
    printf("W   P\n--------------------------");
    for (i = 0; i < CONTEXT_LENGTH - 5; i++)
      printf("-");
    printf("\n");
  } else {
    printf("     F  L  W   P\n-----------------\n");
  }

  size = Graph_Size(&(dB__->Graph_));
  for (i = 0; i < size; i++) {
    printf("%4d: ", i);

    /* handle base positions for all symbols */
    if (i == nextPos) {
      if (i == dB__->F_[3]) {
        printf("T  ");
      } else if (i == dB__->F_[2]) {
        printf("G  ");
      } else if (i == dB__->F_[1]) {
        printf("C  ");
      } else if (i == dB__->F_[0]) {
        printf("A  ");
      } else {
        printf("$  ");
      }

      /* find next position for another symbol */
      while (dB__->F_[next] == i && next <= SYMBOL_COUNT)
        next++;
      nextPos = (next <= SYMBOL_COUNT) ? dB__->F_[next] : -1;
    } else {
      printf("   ");
    }

    /* find edge label of given edge (outgoing edge symbol) */
    GLine_Get(&(dB__->Graph_), (uint32_t) i, &line);

    printf("%d  ", line.L_);

    /* handle finding of all the labels */
    if (labels__) {
      label[CONTEXT_LENGTH + 1] = 0;
      deBruijn_Label(dB__, i, label);

      printf("%s  ", label);
      for (j = 0; j < 5 - CONTEXT_LENGTH; j++) {
        printf(" ");
      }
    }
    printf("%c%c  ", GET_SYMBOL_FROM_VALUE(line.W_), (line.W_ & 0x1) ? 'x' : ' ');
    printf("%d\n", line.P_);
  }
}

int32_t deBruijn_Get_common_suffix_len_(deBruijnRef dB__, int32_t idx1__, int32_t idx2__) {
  int32_t common;
  int32_t symbol1, symbol2;

  DEBRUIJN_VERBOSE(
    printf("[deBruijn]: Calling Get_common_suffix_len on index %d and %d\n", idx1__, idx2__);
  )
  common = 0;

  /* limit size of suffix for better performance */
  while (common < CONTEXT_LENGTH) {
    /* get symbols itself */
    symbol1 = GET_VALUE_FROM_IDX(idx1__, dB__);
    symbol2 = GET_VALUE_FROM_IDX(idx2__, dB__);

    /* dollars are not context (we can check only one - next condition will handle the other) */
    if (symbol1 == VALUE_$) break;

    /* symbols are not the same */
    if (symbol1 != symbol2) break;

    /* continue backwards */
    idx1__ = deBruijn_Backward_(dB__, idx1__);
    idx2__ = deBruijn_Backward_(dB__, idx2__);
    common++;

    if (idx1__ == -1 || idx2__ == -1) break;
  }
  return common;
}

void deBruijn_update_csl(deBruijnRef dB__, int32_t target__) {

#if defined(INTEGER_CONTEXT_SHORTENING) \
  || defined(RAS_CONTEXT_SHORTENING)

  int32_t graph_size;

  graph_size = Graph_Size(&(dB__->Graph_));
  assert(target__ <= graph_size);

  /* there is nothing to update for the root node */
  if (target__ == 0) return;

  Graph_Set_csl(&(dB__->Graph_), target__,
                deBruijn_Get_common_suffix_len_(dB__, target__, target__ - 1));

  /* target is at the bottom of the list - only one csl to update */
  if (target__ == graph_size - 1) return;

  Graph_Set_csl(&(dB__->Graph_), target__ + 1,
                deBruijn_Get_common_suffix_len_(dB__, target__ + 1, target__));

#elif defined(LABEL_CONTEXT_SHORTENING)
  UNUSED(dB__);
  UNUSED(target__);
#endif
}

int32_t deBruijn_shorten_lower(deBruijnRef dB__, int32_t idx__, int32_t ctx_len__) {

  DEBRUIJN_VERBOSE(
    printf("[deBruijn]: Calling Shorten_context on index %d (ctx len: %d)\n", idx__, ctx_len__);
  )

  /* if this is root node it is not possible to shorten context */
  if (idx__ < dB__->F_[0] || ctx_len__ == 0) return 0;

  while (idx__ > 0) {
    /* check for length of common suffix */
#if defined(LABEL_CONTEXT_SHORTENING)
    if (deBruijn_Get_common_suffix_len_(dB__, idx__, idx__ - 1) < ctx_len__)
#elif defined(INTEGER_CONTEXT_SHORTENING)
    if (Graph_Get_csl(&(dB__->Graph_), idx__) < ctx_len__)
#endif
      return idx__;

    /* move one line higher */
    idx__--;
  }
  return 0;
}
int32_t deBruijn_shorten_upper(deBruijnRef dB__, int32_t idx__, int32_t ctx_len__) {

  DEBRUIJN_VERBOSE(
    printf("[deBruijn]: Calling Shorten_context on index %d (ctx len: %d)\n", idx__, ctx_len__);
  )

  /* if this is root node it is not possible to shorten context */
  int gsize = Graph_Size(&(dB__->Graph_));
  if (idx__ < dB__->F_[0] || ctx_len__ == 0) return gsize - 1;

  idx__++;
  while (idx__ < gsize) {
    /* check for length of common suffix */
#if defined(LABEL_CONTEXT_SHORTENING)
    if (deBruijn_Get_common_suffix_len_(dB__, idx__, idx__ - 1) < ctx_len__)
#elif defined(INTEGER_CONTEXT_SHORTENING)
    if (Graph_Get_csl(&(dB__->Graph_), idx__) < ctx_len__)
#endif
      return idx__ - 1;

    /* move one line higher */
    idx__++;
  }
  return gsize - 1;
}


void deBruijn_Get_symbol_frequency(deBruijnRef dB__, uint32_t idx__, cfreq* freq__) {
  Graph_Get_symbol_frequency(&(dB__->Graph_), idx__, freq__);
}

void deBruijn_Get_symbol_frequency_range(deBruijnRef dB__, int32_t lo_, int32_t up_, cfreq* freq__) {

  DEBRUIJN_VERBOSE(
    printf("[deBruijn]: Calling Get_symbol_frequency on range  TODO\n");
  )

  int32_t idx, cnt;
  Graph_Line line;

  freq__->total_ = 0;

  cnt = 0;
  memset(freq__, 0, sizeof(*freq__));
  for (idx = lo_; idx <= up_; idx++) {
    cnt++;

    GLine_Get(&(dB__->Graph_), (uint32_t)idx, &line);
    if (line.W_ == VALUE_$)
      continue;

    freq__->symbol_[line.W_ >> 0x1] = line.P_;
    freq__->total_ += line.P_;
  }

#if defined(FREQ_COUNT_ONCE)
  int32_t i;
  for (cnt = 0, i = 0; i < 4; i++)
    cnt += (freq__->symbol_[i] > 0);
#endif

  freq__->symbol_[VALUE_ESC >> 0x1] = cnt;
  freq__->total_ += cnt;
}

void deBruijn_Insert_test_data(deBruijnRef dB__, const Graph_value *L__, const Graph_value *W__,
                               const int32_t *P__, const int32_t F__[SYMBOL_COUNT],
                               const int32_t size__) {
  int32_t i;
  Graph_Line line;

  DEBRUIJN_VERBOSE(
    printf("[deBruijn]: Inserting test data\n");
  )

  Graph_Init(&(dB__->Graph_));

  memcpy(dB__->F_, F__, sizeof(dB__->F_));

  /* insert test data */
  for (i = 0; i < size__; i++) {
    GLine_Fill(&line, L__[i], W__[i], P__[i]);
    GLine_Insert(&(dB__->Graph_), i, &line);
  }

  /* updated common suffix lengths after insertion is done */
  for (i = 1; i < size__; i += 2)
    deBruijn_update_csl(dB__, i);
  deBruijn_update_csl(dB__, size__ - 1);
}
