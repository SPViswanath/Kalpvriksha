#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void generate_matrix(int size, int (*matrix)[size] ){
    for(int i=0;i<size;i++)
        for(int j=0;j<size;j++)
            *(*(matrix+i)+j) = rand() %256;

}

void rotate_matrix(int size, int (*matrix)[size] ){
    
    int temp=0;
    for(int i=0;i<size;i++){
        for(int j=i+1;j<size;j++){
            temp = *(*(matrix+i)+j);
            *(*(matrix+i)+j) = *(*(matrix+j)+i);
            *(*(matrix+j)+i) = temp;
        }
    }
    
    for(int i=0;i<size;i++){
        for(int j=0;j<size/2;j++){
            temp = *(*(matrix+i)+j);
            *(*(matrix+i)+j) = *(*(matrix+i)+(size-1-j));
            *(*(matrix+i)+(size-1-j)) = temp;
        }
    }
}

void apply_smoothing_filter(int size, int (*matrix)[size] ){
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

void print_matrix(int size, int (*matrix)[size] ){
    for(int i=0;i<size;i++){
        for(int j=0;j<size;j++){
            printf("%d ", *(*(matrix+i)+j));
        }
        printf("\n");
    }
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
    
    
    int matrix[size][size] ;
    generate_matrix(size, matrix);
    
    
    printf("Original Randomly Generated Matrix:\n");
    print_matrix(size, matrix);
    
    printf("Matrix after 90° Clockwise Rotation:\n");
    rotate_matrix(size, matrix);
    print_matrix(size, matrix);
    
    
    apply_smoothing_filter(size, matrix);
    printf("Matrix after Applying 3×3 Smoothing Filter:\n");
    print_matrix(size, matrix);
    
    return 0;
}