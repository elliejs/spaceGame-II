#include "aa_tree.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static inline
aa_node_t * get_node(aa_tree_t * tree, off_t node) {
  return (aa_node_t *)((unsigned char *) tree->node_root + node * tree->node_size);
}

static inline
aa_node_t * get_rel_node(aa_tree_t * tree, aa_node_t * node, off_t rel_off) {
  return (aa_node_t *)((unsigned char *) node + rel_off * tree->node_size);
}

static inline
void * get_data(aa_tree_t * tree, aa_node_t * node) {
  return (void *)((unsigned char *) node + tree->data_off);
}

off_t skew(aa_tree_t * tree, aa_node_t * node) {

  if (node->level == get_rel_node(tree, node, node->left)->level) {
    off_t left = node->left;
    node->left = get_rel_node(tree, node, left)->right + left;
    get_rel_node(tree, node, left)->right = -left;
    return left;
  }
  return 0;
}

off_t split(aa_tree_t * tree, aa_node_t * node) {
  if (node->level == get_rel_node(tree, node, node->right + get_rel_node(tree, node, node->right)->right)->level) {
    off_t right = node->right;
    node->right = get_rel_node(tree, node, right)->left + right;
    get_rel_node(tree, node, right)->left = -right;
    get_rel_node(tree, node, right)->level++;
    return right;
  }
  return 0;
}

off_t successor(aa_tree_t * tree, off_t abs_off) {
  aa_node_t * node = get_node(tree, abs_off);
  abs_off += node->right;
  node = get_node(tree, abs_off);
  while (
    get_node(tree, abs_off + node->left)->left != 0
    && get_node(tree, abs_off + node->left)->right != 0) {
    abs_off += node->left;
    node = get_node(tree, abs_off);
  }
  return abs_off;
}

off_t predecessor(aa_tree_t * tree, off_t abs_off) {
  aa_node_t * node = get_node(tree, abs_off);
  abs_off += node->left;
  node = get_node(tree, abs_off);
  while (
    get_node(tree, abs_off + node->right)->left != 0
    && get_node(tree, abs_off + node->right)->right != 0) {
    abs_off += node->right;
    node = get_node(tree, abs_off);
  }
  return abs_off;
}

void decrease_level(aa_tree_t * tree, aa_node_t * node) {
   unsigned int correct_level = 1 +
   (get_rel_node(tree, node, node->left)->level < get_rel_node(tree, node, node->right)->level
   ? get_rel_node(tree, node, node->left)->level
   : get_rel_node(tree, node, node->right)->level)
   ;

   if (correct_level < node->level) {
     node->level = correct_level;
     if (correct_level < get_rel_node(tree, node, node->right)->level)
      get_rel_node(tree, node, node->right)->level = correct_level;
   }
   return;
}
unsigned int aa_error_check_helper(aa_tree_t * tree, off_t abs_off, off_t max_off, unsigned int agg, unsigned int depth) {
  if (abs_off == 0) {
    for (unsigned int i = 0; i < depth; i++) printf("\t");
    printf("[node %u]: NIL\n", agg);
    return agg + 1;
  }
  if (abs_off < 0 || abs_off > max_off) {
    for (unsigned int i = 0; i < depth; i++) printf("\t");
    printf("[node %u]: off %lld out of bounds\n", agg, abs_off);
    return agg + 1;
  }
  if (get_node(tree, abs_off)->left == 0 && get_node(tree, abs_off)->right == 0) return agg;
  agg = aa_error_check_helper(tree, abs_off + get_node(tree, abs_off)->left, max_off, agg, depth + 1);
  for (unsigned int i = 0; i < depth; i++) printf("\t");
  printf("[node %u]: off %lld\n", agg++, abs_off);
  agg = aa_error_check_helper(tree, abs_off + get_node(tree, abs_off)->right, max_off, agg, depth + 1);
  return agg;
}

void aa_error_check(aa_tree_t * tree, off_t root, off_t max_off) {
  aa_error_check_helper(tree, root, max_off, 0, 0);
}

bool aa_find(aa_tree_t * tree, off_t root, void * data, void ** match) {
  aa_node_t * node = get_node(tree, root);
  while (1) {
    if (node->left == 0 && node->right == 0) return false;
    switch (tree->comparator(data, get_data(tree, node))) {
      case LT:
        ;
        node = get_rel_node(tree, node, node->left);
        break;

      case GT:
        ;
        node = get_rel_node(tree, node, node->right);
        break;

      default:
      case EQ:
        ;
        *match = get_data(tree, node);
        return true;
    }
  }
}

static
off_t aa_insert_helper(aa_tree_t * tree, aa_node_t * node, aa_node_t * to, off_t to_off) {
  if ((node)->left == 0 && (node)->right == 0) {
    *to = (aa_node_t) {
      .left = -to_off,
      .right = -to_off,
      .level = 1
    };
    off_t skew_off = skew(tree, to);
    off_t split_off = split(tree, get_rel_node(tree, to, skew_off));
    return to_off + skew_off + split_off;
  }

  switch (tree->comparator(get_data(tree, to), get_data(tree, node))) {
    case LT:
      ;
      node->left += aa_insert_helper(tree, get_rel_node(tree, node, node->left), to, to_off);
      break;

    case GT:
      ;
      node->right += aa_insert_helper(tree, get_rel_node(tree, node, node->right), to, to_off);
      break;

    default:
    case EQ:
      ;
      printf("[aa_tree] [ANGRY]: Inserting a node which exists in the tree\n");
  }

  off_t skew_off = skew(tree, node);
  off_t split_off = split(tree, get_rel_node(tree, node, skew_off));
  return skew_off + split_off;
}

void aa_insert(aa_tree_t * tree, off_t * root, off_t to) {
  *root += aa_insert_helper(tree, get_node(tree, *root), get_node(tree, to), to);
}

static
off_t aa_delete_helper(aa_tree_t * tree, off_t abs_off, void * data) {
  aa_node_t * node = get_node(tree, abs_off);
  off_t delta_off = 0;
  if (node->left == 0 && node->right == 0) return 0;
  switch (tree->comparator(data, get_data(tree, node))) {
    case LT:
      ;
      node->left += aa_delete_helper(tree, abs_off + node->left, data);
      break;

    case GT:
      ;
      node->right += aa_delete_helper(tree, abs_off + node->right, data);
      break;

    default:
    case EQ:
      ;
      aa_node_t * left = get_node(tree, abs_off + node->left);
      aa_node_t * right = get_node(tree, abs_off + node->right);
      if (
        left->left   == 0 &&
        left->right  == 0 &&
        right->left  == 0 &&
        right->right == 0) {
        return -abs_off;
      }
      else if (left->left == 0 && left->right == 0) {
        off_t succ = successor(tree, abs_off);
        delta_off = succ - abs_off;
        off_t right = abs_off + node->right;
        node->right += delta_off;
        node->right += aa_delete_helper(tree, right, data);
      } else {
        off_t pred = predecessor(tree, abs_off);
        delta_off = pred - abs_off;
        off_t left = abs_off + node->left;
        node->left += delta_off;
        node->left += aa_delete_helper(tree, left, data);
      }
      break;
  }

  decrease_level(tree, node);
  off_t skew_off = skew(tree, node);
  node += skew_off;
  node->right += skew(tree, get_rel_node(tree, node, node->right));
  get_rel_node(tree, node, node->right)->right += skew(tree, get_rel_node(tree, node, get_rel_node(tree, node, node->right)->right));
  off_t split_off = split(tree, node);
  node = get_rel_node(tree, node, split_off);
  node->right += split(tree, get_rel_node(tree, node, node->right));
  return skew_off + split_off + delta_off;
}

void aa_delete(aa_tree_t * tree, off_t * root, void * data) {
  *root += aa_delete_helper(tree, *root, data);
}

aa_tree_t create_aa_tree(void * node_root, size_t node_size, void * data_root,
                         compare_t (* comparator) (void * a, void * b)) {
  aa_tree_t tree = (aa_tree_t) {
    .node_root = node_root,
    .node_size = node_size,
    .data_off = (uintptr_t) data_root - (uintptr_t) node_root,

    .comparator = comparator,
  };

  return tree;
}

aa_tree_t * rebase_aa_tree(aa_tree_t * tree, void * node_root, void * data_root) {
  tree->node_root = node_root;
  tree->data_off = (uintptr_t) data_root - (uintptr_t) node_root;

  return tree;
}
