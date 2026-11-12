#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "helpers.h"

#define MAX_ROWS 1024 // adjust as needed. The csv is 815x5
#define NUM_FEATURES 5

void compute_barycenter(size_t features, size_t rows, float *X, float *out_barycenter);
void calculate_barycenter(size_t features, size_t rows, float *X, float *out_barycenter);
size_t calculate_barycenter_criteria(float *classPos1, float *classNeg1, float *X, float *filtered, size_t numFeatures, size_t rows);

void printCriteria(float *X){
	for (size_t i = 0; i < 5; i++)
	{
		printf("Row %zu: x1 = %.2f, x2 = %.2f, x3 = %.2f, x4 = %.2f, x5 = %.2f\n",
			   i,
			   X[i * NUM_FEATURES + 0],
			   X[i * NUM_FEATURES + 1],
			   X[i * NUM_FEATURES + 2],
			   X[i * NUM_FEATURES + 3],
			   X[i * NUM_FEATURES + 4]);
	}
}

void correctnessCheckCriteria(float *X, float *Y, size_t rows, size_t features)
{
	size_t mismatchCount = 0;

	for (size_t i = 0; i < rows; i++)
	{
		for (size_t j = 0; j < features; j++)
		{
			if( X[i * features + j] != Y[i * features + j])
			{
				mismatchCount++;
				printf("Mismatch at row %zu, feature %zu: X = %.2f, Y = %.2f\n", i, j, X[i * features + j], Y[i * features + j]);
			}
		}
	}
	if (mismatchCount > 0){
		printf("Total mismatches: %zu\n", mismatchCount);
	}
	else
	{
		printf("All values match between X and Y.\n");
	}
	
}

void printCalculatedBarycenter(float *barycenter, size_t features)
{
	printf("Barycenter Vector:\n[ ");
	for (size_t i = 0; i < features; i++)
	{
		printf("%.4f ", barycenter[i]);
	}
	printf("]\n");
}


void correctnessCheckEquations(float *barycenterC, float *barycenterASM, size_t features)
{
	size_t mismatchCount = 0;

	for (size_t i = 0; i < features; i++)
	{
		if (barycenterC[i] != barycenterASM[i])
		{
			mismatchCount++;
			printf("Mismatch at feature %zu: C = %.4f, ASM = %.4f\n", i, barycenterC[i], barycenterASM[i]);
		}
	}

	if (mismatchCount > 0)
	{
		printf("Total mismatches in barycenter calculations: %zu\n", mismatchCount);
	}
	else
	{
		printf("All barycenter values match between C and ASM implementations.\n");
	}
}
int main()
{
	// STEP 0: Initialize memory space for classes w/ features x1-x5 as rows
	float *X_classNeg1 = malloc(MAX_ROWS * NUM_FEATURES * sizeof(float));
	float *X_class1 = malloc(MAX_ROWS * NUM_FEATURES * sizeof(float));

	// STEP 1: Read from csv
	FILE *file = fopen("../data.csv", "r");
	if (!file)
	{
		// Clean up before exiting
		free(X_classNeg1);
		free(X_class1);
		return 1; // indicate error
	}

	char line[MAX_ROWS];
	// skip header by reading first line
	fgets(line, sizeof(line), file);

	int count_neg1 = 0; // # of rows where y == -1
	int count_1 = 0;	// # of rows where y == 1

	while (fgets(line, sizeof(line), file))
	{
		// remove trailing \n
		line[strcspn(line, "\r\n")] = 0;

		// to store unclassified rows
		float temp_features[NUM_FEATURES];

		int label = -1;
		size_t featureCol = 0;

		char *token = strtok(line, ","); // Get first token @ featureCol 0
		while (token != NULL)
		{
			if (featureCol < NUM_FEATURES)
			{
				// store all unclassified rows
				temp_features[featureCol] = (float)atof(token);
			}
			else if (featureCol == NUM_FEATURES)
			{
				label = atoi(token); // 6th column is y
			}
			token = strtok(NULL, ","); // move to next row
			featureCol++;
		}

		// move row into class
		if (label == -1)
		{
			for (int j = 0; j < NUM_FEATURES; j++)
			{
				X_classNeg1[count_neg1 * NUM_FEATURES + j] = temp_features[j];
			}
			count_neg1++;
		}
		else if (label == 1)
		{
			for (int j = 0; j < NUM_FEATURES; j++)
			{
				X_class1[count_1 * NUM_FEATURES + j] = temp_features[j];
			}
			count_1++;
		}
	}
	fclose(file);

	// Correctness check of data extraction
	printf("======== DATA EXTRACTION PREVIEW ========\n");
	printf("Extracted %d samples for Class 0\n", count_neg1);
	printf("Extracted %d samples for Class 1\n", count_1);
	printf("CLASS -1 (First 5 rows):\n");
	for (size_t i = 0; i < 5; i++)
	{
		// [i * NUM_FEATURES + col]
		printf("Row %zu: x1 = %.2f, x2 = %.2f\n",
			   i,
			   X_classNeg1[i * NUM_FEATURES + 0],
			   X_classNeg1[i * NUM_FEATURES + 1]);
	}
	printf("\nCLASS 1 (First 5 rows):\n");
	for (size_t i = 0; i < 5; i++)
	{
		// [i * NUM_FEATURES + col]
		printf("Row %zu: x1 = %.2f, x2 = %.2f\n",
			   i,
			   X_class1[i * NUM_FEATURES + 0],
			   X_class1[i * NUM_FEATURES + 1]);
	}

	// Example barycenter calculation for both classes
	float z_B_neg1[NUM_FEATURES];
	float z_B_pos1[NUM_FEATURES];

	printf("\n======== BARYCENTER CALCULATION ========\n");
	printf("\n======== BARYCENTER EQUATIONS ASM RESULTS ========\n");
	printf("Calculating barycenter for Class -1:\n");
	if (count_neg1 > 0)
	{
		calculate_barycenter(NUM_FEATURES, count_neg1, X_classNeg1, z_B_neg1);
		printCalculatedBarycenter(z_B_neg1, NUM_FEATURES);
	}

	printf("Calculating barycenter for Class 1:\n");
	if (count_1 > 0)
	{
		calculate_barycenter(NUM_FEATURES, count_1, X_class1, z_B_pos1);
		printCalculatedBarycenter(z_B_pos1, NUM_FEATURES);
	}

	printf("\n======== BARYCENTER EQUATIONS C RESULTS ========\n");
	printf("Calculating barycenter for Class -1:\n");
	if (count_neg1 > 0)
	{
		compute_barycenter(NUM_FEATURES, count_neg1, X_classNeg1, z_B_neg1);
		printCalculatedBarycenter(z_B_neg1, NUM_FEATURES);
	}

	printf("Calculating barycenter for Class 1:\n");
	if (count_1 > 0)
	{
		compute_barycenter(NUM_FEATURES, count_1, X_class1, z_B_pos1);
		printCalculatedBarycenter(z_B_pos1, NUM_FEATURES);
	}



	/////////////////////////////////////////////////////////
	// STEP 3: Apply C Barycenter Criteria
	float *filtered_neg1 = malloc(count_neg1 * NUM_FEATURES * sizeof(float));
	float *filtered_pos1 = malloc(count_1 * NUM_FEATURES * sizeof(float));
	float *filtered_neg1_asm = malloc(count_neg1 * NUM_FEATURES * sizeof(float));
	float *filtered_pos1_asm = malloc(count_1 * NUM_FEATURES *sizeof(float));

	size_t retained_neg1_count = 0;
	size_t retained_pos1_count = 0;

	if (filtered_neg1 == NULL || filtered_pos1 == NULL || filtered_neg1_asm == NULL || filtered_pos1_asm == NULL) {
		fprintf(stderr, "Memory allocation failed.\n");
		free(filtered_neg1);
		free(filtered_pos1);
		free(filtered_neg1_asm);
		free(filtered_pos1_asm);
		free(X_classNeg1);
		free(X_class1);
		return 1;
	}

	retained_neg1_count = calculate_barycenter_criteria(z_B_pos1, z_B_neg1, X_classNeg1, filtered_neg1_asm, NUM_FEATURES, count_neg1);
	retained_pos1_count = calculate_barycenter_criteria(z_B_neg1, z_B_pos1, X_class1, filtered_pos1_asm, NUM_FEATURES, count_1);

	
	printf("\n======== BARYCENTER CRITERIA ASM RESULTS ========\n");
	printf("Class -1: %zu / %d retained\n", retained_neg1_count, count_neg1);
	printf("Class  1: %zu / %d retained\n", retained_pos1_count, count_1);
	printf("Retained Class -1 Samples:\n");
	
	printCriteria(filtered_neg1_asm);
	printf("\nRetained Class 1 Samples:\n");
	printCriteria(filtered_pos1_asm);

	printf("\n======================================================\n");

	retained_neg1_count = calculate_barycenter_criteria_c(NUM_FEATURES, count_neg1, X_classNeg1, z_B_neg1, z_B_pos1, filtered_neg1);
	retained_pos1_count = calculate_barycenter_criteria_c(NUM_FEATURES, count_1, X_class1, z_B_pos1, z_B_neg1, filtered_pos1);

	printf("\n======== BARYCENTER CRITERIA RESULTS ========\n");
	printf("Class -1: %zu / %d retained\n", retained_neg1_count, count_neg1);
	printf("Class  1: %zu / %d retained\n", retained_pos1_count, count_1);

	printf("Retained Class -1 Samples:\n");
	printCriteria(filtered_neg1);
	printf("Retained Class 1 Samples:\n");
	printCriteria(filtered_pos1);
	printf("\n======================================================\n");
	
	printf("\n======== COMPARISON OF RESULTS ========\n");

	printf("Comparing calculated centriods of C with calculated centriods of ASM for class -1:\n");
	correctnessCheckEquations(z_B_neg1, z_B_neg1, NUM_FEATURES);
	printf("Comparing calculated centriods of C with calculated centriods of ASM for class 1:\n");
	correctnessCheckEquations(z_B_pos1, z_B_pos1, NUM_FEATURES);
	printf("Comparing filtered_neg1 of C with filtered_neg1_asm:\n");
	correctnessCheckCriteria(filtered_neg1, filtered_neg1_asm, retained_neg1_count, NUM_FEATURES);
	printf("Comparing filtered_pos1 of C with filtered_pos1_asm:\n");
	correctnessCheckCriteria(filtered_pos1, filtered_pos1_asm, retained_pos1_count, NUM_FEATURES);


	free(filtered_neg1);
	free(filtered_pos1);
	free(filtered_neg1_asm);
	free(filtered_pos1_asm);

	// STEP #: Free heap memory
	free(X_classNeg1);
	free(X_class1);

	return 0;
}
