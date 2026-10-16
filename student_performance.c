#include <stdio.h>
#include <string.h>

#define subjects 3

struct stud{
    int roll_no;
    char name[50];
    int marks[subjects];
};

int total_marks(int mark1, int mark2, int mark3){
    return mark3+mark2+mark1;
}

float average_marks(int total){
    return (float)(total)/subjects;
}

int valid_mark(int mark){
    return (mark>=0 && mark<=100) ?1:0;
}

int unique_roll_no(int roll_no, int n, struct stud *student){
    
    for(int i=0;i<n;i++){
        if(student[i].roll_no == roll_no)
            return 0;
    }
    
    return 1;
    
}

char calculate_grade(float avg){
    char c;
    if(avg>=85)c='A';
    else if(avg>=70 && avg<85)c='B';
    else if(avg>=50 && avg<70)c='C';
    else if(avg>=35 && avg<50)c='D';
    else if(avg<35)c='F';
    
    return c;
}

void calculate_performance(char grade){
    switch(grade){
        case 'A':
            printf("Performance: *****");
            break; 
        case 'B':
            printf("Performance: ****");
            break;
        case 'C':
            printf("Performance: ***");
            break;
        case 'D':
            printf("Performance: **");
            break;
        
        
    }
    printf("\n");
}

void recursive_print(struct stud student[], int idx){
    if(idx < 0){
        return;
    }
    recursive_print(student, idx-1);
    printf("%d ",student[idx].roll_no);
}
int main()
{
    int n;
    scanf("%d", &n);
    if(n<1 || n>100){
        printf("Error : Invalid Student count.");
        return 1;
    }
    
    struct stud student[n];
    
    int students_entered=0;
    for(int i=0;i<n;i++){
       
        char name[50];
        // sscanf(c, "%d %50[^0-9] %d %d %d", &student[i].roll_no, name, &student[i].mark1, &student[i].mark2, &student[i].mark3);
        
        while(1){
            scanf("%d", &student[i].roll_no);
            if(unique_roll_no(student[i].roll_no, students_entered, student))
                break;
            else
                printf("Error: Roll number exist.");
        }
        
        scanf(" %49[^0-9]", name);
        
        int len = strlen(name);

        while (len > 0 && name[len - 1] == ' ') {
            name[len - 1] = '\0';
            len--;
        }
        strcpy(student[i].name, name);
        
       for(int j=0;j<subjects;j++){
           int mark;
           while(1){
               scanf("%d", &mark);
               if(valid_mark(mark)){
                   student[i].marks[j] = mark;
                   break;
               }
           }
       }
       
       students_entered++;
    }
       
    for(int i=0;i<n;i++){
        printf("\n");
        printf("Roll: %d\n", student[i].roll_no);
        printf("Name: %s\n", student[i].name);
        
        int total = total_marks(student[i].marks[0], student[i].marks[1], student[i].marks[2]);
        printf("Total: %d\n", total);
        
        float avg  = average_marks(total);
        printf("Average: %.2f\n", avg);
        
        char grade = calculate_grade(avg);
        printf("Grade: %c\n", grade);
        
        if(grade == 'F')continue;
        
        calculate_performance(grade);
        
        
    }
    printf("\n");
    printf("List of Roll Numbers (via recursion): ");
    recursive_print(student, n-1);
       
    return 0;
}