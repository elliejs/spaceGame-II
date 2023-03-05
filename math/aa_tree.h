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
  unsigned int level;
}
aa_node_t;

typedef
struct aa_tree_s {
  /* node storage */
  void * node_root;
  size_t node_size;
  /* data storage */
  uintptr_t data_off;

  compare_t (* comparator) (void * a,    void * b);
}
aa_tree_t;


bool aa_find(aa_tree_t * tree, off_t root, void * data, void ** match);
void aa_insert(aa_tree_t * tree, off_t * root, off_t to);
void aa_delete(aa_tree_t * tree, off_t * root, void * data);
void aa_error_check(aa_tree_t * tree, off_t root, off_t max_off);

aa_tree_t create_aa_tree(void * node_root, size_t node_size, void * data_root,
                         compare_t (* comparator) (void * a, void * b));

aa_tree_t * rebase_aa_tree(aa_tree_t * tree, void * node_root, void * data_root);
#endif /* end of include guard: AA_TREE_H */
