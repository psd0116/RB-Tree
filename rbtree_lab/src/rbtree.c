#include "rbtree.h"

#include <stdlib.h>

// 트리 생성하기
rbtree *new_rbtree(void) {
  // tree 구조체 동적 할당
  rbtree *p = (rbtree *)calloc(1, sizeof(rbtree));
  
  // nil 노드 생성 및 초기화
  node_t *nil = (node_t *)calloc(1,sizeof(node_t));
  nil->color = RBTREE_BLACK;
  
  // tree와 nil과 root를 nil 노드로 설정 (tree가 빈 경우 root는 nil 노드여야 한다)
  p->nil = nil;
  p->root = nil;

  return p;
}

// 각 노드와 그 자식들의 메모리를 반환 (메모리 누수 안나게)
// 후위 순회를 이용한 노드 삭제 헬퍼 함수
void traverse_and_delete_node(rbtree *t, node_t *node){
  if (node->left != t->nil){
    traverse_and_delete_node(t, node->left);
  }
  if (node->right != t->nil){
    traverse_and_delete_node(t, node->right);
  }

  // 현재 노드 메모리 반환
  free(node);
}

void delete_rbtree(rbtree *t) {
  node_t *node = t->root;
  if (node != t->nil){
    // 루트부터 순회하며 모든 노드 삭제
    traverse_and_delete_node(t,node);
  }
  free(t->nil);
  // 트리 구조체 자체 삭제
  free(t);
}

void left_rotate(rbtree *t, node_t *x){
  node_t *y = x->right; // y라는 노드 설정
  x->right = y->left; // y의 왼쪽 서브트리를 x의 오른쪽 서브트리로 옮김

  if (y->left != t->nil){
    y->left->parent = x;
  }
  // x의 부모를 y에 연결시킨다.
  y->parent = x->parent;

  if (x->parent == t->nil){ // x가 루트였다면
    t->root = y;
  } else if (x == x->parent->left){ // x가 왼쪽 자식이었다면
    x->parent->left = y;
  } else { // x가 오른쪽 자식이었다면
    x->parent->right = y;
  }

  y->left = x; // x를 y의 왼쪽 자식으로 설정
  x->parent = y;
}

// left_rotate와 정반대로 작동한다.
void right_rotate(rbtree *t, node_t *x){
  node_t *y = x->left; // y라는 노드 설정
  x->left = y->right;

  if (y->right != t->nil){
    y->right->parent = x;
  }
  
  y->parent = x->parent;

  if (x->parent == t->nil){
    t->root = y;
  } else if (x == x->parent->left){
    x->parent->left = y;
  } else {
    x->parent->right = y;
  }

  y->right = x;
  x->parent = y; 
}

void rbtree_fixup(rbtree *t, node_t *z);

node_t *rbtree_insert(rbtree *t, const key_t key)
{
  // 1. 새로운 노드 생성 및 초기화
  node_t *new_node = (node_t *)calloc(1, sizeof(node_t));
  new_node->key = key;
  new_node->color = RBTREE_RED;
  new_node->left = t->nil;
  new_node->right = t->nil;
  new_node->parent = t->nil; // 부모도 nil로 초기화

  // 2. 삽입 위치 탐색
  node_t *y = t->nil;  // y는 부모가 될 노드
  node_t *cur = t->root; // cur는 탐색용 포인터

  while (cur != t->nil){
    y = cur; // 부모노드(y)를 한 칸 아래로 이동
    if (new_node->key < cur->key){
      cur = cur->left;
    } else {
      cur = cur->right;
    }
  } // while 루프 종료

  // 3. 노드 삽입 (링크 연결)
  new_node->parent = y; // 새 노드의 부모를 y로 설정

  if (y == t->nil){
    // 트리가 비어있던 경우 (y가 nil이면)
    t->root = new_node;
  } else if (new_node->key < y->key){
    // y의 왼쪽 자식으로 삽입
    y->left = new_node;
  } else {
    // y의 오른쪽 자식으로 삽입
    y->right = new_node;
  }

  // 4. RB트리 속성 복구
  rbtree_fixup(t, new_node);

  // 5. 삽입된 노드 반환
  return new_node;
}

void rbtree_fixup(rbtree *t, node_t *z) {
    node_t *y; // y: 삼촌(uncle) 노드

    // 속성 4 위반: 부모(z->parent)가 RED일 때만 문제가 됨
    while (z->parent->color == RBTREE_RED) {
        
        // 부모가 할아버지의 왼쪽 자식인 경우
        if (z->parent == z->parent->parent->left) {
            y = z->parent->parent->right; // y = 삼촌
            
            // Case 1: 삼촌(y)이 RED인 경우 -> Recolor
            if (y->color == RBTREE_RED) {
                z->parent->color = RBTREE_BLACK;
                y->color = RBTREE_BLACK;
                z->parent->parent->color = RBTREE_RED;
                z = z->parent->parent; // z를 할아버지로 이동시켜 계속 검사
            } 
            // Case 2 & 3: 삼촌(y)이 BLACK인 경우
            else { 
                // Case 2: z가 부모의 오른쪽 자식인 경우 (꺾인 모양 'Triangle')
                // -> Rotate to Case 3 form
                if (z == z->parent->right) {
                    z = z->parent;
                    left_rotate(t, z);
                }
                // Case 3: z가 부모의 왼쪽 자식인 경우 (직선 모양 'Line')
                // -> Recolor and Rotate
                z->parent->color = RBTREE_BLACK;
                z->parent->parent->color = RBTREE_RED;
                right_rotate(t, z->parent->parent);
            }
        } 
        // 부모가 할아버지의 오른쪽 자식인 경우 (위와 대칭)
        else {
            y = z->parent->parent->left; // y = 삼촌
            
            // Case 1
            if (y->color == RBTREE_RED) {
                z->parent->color = RBTREE_BLACK;
                y->color = RBTREE_BLACK;
                z->parent->parent->color = RBTREE_RED;
                z = z->parent->parent;
            } 
            // Case 2 & 3
            else {
                // Case 2
                if (z == z->parent->left) {
                    z = z->parent;
                    right_rotate(t, z);
                }
                // Case 3
                z->parent->color = RBTREE_BLACK;
                z->parent->parent->color = RBTREE_RED;
                left_rotate(t, z->parent->parent);
      }
    }
  }
  // 속성 2: 루트는 항상 BLACK
  t->root->color = RBTREE_BLACK;
}

node_t *rbtree_find(const rbtree *t, const key_t key) {
  node_t *cur = t->root;

  // cur이 nil 노드(센티넬)에 도달할 때까지 탐색
  while (cur != t->nil){
    if (key == cur->key){
      return cur; // key를 찾음
    } else if (key < cur->key){
      cur = cur->left; // 키가 더 작으면 왼쪽으로
    } else {
      cur = cur->right; // 키가 더 크면 오른쪽으로
    }
  }
  return NULL; // 트리에 키가 없음 (NULL 반환)
}

node_t *rbtree_min(const rbtree *t) {
  node_t *cur = t->root;
  // 트리가 비어있으면 NULL 반환
  if (cur == t->nil){
    return NULL;
  }
  // 가장 왼쪽 노드를 찾아서 이동
  while (cur->left != t->nil){
    cur = cur->left;
  }
  return cur;
}

node_t *rbtree_max(const rbtree *t) {
  node_t *cur = t->root;
  
  // 트리가 비어 있으면 NULL 반환
  if (cur == t->nil){
    return NULL;
  }
  
  // 가장 오른쪽 노드를 찾아 이동
  while (cur->right != t->nil){
    cur = cur->right;
  }
  
  return cur;
}

// 노드 삭제 가장 복잡하다.
// 1. rbtree_transplant : u 노드의 자리에 v 노드를 연결시키는 함수
// 2. rbtree_min_node : (삭제시) 특정 서브트리(node)의 최소값 노드를 찾는 함수
// 3. rbtree_erase_fixup : 삭제 후 RBTree 속성을 복구하는 함수

// rbtree min과 다르게 입력으로 트리가 아닌 노드를 받는다
node_t *rbtree_min_node(rbtree *t, node_t *node){
  while (node->left != t->nil){
    node = node->left;
  }
  return node;
}

// u의 자리에 v를 이식하는 헬퍼 함수
void rbtree_transplant(rbtree *t, node_t *u, node_t *v){
  if (u->parent == t->nil){
    t->root = v;
  } else if (u == u->parent->left) { // u가 왼쪽 자식이었다면
    u->parent->left = v;
  } else { // u가 오른쪽 자식이었다면
    u->parent->right = v;
  }
  v->parent = u->parent; // v의 부모를 u의 부모로 설정한다. (v가 nil이어도 받아야 한다.)
}

void rbtree_erase_fixup(rbtree *t, node_t *x){
  node_t *w; // w : 형제 노드
  
  while  (x != t->root && x->color == RBTREE_BLACK){
    if (x == x->parent->left){
      w = x->parent->right; // w : 형제
      
      // case 1 : 형제(w)가 RED
      if (w->color == RBTREE_RED){
        w->color = RBTREE_BLACK;
        x->parent->color = RBTREE_RED;
        left_rotate(t, x->parent);
        w = x->parent->right; // Case 2, 3, 4로 전환
      }

      // case 2 : 형제가 BLACK, 형제의 두 자식도 BLACK
      if (w->left->color == RBTREE_BLACK && w->right->color == RBTREE_BLACK){
        w->color = RBTREE_RED; // 형제만 RED로
        x = x->parent; // x를 부모로 올려서 다시 검사
      }
      // case 3,4
      else {
        // case 3 : 형제의 왼쪽 자식만 RED (꺾인 모양)
        if (w->right->color == RBTREE_BLACK){
          w->left->color = RBTREE_BLACK;
          w->color = RBTREE_RED;
          right_rotate(t, w);
          w = x->parent->right; // case 4로 전환
        }

        // case 4 : 형제의 오른쪽 자식이 RED (직선 모양)
        w->color = x->parent->color;
        x->parent->color = RBTREE_BLACK;
        w->right->color = RBTREE_BLACK;
        left_rotate(t, x->parent);
        x = t->root; // 루프 종료
      }
    }
    // x가 오른쪽 자식인 경우(위와 대칭)
    else {
      w = x->parent->left;
      // case 1 : 형제가 RED
      if (w->color == RBTREE_RED){
        w->color = RBTREE_BLACK;
        x->parent->color = RBTREE_RED;
        right_rotate(t, x->parent);
        w = x->parent->left;
      }
      // case 2 : 형제가 BLACK, 형제의 두 자식도 BLACK
      if (w->right->color == RBTREE_BLACK && w->left->color == RBTREE_BLACK){
        w->color = RBTREE_RED;
        x = x->parent;
      }
      // case 3, 4
      else {
        // case 3 : 형제의 오른쪽 자식만 RED (꺾인 모양)
        if (w->left->color == RBTREE_BLACK){
          w->right->color = RBTREE_BLACK;
          w->color = RBTREE_RED;
          left_rotate(t,w);
          w = x->parent->left;
        }
        // case 4 : 형제의 왼쪽 자식이 RED (직선 모양)
        w->color = x->parent->color;
        x->parent->color = RBTREE_BLACK;
        w->left->color = RBTREE_BLACK;
        right_rotate(t, x->parent);
        x = t->root;
      }
    }
  }
  x->color = RBTREE_BLACK; // x가 루트이거나 RED가 되면 BLACK으로 변경
}

int rbtree_erase(rbtree *t, node_t *p) {
  // x : y의 원래 위치로 이동해올 노드 (fixup의 시작점)
  // y : 실제로 트리에서 제거될 노드
  // p : 삭제할 코드

  node_t *x;
  node_t *y = p;
  color_t y_original_color = y->color;

  // 실제로 제거될 노드(y)와 그 자리를 대체할 노드(x) 찾기

  // case 1 : p의 왼쪽 자식이 nil(자식이 0 or 1개)
  if (p->left == t->nil){
    x = p->right; // x는 p의 오른쪽 자식 (nil일 수도 있음)
    rbtree_transplant(t, p, p->right); // p의 자리에 p->right를 이식
  }
  // case 2 : p의 오른쪽 자식이 nil (자식이 1개)
  else if (p->right == t->nil){
    x = p->left; // x는 p의 왼쪽 자식
    rbtree_transplant(t, p, p->left); // p의 자리에 p->left를 이식
  }
  // case 3 : p의 자식이 2개
  else {
    // y = p의 successor (오른쪽 서브트리에서 가장 작은 노드)
    y = rbtree_min_node(t, p->right);
    y_original_color = y->color; // y의 색을 알아야 한다
    x = y->right; // x는 y의 오른쪽 자식 (y는 min이므로 왼쪽 자식이 없다.)

    if (y->parent == p){
      // y가 p의 바로 오른쪽 자식인 경우
      x->parent = y; // x가 nil일 수도 있으므로 부모를 y로 설정
    } else {
      // y가 z의 자식이 아닌 경우(더 깊이 있는 경우)
      rbtree_transplant(t, y, y->right); // y의 자리에 y->right를 이식
      y->right = p->right; // z의 오른쪽 자식을 y에게 넘김
      y->right->parent = y;
    }
    // y를 z의 자리로 이식
    rbtree_transplant(t, p, y);
    y->left = p->left; // z의 왼쪽 자식을 y에게 넘김
    y->left->parent = y;
    y->color = p->color; // z의 색을 y가 물려받음
  }
  // 속성 복구
  // 제거된 노드(y)의 원래 색이 BLACK이었다면 RBTree 속성 위반 가능성 높음
  if (y_original_color == RBTREE_BLACK){
    rbtree_erase_fixup(t, x); // x를 기준으로 복구 시작
  }
  free(p); // 삭제 요청된 노드(z) 메모리 해제
  return 0; // 성공!
}
// 중위 순회 헬퍼 함수
void inorder_travers(const rbtree *t, node_t *node, key_t *arr, size_t *index, const size_t n){
  // 배열이 꽉 찼거나, nil 노드에 도달하면 중단
  if (node == t->nil || *index >= n){
    return;
  }
  
  // 1. 왼쪽 서브트리 순회
  inorder_travers(t, node->left, arr, index, n);

  // 2. 현재 노드 방문 (배열에 값 저장)
  if (*index < n){
    arr[*index] = node->key;
    (*index)++; // 배열 인덱스 증가
  } else {
    return; // 배열 꽉참
  }

  // 3. 오른쪽 서브트리 순회
  inorder_travers(t, node->right, arr, index, n);
}
int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n) {
  if (t->root == t->nil || n == 0){
    return 0; // 빈 트리거나 배열 크기가 0인경우
  }

  size_t index = 0; // 배열에 저장된 요소의 수를 추적할 인덱스

  //중위 순회
  inorder_travers(t, t->root, arr, &index, n);

  return index; // 배열에 저장된 총 요소의 수 반환
}