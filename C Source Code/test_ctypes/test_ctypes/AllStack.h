#ifndef ALLSTACK_H
#define ALLSTACK_H



void StackInit(Stack* , int);

void StackPush(Stack* s, TreeNode* item);

TreeNode* StackPop(Stack* s);

int StackIsEmpty(Stack* s);


#endif // ALLSTACK_H

