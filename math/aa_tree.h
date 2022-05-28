#ifndef AA_TREE_H
#define AA_TREE_H

#include <stdbool.h>

typedef
enum compare_e {
  LT,
  EQ,
  GT
}
compare_t;

typedef
struct aa_node_s {
  struct aa_node_s * left;
  struct aa_node_s * right;
  void * data;
  unsigned int level;
}
aa_node_t;

typedef
struct aa_tree_s {
  compare_t (*comparator) (void * a, void * b);
  aa_node_t * root;
  aa_node_t nil;
}
aa_tree_t;


bool find(aa_tree_t * tree, void * data, aa_node_t ** match);
void insert(aa_tree_t * tree, void * data, aa_node_t * to);
void delete(aa_tree_t * tree, void * data);

#endif /* end of include guard: AA_TREE_H */
