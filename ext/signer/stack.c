#include <stdlib.h>

#include "stack.h"
#include "util.h"

signer_stack *signer_stack_make() {
    signer_stack *stk = malloc(sizeof(signer_stack));
    check_mem(stk);

    stk->base = stk;
    stk->next = NULL;

    return stk;
error:
    return NULL;
}

signer_stack *signer_stack_push(signer_stack *stk, void *value) {
    signer_stack *new = signer_stack_make();
    if (new == NULL) {
        signer_stack_destroy(stk);

        return NULL;
    }

    new->value = value;
    new->next = stk;
    new->base = stk->base;

    return new;
}

signer_stack *signer_stack_pop(signer_stack *stk) {
    if (stk->next == NULL || stk->base == stk) {
        return stk;
    }

    signer_stack *next = stk->next;
    stk->next = NULL;
    signer_stack_destroy(stk);

    return next;
}

void signer_stack_destroy(signer_stack *stack) {
    if (stack == NULL) {
        return;
    }

    stack->value = NULL;
    stack->base = NULL;
    if (stack->next != NULL) {
        signer_stack_destroy(stack->next);
    }
    stack->next = NULL;

    free(stack);
}

int signer_stack_empty(signer_stack *stack) {
    return stack == stack->base;
}
