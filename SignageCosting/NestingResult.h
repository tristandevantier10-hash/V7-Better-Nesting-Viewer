#pragma once

struct NestingResult
{
    double requiredLengthM = 0.0;
    double consumedAreaM2 = 0.0;

    int piecesAcross = 0;
    int rows = 0;

    bool rotated = false;

    // NEW
    double theoreticalAreaM2 = 0.0;
    double wastePercent = 0.0;
    double efficiencyPercent = 0.0;
    double wasteAreaM2;
};
