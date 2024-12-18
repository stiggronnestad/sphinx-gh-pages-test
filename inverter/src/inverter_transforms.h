#ifndef EVERT_INVERTER_TRANSFORMS_H_
#define EVERT_INVERTER_TRANSFORMS_H_

#include <arm_math.h>
#include "inverter_math.h"

typedef struct
{
    float32_t a, b, c;
    float32_t alpha, beta, ab0;
    float32_t d, q, dq0;
} EVERT_INVERTER_AbcDq0TypeDef;

typedef struct
{
    float32_t a, b, c;
    float32_t alpha, beta, ab0;
    float32_t d, q, dq0;
} EVERT_INVERTER_Dq0AbcTypeDef;

static inline void EVERT_INVERTER_InitAbcDq0(EVERT_INVERTER_AbcDq0TypeDef *abcdq0)
{
    abcdq0->a = 0.0f;
    abcdq0->b = 0.0f;
    abcdq0->c = 0.0f;
    abcdq0->alpha = 0.0f;
    abcdq0->beta = 0.0f;
    abcdq0->ab0 = 0.0f;
    abcdq0->d = 0.0f;
    abcdq0->q = 0.0f;
    abcdq0->dq0 = 0.0f;
}

static inline void EVERT_INVERTER_InitDq0Abc(EVERT_INVERTER_Dq0AbcTypeDef *dq0abc)
{
    dq0abc->a = 0.0f;
    dq0abc->b = 0.0f;
    dq0abc->c = 0.0f;
    dq0abc->alpha = 0.0f;
    dq0abc->beta = 0.0f;
    dq0abc->ab0 = 0.0f;
    dq0abc->d = 0.0f;
    dq0abc->q = 0.0f;
    dq0abc->dq0 = 0.0f;
}

/// @brief Multistep transform incorporating Clarke and Park transformations
/// @param abcdq0
/// @param a
/// @param b
/// @param c
/// @param omega_t
static inline void EVERT_INVERTER_TransformAbcToDq0(EVERT_INVERTER_AbcDq0TypeDef *abcdq0, float32_t a, float32_t b, float32_t c, float32_t sine, float32_t cosine)
{
    // Cache ABC for debugging purposes
    abcdq0->a = a;
    abcdq0->b = b;
    abcdq0->c = c;

    // Clarke transformation
    // C:\Users\stig\OneDrive - Informasjonskontroll AS\02. Prosjekter\Evert\DQ_THEORY_OVERVIEW\DQ_THEORY_OVERVIEW_1v00.pdf
    // 2.1 Clarke Transformation
    abcdq0->alpha = (EVERT_INVERTER_MATH_K_2_OVER_3 * a) - (EVERT_INVERTER_MATH_K_1_OVER_3 * b) - (EVERT_INVERTER_MATH_K_1_OVER_3 * c);
    abcdq0->beta = (EVERT_INVERTER_MATH_K_1_OVER_SQRT3 * b) - (EVERT_INVERTER_MATH_K_1_OVER_SQRT3 * c);
    abcdq0->ab0 = (EVERT_INVERTER_MATH_K_1_OVER_3 * a) + (EVERT_INVERTER_MATH_K_1_OVER_3 * b) + (EVERT_INVERTER_MATH_K_1_OVER_3 * c);

    // Park transformation
    // C:\Users\stig\OneDrive - Informasjonskontroll AS\02. Prosjekter\Evert\DQ_THEORY_OVERVIEW\DQ_THEORY_OVERVIEW_1v00.pdf
    // 2.2 Park Transformation
    abcdq0->d = ((+1) * sine * abcdq0->alpha) + ((-1) * cosine * abcdq0->beta);
    abcdq0->q = ((+1) * cosine * abcdq0->alpha) + ((+1) * sine * abcdq0->beta);
    abcdq0->dq0 = abcdq0->ab0;
}

/// @brief Multistep transform incorporating inverse Clarke and Park transformations
/// @param dq0abc
/// @param d
/// @param q
/// @param omega_t
static inline void EVERT_INVERTER_TransformDq0ToAbc(EVERT_INVERTER_Dq0AbcTypeDef *dq0abc, float32_t d, float32_t q, float32_t sine, float32_t cosine)
{
    // Cache DQ0 for debugging purposes
    dq0abc->d = d;
    dq0abc->q = q;

    // Inverse Park transformation
    // C:\Users\stig\OneDrive - Informasjonskontroll AS\02. Prosjekter\Evert\DQ_THEORY_OVERVIEW\DQ_THEORY_OVERVIEW_1v00.pdf
    // 2.3 Inverse Park Transformation
    dq0abc->alpha = (1.0f * sine * dq0abc->d) + (1.0f * cosine * dq0abc->q);
    dq0abc->beta = (-1.0f * cosine * dq0abc->d) + (1.0f * sine * dq0abc->q);
    dq0abc->ab0 = dq0abc->dq0;

    // Inverse Clarke transformation
    // C:\Users\stig\OneDrive - Informasjonskontroll AS\02. Prosjekter\Evert\DQ_THEORY_OVERVIEW\DQ_THEORY_OVERVIEW_1v00.pdf
    // 2.4 Inverse Clarke Transformation
    dq0abc->a = ((+1) * dq0abc->alpha) + ((+1) * dq0abc->ab0);
    dq0abc->b = ((-0.5f) * dq0abc->alpha) + ((+0.8660254037844386f) * dq0abc->beta) + ((+1) * dq0abc->ab0);
    dq0abc->c = ((-0.5f) * dq0abc->alpha) + ((-0.8660254037844386f) * dq0abc->beta) + ((+1) * dq0abc->ab0);
}

#endif // EVERT_INVERTER_TRANSFORMS_H_