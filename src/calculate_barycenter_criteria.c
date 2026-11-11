
#include <stddef.h>
#include <math.h>

// filters data points using the barycenter criteria
// returns the number of retained rows
size_t calculate_barycenter_criteria_c(size_t features, size_t rows, float *X, float *own_barycenter, float *opposite_barycenter, float *filtered)
{
    if (features == 0 || rows == 0 || X == NULL || own_barycenter == NULL || opposite_barycenter == NULL || filtered == NULL) return 0;

    // calculate distance between the two class barycenters
    float barycenter_distance = 0.0f;

    for (size_t i = 0; i < features; i++)
    {
        float difference = own_barycenter[i] - opposite_barycenter[i];
        barycenter_distance += difference * difference;
    }

    barycenter_distance = sqrtf(barycenter_distance);
    size_t retained_rows = 0;

    // check every datapoint against the opposing barycenter
    for (size_t i = 0; i < rows; i++)
    {
        float point_distance = 0.0f;

        for (size_t j = 0; j < features; j++)
        {
            float difference = X[i * features + j] - opposite_barycenter[j];
            point_distance += difference * difference;
        }

        point_distance = sqrtf(point_distance);

        // retain point if it satisfies the barycenter criteria
        if (point_distance <= barycenter_distance)
        {
            for (size_t j = 0; j < features; j++)
                filtered[retained_rows * features + j] = X[i * features + j];

            retained_rows++;
        }
    }

    return retained_rows;
}
