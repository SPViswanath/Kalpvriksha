#include <stdio.h>
#include <stdlib.h>
struct User{
   int id;
   char name[50];
   int age;
};

int unique_id(int target){
    FILE *fp = fopen("users.txt", "r");
    if(fp == NULL){
        printf("File not opening...!\n");
        return 0;
    }

    int e_id;
    while(fscanf(fp, "%d %*s %*d", &e_id) == 1){
        if(e_id == target){
            return 0;
        }
    }
    return 1;
}

void create_user(){
    FILE *fp = fopen("users.txt", "a");
    if(fp == NULL){
        printf("File not opening...!\n");
        return;
    }

    struct User u1;
    
    while(1){
        printf("Enter ID:");
        scanf("%d", &u1.id);
        if(unique_id(u1.id) == 1){
            break;
        }
        printf("Id exist try again...\n");
    }
    
    printf("Enter Name:");
    scanf("%s", u1.name);
    printf("Enter age:");
    scanf("%d", &u1.age);
    
    fprintf(fp, "%d %s %d\n", u1.id, u1.name, u1.age);
    fclose(fp);
}

void update_user(){
    FILE *fp = fopen("users.txt", "r");
    if(fp == NULL){
        printf("File not opening...!\n");
        return;
    }
    FILE *t = fopen("temp.txt", "w");
    if(t == NULL){
        printf("File not opening...!\n");
        return;
    }

    struct User u1;
    int target;
    printf("\nEnter Id: ");
    scanf("%d",&target);
    int f=0;
    while(fscanf(fp, "%d %s %d",&u1.id, u1.name, &u1.age) == 3){
        if(u1.id == target){
            f=1;
            char s[50];
            printf("Enter new Name : ");
            scanf("%s",s);
            int m_age;
            printf("Enter new Age : ");
            scanf("%d",&m_age);
            
            fprintf(t, "%d %s %d\n", u1.id, s, m_age);
            
        }
        else
        fprintf(t, "%d %s %d\n", u1.id, u1.name, u1.age);
    }
    
    if(!f){
        printf("User not found\n");
    }
    fclose(fp);
    fclose(t);
    remove("users.txt");
    rename("temp.txt", "users.txt");
    
}

void delete_user(){
    FILE *fp = fopen("users.txt", "r");
    if(fp == NULL){
        printf("File not opening...!\n");
        return;
    }
    FILE *t = fopen("temp.txt", "w");
    if(t == NULL){
        printf("File not opening...!\n");
        return;
    }

    struct User u1;
    int target;
    printf("\nEnter Id to delete: ");
    scanf("%d",&target);
    int f=0;
    while(fscanf(fp, "%d %s %d",&u1.id, u1.name, &u1.age) == 3){
        if(u1.id == target){
            f=1;
            printf("User found\n");
        }
        else
        fprintf(t, "%d %s %d\n", u1.id, u1.name, u1.age);
    }
    if(!f){
        printf("User not found\n");
    }
    fclose(fp);
    fclose(t);
    remove("users.txt");
    rename("temp.txt", "users.txt");
    
}

void display_users(){
    FILE *fp = fopen("users.txt", "r");
    if(fp == NULL){
        printf("File not opening..!\n");
        return;
    }
    struct User u1;
    printf("User List \n");
    while(fscanf(fp, "%d %s %d", &u1.id, u1.name, &u1.age) == 3){
        printf("Id: %d | Name: %s | Age: %d\n", u1.id, u1.name, u1.age);
    }
    fclose(fp);
  
}

int main(){
    
    int op;
    
    while(1){
        
        printf("1.Create User\n2.Update user\n3.Delete User\n4.Display user\n5.Exit\n");
        printf("Enter the Operation: ");
        scanf("%d", &op);
        
        switch(op){
            case 1: 
                create_user();
                break;
            case 2:
                update_user();
                break;
            case 3: 
                delete_user();
                break;
            case 4:
                display_users();
                break;
            case 5:
                printf("Exiting...");
                exit(0);
            default:
                printf("Enter valid option");
                break;
            
        }
    }
    return 0;
}