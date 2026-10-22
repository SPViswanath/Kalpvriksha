#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NAME_LEN 50
#define DESIG_LEN 50
#define LINE_LEN 200
#define MAX_EMPLOYEES 100
#define DESIGNATION_COUNT 7

typedef struct Employee {
    int id;
    char name[NAME_LEN];
    char designation[DESIG_LEN];

    struct Employee **reports;      
    int reportCount;

    struct Employee *manager;       
} Employee;

Employee *employees[MAX_EMPLOYEES];
int totalEmployees = 0;

const char *designationList[DESIGNATION_COUNT] = {
    "CEO",
    "Engineering Manager",
    "HR Manager",
    "Developer",
    "HR Executive",
    "Team Lead",
    "Tester"
};

int readInteger(int *value) {
    int number, ch;
    int result = scanf("%d", &number);

    if (result == EOF) return -1;
    if (result != 1) {
        while ((ch = getchar()) != '\n' && ch != EOF);
        printf("Enter a valid Integer\n");
        return 0;
    }
    while ((ch = getchar()) != '\n' && ch != EOF);
    *value = number;
    return 1;
}

int readLine(const char *prompt, char *buf, int size) {
    printf("%s", prompt);
    if (fgets(buf, size, stdin) == NULL) return 0;
    buf[strcspn(buf, "\n")] = '\0';
    return 1;
}

int matchDesignation(const char *start, size_t len) {
    int desIndex;
    size_t desLen;
    for (desIndex = 0; desIndex < DESIGNATION_COUNT; desIndex++) {
        if (strlen(designationList[desIndex]) != len) continue;
        for (desLen = 0; desLen < len; desLen++) {
            if (tolower((unsigned char)start[desLen]) !=
                tolower((unsigned char)designationList[desIndex][desLen])) break;
        }
        if (desLen == len) return desIndex;
    }
    return -1;
}

int parseEmployeeLine(char *line, int *id, char *name, int *desIndex, int *managerId) {
    int n = 0;
    *managerId = 0;

    char *arrow = strstr(line, "->");
    if (arrow) {
        *arrow = '\0';
        if (sscanf(arrow + 2, "%d", managerId) != 1) return 0;
    }
    if (sscanf(line, "%d %49s %n", id, name, &n) != 2 || *id <= 0) return 0;

    int len = strlen(line + n);
    while (len > 0 && isspace((unsigned char)line[n + len - 1])) len--;
    *desIndex = matchDesignation(line + n, len);
    return *desIndex >= 0;
}

Employee *findEmployee(int id) {
    for (int i = 0; i < totalEmployees; i++)
        if (employees[i]->id == id) return employees[i];
    return NULL;
}

Employee *createEmployee(int id, const char *name, const char *designation) {
    Employee *emp = (Employee *)malloc(sizeof(Employee));
    emp->id = id;
    strcpy(emp->name, name);
    strcpy(emp->designation, designation);
    emp->reports = NULL;
    emp->reportCount = 0;
    emp->manager = NULL;
    return emp;
}

void addToReports(Employee *manager, Employee *emp) {
    manager->reports = (Employee **)realloc(manager->reports,
                       (manager->reportCount + 1) * sizeof(Employee *));
    manager->reports[manager->reportCount++] = emp;
    emp->manager = manager;
}

void add_employee(void) {
    char line[LINE_LEN];
    char name[NAME_LEN];
    int id, desIndex, managerId;
    Employee *manager = NULL;

    if (totalEmployees >= MAX_EMPLOYEES) {
        printf("Employee limit reached.\n");
        return;
    }

    if (!readLine("Enter employee: ", line, LINE_LEN)) return;

    if (!parseEmployeeLine(line, &id, name, &desIndex, &managerId)) {
        printf("Invalid input. Format: ID Name Designation [-> ManagerID]\n");
        return;
    }

    if (findEmployee(id) != NULL) {
        printf("Employee ID %d already exists.\n", id);
        return;
    }
    if (managerId == id) {
        printf("An employee cannot report to themselves.\n");
        return;
    }
    if (managerId != 0) {
        manager = findEmployee(managerId);
        if (manager == NULL) {
            printf("Manager ID %d does not exist.\n", managerId);
            return;
        }
    }

    Employee *emp = createEmployee(id, name, designationList[desIndex]);
    if (manager != NULL)
        addToReports(manager, emp);

    employees[totalEmployees++] = emp;
    printf("Employee %d added successfully.\n", id);
}

int removeFromList(Employee **list, int *count, const Employee *emp) {
    for (int i = 0; i < *count; i++) {
        if (list[i] == emp) {
            for (int j = i; j < *count - 1; j++)
                list[j] = list[j + 1];
            (*count)--;
            return 1;
        }
    }
    return 0;
}

Employee *findLeastSibling(const Employee *manager) {
    Employee *best = NULL;
    for (int i = 0; i < manager->reportCount; i++) {
        Employee *sibling = manager->reports[i];
        if (best == NULL || sibling->reportCount < best->reportCount)
            best = sibling;
    }
    return best;
}

void remove_employee(void) {
    int id;

    printf("Enter Employee ID to remove: ");
    if (readInteger(&id) != 1) return;

    Employee *emp = findEmployee(id);
    if (emp == NULL) {
        printf("Employee ID %d not found.\n", id);
        return;
    }

    if (emp->manager == NULL || strcmp(emp->designation, "CEO") == 0) {
        printf("The top-level Employee cannot be removed.\n");
        return;
    }

    Employee *manager = emp->manager;
    removeFromList(manager->reports, &manager->reportCount, emp);

    Employee *newBoss = findLeastSibling(manager);
    if (newBoss == NULL) newBoss = manager;

    for (int i = 0; i < emp->reportCount; i++)
        addToReports(newBoss, emp->reports[i]);

    removeFromList(employees, &totalEmployees, emp);
    free(emp->reports);
    free(emp);
    printf("Employee %d removed successfully.\n", id);
}

int displayHierarchy(const Employee *emp, int level) {
    printf("%*s%d - %s - %s\n", level * 4, "", emp->id, emp->name, emp->designation);

    int count = 1;
    for (int i = 0; i < emp->reportCount; i++)
        count += displayHierarchy(emp->reports[i], level + 1);
    return count;
}

void freeAll(void) {
    for (int i = 0; i < totalEmployees; i++) {
        free(employees[i]->reports);
        free(employees[i]);
    }
    totalEmployees = 0;
}

int main(void) {
    int choice, r;
    while (1) {
        printf("\n===== EMPLOYEE HIERARCHY MANAGEMENT =====\n");
        printf("1. Add Employee\n");
        printf("2. Remove Employee\n");
        printf("3. Display Employee Hierarchy\n");
        printf("4. Display Complete Hierarchy\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");

        r = readInteger(&choice);
        if (r == -1) break;
        if (r == 0) continue;

        switch (choice) {
            case 1:
                add_employee();
                break;

            case 2:
                remove_employee();
                break;

            case 3: {
                int id;
                printf("Enter Employee ID: ");
                if (readInteger(&id) == 1) {
                    Employee *emp = findEmployee(id);
                    if (emp == NULL) {
                        printf("Employee ID %d not found.\n", id);
                    } else {
                        int count = displayHierarchy(emp, 0);
                        printf("Total Employees Under %d: %d\n", id, count - 1);
                    }
                }
                break;
            }

            case 4: {
                int total = 0;
                printf("\n===== ORGANIZATION HIERARCHY =====\n");
                for (int i = 0; i < totalEmployees; i++)
                    if (employees[i]->manager == NULL)
                        total += displayHierarchy(employees[i], 0);
                printf("Total Employees: %d\n", total);
                break;
            }

            case 5:
                freeAll();
                printf("Exiting...\n");
                return 0;

            default:
                printf("Invalid choice. Try again.\n");
        }
    }
    freeAll();      /* reached on EOF */
    return 0;
}