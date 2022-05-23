#ifndef AA_TREE_H
#define AA_TREE_H

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
}
aa_tree_t;

aa_node_t * find_or_insert(aa_tree_t * tree, void * data);
void delete(aa_tree_t * tree, void * data);
aa_tree_t create_tree(compare_t (*comparator) (void * a, void * b));

#endif /* end of include guard: AA_TREE_H */
