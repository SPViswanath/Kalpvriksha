#include <stdio.h>
#include <string.h>

int main()
{
    const int STANDARD = 150;
    const int PREMIUM = 250;
    const int RECLINER = 400;
    
    char name[100];
    scanf(" %s",name);
    
    int name_length = strlen(name);
    while(name_length>0 && name[name_length-1] == ' '){
        name[name_length-1] = '\0'; //remove trailing space
        name_length--;
    }
    
    int ticket_count;
    while(1){
        if((scanf("%d",&ticket_count))!=1){
            
            int ch;
            while((ch = getchar())!='\n' && ch!=EOF)
            
            printf("Error: Enter Invalid ticket_count.");
            continue;
        }
        if(ticket_count>=1 && ticket_count<=20)
            break;
            
        printf("Error: Enter Invalid ticket_count.");
    }
    
    int category;
    scanf("%d", &category);
    
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