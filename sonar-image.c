#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void generate_matrix(int size, int **matrix ){
    for(int i=0;i<size;i++)
        for(int j=0;j<size;j++)
           *(*(matrix+i)+j) = rand() %256;
}

void rotate_matrix(int size, int **matrix ){
    
    for(int i=0;i<size;i++){
        for(int j=i+1;j<size;j++){
            int *address1 = *(matrix+i)+j;
            int *address2 = *(matrix+j)+i;
            
            int temp = *address1;
            *address1 = *address2;
            *address2 = temp;
            
        }
    }
    
    for(int i=0;i<size;i++){
        for(int j=0;j<size/2;j++){
            int *address1 = *(matrix+i)+j;
            int *address2 = *(matrix+i)+(size-1-j);
            
            int temp = *address1;
            *address1 = *address2;
            *address2 = temp;
            
        }
    }
}


void apply_smoothing_filter(int size, int **matrix ){
    int current_row[size], previous_row[size];
    
    for(int row=0;row<size;row++){
        
        for(int col=0;col<size;col++){
            *(current_row+col) = *(*(matrix+row)+col);
        }
        
        for(int col=0;col<size;col++){
            int sum=0, count=0, average=0;
            
            for(int i = row-1; i<=row+1;i++){
                int *ptr = NULL;
                if(i<0 || i>=size)
                    continue;
                if(i == row-1)ptr = previous_row;
                else if(i == row)ptr = current_row;
                else ptr = *(matrix+row+1);
                
                for(int j=col-1; j<=col+1;j++){
                    if(j<0 || j>=size)
                        continue;
                    sum+= *(ptr+j);
                    count++;
                }
            }
            
            average = sum/count;
            *(*(matrix+row)+col) = average; 
        }
        
        for(int col=0;col<size;col++){
            *(previous_row+col) = *(current_row+col);
        }
    }
}

void print_matrix(int size, int **matrix ){
    for(int i=0;i<size;i++){
        for(int j=0;j<size;j++){
            printf("%d ", *(*(matrix+i)+j));
        }
        printf("\n");
    }
}

void freeAll(int **matrix, int size){
    if(matrix == NULL){
        return;
    }
    
    for(int i=0;i<size;i++){
        free(*(matrix+i));
    }
    free(matrix);
}
int main(){
    srand((unsigned)time(NULL));
    int size;
    while(1){
        printf("Enther the size(2-9): \n");
        int result = scanf("%d", &size);
        if(result!=1 || (size<2 || size>10)){
            printf("Error: Enter valid size between 2 and 10(inclusive).\n");
           
            int charc;
            while((charc = getchar())!='\n'&& charc!=EOF);
           
            continue;
        }
        
        int charc;
        while((charc = getchar())!='\n'&& charc!=EOF);
        
        break;
        
    }
    
    int **matrix = malloc(size * sizeof(*matrix));
    // int matrix[size][size] ;
    for(int i=0;i<size;i++){
        *(matrix+i)= malloc(size * sizeof(int));
        if((*matrix+i) == NULL){
            freeAll(matrix, size)
            printf("Memory Allocation failed!");
            return 1;
        }
    }
    generate_matrix(size, matrix);
    
    
    printf("Original Randomly Generated Matrix:\n");
    print_matrix(size, matrix);
    
    printf("Matrix after 90° Clockwise Rotation:\n");
    rotate_matrix(size, matrix);
    print_matrix(size, matrix);
    
    
    apply_smoothing_filter(size, matrix);
    printf("Matrix after Applying 3×3 Smoothing Filter:\n");
    print_matrix(size, matrix);
    
    freeAll(matrix, size);
    return 0;
}