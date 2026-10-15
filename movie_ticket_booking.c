#include <stdio.h>
#include <string.h>
#include <ctype.h>

int valid_ticket_count(int *ticket_count){
    while(1){
        if((scanf("%d",ticket_count))!=1){
            
            int ch;
            while((ch = getchar())!='\n' && ch!=EOF);
            
            printf("Error: Enter Invalid ticket_count.\n");
            continue;
        }
        if(*ticket_count>=1 && *ticket_count<=20)
            return 1;
            
        printf("Error: Enter Invalid ticket_count.");
    } 
}

int valid_customer_name(char *name, size_t size){
    if(name == NULL || size == 0)
        return 0;
    
    char temp_buffer[size];
    while(1){
        if(fgets(temp_buffer, sizeof(temp_buffer), stdin)==NULL){
            printf("Error: Enter Proper name.\n");
            return 0;
        }
        
        temp_buffer[strcspn(temp_buffer, "\n")] = '\0';
        
        size_t start=0;
        while(temp_buffer[start]!='\0' && isspace((unsigned char) temp_buffer[start])){
                start++;
        }
        
        
        size_t end = strlen(temp_buffer);
        while(end > start && isspace((unsigned char) temp_buffer[end-1])){
            end--;
        }
        temp_buffer[end] = '\0';
        
        size_t len = end-start;
        
        if(start == end){
            printf("Error: Name cannot be empty.\n");
            continue;
        }
        if(len>=size){
            printf("Error: Name is too long.\n");
            continue;
        }
        
        int i=0;
        while(start<end){
            name[i++] = temp_buffer[start++];
        }
        name[i]='\0';
        
        int valid=1;
        for(i=0; name[i]!='\0'; i++){
            if(isspace((unsigned char) name[i])){
                printf("Error: Enter name without spaces.\n");
                valid = 0;
                break;
            }   
        }
        
        if(valid)return 1;
    }
}

int main()
{
    const int STANDARD = 150;
    const int PREMIUM = 250;
    const int RECLINER = 400;
    
    char name[100];
    if (!valid_customer_name(name, sizeof(name))) {
        printf("Error: Invalid name.\n");
        return 1;  
    }
    
    int ticket_count;
    if (!valid_ticket_count(&ticket_count)) {
        printf("Error: Invalid ticket_count.\n");
        return 1;
    }
    
    int category;
    while(1){
        if(scanf("%d", &category)!=1){
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
        
        printf("Error: Invalid category.\n");
        continue;
        }
        
        if(category>=1 && category<=3)break;
        
        printf("Error: Invalid category.\n");
        
    }
    
    float ticket_price;
    char ticket_category[50];
    
    switch(category){
        case 1:
            ticket_price = STANDARD;
            strcpy(ticket_category, "Standard");
            break;
        case 2:
            ticket_price = PREMIUM;
            strcpy(ticket_category, "Premium");
            break;
        case 3:
            ticket_price = RECLINER;
            strcpy(ticket_category, "Recliner");
            break;
        default:
            printf("Error: Invalid Category.");
            return 1;
    }
    
    float subtotal =ticket_price * ticket_count;
    
    printf("Customer: %s\n",name);
    printf("Ticket Category: %s\n", ticket_category);
    printf("Tickets: %d\n", ticket_count);
    printf("Subtotal: %.2f\n", subtotal);
    
    float discount = 0;
    
    if(ticket_count>=5){
        discount = subtotal * 0.10;
    }
    
    printf("Discount: %.2f\n", discount);
    printf("Final Bill: %.2f\n", subtotal-discount);
    
    
    
    return 0;
}