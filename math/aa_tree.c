#include "aa_tree.h"
#include <stdlib.h>
#include <stdio.h>

unsigned int aa_error_check_helper(aa_node_t * root, off_t abs_off, off_t max_off, unsigned int node_idx, unsigned int depth) {
  if (abs_off == 0) {
    for (unsigned int i = 0; i < depth; i++) printf("\t");
    printf("[node %u]: NIL\n", node_idx);
    return node_idx + 1;
  }
  if (abs_off < 0 || abs_off > max_off) {
    for (unsigned int i = 0; i < depth; i++) printf("\t");
    printf("[node %u]: off %ld out of bounds\n", node_idx, abs_off);
    return node_idx + 1;
  }
  if ((root + abs_off)->left == 0 && (root + abs_off)->right == 0) return node_idx;
  node_idx = aa_error_check_helper(root, abs_off + (root + abs_off)->left, max_off, node_idx, depth + 1);
  for (unsigned int i = 0; i < depth; i++) printf("\t");
  printf("[node %u]: off %ld\n", node_idx++, abs_off);
  node_idx = aa_error_check_helper(root, abs_off + (root + abs_off)->right, max_off, node_idx, depth + 1);
  return node_idx;
}

void aa_error_check(aa_node_t * root, off_t abs_off, off_t max_off) {
//   printf("abs_off %ld, max_off %ld\n", abs_off, max_off);
  aa_error_check_helper(root, abs_off, max_off, 0, 0);
}

off_t skew(aa_node_t * tree) {
  if (tree->level == (tree + tree->left)->level) {
    off_t left = tree->left;
    tree->left = (tree + left)->right + left;
    (tree + left)->right = -left;
    return left;
  }
  return 0;
}

off_t split(aa_node_t * tree) {
  if (tree->level == (tree + tree->right + (tree + tree->right)->right)->level) {
    off_t right = tree->right;
    tree->right = (tree + right)->left + right;
    (tree + right)->left = -right;
    (tree + right)->level++;
    return right;
  }
  return 0;
}

aa_node_t * successor(aa_node_t * node) {
  node = (node + node->right);
  while ((node + node->left)->left != 0 && (node + node->left)->right != 0) node = (node + node->left);
  return node;
}

aa_node_t * predecessor(aa_node_t * node) {
  node = (node + node->left);
  while ((node + node->right)->right != 0 && (node + node->right)->left != 0) node = (node + node->right);
  return node;
}

void decrease_level(aa_node_t * node) {
   unsigned int correct_level = 1 +
   ((node + node->left)->level < (node + node->right)->level
   ?(node + node->left)->level
   :(node + node->right)->level)
   ;

   if (correct_level < node->level) {
     node->level = correct_level;
     if (correct_level < (node + node->right)->level)
      (node + node->right)->level = correct_level;
   }
   return;
}

static
bool aa_find_helper(compare_t (* comparator) (void * a, void * b), aa_node_t * node, void * data, aa_node_t ** match) {
  if (node->left == 0 && node->right == 0) return false;
  switch (comparator(data, node->data)) {
    case LT:
      ;
      return aa_find_helper(comparator, (node + node->left), data, match);
      break;

    case GT:
      ;
      return aa_find_helper(comparator, (node + node->right), data, match);
      break;

    default:
    case EQ:
      *match = node;
      return true;
  }
}

bool aa_find(compare_t (* comparator) (void * a, void * b), aa_node_t * root, void * data, aa_node_t ** match) {
  return aa_find_helper(comparator, root, data, match);
}

static
off_t aa_insert_helper(compare_t (* comparator) (void * a, void * b), aa_node_t * node, void * data, aa_node_t * to, off_t to_off) {
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
    off_t skew_off = skew(node);
//     printf("skew_off %ld\n", skew_off);
    off_t split_off = split(node + skew_off);
//     printf("split_off %ld\n", split_off);
    return to_off + skew_off + split_off;
  }

  switch (comparator(data, node->data)) {
    case LT:
      ;
//       printf("LT %ld\n", node->left);
      node->left += aa_insert_helper(comparator, (node + node->left), data, to, to_off);
//       printf("LEFT THEN: %ld\n", node->left);
      break;

    case GT:
      ;
//       printf("GT %ld\n", node->right);
      node->right += aa_insert_helper(comparator, (node + node->right), data, to, to_off);
//       printf("RIGHT THEN: %ld\n", node->right);
      break;

    default:
    case EQ:
      ;
      printf("[aa_tree] [ANGRY]: Inserting a node which exists in the tree\n");
  }

  off_t skew_off = skew(node);
//   printf("skew_off %ld\n", skew_off);
  off_t split_off = split(node + skew_off);
//   printf("split_off %ld\n", split_off);
  return skew_off + split_off;
//   return abs_off + split(node + skew(node));
}

void aa_insert(compare_t (* comparator) (void * a, void * b), off_t * root, aa_node_t * tree, void * data, off_t to_off) {
//   printf("starting root %ld\n", *root);
  *root += aa_insert_helper(comparator, (tree + *root), data, tree + to_off, to_off);
//   printf("ending root %ld\n", *root);
}

static
off_t aa_delete_helper(compare_t (* comparator) (void * a, void * b), aa_node_t * node, void * data, off_t abs_off) {
  if (node->left == 0 && node->right == 0) return 0;
  switch (comparator(data, node->data)) {
    case LT:
      ;
      node->left += aa_delete_helper(comparator, (node + node->left), data, abs_off - node->left);
      break;

    case GT:
      ;
      node->right += aa_delete_helper(comparator, (node + node->right), data, abs_off - node->right);
      break;

    default:
    case EQ:
      if ((node + node->left)->left == 0 && (node + node->left)->right == 0 && (node + node->right)->left == 0 && (node + node->right)->right == 0) {
        printf("free(node)\n");
//         free(node);
        return -abs_off;
      }
      else if ((node + node->left)->left == 0 && (node + node->left)->right == 0) {
        aa_node_t * succ = successor(node);
        node->data = succ->data;
        node->right += aa_delete_helper(comparator, (node + node->right), succ->data, abs_off - node->right);
      } else {
        aa_node_t * pred = predecessor(node);
        node->data = pred->data;
        node->left += aa_delete_helper(comparator, node + node->left, pred->data, abs_off - node->left);
      }
      break;
  }

  decrease_level(node);
  off_t skew_off = skew(node);
  node += skew_off;
  node->right += skew(node + node->right);
  (node + node->right)->right += skew(node + (node + node->right)->right);
  off_t split_off = split(node);
  node += split_off;
  node->right += split(node + node->right);
  return skew_off + split_off;
}

void aa_delete(compare_t (* comparator) (void * a, void * b), off_t * root, aa_node_t * tree, void * data) {
  *root += aa_delete_helper(comparator, tree + *root, data, *root);
}
