#ifndef AA_TREE_H
#define AA_TREE_H

#include <stdbool.h>
#include <sys/types.h>

typedef
enum compare_e {
  LT,
  EQ,
  GT
}
compare_t;

typedef
struct aa_node_s {
  off_t left;
  off_t right;
  void * data;
  unsigned int level;
}
aa_node_t;



bool aa_find(compare_t (* comparator) (void * a, void * b), aa_node_t * tree, off_t root, void * data, aa_node_t ** match, size_t node_size);
void aa_insert(compare_t (* comparator) (void * a, void * b), aa_node_t * tree, off_t * root, void * data, off_t to, size_t node_size);
void aa_delete(compare_t (* comparator) (void * a, void * b), aa_node_t * tree, off_t * root, void * data, size_t node_size);
void aa_error_check(aa_node_t * root, off_t abs_off, off_t max_off, size_t node_size);

#endif /* end of include guard: AA_TREE_H */
