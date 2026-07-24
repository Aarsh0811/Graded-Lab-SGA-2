#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define FILE_NAME "employee_records.dat"

typedef struct
{
    int id;
    char name[30];
    double salary;
} Employee;

void write_record(int fd, Employee *employee)
{
    ssize_t bytes = write(fd, employee, sizeof(Employee));

    if (bytes != sizeof(Employee))
    {
        perror("write");
        close(fd);
        exit(EXIT_FAILURE);
    }
}

void display_record(Employee *employee)
{
    printf("ID: %d, Name: %s, Salary: %.2f\n",
           employee->id,
           employee->name,
           employee->salary);
}

int main(void)
{
    Employee employees[3] =
    {
        {101, "Aman", 45000.00},
        {102, "Priya", 52000.00},
        {103, "Rahul", 48000.00}
    };

    int fd = open(FILE_NAME, O_CREAT | O_TRUNC | O_RDWR, 0644);

    if (fd == -1)
    {
        perror("open");
        return EXIT_FAILURE;
    }

    printf("File created successfully.\n");

    for (int i = 0; i < 3; i++)
    {
        write_record(fd, &employees[i]);
    }

    printf("Three employee records were written.\n");

    Employee updated = {102, "Priya", 60000.00};

    off_t update_position = 1 * sizeof(Employee);

    if (lseek(fd, update_position, SEEK_SET) == -1)
    {
        perror("lseek update");
        close(fd);
        return EXIT_FAILURE;
    }

    write_record(fd, &updated);

    printf("Employee 102 was updated without rewriting the entire file.\n");

    Employee retrieved;

    off_t retrieve_position = 2 * sizeof(Employee);

    if (lseek(fd, retrieve_position, SEEK_SET) == -1)
    {
        perror("lseek retrieve");
        close(fd);
        return EXIT_FAILURE;
    }

    ssize_t bytes = read(fd, &retrieved, sizeof(Employee));

    if (bytes != sizeof(Employee))
    {
        perror("read");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("\nRetrieved third record:\n");
    display_record(&retrieved);

    if (lseek(fd, 0, SEEK_SET) == -1)
    {
        perror("lseek beginning");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("\nAll records after update:\n");

    while ((bytes = read(fd, &retrieved, sizeof(Employee))) == sizeof(Employee))
    {
        display_record(&retrieved);
    }

    if (bytes == -1)
    {
        perror("read");
    }

    if (close(fd) == -1)
    {
        perror("close");
        return EXIT_FAILURE;
    }

    printf("\nFile closed successfully.\n");

    return EXIT_SUCCESS;
}
