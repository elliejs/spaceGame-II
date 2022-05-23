#include "aa_tree.h"
#include <stdlib.h>
#include <stdio.h>

static
aa_node_t nil = (aa_node_t) {
  .left = &nil,
  .right = &nil,
  .data = NULL,
  .level = 0
};

#define NIL &nil

aa_tree_t create_tree(compare_t (*comparator) (void * a, void * b)) {
  return (aa_tree_t) {
    .comparator = comparator,
    .root = NIL
  };
}

aa_node_t * skew(aa_node_t * tree) {
  if (tree->level == tree->left->level) {
    aa_node_t * left = tree->left;
    tree->left = left->right;
    left->right = tree;
    return left;
  }
  return tree;
}

aa_node_t * split(aa_node_t * tree) {
  if (tree->level == tree->right->right->level) {
    aa_node_t * right = tree->right;
    tree->right = right->left;
    right->left = tree;
    right->level++;
    return right;
  }
  return tree;
}

static
aa_node_t * insert_helper(compare_t (* comparator) (void * a, void * b), aa_node_t * node, void * data, aa_node_t ** match) {
  if (node == NIL) {
    node = malloc(sizeof(aa_node_t));
    *node = (aa_node_t) {
      .left = NIL,
      .right = NIL,
      .data = data,
      .level = 1
    };
  }

  switch (comparator(data, node->data)) {
    case LT:
      ;
      node->left = insert_helper(comparator, node->left, data, match);
      break;

    case GT:
      ;
      node->right = insert_helper(comparator, node->right, data, match);
      break;

    default:
    case EQ:
      ;
      *match = node;
      break;
  }

  return split(skew(node));
}

aa_node_t * find_or_insert(aa_tree_t * tree, void * data) {
  aa_node_t * res;
  tree->root = insert_helper(tree->comparator, tree->root, data, &res);
  return res;
}

aa_node_t * successor(aa_node_t * node) {
  node = node->right;
  while (node->left != NIL) node = node->left;
  return node;
}

aa_node_t * predecessor(aa_node_t * node) {
  node = node->left;
  while (node->right != NIL) node = node->right;
  return node;
}

aa_node_t * decrease_level(aa_node_t * node) {
   unsigned int correct_level =
   (node->left->level < node->right->level ?
    node->left->level                      :
    node->right->level                     ) + 1;

   if (correct_level < node->level) {
     node->level = correct_level;
     if (correct_level < node->right->level)
      node->right->level = correct_level;
   }
   return node;
}

aa_node_t * delete_helper(compare_t (* comparator) (void * a, void * b), aa_node_t * node, void * data) {
  if (node == NIL) return NIL;
  switch (comparator(data, node->data)) {
    case LT:
      ;
      node->left = delete_helper(comparator, node->left, data);
      break;

    case GT:
      ;
      node->right = delete_helper(comparator, node->right, data);
      break;

    default:
    case EQ:
      if (node->left == NIL && node->right == NIL) {
        free(node);
        return NIL;
      }
      else if (node->left == NIL) {
        aa_node_t * succ = successor(node);
        node->data = succ->data;
        node->right = delete_helper(comparator, node->right, succ->data);
      } else {
        aa_node_t * pred = predecessor(node);
        node->data = pred->data;
        node->left = delete_helper(comparator, node->left, pred->data);
      }
      break;
  }
  node = decrease_level(node);
  node = skew(node);
  node->right = skew(node->right);
  node->right->right = skew(node->right->right);
  node = split(node);
  node->right = split(node->right);
  return node;
}

void delete(aa_tree_t * tree, void * data) {
  tree->root = delete_helper(tree->comparator, tree->root, data);
}
