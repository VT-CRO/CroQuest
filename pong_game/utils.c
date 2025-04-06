#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// This func just generates a normal.
double generate_normal(double mu, double sigma) {
    double u1 = (double)rand() / RAND_MAX;
    double u2 = (double)rand() / RAND_MAX;
    double z0 = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
    return mu + z0 * sigma;
}

//This functions does something called an inverse phi, which instead of taking a z-score and returning probability, it takes a probability and returns a z-score (z-score is the number of standard deviations from the mean).
double inversePhi(double p) {
    if (p <= 0.0) return -INFINITY;
    if (p >= 1.0) return INFINITY;

    static const double a1 = -39.69683028665376;
    static const double a2 = 220.9460984245205;
    static const double a3 = -275.9285104469687;
    static const double a4 = 138.3577518672690;
    static const double a5 = -30.66479806614716;
    static const double a6 = 2.506628277459239;

    static const double b1 = -54.47609879822406;
    static const double b2 = 161.5858368580409;
    static const double b3 = -155.6989798598866;
    static const double b4 = 66.80131188771972;
    static const double b5 = -13.28068155288572;

    static const double c1 = -7.784894002430293;
    static const double c2 = -34.15349256737399;
    static const double c3 =  3.224671290700398;
    static const double c4 =  2.445134137142996;

    static const double d1 =  7.784695709041462;
    static const double d2 =  34.01321804917505;
    static const double d3 =  3.224671290700398;
    static const double d4 =  2.445134137142996;

    const double p_low  = 0.02425;
    const double p_high = 1.0 - p_low;

    double q, r, z;

    if (p < p_low) {
        q = sqrt(-2.0*log(p));
        z = (((((c1*q + c2)*q + c3)*q + c4)*q + d1)*q + d2)
          / ((((d3*q + d4)*q + 1.0)*q + 0.0)*q + 1.0);
        if (p < 0.0000001) z = -6;
    } else if (p > p_high) {
        q = sqrt(-2.0*log(1.0 - p));
        z = -(((((c1*q + c2)*q + c3)*q + c4)*q + d1)*q + d2)
            / ((((d3*q + d4)*q + 1.0)*q + 0.0)*q + 1.0);
        if (p > 0.9999999) z = 6;
    } else {
        q = p - 0.5;
        r = q*q;
        z = (((((a1*r + a2)*r + a3)*r + a4)*r + a5)*r + a6)*q
            / (((((b1*r + b2)*r + b3)*r + b4)*r + b5)*r + 1.0);
    }
    return z;
}

// Fort these values, current chances of losing:
//after 5: 34.93%
//after 10: 81.31%
//after 15: 97.28%
//after 20: 99.79%

double chance_should_lose(int level_index) {
    if (level_index < 3) {
        return 0.0;
    } else if (level_index < 5) {
        return 0.1;
    } else if (level_index < 10) {
        return 0.2;
    } else if (level_index < 15) {
        return 0.3;
    } else {
        return 0.4;
    }
}

double calculate_offset(int level_index, double platform_width) {
    double chance = chance_should_lose(level_index);
    double lose_offset = platform_width / 2.0;
    double p = 1.0 - chance / 2.0;
    double z = inversePhi(p); //gets the z-score required for the chance of losing.
    double sigma = lose_offset / z;
    return generate_normal(0.0, sigma);
}

/*
Returns the minimum value
*/
int min(int value1, int value2)
{
    return value1 >= value2 ? value2 : value1;
}

/*
Returns the maximum value
*/
int max(int value1, int value2)
{
    return value1 >= value2 ? value1 : value2;
}