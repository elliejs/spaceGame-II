#include "aa_tree.h"
#include <stdlib.h>
#include <stdio.h>

static inline
aa_node_t * get_node(aa_node_t * node, off_t abs_off, size_t node_size) {
  return (aa_node_t *)((unsigned char *) node + abs_off * node_size);
}

unsigned int aa_error_check_helper(aa_node_t * root, off_t abs_off, off_t max_off, unsigned int node_idx, unsigned int depth, size_t node_size) {
  if (abs_off == 0) {
    for (unsigned int i = 0; i < depth; i++) printf("\t");
    printf("[node %u]: NIL\n", node_idx);
    return node_idx + 1;
  }
  if (abs_off < 0 || abs_off > max_off) {
    for (unsigned int i = 0; i < depth; i++) printf("\t");
    printf("[node %u]: off %lld out of bounds\n", node_idx, abs_off);
    return node_idx + 1;
  }
  if (get_node(root, abs_off, node_size)->left == 0 && get_node(root, abs_off, node_size)->right == 0) return node_idx;
  node_idx = aa_error_check_helper(root, abs_off + get_node(root, abs_off, node_size)->left, max_off, node_idx, depth + 1, node_size);
  for (unsigned int i = 0; i < depth; i++) printf("\t");
  printf("[node %u]: off %lld\n", node_idx++, abs_off);
  node_idx = aa_error_check_helper(root, abs_off + get_node(root, abs_off, node_size)->right, max_off, node_idx, depth + 1, node_size);
  return node_idx;
}

void aa_error_check(aa_node_t * root, off_t abs_off, off_t max_off, size_t node_size) {
  aa_error_check_helper(root, abs_off, max_off, 0, 0, node_size);
}

off_t skew(aa_node_t * tree, size_t node_size) {

  if (tree->level == get_node(tree, tree->left, node_size)->level) {
    off_t left = tree->left;
    tree->left = get_node(tree, left, node_size)->right + left;
    get_node(tree, left, node_size)->right = -left;
    return left;
  }
  return 0;
}

off_t split(aa_node_t * tree, size_t node_size) {
  if (tree->level == get_node(tree, tree->right + get_node(tree, tree->right, node_size)->right, node_size)->level) {
    off_t right = tree->right;
    tree->right = get_node(tree, right, node_size)->left + right;
    get_node(tree, right, node_size)->left = -right;
    get_node(tree, right, node_size)->level++;
    return right;
  }
  return 0;
}

aa_node_t * successor(aa_node_t * node, size_t node_size) {
  node = get_node(node, node->right, node_size);
  while (get_node(node, node->left, node_size)->left != 0 && get_node(node, node->left, node_size)->right != 0) node = get_node(node, node->left, node_size);
  return node;
}

aa_node_t * predecessor(aa_node_t * node, size_t node_size) {
  node = get_node(node, node->left, node_size);
  while (get_node(node, node->right, node_size)->right != 0 && get_node(node, node->right, node_size)->left != 0) node = get_node(node, node->right, node_size);
  return node;
}

void decrease_level(aa_node_t * node, unsigned int node_size) {
   unsigned int correct_level = 1 +
   (get_node(node, node->left, node_size)->level < get_node(node, node->right, node_size)->level
   ?get_node(node, node->left, node_size)->level
   :get_node(node, node->right, node_size)->level)
   ;

   if (correct_level < node->level) {
     node->level = correct_level;
     if (correct_level < get_node(node, node->right, node_size)->level)
      get_node(node, node->right, node_size)->level = correct_level;
   }
   return;
}

static
bool aa_find_helper(compare_t (* comparator) (void * a, void * b), aa_node_t * node, void * data, aa_node_t ** match, size_t node_size) {
  if (node->left == 0 && node->right == 0) return false;
  switch (comparator(data, node->data)) {
    case LT:
      ;
      return aa_find_helper(comparator, get_node(node, node->left, node_size), data, match, node_size);
      break;

    case GT:
      ;
      return aa_find_helper(comparator, get_node(node, node->right, node_size), data, match, node_size);
      break;

    default:
    case EQ:
      *match = node;
      return true;
  }
}

bool aa_find(compare_t (* comparator) (void * a, void * b), aa_node_t * tree, off_t root, void * data, aa_node_t ** match, size_t node_size) {
  return aa_find_helper(comparator, get_node(tree, root, node_size), data, match, node_size);
}

static
off_t aa_insert_helper(compare_t (* comparator) (void * a, void * b), aa_node_t * node, void * data, aa_node_t * to, off_t to_off, size_t node_size) {
//   printf("to_off %ld\n", to_off);
  if ((node)->left == 0 && (node)->right == 0) {
//     printf("node is NIL\n");
    *to = (aa_node_t) {
      .left = -to_off,
      .right = -to_off,
      .data = data,
      .level = 1
    };
    node = to;
    off_t skew_off = skew(node, node_size);
//     printf("skew_off %ld\n", skew_off);
    off_t split_off = split(get_node(node, skew_off, node_size), node_size);
//     printf("split_off %ld\n", split_off);
    return to_off + skew_off + split_off;
  }

  switch (comparator(data, node->data)) {
    case LT:
      ;
//       printf("LT %ld\n", node->left);
      node->left += aa_insert_helper(comparator, get_node(node, node->left, node_size), data, to, to_off, node_size);
//       printf("LEFT THEN: %ld\n", node->left);
      break;

    case GT:
      ;
//       printf("GT %ld\n", node->right);
      node->right += aa_insert_helper(comparator, get_node(node, node->right, node_size), data, to, to_off, node_size);
//       printf("RIGHT THEN: %ld\n", node->right);
      break;

    default:
    case EQ:
      ;
      printf("[aa_tree] [ANGRY]: Inserting a node which exists in the tree\n");
  }

  off_t skew_off = skew(node, node_size);
//   printf("skew_off %ld\n", skew_off);
  off_t split_off = split(get_node(node, skew_off, node_size), node_size);
//   printf("split_off %ld\n", split_off);
  return skew_off + split_off;
//   return abs_off + split(node + skew(node));
}

void aa_insert(compare_t (* comparator) (void * a, void * b), aa_node_t * tree, off_t * root, void * data, off_t to_off, size_t node_size) {
//   printf("starting root %ld\n", *root);
  *root += aa_insert_helper(comparator, get_node(tree, *root, node_size), data, get_node(tree, to_off, node_size), to_off, node_size);
//   printf("ending root %ld\n", *root);
}

static
off_t aa_delete_helper(compare_t (* comparator) (void * a, void * b), aa_node_t * node, void * data, off_t abs_off, size_t node_size) {
  if (node->left == 0 && node->right == 0) return 0;
  switch (comparator(data, node->data)) {
    case LT:
      ;
      node->left += aa_delete_helper(comparator, get_node(node, node->left, node_size), data, abs_off - node->left, node_size);
      break;

    case GT:
      ;
      node->right += aa_delete_helper(comparator, get_node(node, node->right, node_size), data, abs_off - node->right, node_size);
      break;

    default:
    case EQ:
      if (get_node(node, node->left, node_size)->left == 0 && get_node(node, node->left, node_size)->right == 0 && get_node(node, node->right, node_size)->left == 0 && get_node(node, node->right, node_size)->right == 0) {
        printf("free(node)\n");
//         free(node);
        return -abs_off;
      }
      else if (get_node(node, node->left, node_size)->left == 0 && get_node(node, node->left, node_size)->right == 0) {
        aa_node_t * succ = successor(node, node_size);
        node->data = succ->data;
        node->right += aa_delete_helper(comparator, get_node(node, node->right, node_size), succ->data, abs_off - node->right, node_size);
      } else {
        aa_node_t * pred = predecessor(node, node_size);
        node->data = pred->data;
        node->left += aa_delete_helper(comparator, get_node(node, node->left, node_size), pred->data, abs_off - node->left, node_size);
      }
      break;
  }

  decrease_level(node, node_size);
  off_t skew_off = skew(node, node_size);
  node += skew_off;
  node->right += skew(get_node(node, node->right, node_size), node_size);
  get_node(node, node->right, node_size)->right += skew(get_node(node, get_node(node, node->right, node_size)->right, node_size), node_size);
  off_t split_off = split(node, node_size);
  node = get_node(node, split_off, node_size);
  node->right += split(get_node(node, node->right, node_size), node_size);
  return skew_off + split_off;
}

void aa_delete(compare_t (* comparator) (void * a, void * b), aa_node_t * tree,  off_t * root, void * data, size_t node_size) {
  *root += aa_delete_helper(comparator, get_node(tree, *root, node_size), data, *root, node_size);
}
