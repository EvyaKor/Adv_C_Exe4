#define _CRT_SECURE_NO_WARNINGS
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>*/ //uncomment this block to check for heap memory allocation leaks.
// Read https://docs.microsoft.com/en-us/visualstudio/debugger/finding-memory-leaks-using-the-crt-library?view=vs-2019
#include <stdio.h>
#include <string.h>
#define MAX 1023

typedef struct StudentCourseGrade
{
	char courseName[35];
	int grade;
}StudentCourseGrade;

typedef struct Student
{
	char name[35];
	StudentCourseGrade* grades; //dynamic array of courses
	int numberOfCourses;
}Student;


//Part A
void countStudentsAndCourses(const char* fileName, int** coursesPerStudent, int* numberOfStudents);
int countPipes(const char* lineBuffer, int maxCount);
char*** makeStudentArrayFromFile(const char* fileName, int** coursesPerStudent, int* numberOfStudents);
void printStudentArray(const char* const* const* students, const int* coursesPerStudent, int numberOfStudents);
void factorGivenCourse(char** const* students, const int* coursesPerStudent, int numberOfStudents, const char* courseName, int factor);
void studentsToFile(char*** students, int* coursesPerStudent, int numberOfStudents);

//Part B
Student* transformStudentArray(char*** students, const int* coursesPerStudent, int numberOfStudents);
void writeToBinFile(const char* fileName, Student* students, int numberOfStudents);
Student* readFromBinFile(const char* fileName);

int main()
{
	// Part A
	int* coursesPerStudent = NULL;
	int numberOfStudents = 0;
	char*** students = makeStudentArrayFromFile("studentList.txt", &coursesPerStudent, &numberOfStudents);
	factorGivenCourse(students, coursesPerStudent, numberOfStudents, "Advanced Topics in C", +5);
	printStudentArray(students, coursesPerStudent, numberOfStudents);
	//studentsToFile(students, coursesPerStudent, numberOfStudents); //this frees all memory. Part B fails if this line runs. uncomment for testing (and comment out Part B)
	
	
	// Part B
	Student* transformedStudents = transformStudentArray(students, coursesPerStudent, numberOfStudents);
	writeToBinFile("students.bin", transformedStudents, numberOfStudents);
	Student* testReadStudents = readFromBinFile("students.bin");


	// Free allocated memory
	for (int i = 0; i < numberOfStudents; i++)
	{
		free(transformedStudents[i].grades);
		free(testReadStudents[i].grades);
	}
	free(transformedStudents);
	free(testReadStudents);
	studentsToFile(students, coursesPerStudent, numberOfStudents); //this frees all memory from part A


	_CrtDumpMemoryLeaks(); //uncomment this block to check for heap memory allocation leaks.
	// Read https://docs.microsoft.com/en-us/visualstudio/debugger/finding-memory-leaks-using-the-crt-library?view=vs-2019

	return 0;
}

void countStudentsAndCourses(const char* fileName, int** coursesPerStudent, int* numberOfStudents)
{
	FILE* fptr; // Pointer for the file
	char str[MAX]; // variable for reading lines from file
	int students_counter = 0; // variable for counting lines

	fptr = fopen(fileName, "rt"); // Open file - Read mode
	if (fptr == NULL) // In case file did not open
	{
		printf("Error opening file!");
		exit(1);
	}

	// Get number of students
	while (!feof(fptr)) 
	{
		fgets(str, MAX, fptr);
		students_counter++;
	}
	*numberOfStudents = students_counter;

	// Memory allocation - dynamic int array
	*coursesPerStudent = (int*)malloc(students_counter * sizeof(int));
	if (!*coursesPerStudent)
	{
		printf("Memory allocation failed!\n");
		exit(1);
	}
	
	rewind(fptr); // Return pointer to start of file
	int i = 0;

	// Get number of pipes (row by row) and insert to array 
	while (!feof(fptr))
	{
		fgets(str, MAX, fptr);
		*(*coursesPerStudent + i) = countPipes(str, strlen(str));
		i++;
	}
	
	fclose(fptr); // Close file
}

int countPipes(const char* lineBuffer, int maxCount)
{
	int counter = 0;
	char* chr = lineBuffer;

	// In case pointer is NULL
	if (lineBuffer == NULL)
	{
		return -1;
	}

	// In case maxCount <= 0
	if (maxCount <= 0)
	{
		return 0;
	}

	for (int i = 0; i < maxCount; i++)
	{
		if (*chr == '|')
		{
			counter++;
		}
		chr++;
	}
	return counter;
}

char*** makeStudentArrayFromFile(const char* fileName, int** coursesPerStudent, int* numberOfStudents)
{
	countStudentsAndCourses(fileName, coursesPerStudent, numberOfStudents);

	// Memory allocation - dynamic char** array - primary array
	char*** students_arr = (char***)malloc(*numberOfStudents * sizeof(char**));
	if (!students_arr)
	{
		printf("Memory allocation failed!\n");
		exit(1);
	}

	// Memory allocation - dynamic char* arrays - secondary arrays
	int i;
	for (i = 0; i < *numberOfStudents; i++)
	{
		// Each element in primary array is a secondary array of char**
		students_arr[i] = (char**)malloc((((*(*coursesPerStudent + i)) * 2) + 1) * sizeof(char*));
		if (!students_arr[i])
		{
			printf("Memory allocation failed!\n");
			exit(1);
		}
	}

	FILE* fptr; // Pointer for the file
	fptr = fopen(fileName, "rt"); // Open file - Read mode
	if (fptr == NULL) // In case file did not open
	{
		printf("Error opening file!");
		exit(1);
	}
	char str[MAX]; // variable for reading lines from file
	char* token = str;; // variable for holding token (names/courses/grades)
	int token_size; // variable for holding token size
	i = 0;
	int j = 0;

	// Memory allocation - dynamic char arrays - tertiary arrays
	while (!feof(fptr))
	{
		fgets(str, MAX, fptr); // Read line by line from file
		token = strtok(str, "|,"); // Get token
		while (token != NULL)
		{
			token_size = strlen(token); // Get token size
			// Each element in primary->secondary array is a tertiary array of char*
			*(students_arr[i] + j) = (char*)malloc(token_size * sizeof(char) + 1);
			if (!*(students_arr[i] + j))
			{
				printf("Memory allocation failed!\n");
				exit(1);
			}
			strcpy(*(students_arr[i] + j), token); // Copy token to tertiary array
			j++;
			token = strtok(NULL, "\n|,"); // Re-Get token (also exclude '\n') 
		}
		j = 0;
		i++;
	}
	// Now each token (names/courses/grades) is sorted by array's hierarchy
	// primary->secondary->tertiary
	
	fclose(fptr); // Close file
	return students_arr;
}

void factorGivenCourse(char** const* students, const int* coursesPerStudent, int numberOfStudents, const char* courseName, int factor)
{
	// In case factor is not between [-20,20]
	if (factor < -20 || factor > 20)
	{
		return;
	}

	int i = 0;
	int j = 0;
	int new_grade; // Variable for holding the new grade
	for (i = 0; i < numberOfStudents; i++)
	{
		for (j = 0; j < ((coursesPerStudent[i] * 2) + 1); j++)
		{
			// Compare courseName with each student in the given array
			if (strcmp(*(students[i] + j), courseName) == 0)
			{
				new_grade = atoi(*(students[i] + j + 1)); // Convert the string to int value
				new_grade = new_grade + factor; // Add factor to grade
				if (new_grade < 0)
				{ // In case new grade is less then 0
					_itoa(0, *(students[i] + j + 1), 10);
					break;
				}
				if (new_grade > 100)
				{ // In case new grade is more then 100
					_itoa(100, *(students[i] + j + 1), 10);
					break;
				}
				_itoa(new_grade, *(students[i] + j + 1), 10); // Convert int value to string and input back to array
			}
		}
		j = 0;
	}
}

void printStudentArray(const char* const* const* students, const int* coursesPerStudent, int numberOfStudents)
{
	for (int i = 0; i < numberOfStudents; i++)
	{
		printf("name: %s\n*********\n", students[i][0]);
		for (int j = 1; j <= 2 * coursesPerStudent[i]; j += 2)
		{
			printf("course: %s\n", students[i][j]);
			printf("grade: %s\n", students[i][j + 1]);
		}
		printf("\n");
	}
}

void studentsToFile(char*** students, int* coursesPerStudent, int numberOfStudents)
{
	FILE* fptr; // Pointer for the file
	fptr = fopen("studentList.txt", "w+t"); // Open file - Write+ mode
	if (fptr == NULL) // In case file did not open
	{
		printf("Error opening file!");
		exit(1);
	}

	int i = 0;
	int j = 0;
	for (i = 0; i < numberOfStudents; i++)
	{
		fprintf(fptr, *(students[i])); // Write "StudentName" to file
		for (j = 1; j <= coursesPerStudent[i] * 2; j += 2)
		{ // Write "|Course,Grade" to file
			fprintf(fptr, "|%s,%s", *(students[i] + j), *(students[i] + j + 1));
		}
		if (i != numberOfStudents - 1)
		{ // Write "\n" at the end of every line (besides the last line)
			fprintf(fptr, "\n");
		}
		j = 1;
	}
	fclose(fptr); // Close file
	
	// Free allocated memory
	for (i = 0; i < numberOfStudents; i++)
	{
		for (j = 0; j < coursesPerStudent[i] * 2 + 1; j++)
		{ 
			free(*(students[i] + j)); // Free tertiary level
		}
		free(students[i]); // Free secondary level
	}
	free(students); // Free primary level
	free(coursesPerStudent); // Free coursesPerStudent
}

void writeToBinFile(const char* fileName, Student* students, int numberOfStudents)
{
	FILE* fptr; // Pointer for the file
	fptr = fopen(fileName, "wb"); // Open file - Read mode
	if (fptr == NULL) // In case file did not open
	{
		printf("Error opening file!");
		exit(1);
	}

	fwrite(&numberOfStudents, sizeof(numberOfStudents), 1, fptr); // Write number of student to file
	int i = 0;
	int j = 0;
	for (i = 0; i < numberOfStudents; i++)
	{
		fwrite(students[i].name, sizeof(students[i].name), 1, fptr); // Write student name to file
		fwrite(&students[i].numberOfCourses, sizeof(students[i].numberOfCourses), 1, fptr); // Write number of courses to file
		for (j = 0; j < students[i].numberOfCourses; j++)
		{
			fwrite(students[i].grades[j].courseName, sizeof(students[i].grades[j].courseName) + 1, 1, fptr); // Write course name to file (add one byte for '\0')
			fwrite(&students[i].grades[j].grade, sizeof(students[i].grades[j].grade), 1, fptr); // Write course grade to file
		}
	}
	fclose(fptr); // Close file
}

Student* readFromBinFile(const char* fileName)
{
	FILE* fptr; // Pointer for the file
	fptr = fopen(fileName, "rb"); // Open file - Read mode
	if (fptr == NULL) // In case file did not open
	{
		printf("Error opening file!");
		exit(1);
	}

	int numberOfStudents; // Variable for holding size of dynamic students array
	fread(&numberOfStudents, sizeof(int), 1, fptr); // Get size

	// Memory allocation - dynamic Student array
	Student* students_dynamic_arr = (Student*)malloc(numberOfStudents * sizeof(Student));
	if (!students_dynamic_arr)
	{
		printf("Memory allocation failed!\n");
		exit(1);
	}

	int i = 0;
	int j = 0;
	char temp_str[35] = {0}; // Variable for holding a string
	int temp_int; // Variable for holding an integer
	for (i = 0; i < numberOfStudents; i++)
	{
		fread(temp_str, 35, 1, fptr); // Get student name
		strcpy(students_dynamic_arr[i].name, temp_str); // Copy student name to new array name field
		fread(&temp_int, sizeof(int), 1, fptr); // Get number of courses
		students_dynamic_arr[i].numberOfCourses = temp_int; // Put number of courses to new array numberOfCourses field
		// Memory allocation - dynamic array of courses and grades
		students_dynamic_arr[i].grades = (StudentCourseGrade*)malloc(students_dynamic_arr[i].numberOfCourses * sizeof(StudentCourseGrade));
		if (!students_dynamic_arr[i].grades)
		{
			printf("Memory allocation failed!\n");
			exit(1);
		}

		for (j = 0; j < students_dynamic_arr[i].numberOfCourses; j++)
		{
			fread(temp_str, 35, 1, fptr); // Get course name
			strcpy(students_dynamic_arr[i].grades[j].courseName, temp_str); // Copy course name to new array courseName field
			fseek(fptr, 1, SEEK_CUR); // Move pointer one byte (skip '\0')
			fread(&temp_int, sizeof(int), 1, fptr); // Get course grade
			students_dynamic_arr[i].grades[j].grade = temp_int; // Put course grade in new array grade field
		}
	}
	fclose(fptr); // Close file
	return students_dynamic_arr;
}

Student* transformStudentArray(char*** students, const int* coursesPerStudent, int numberOfStudents)
{
	// Memory allocation - dynamic Student array
	Student* students_dyn_arr = (Student*)malloc(numberOfStudents * sizeof(Student));
	if (!students_dyn_arr)
	{
		printf("Memory allocation failed!\n");
		exit(1);
	}

	int i = 0;
	int j = 0;
	int k = 0;
	for (i = 0; i < numberOfStudents; i++)
	{
		strcpy(students_dyn_arr[i].name, *(students[i] + j)); // Copy student name from received array to new array name field
		students_dyn_arr[i].numberOfCourses = coursesPerStudent[i]; // Copy number of courses from received array to new array numberOfCourses field
		// Memory allocation - dynamic array of courses and grades
		students_dyn_arr[i].grades = (StudentCourseGrade*)malloc(coursesPerStudent[i] * sizeof(StudentCourseGrade));
		if (!students_dyn_arr[i].grades)
		{
			printf("Memory allocation failed!\n");
			exit(1);
		}

		for (j = 1; j <= coursesPerStudent[i] * 2; j++)
		{
			strcpy(students_dyn_arr[i].grades[k].courseName, *(students[i] + j)); // Copy course name from received array to new array courseName field
			j++;
			students_dyn_arr[i].grades[k].grade = atoi(*(students[i] + j)); // Convert course grade from received array to integer in new array grade field
			k++;
		}
		k = 0;
		j = 0;
	}
	return students_dyn_arr;
}
