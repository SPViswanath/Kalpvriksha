#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Players_data.h"

#define MAX_TEAMS        10
#define MAX_NAME_LEN     50
#define MAX_TEAM_PLAYERS 50
#define MIN_ID           1
#define MAX_ID           1000
#define INPUT_BUFFER_LEN 128

typedef enum PlayerRole{
    BATSMAN = 1,
    BOWLER = 2,
    ALL_ROUNDER = 3
}PlayerRole;

typedef struct Player_Record{
    int id;
    char *name;
    const char* team;
    PlayerRole role;
    int totalRuns;
    float battingAverage;
    float strikeRate;
    int wickets;
    float economyRate;
    float PerformanceIndex;

    struct Player_Record *next;
} Player_Record;

typedef struct Team{
    int TeamId;
    char *TeamName;
    int TotalPlayers;
    float AverageBattingStrikerate;
    double strike_rate_sum;
    int strike_rate_count;
    Player_Record *batsman_head;
    Player_Record *bowler_head;
    Player_Record *all_rounder_head;
    struct Team *next_team;
}Team;

Team *Teams[MAX_TEAMS];
int num_teams = 0;

void freeAll(void);

const char *role_name(PlayerRole role){
    if(role == BATSMAN)
        return "Batsman";
    if(role == BOWLER)
        return "Bowler";
    return "All-rounder";
}

void exit_on_alloc_failure(void){
    printf("Memory Allocation Failed\n");
    freeAll();
    exit(1);
}

float calculate_performance_index(const Player_Record *player){
    float batting_part = (player->battingAverage * player->strikeRate) / 100.0f;

    if(player->role == BATSMAN)
        return batting_part;
    if(player->role == BOWLER)
        return (player->wickets * 2) + (100.0f - player->economyRate);
    return batting_part + (player->wickets * 2);
}

Player_Record **get_role_head(Team *team, PlayerRole role){
    if(role == BATSMAN)
        return &team->batsman_head;
    if(role == BOWLER)
        return &team->bowler_head;
    return &team->all_rounder_head;
}

Team *createTeamNode(int index, const char *name){
    Team *newNode = calloc(1, sizeof(Team));
    if(newNode == NULL){
        exit_on_alloc_failure();
    }

    newNode->TeamId = index;
    size_t namelen = strlen(name);
    newNode->TeamName = malloc(namelen+1);
    if(newNode->TeamName == NULL){
        free(newNode);
        exit_on_alloc_failure();
    }
    strcpy(newNode->TeamName, name);
    newNode->TotalPlayers = 0;
    newNode->AverageBattingStrikerate = 0.0f;

    newNode->batsman_head = NULL;
    newNode->bowler_head = NULL;
    newNode->all_rounder_head = NULL;
    newNode->next_team = NULL;
    return newNode;
}

void initialise_teams(){
    for(size_t team_index = 0; team_index < (size_t)teamCount && team_index < MAX_TEAMS; team_index++){
        Teams[team_index] = createTeamNode(team_index+1, teams[team_index]);
        num_teams++;
    }
}

PlayerRole getRole(const char *role){
    if (strcmp(role, "Batsman") == 0)
        return BATSMAN;

    if (strcmp(role, "Bowler") == 0)
        return BOWLER;

    if (strcmp(role, "All-rounder") == 0)
        return ALL_ROUNDER;

    printf("Invalid player role: %s\n", role);
    freeAll();
    exit(1);
}

Player_Record *createPlayer(const Player *data){
    Player_Record *newNode = malloc(sizeof(Player_Record));
    if(newNode == NULL){
        exit_on_alloc_failure();
    }
    newNode->id = data->id;
    size_t namelen = strlen(data->name);
    newNode->name = malloc(namelen+1);
    if(newNode->name == NULL){
        free(newNode);
        exit_on_alloc_failure();
    }
    strcpy(newNode->name, data->name);
    newNode->team = NULL;
    newNode->role = getRole(data->role);
    newNode->totalRuns = data->totalRuns;
    newNode->battingAverage = data->battingAverage;
    newNode->strikeRate = data->strikeRate;
    newNode->wickets = data->wickets;
    newNode->economyRate = data->economyRate;
    newNode->PerformanceIndex = calculate_performance_index(newNode);

    newNode->next = NULL;

    return newNode;
}

Team *findTeam(const char *teamname){
    int low = 0, high = num_teams - 1;
    while(low <= high){
        int mid = (low + high) / 2;
        int cmp = strcmp(teamname, Teams[mid]->TeamName);
        if(cmp == 0)
            return Teams[mid];
        if(cmp > 0)
            low = mid + 1;
        else
            high = mid - 1;
    }
    return NULL;
}

Team *find_team_by_id(int team_id){
    int low = 0, high = num_teams - 1;
    while(low <= high){
        int mid = (low + high) / 2;
        if(Teams[mid]->TeamId == team_id)
            return Teams[mid];
        if(Teams[mid]->TeamId < team_id)
            low = mid + 1;
        else
            high = mid - 1;
    }
    return NULL;
}

void insertintolist(Player_Record **head, Player_Record *player){
    if(*head == NULL || (*head)->PerformanceIndex < player->PerformanceIndex){
        player->next = *head;
        *head = player;
        return;
    }
    Player_Record *node = *head;
    while(node->next != NULL && node->next->PerformanceIndex >= player->PerformanceIndex){
        node = node->next;
    }
    player->next = node->next;
    node->next = player;
}

void insert_player(Team *playerteam, Player_Record *player){
    player->team = playerteam->TeamName;
    insertintolist(get_role_head(playerteam, player->role), player);
    playerteam->TotalPlayers++;

    if(player->role != BOWLER){
        playerteam->strike_rate_sum += player->strikeRate;
        playerteam->strike_rate_count++;
        playerteam->AverageBattingStrikerate =
            (float)(playerteam->strike_rate_sum / playerteam->strike_rate_count);
    }
}

void initialise_players(){
    for(size_t player_index = 0; player_index < (size_t)playerCount; player_index++){

        Team *playerteam = findTeam(players[player_index].team);
        if(playerteam == NULL){
            printf("Team Not Found: %s\n", players[player_index].team);
            freeAll();
            exit(1);
        }
        Player_Record *newNode = createPlayer(&players[player_index]);
        insert_player(playerteam, newNode);
    }
}

int player_id_exists(int player_id){
    for(int team_index = 0; team_index < num_teams; team_index++){
        for(int role = BATSMAN; role <= ALL_ROUNDER; role++){
            Player_Record *node = *get_role_head(Teams[team_index], (PlayerRole)role);
            while(node != NULL){
                if(node->id == player_id)
                    return 1;
                node = node->next;
            }
        }
    }
    return 0;
}

void trim_whitespace(char *text){
    size_t length = strlen(text);
    while(length > 0 && isspace((unsigned char)text[length - 1])){
        text[--length] = '\0';
    }
    size_t start = 0;
    while(text[start] != '\0' && isspace((unsigned char)text[start])){
        start++;
    }
    if(start > 0){
        memmove(text, text + start, strlen(text + start) + 1);
    }
}

int read_line(char *buffer, size_t size){
    if(fgets(buffer, (int)size, stdin) == NULL)
        return 0;
    if(strchr(buffer, '\n') == NULL){
        int ch;
        while((ch = getchar()) != '\n' && ch != EOF)
            ;
    }
    return 1;
}

int read_int(const char *prompt, int min, int max, int *value){
    char buffer[INPUT_BUFFER_LEN];
    char *end;
    printf("%s", prompt);
    if(!read_line(buffer, sizeof(buffer)))
        return 0;
    long parsed = strtol(buffer, &end, 10);
    while(isspace((unsigned char)*end))
        end++;
    if(end == buffer || *end != '\0' || parsed < min || parsed > max){
        printf("Invalid input (expected integer %d-%d).\n", min, max);
        return 0;
    }
    *value = (int)parsed;
    return 1;
}

int read_float(const char *prompt, float min, float max, float *value){
    char buffer[INPUT_BUFFER_LEN];
    char *end;
    printf("%s", prompt);
    if(!read_line(buffer, sizeof(buffer)))
        return 0;
    double parsed = strtod(buffer, &end);
    while(isspace((unsigned char)*end))
        end++;
    if(end == buffer || *end != '\0' || parsed < min || parsed > max){
        printf("Invalid input (expected number %.0f-%.0f).\n", min, max);
        return 0;
    }
    *value = (float)parsed;
    return 1;
}

int read_role(PlayerRole *role){
    int choice;
    if(!read_int("Enter Role (1-Batsman, 2-Bowler, 3-All-rounder): ", 1, 3, &choice))
        return 0;
    *role = (PlayerRole)choice;
    return 1;
}

int read_team(Team **team){
    int team_id;
    if(!read_int("Enter Team ID: ", MIN_ID, MAX_ID, &team_id))
        return 0;
    *team = find_team_by_id(team_id);
    if(*team == NULL){
        printf("Team with ID %d not found.\n", team_id);
        return 0;
    }
    return 1;
}

void print_line(int width){
    for(int i = 0; i < width; i++)
        putchar('=');
    putchar('\n');
}

void print_header(int show_team){
    if(show_team){
        print_line(100);
        printf("%-5s %-24s %-14s %-12s %6s %6s %6s %5s %5s %10s\n",
               "ID", "Name", "Team", "Role", "Runs", "Avg", "SR", "Wkts", "ER", "Perf.Index");
        print_line(100);
    }
    else{
        print_line(86);
        printf("%-5s %-24s %-12s %6s %6s %6s %5s %5s %10s\n",
               "ID", "Name", "Role", "Runs", "Avg", "SR", "Wkts", "ER", "Perf.Index");
        print_line(86);
    }
}

void print_player(const Player_Record *player, int show_team){
    if(show_team){
        printf("%-5d %-24.24s %-14.14s %-12s %6d %6.1f %6.1f %5d %5.1f %10.2f\n",
               player->id, player->name, player->team, role_name(player->role),
               player->totalRuns, player->battingAverage, player->strikeRate,
               player->wickets, player->economyRate, player->PerformanceIndex);
    }
    else{
        printf("%-5d %-24.24s %-12s %6d %6.1f %6.1f %5d %5.1f %10.2f\n",
               player->id, player->name, role_name(player->role),
               player->totalRuns, player->battingAverage, player->strikeRate,
               player->wickets, player->economyRate, player->PerformanceIndex);
    }
}

void add_player(){
    Team *playerteam;
    int id, role, runs, wickets;
    float average, strike_rate, economy;
    char name[INPUT_BUFFER_LEN];

    if(!read_team(&playerteam))
        return;
    if(playerteam->TotalPlayers >= MAX_TEAM_PLAYERS){
        printf("Team %s already has the maximum of %d players.\n",
               playerteam->TeamName, MAX_TEAM_PLAYERS);
        return;
    }

    printf("Enter Player Details:\n");
    if(!read_int("Player ID: ", MIN_ID, MAX_ID, &id))
        return;
    if(player_id_exists(id)){
        printf("Player ID %d already exists.\n", id);
        return;
    }

    printf("Name: ");
    if(!read_line(name, sizeof(name)))
        return;
    trim_whitespace(name);
    size_t namelen = strlen(name);
    if(namelen < 1 || namelen > MAX_NAME_LEN){
        printf("Invalid name (length must be 1-%d).\n", MAX_NAME_LEN);
        return;
    }

    if(!read_int("Role (1-Batsman, 2-Bowler, 3-All-rounder): ", 1, 3, &role))
        return;
    if(!read_int("Total Runs: ", 0, 1000000, &runs))
        return;
    if(!read_float("Batting Average: ", 0, 1000, &average))
        return;
    if(!read_float("Strike Rate: ", 0, 1000, &strike_rate))
        return;
    if(!read_int("Wickets: ", 0, 10000, &wickets))
        return;
    if(!read_float("Economy Rate: ", 0, 100, &economy))
        return;

    Player data = {id, name, playerteam->TeamName, role_name((PlayerRole)role),
                   runs, average, strike_rate, wickets, economy};
    Player_Record *newNode = createPlayer(&data);
    insert_player(playerteam, newNode);
    printf("Player added successfully to Team %s!\n", playerteam->TeamName);
}

void display_players_specific_Team(){
    Team *playerteam;
    if(!read_team(&playerteam))
        return;

    printf("Players of Team %s:\n", playerteam->TeamName);
    print_header(0);
    for(int role = BATSMAN; role <= ALL_ROUNDER; role++){
        Player_Record *node = *get_role_head(playerteam, (PlayerRole)role);
        while(node != NULL){
            print_player(node, 0);
            node = node->next;
        }
    }
    print_line(86);
    printf("Total Players: %d\n", playerteam->TotalPlayers);
    printf("Average Batting Strike Rate: %.2f\n", playerteam->AverageBattingStrikerate);
}

int compare_teams(const void *first, const void *second){
    const Team *team_a = *(Team *const *)first;
    const Team *team_b = *(Team *const *)second;
    if(team_a->AverageBattingStrikerate < team_b->AverageBattingStrikerate)
        return 1;
    if(team_a->AverageBattingStrikerate > team_b->AverageBattingStrikerate)
        return -1;
    return team_a->TeamId - team_b->TeamId;
}

void display_Teams_Avg_batting_st_rate(){
    Team *sorted_teams[MAX_TEAMS];
    memcpy(sorted_teams, Teams, sizeof(Team *) * num_teams);
    qsort(sorted_teams, num_teams, sizeof(Team *), compare_teams);

    printf("Teams Sorted by Average Batting Strike Rate\n");
    print_line(60);
    printf("%-4s %-18s %-14s %s\n", "ID", "Team Name", "Avg Bat SR", "Total Players");
    print_line(60);
    for(int i = 0; i < num_teams; i++){
        printf("%-4d %-18s %-14.2f %d\n", sorted_teams[i]->TeamId, sorted_teams[i]->TeamName,
               sorted_teams[i]->AverageBattingStrikerate, sorted_teams[i]->TotalPlayers);
    }
    print_line(60);
}

void display_Top_k(){
    Team *playerteam;
    PlayerRole role;
    int k;
    if(!read_team(&playerteam))
        return;
    if(!read_role(&role))
        return;
    if(!read_int("Enter number of players: ", 1, 100000, &k))
        return;

    Player_Record *node = *get_role_head(playerteam, role);
    if(node == NULL){
        printf("Team %s has no %s players.\n", playerteam->TeamName, role_name(role));
        return;
    }

    printf("Top %d %s players of Team %s:\n", k, role_name(role), playerteam->TeamName);
    print_header(0);
    int shown = 0;
    while(node != NULL && shown < k){
        print_player(node, 0);
        node = node->next;
        shown++;
    }
    print_line(86);
    if(shown < k)
        printf("Only %d %s player(s) available in this team.\n", shown, role_name(role));
}

void heap_sift_down(Player_Record **heap, int size, int index){
    while(1){
        int left = 2 * index + 1;
        int right = left + 1;
        int largest = index;
        if(left < size && heap[left]->PerformanceIndex > heap[largest]->PerformanceIndex)
            largest = left;
        if(right < size && heap[right]->PerformanceIndex > heap[largest]->PerformanceIndex)
            largest = right;
        if(largest == index)
            return;
        Player_Record *temp = heap[index];
        heap[index] = heap[largest];
        heap[largest] = temp;
        index = largest;
    }
}

void display_Players_Specific_role(){
    PlayerRole role;
    if(!read_role(&role))
        return;

    Player_Record *heap[MAX_TEAMS];
    int heap_size = 0;
    for(int team_index = 0; team_index < num_teams; team_index++){
        Player_Record *first = *get_role_head(Teams[team_index], role);
        if(first != NULL)
            heap[heap_size++] = first;
    }
    if(heap_size == 0){
        printf("No %s players found.\n", role_name(role));
        return;
    }
    for(int i = heap_size / 2 - 1; i >= 0; i--)
        heap_sift_down(heap, heap_size, i);

    printf("%s players of all teams (by performance index):\n", role_name(role));
    print_header(1);
    while(heap_size > 0){
        Player_Record *top = heap[0];
        print_player(top, 1);
        if(top->next != NULL)
            heap[0] = top->next;
        else
            heap[0] = heap[--heap_size];
        if(heap_size > 0)
            heap_sift_down(heap, heap_size, 0);
    }
    print_line(100);
}

void free_list(Player_Record *head){
    while(head != NULL){
        Player_Record *next = head->next;
        free(head->name);
        free(head);
        head = next;
    }
}

void freeAll(){
    for(int team_index = 0; team_index < MAX_TEAMS; team_index++){
        Team *team = Teams[team_index];
        if(team == NULL)
            continue;
        free_list(team->batsman_head);
        free_list(team->bowler_head);
        free_list(team->all_rounder_head);
        free(team->TeamName);
        free(team);
        Teams[team_index] = NULL;
    }
    num_teams = 0;
}

int main(void){

    initialise_teams();
    initialise_players();

    while(1){
        int choice;
        printf("\n"
       "======================================================================\n"
       "                 ICC ODI Player Performance Analyzer\n"
       "======================================================================\n"
       "  1. Add Player to Team\n"
       "  2. Display Players of a Specific Team\n"
       "  3. Display Teams by Average Batting Strike Rate\n"
       "  4. Display Top K Players of a Specific Team by Role\n"
       "  5. Display All Players of a Specific Role Across All Teams\n"
       "     by Performance Index\n"
       "  6. Exit\n"
       "======================================================================\n");

        if(!read_int("  Enter your choice: ", 1, 6, &choice)){
            if(feof(stdin)){
                freeAll();
                return 0;
            }
            continue;
        }

        switch (choice){
                case 1:
                    add_player();
                    break;
                case 2:
                    display_players_specific_Team();
                    break;
                case 3:
                    display_Teams_Avg_batting_st_rate();
                    break;
                case 4:
                    display_Top_k();
                    break;
                case 5:
                    display_Players_Specific_role();
                    break;
                case 6:
                    freeAll();
                    exit(0);
        }
    }
}