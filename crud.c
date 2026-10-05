#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct User{
   int id;
   char name[50];
   int age;
};

int unique_id(int target){
    FILE *fp = fopen("users.txt", "r");
    if(fp == NULL){
        printf("File not opening...!\n");
        return 1;
    }

    int e_id;
    char c[100];
    while(fgets(c, sizeof c, fp) != NULL){
        e_id = atoi(strtok(c, "|"));
        
        if(e_id == target){
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);
    
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
    getchar();
    
    printf("Enter Name:");
    fgets(u1.name, sizeof(u1.name), stdin);
    u1.name[strcspn(u1.name, "\n")] = '\0';
    
    printf("Enter age:");
    scanf("%d", &u1.age);
    
    fprintf(fp, "%d|%s|%d\n", u1.id, u1.name, u1.age);
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
        fclose(fp);
        return;
    }

    struct User u1;
    int target;
    printf("\nEnter Id: ");
    scanf("%d",&target);
    getchar();
    
    int f=0;
    char mp[100];
    while(fgets(mp, sizeof(mp), fp) != NULL){
        
        u1.id = atoi(strtok(mp, "|"));
        strcpy(u1.name, strtok(NULL, "|"));
        u1.age = atoi(strtok(NULL, "|"));
        
        if(u1.id == target){
            f=1;
            char s[50];
            printf("Enter new Name : ");
            
            while(1){
                if(fgets(s, sizeof(s), stdin)==NULL){
                    printf("Input error Try again");
                    continue;
                }
                s[strcspn(s,"\n")]='\0';
                
                if(strlen(s) == 0){
                    printf("Name cannot be empty. Enter again: ");
                    continue;
                }
                break;
            }
            
            int m_age;
            printf("Enter new Age : ");
            scanf("%d",&m_age);
            
            fprintf(t, "%d|%s|%d\n", u1.id, s, m_age);
            
        }
        else
        fprintf(t, "%d|%s|%d\n", u1.id, u1.name, u1.age);
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
        fclose(fp);
        return;
    }

    struct User u1;
    int target;
    printf("\nEnter Id to delete: ");
    scanf("%d",&target);
    int f=0;
    
    char line[100];
    while(fgets(line, sizeof(line), fp) != NULL){
        u1.id= atoi(strtok(line, "|"));
        strcpy(u1.name, strtok(NULL, "|"));
        u1.age= atoi(strtok(NULL, "|"));
        
        if(u1.id == target){
            f=1;
            printf("User found\n");
        }
        else
        fprintf(t, "%d|%s|%d\n", u1.id, u1.name, u1.age);
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
    char cd[100];
    
    printf("User List \n");
    while(fgets(cd, sizeof(cd), fp)!=NULL){
         u1.id = atoi(strtok(cd, "|"));

        strcpy(u1.name, strtok(NULL, "|"));

        u1.age = atoi(strtok(NULL, "|"));

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