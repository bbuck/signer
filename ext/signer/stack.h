#ifndef _stack_h
#define _stack_h

typedef struct signer_stack signer_stack;

struct signer_stack {
    void *value;
    signer_stack *next;
    signer_stack *base;
};

signer_stack *signer_stack_make();
signer_stack *signer_stack_push(signer_stack *stk, void *value);
signer_stack *signer_stack_pop(signer_stack *stk);
void signer_stack_destroy(signer_stack *stack);
int signer_stack_empty(signer_stack *stack);

#endif
