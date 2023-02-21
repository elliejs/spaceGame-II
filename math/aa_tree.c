#include "aa_tree.h"
#include <stdlib.h>
#include <stdio.h>

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

aa_node_t * successor(aa_node_t * node, aa_node_t * nil) {
  node = node->right;
  while (node->left != nil) node = node->left;
  return node;
}

aa_node_t * predecessor(aa_node_t * node, aa_node_t * nil) {
  node = node->left;
  while (node->right != nil) node = node->right;
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

static
bool aa_find_helper(compare_t (* comparator) (void * a, void * b), aa_node_t * nil, aa_node_t * node, void * data, aa_node_t ** match) {
  if (node == nil) return false;
  switch (comparator(data, node->data)) {
    case LT:
      ;
      return aa_find_helper(comparator, nil, node->left, data, match);
      break;

    case GT:
      ;
      return aa_find_helper(comparator, nil, node->right, data, match);
      break;

    default:
    case EQ:
      *match = node;
      return true;
  }
}

bool aa_find(aa_tree_t * tree, void * data, aa_node_t ** match) {
  return aa_find_helper(tree->comparator, &(tree->nil), tree->root, data, match);
}

static
aa_node_t * aa_insert_helper(compare_t (* comparator) (void * a, void * b), aa_node_t * nil, aa_node_t * node, void * data, aa_node_t * to) {
  if (node == nil) {
    *to = (aa_node_t) {
      .left = nil,
      .right = nil,
      .data = data,
      .level = 1
    };
    node = to;
    return split(skew(node));
  }

  switch (comparator(data, node->data)) {
    case LT:
      ;
      node->left = aa_insert_helper(comparator, nil, node->left, data, to);
      break;

    case GT:
      ;
      node->right = aa_insert_helper(comparator, nil, node->right, data, to);
      break;

    default:
    case EQ:
      ;
      printf("[aa_tree] [ANGRY]: Inserting a node which exists in the tree\n");
  }
  return split(skew(node));
}

void aa_insert(aa_tree_t * tree, void * data, aa_node_t * to) {
  tree->root = aa_insert_helper(tree->comparator, &(tree->nil), tree->root, data, to);
}

static
aa_node_t * aa_delete_helper(compare_t (* comparator) (void * a, void * b), aa_node_t * nil, aa_node_t * node, void * data) {
  if (node == nil) return nil;
  switch (comparator(data, node->data)) {
    case LT:
      ;
      node->left = aa_delete_helper(comparator, nil, node->left, data);
      break;

    case GT:
      ;
      node->right = aa_delete_helper(comparator, nil, node->right, data);
      break;

    default:
    case EQ:
      if (node->left == nil && node->right == nil) {
        // printf("free(node)\n");
        free(node);
        return nil;
      }
      else if (node->left == nil) {
        aa_node_t * succ = successor(node, nil);
        node->data = succ->data;
        node->right = aa_delete_helper(comparator, nil, node->right, succ->data);
      } else {
        aa_node_t * pred = predecessor(node, nil);
        node->data = pred->data;
        node->left = aa_delete_helper(comparator, nil, node->left, pred->data);
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

void aa_delete(aa_tree_t * tree, void * data) {
  tree->root = aa_delete_helper(tree->comparator, &(tree->nil), tree->root, data);
}
