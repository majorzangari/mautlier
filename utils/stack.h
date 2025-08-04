//
// Created by Major Zangari on 6/12/25.
//

#ifndef MAUTLIER_STACK_H
#define MAUTLIER_STACK_H

#include <stdio.h>

#define DEFINE_STACK_TYPE(name, stack_type, capacity)                          \
                                                                               \
  typedef struct {                                                             \
    stack_type data[capacity];                                                 \
    int top;                                                                   \
  } name;                                                                      \
                                                                               \
  static inline void name##_init(name *stack) { stack->top = -1; }             \
                                                                               \
  static inline void name##_push(name *stack, stack_type value) {              \
    if (stack->top + 1 >= capacity) {                                          \
      fprintf(stderr, "Stack is full; cannot push in %s\n", #name);            \
    }                                                                          \
    stack->data[++stack->top] = value;                                         \
  }                                                                            \
                                                                               \
  static inline stack_type name##_pop(name *stack) {                           \
    if (stack->top == -1) {                                                    \
      fprintf(stderr, "Stack is empty; cannot pop in %s\n", #name);            \
    }                                                                          \
    return stack->data[stack->top--];                                          \
  }                                                                            \
                                                                               \
  static inline stack_type name##_peek(name *stack) {                          \
    if (stack->top == -1) {                                                    \
      fprintf(stderr, "Stack is empty; cannot peek in %s\n", #name);           \
    }                                                                          \
    return stack->data[stack->top];                                            \
  }

// TODO: add way to iterate

#endif
