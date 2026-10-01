#include <stdio.h>
#include <ctype.h>
#include<stdlib.h>
#include<string.h>
#define size 100

int num_stack[size];
int num_top=-1;

char op_stack[size];
int op_top=-1;

void push_num(int num){
    if(num_top == size-1){
        printf("num_stack Overflow\n");
        exit(EXIT_FAILURE);
    }
    else num_stack[++num_top] = num;
}

int pop_num(){
    if(num_top == -1){
        printf("num_stack underflow\n");
        exit(EXIT_FAILURE);
    }
    return num_stack[num_top--];
}

void push_op(char op){
    if(op_top == size-1){
        printf("op_stack overflow\n");
        exit(EXIT_FAILURE);
     }   
    else
        op_stack[++op_top] = op;
}

char pop_op(){
    if(op_top == -1){
        printf("op_stack underflow!\n");
        exit(EXIT_FAILURE);
    }
    return op_stack[op_top--];
}

char peek_op(){
    if(op_top == -1){
        printf("op_stack underflow!\n");
        exit(EXIT_FAILURE);
    }
    return op_stack[op_top];
    
}

int precedence(char op){
    int res = 0;
    switch(op){
        case '*':
        case '/':
            res=2;
            break;
        case '+':
        case '-':
            res=1;
            break;
        default:
            res=0;
            break;
    }
    return res;
}

int cal(int first, int sec, char exec_op){
    int res = 0;
    switch(exec_op){
        case '*':
            res=first*sec;
            break;
        case '/':
            if(sec == 0){
                printf("Error: Divide by zero");
                exit(EXIT_FAILURE);
            }
            res=first/sec;
            break;
        case '+':
            res=first+sec;
            break;
        case '-':
            res=first-sec;
            break;
            
    }
    return res;
}


int eval(char s[]){
    int idx=0;
    int expecting_char = 1;
    
    while(s[idx]!='\0'){
        
        if(isspace(s[idx])){
            idx++;
            continue;
        }
        
        if(isdigit(s[idx])){
            int cur_num = 0;
            
            if(expecting_char != 1){
                printf("Error : Invalid expression.");
                exit(EXIT_FAILURE);
            }
            expecting_char = 0;
            
            while(isdigit(s[idx])){
                cur_num = cur_num*10 + s[idx]-'0';
                idx++;
            }
            push_num(cur_num);
            continue;
        }
        
        if(s[idx] == '+' || s[idx] == '-' || s[idx] == '*' || s[idx] == '/'){
            char cur_op = s[idx];
            
            if(expecting_char != 0){
                printf("Error : Invalid expression.");
                exit(EXIT_FAILURE);
            }
            expecting_char = 1;
            
            while(op_top!=-1  && precedence(peek_op()) >= precedence(cur_op)){
                if(num_top < 1){
                    printf("Error: Invalid expression.\n");
                    exit(EXIT_FAILURE);
                }
                char exec_op = pop_op();
                int sec_num = pop_num();
                int first_num = pop_num();
                int temp = cal(first_num, sec_num, exec_op);
                push_num(temp);
            }
            
            push_op(cur_op);
            idx++;
            continue;
            
        }
        printf("Invalid char found: expression invalid");
        exit(EXIT_FAILURE);
        
    }
    if(expecting_char){
        printf("Error: Invalid expression");
        exit(EXIT_FAILURE);
    }
    while(op_top!=-1){
        if(num_top < 1){
            printf("Error: Invalid expression.\n");
            exit(EXIT_FAILURE);
        }
        char exec_op = pop_op();
        int sec_num = pop_num();
        int first_num = pop_num();
        int temp = cal(first_num, sec_num, exec_op);
        push_num(temp);
    }
    
    return num_stack[num_top];
} 

int main(void)
{
    char s[50];
  
    printf("Enter Your Expression: ");
    fgets(s, sizeof(s), stdin);
    
    s[strcspn(s, "\n")] = '\0';
    
    printf("Result = %d", eval(s));
    return 0;
}