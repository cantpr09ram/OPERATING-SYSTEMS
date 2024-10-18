#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>

#define SIZE 4096

int main() {
    char source[SIZE], target[SIZE];
    char buffer[SIZE];
    FILE *sourceFile, *targetFile;

    printf("Enter the source file name: ");
    fgets(source, SIZE, stdin);
    source[strcspn(source, "\n")] = '\0';

    if (!(sourceFile = fopen(source, "r"))) {
        perror("Error opening the source file");
        exit(1);
    }

    printf("Enter the target file name: ");
    fgets(target, SIZE, stdin);
    target[strcspn(target, "\n")] = '\0';

    if (fopen(target, "r") != NULL) {
        printf("Target file already exists. Overwrite? (y/n): ");
        char response;
        scanf(" %c", &response);
        if (response != 'y') {
            fclose(sourceFile);
            exit(0);
        }
    }

    if (!(targetFile = fopen(target, "a"))) {
        perror("Error opening the target file");
        fclose(sourceFile);
        exit(1);
    }

    size_t n;
    while ((n = fread(buffer, 1, SIZE, sourceFile)) > 0) {
        if (fwrite(buffer, 1, n, targetFile) != n) {
            printf("Error writing to the target file.\n");
            fclose(sourceFile);
            fclose(targetFile);
            exit(1);
        }
    }

    printf("File copied successfully.\n");

    if (fclose(sourceFile) != 0 || fclose(targetFile) != 0) {
        printf("Error closing the files.\n");
        exit(1);
    }

    return 0;
}
