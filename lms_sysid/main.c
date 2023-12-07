#include "xlaudio.h"
#include "xlaudio_armdsp.h"
#include <stdlib.h>
#include <stdio.h>

#define BETA 0.001f
#define N 50

float32_t w[N] = {0.0};
float32_t x[N] = {0.0};

static int luptr = 0;

static float32_t firstorder;

// #define FIRSTORDER

uint16_t processSample(uint16_t s) {
    int i;

    if (xlaudio_pushButtonRightDown()) {
        luptr = (luptr + 1) % N;
        if (luptr == 0)
            xlaudio_debugpinhigh();
        else
            xlaudio_debugpinlow();
        return xlaudio_f32_to_dac14(w[luptr]);
    } else {

        float32_t input = xlaudio_adc14_to_f32(s);

        float32_t rnd = ((rand() % 0x1000) * 1.0 / 4096.0 - 0.5) * 0.05; // -0.1 .. 0.1

        x[0] = rnd;

        float32_t y = 0.;
        for (i=0; i<N; i++) {
            y += w[i]*x[i];
        }

        // error
        float32_t e = input - y;

        if (xlaudio_pushButtonLeftDown()) {
            // clear coefficients
            for (i=N; i >= 0; i--)
                w[i] = 0.;
        } else {
            // update coefficients
            for (i=N; i >= 0; i--)
                w[i] += BETA * e * x[i];
        }

        // shift delay line
        for (i=N-1; i > 0; i--)
            x[i] = x[i-1];


        float32_t output = rnd;

#ifdef FIRSTORDER
        // artificial first order filter for system identification
        output = output + 0.9 * firstorder;
        firstorder = output;
#endif

        return xlaudio_f32_to_dac14(output);
    }
}


int main(void) {
    WDT_A_hold(WDT_A_BASE);

    xlaudio_init_intr(FS_8000_HZ, XLAUDIO_MIC_IN, processSample);

    xlaudio_run();

    return 1;
}
